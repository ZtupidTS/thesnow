#include <filezilla.h>
#include "ControlSocket.h"
#include "ftpcontrolsocket.h"
#include "sftpcontrolsocket.h"
#include "directorycache.h"
#include "logging_private.h"
#include "httpcontrolsocket.h"
#include "ratelimiter.h"
#include "pathcache.h"

class wxFzEngineEvent : public wxEvent
{
public:
	wxFzEngineEvent(int id, enum EngineNotificationType eventType, int data = 0);
	virtual wxEvent *Clone() const;

	enum EngineNotificationType m_eventType;
	int data;
};

extern const wxEventType fzEVT_ENGINE_NOTIFICATION;
typedef void (wxEvtHandler::*fzEngineEventFunction)(wxFzEngineEvent&);

#define EVT_FZ_ENGINE_NOTIFICATION(id, fn) \
	DECLARE_EVENT_TABLE_ENTRY(             \
		fzEVT_ENGINE_NOTIFICATION, id, -1, \
		(wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( fzEngineEventFunction, &fn ), \
		(wxObject *) NULL                  \
	),

std::list<CFileZillaEnginePrivate*> CFileZillaEnginePrivate::m_engineList;
int CFileZillaEnginePrivate::m_activeStatus[2] = {0, 0};
std::list<CFileZillaEnginePrivate::t_failedLogins> CFileZillaEnginePrivate::m_failedLogins;

DEFINE_EVENT_TYPE(fzEVT_ENGINE_NOTIFICATION);

wxFzEngineEvent::wxFzEngineEvent(int id, enum EngineNotificationType eventType, int data /*=0*/) : wxEvent(id, fzEVT_ENGINE_NOTIFICATION)
{
	m_eventType = eventType;
	wxFzEngineEvent::data = data;
}

wxEvent *wxFzEngineEvent::Clone() const
{
	return new wxFzEngineEvent(*this);
}


BEGIN_EVENT_TABLE(CFileZillaEnginePrivate, wxEvtHandler)
	EVT_FZ_ENGINE_NOTIFICATION(wxID_ANY, CFileZillaEnginePrivate::OnEngineEvent)
	EVT_TIMER(wxID_ANY, CFileZillaEnginePrivate::OnTimer)
END_EVENT_TABLE();

CFileZillaEnginePrivate::CFileZillaEnginePrivate()
	: m_retryTimer(this)
{
	m_pRateLimiter = 0;
	m_maySendNotificationEvent = true;
	m_pEventHandler = 0;
	m_pControlSocket = 0;
	m_pCurrentCommand = 0;
	m_bIsInCommand = false;
	m_nControlSocketError = 0;
	m_asyncRequestCounter = 0;
	m_engineList.push_back(this);
	m_retryCount = 0;

	static int id = 0;
	m_engine_id = ++id;

	m_pLogging = new CLogging(this);
}

CFileZillaEnginePrivate::~CFileZillaEnginePrivate()
{
	delete m_pControlSocket;
	delete m_pCurrentCommand;

	// Delete notification list
	for (std::list<CNotification *>::iterator iter = m_NotificationList.begin(); iter != m_NotificationList.end(); ++iter)
		delete *iter;

	// Remove ourself from the engine list
	for (std::list<CFileZillaEnginePrivate*>::iterator iter = m_engineList.begin(); iter != m_engineList.end(); ++iter)
		if (*iter == this)
		{
			m_engineList.erase(iter);
			break;
		}

	delete m_pLogging;

	if (m_pRateLimiter)
		m_pRateLimiter->Free();

	if (m_engineList.empty())
		CSocket::Cleanup(true);
}

bool CFileZillaEnginePrivate::SendEvent(enum EngineNotificationType eventType, int data /*=0*/)
{
	wxFzEngineEvent evt(wxID_ANY, eventType, data);
	wxPostEvent(this, evt);
	return true;
}

void CFileZillaEnginePrivate::OnEngineEvent(wxFzEngineEvent &event)
{
	switch (event.m_eventType)
	{
	case engineCancel:
		if (!IsBusy())
			break;
		if (m_pControlSocket)
			m_pControlSocket->Cancel();
		else if (m_pCurrentCommand)
			ResetOperation(FZ_REPLY_CANCELED);
		break;
	case engineTransferEnd:
		if (m_pControlSocket)
			m_pControlSocket->TransferEnd();
	default:
		break;
	}
}

bool CFileZillaEnginePrivate::IsBusy() const
{
	return m_pCurrentCommand != 0;
}

bool CFileZillaEnginePrivate::IsConnected() const
{
	if (!m_pControlSocket)
		return false;

	return m_pControlSocket->Connected();
}

const CCommand *CFileZillaEnginePrivate::GetCurrentCommand() const
{
	return m_pCurrentCommand;
}

enum Command CFileZillaEnginePrivate::GetCurrentCommandId() const
{
	if (!m_pCurrentCommand)
		return cmd_none;

	else
		return GetCurrentCommand()->GetId();
}

void CFileZillaEnginePrivate::AddNotification(CNotification *pNotification)
{
	m_lock.Enter();
	m_NotificationList.push_back(pNotification);

	if (m_maySendNotificationEvent && m_pEventHandler)
	{
		m_maySendNotificationEvent = false;
		m_lock.Leave();
		wxFzEvent evt(wxID_ANY);
		evt.SetEventObject(this);
		wxPostEvent(m_pEventHandler, evt);
	}
	else
		m_lock.Leave();
}

int CFileZillaEnginePrivate::ResetOperation(int nErrorCode)
{
	m_pLogging->LogMessage(Debug_Debug, _T("CFileZillaEnginePrivate::ResetOperation(%d)"), nErrorCode);

	if (nErrorCode & FZ_REPLY_DISCONNECTED)
		m_lastListDir.Clear();

	if (m_pCurrentCommand)
	{
		if ((nErrorCode & FZ_REPLY_NOTSUPPORTED) == FZ_REPLY_NOTSUPPORTED)
		{
			wxASSERT(m_bIsInCommand);
			m_pLogging->LogMessage(Error, _("Command not supported by this protocol"));
		}

		if (m_pCurrentCommand->GetId() == cmd_connect)
		{
			if (!(nErrorCode & ~(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED | FZ_REPLY_TIMEOUT | FZ_REPLY_CRITICALERROR | FZ_REPLY_PASSWORDFAILED)) &&
				nErrorCode & (FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED))
			{
				const CConnectCommand *pConnectCommand = (CConnectCommand *)m_pCurrentCommand;

				RegisterFailedLoginAttempt(pConnectCommand->GetServer(), (nErrorCode & FZ_REPLY_CRITICALERROR) == FZ_REPLY_CRITICALERROR);

				if ((nErrorCode & FZ_REPLY_CRITICALERROR) != FZ_REPLY_CRITICALERROR)
				{
					++m_retryCount;
					if (m_retryCount < m_pOptions->GetOptionVal(OPTION_RECONNECTCOUNT) && pConnectCommand->RetryConnecting())
					{
						unsigned int delay = GetRemainingReconnectDelay(pConnectCommand->GetServer());
						if (!delay)
							delay = 1;
						m_pLogging->LogMessage(Status, _("Waiting to retry..."));
						m_retryTimer.Start(delay, true);
						return FZ_REPLY_WOULDBLOCK;
					}
				}
			}
		}

		if (!m_bIsInCommand)
		{
			COperationNotification *notification = new COperationNotification();
			notification->nReplyCode = nErrorCode;
			notification->commandId = m_pCurrentCommand->GetId();
			AddNotification(notification);
		}
		else
			m_nControlSocketError |= nErrorCode;

		delete m_pCurrentCommand;
		m_pCurrentCommand = 0;
	}
	else if (nErrorCode & FZ_REPLY_DISCONNECTED)
	{
		if (!m_bIsInCommand)
		{
			COperationNotification *notification = new COperationNotification();
			notification->nReplyCode = nErrorCode;
			notification->commandId = cmd_none;
			AddNotification(notification);
		}
	}

	return nErrorCode;
}

void CFileZillaEnginePrivate::SetActive(int direction)
{
	if (m_pControlSocket)
		m_pControlSocket->SetAlive();

	if (!m_activeStatus[direction])
		AddNotification(new CActiveNotification(direction));
	m_activeStatus[direction] = 2;
}

unsigned int CFileZillaEnginePrivate::GetNextAsyncRequestNumber()
{
	wxCriticalSectionLocker lock(m_lock);
	return ++m_asyncRequestCounter;
}

// Command handlers
int CFileZillaEnginePrivate::Connect(const CConnectCommand &command)
{
	if (IsConnected())
		return FZ_REPLY_ALREADYCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	m_retryCount = 0;

	if (m_pControlSocket)
	{
		// Need to delete before setting m_pCurrentCommand.
		// The destructor can call CFileZillaEnginePrivate::ResetOperation
		// which would delete m_pCurrentCommand
		delete m_pControlSocket;
		m_pControlSocket = 0;
		m_nControlSocketError = 0;
	}

	m_pCurrentCommand = command.Clone();

	if (command.GetServer().GetPort() != CServer::GetDefaultPort(command.GetServer().GetProtocol()))
	{
		ServerProtocol protocol = CServer::GetProtocolFromPort(command.GetServer().GetPort(), true);
		if (protocol != UNKNOWN && protocol != command.GetServer().GetProtocol())
			m_pLogging->LogMessage(Status, _("Selected port usually in use by a different protocol."));
	}

	return ContinueConnect();
}

int CFileZillaEnginePrivate::Disconnect(const CDisconnectCommand &command)
{
	if (!IsConnected())
		return FZ_REPLY_OK;

	m_pCurrentCommand = command.Clone();
	int res = m_pControlSocket->Disconnect();
	if (res == FZ_REPLY_OK)
	{
		delete m_pControlSocket;
		m_pControlSocket = 0;
	}

	return res;
}

int CFileZillaEnginePrivate::Cancel(const CCancelCommand &command)
{
	if (!IsBusy())
		return FZ_REPLY_OK;

	if (m_retryTimer.IsRunning())
	{
		wxASSERT(m_pCurrentCommand && m_pCurrentCommand->GetId() == cmd_connect);

		delete m_pControlSocket;
		m_pControlSocket = 0;

		delete m_pCurrentCommand;
		m_pCurrentCommand = 0;

		m_retryTimer.Stop();

		m_pLogging->LogMessage(::Error, _("Connection attempt interrupted by user"));
		COperationNotification *notification = new COperationNotification();
		notification->nReplyCode = FZ_REPLY_DISCONNECTED|FZ_REPLY_CANCELED;
		notification->commandId = cmd_connect;
		AddNotification(notification);

		return FZ_REPLY_WOULDBLOCK;
	}

	SendEvent(engineCancel);

	return FZ_REPLY_WOULDBLOCK;
}

int CFileZillaEnginePrivate::List(const CListCommand &command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (command.GetPath().IsEmpty() && command.GetSubDir() != _T(""))
		return FZ_REPLY_SYNTAXERROR;

	if (command.GetFlags() & LIST_FLAG_LINK && command.GetSubDir() == _T(""))
		return FZ_REPLY_SYNTAXERROR;

	bool refresh = (command.GetFlags() & LIST_FLAG_REFRESH) != 0;
	bool avoid = (command.GetFlags() & LIST_FLAG_AVOID) != 0;
	if (refresh && avoid)
		return FZ_REPLY_SYNTAXERROR;

	if (!refresh && !command.GetPath().IsEmpty())
	{
		const CServer* pServer = m_pControlSocket->GetCurrentServer();
		if (pServer)
		{
			CServerPath path(CPathCache::Lookup(*pServer, command.GetPath(), command.GetSubDir()));
			if (path.IsEmpty() && command.GetSubDir().IsEmpty())
				path = command.GetPath();
			if (!path.IsEmpty())
			{
				CDirectoryListing *pListing = new CDirectoryListing;
				CDirectoryCache cache;
				bool is_outdated = false;
				bool found = cache.Lookup(*pListing, *pServer, path, true, is_outdated);
				if (found && !is_outdated)
				{
					if (pListing->m_hasUnsureEntries)
						refresh = true;
					else
					{
						if (!avoid)
						{
							m_lastListDir = pListing->path;
							m_lastListTime = wxDateTime::Now();
							CDirectoryListingNotification *pNotification = new CDirectoryListingNotification(pListing->path);
							AddNotification(pNotification);
						}
						delete pListing;
						return FZ_REPLY_OK;
					}
				}
				if (is_outdated)
					refresh = true;
				delete pListing;
			}
		}
	}
	if (IsBusy())
		return FZ_REPLY_BUSY;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->List(command.GetPath(), command.GetSubDir(), command.GetFlags());
}

int CFileZillaEnginePrivate::FileTransfer(const CFileTransferCommand &command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->FileTransfer(command.GetLocalFile(), command.GetRemotePath(), command.GetRemoteFile(), command.Download(), command.GetTransferSettings());
}

int CFileZillaEnginePrivate::RawCommand(const CRawCommand& command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	if (command.GetCommand() == _T(""))
		return FZ_REPLY_SYNTAXERROR;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->RawCommand(command.GetCommand());
}

int CFileZillaEnginePrivate::Delete(const CDeleteCommand& command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	if (command.GetPath().IsEmpty() ||
		command.GetFiles().empty())
		return FZ_REPLY_SYNTAXERROR;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->Delete(command.GetPath(), command.GetFiles());
}

int CFileZillaEnginePrivate::RemoveDir(const CRemoveDirCommand& command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	if (command.GetPath().IsEmpty() ||
		command.GetSubDir() == _T(""))
		return FZ_REPLY_SYNTAXERROR;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->RemoveDir(command.GetPath(), command.GetSubDir());
}

int CFileZillaEnginePrivate::Mkdir(const CMkdirCommand& command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	if (command.GetPath().IsEmpty() || !command.GetPath().HasParent())
		return FZ_REPLY_SYNTAXERROR;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->Mkdir(command.GetPath());
}

int CFileZillaEnginePrivate::Rename(const CRenameCommand& command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	if (command.GetFromPath().IsEmpty() || command.GetToPath().IsEmpty() ||
		command.GetFromFile() == _T("") || command.GetToFile() == _T(""))
		return FZ_REPLY_SYNTAXERROR;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->Rename(command);
}

int CFileZillaEnginePrivate::Chmod(const CChmodCommand& command)
{
	if (!IsConnected())
		return FZ_REPLY_NOTCONNECTED;

	if (IsBusy())
		return FZ_REPLY_BUSY;

	if (command.GetPath().IsEmpty() || command.GetFile().IsEmpty() ||
		command.GetPermission() == _T(""))
		return FZ_REPLY_SYNTAXERROR;

	m_pCurrentCommand = command.Clone();
	return m_pControlSocket->Chmod(command);
}

void CFileZillaEnginePrivate::SendDirectoryListingNotification(const CServerPath& path, bool onList, bool modified, bool failed)
{
	wxASSERT(m_pControlSocket);

	const CServer* const pOwnServer = m_pControlSocket->GetCurrentServer();
	wxASSERT(pOwnServer);

	m_lastListDir = path;

	if (failed)
	{
		CDirectoryListingNotification *pNotification = new CDirectoryListingNotification(path, false, true);
		AddNotification(pNotification);
		m_lastListTime = CTimeEx::Now();

		// On failed messages, we don't notify other engines
		return;
	}

	CDirectoryCache cache;

	CTimeEx changeTime;
	if (!cache.GetChangeTime(changeTime, *pOwnServer, path))
		return;

	CDirectoryListingNotification *pNotification = new CDirectoryListingNotification(path, !onList);
	AddNotification(pNotification);
	m_lastListTime = changeTime;

	if (!modified)
		return;

	// Iterate over the other engine, send notification if last listing
	// directory is the same
	for (std::list<CFileZillaEnginePrivate*>::iterator iter = m_engineList.begin(); iter != m_engineList.end(); ++iter)
	{
		CFileZillaEnginePrivate* const pEngine = *iter;
		if (!pEngine->m_pControlSocket || pEngine->m_pControlSocket == m_pControlSocket)
			continue;

		const CServer* const pServer = pEngine->m_pControlSocket->GetCurrentServer();
		if (!pServer || *pServer != *pOwnServer)
			continue;

		if (pEngine->m_lastListDir != path)
			continue;

		if (pEngine->m_lastListTime.GetTime().IsValid() && changeTime <= pEngine->m_lastListTime)
			continue;

		pEngine->m_lastListTime = changeTime;
		CDirectoryListingNotification *pNotification = new CDirectoryListingNotification(path, true);
		pEngine->AddNotification(pNotification);
	}
}

void CFileZillaEnginePrivate::RegisterFailedLoginAttempt(const CServer& server, bool critical)
{
	std::list<t_failedLogins>::iterator iter = m_failedLogins.begin();
	while (iter != m_failedLogins.end())
	{
		const wxTimeSpan span = wxDateTime::UNow() - iter->time;
		if (span.GetSeconds() >= m_pOptions->GetOptionVal(OPTION_RECONNECTDELAY) ||
			iter->server == server || (!critical && (iter->server.GetHost() == server.GetHost() && iter->server.GetPort() == server.GetPort())))
		{
			std::list<t_failedLogins>::iterator prev = iter;
			++iter;
			m_failedLogins.erase(prev);
		}
		else
			++iter;
	}

	t_failedLogins failure;
	failure.server = server;
	failure.time = wxDateTime::UNow();
	m_failedLogins.push_back(failure);
}

unsigned int CFileZillaEnginePrivate::GetRemainingReconnectDelay(const CServer& server)
{
	std::list<t_failedLogins>::iterator iter = m_failedLogins.begin();
	while (iter != m_failedLogins.end())
	{
		const wxTimeSpan span = wxDateTime::UNow() - iter->time;
		const int delay = m_pOptions->GetOptionVal(OPTION_RECONNECTDELAY);
		if (span.GetSeconds() >= delay)
		{
			std::list<t_failedLogins>::iterator prev = iter;
			++iter;
			m_failedLogins.erase(prev);
		}
		else if (!iter->critical && iter->server.GetHost() == server.GetHost() && iter->server.GetPort() == server.GetPort())
			return delay * 1000 - span.GetMilliseconds().GetLo();
		else if (iter->server == server)
			return delay * 1000 - span.GetMilliseconds().GetLo();
		else
			++iter;
	}

	return 0;
}

void CFileZillaEnginePrivate::OnTimer(wxTimerEvent& event)
{
	if (!m_pCurrentCommand || m_pCurrentCommand->GetId() != cmd_connect)
	{
		wxFAIL_MSG(_T("CFileZillaEnginePrivate::OnTimer called without pending cmd_connect"));
		return;
	}
	wxASSERT(!IsConnected());

#if 0//__WXDEBUG__
	const CConnectCommand *pConnectCommand = (CConnectCommand *)m_pCurrentCommand;
	wxASSERT(!GetRemainingReconnectDelay(pConnectCommand->GetServer()));
#endif

	delete m_pControlSocket;
	m_pControlSocket = 0;

	ContinueConnect();
}

int CFileZillaEnginePrivate::ContinueConnect()
{
	const CConnectCommand *pConnectCommand = (CConnectCommand *)m_pCurrentCommand;
	const CServer& server = pConnectCommand->GetServer();
	unsigned int delay = GetRemainingReconnectDelay(server);
	if (delay)
	{
		m_pLogging->LogMessage(Status, wxPLURAL("Delaying connection for %d second due to previously failed connection attempt...", "Delaying connection for %d seconds due to previously failed connection attempt...", (delay + 999) / 1000), (delay + 999) / 1000);
		m_retryTimer.Start(delay, true);
		return FZ_REPLY_WOULDBLOCK;
	}

	switch (server.GetProtocol())
	{
	case FTP:
	case FTPS:
	case FTPES:
	case INSECURE_FTP:
		m_pControlSocket = new CFtpControlSocket(this);
		break;
	case SFTP:
		m_pControlSocket = new CSftpControlSocket(this);
		break;
	case HTTP:
	case HTTPS:
		m_pControlSocket = new CHttpControlSocket(this);
		break;
	default:
		m_pLogging->LogMessage(Debug_Warning, _T("Not a valid protocol: %d"), server.GetProtocol());
		return FZ_REPLY_SYNTAXERROR|FZ_REPLY_DISCONNECTED;
	}

	int res = m_pControlSocket->Connect(server);
	if (m_retryTimer.IsRunning())
		return FZ_REPLY_WOULDBLOCK;

	return res;
}

void CFileZillaEnginePrivate::InvalidateCurrentWorkingDirs(const CServerPath& path)
{
	wxASSERT(m_pControlSocket);
	const CServer* const pOwnServer = m_pControlSocket->GetCurrentServer();
	wxASSERT(pOwnServer);

	for (std::list<CFileZillaEnginePrivate*>::iterator iter = m_engineList.begin(); iter != m_engineList.end(); ++iter)
	{
		if (*iter == this)
			continue;

		CFileZillaEnginePrivate* pEngine = *iter;
		if (!pEngine->m_pControlSocket)
			continue;

		const CServer* const pServer = pEngine->m_pControlSocket->GetCurrentServer();
		if (!pServer || *pServer != *pOwnServer)
			continue;

		pEngine->m_pControlSocket->InvalidateCurrentWorkingDir(path);
	}
}
