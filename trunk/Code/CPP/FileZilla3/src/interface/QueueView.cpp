#include <filezilla.h>
#include "queue.h"
#include "Mainfrm.h"
#include "Options.h"
#include "StatusView.h"
#include "statuslinectrl.h"
#include "xmlfunctions.h"
#include "filezillaapp.h"
#include "ipcmutex.h"
#include "state.h"
#include "asyncrequestqueue.h"
#include "defaultfileexistsdlg.h"
#include "filter.h"
#include <wx/dnd.h>
#include "dndobjects.h"
#include "loginmanager.h"
#include "aui_notebook_ex.h"
#include "queueview_failed.h"
#include "queueview_successful.h"
#include "commandqueue.h"
#include <wx/utils.h>
#include <wx/progdlg.h>
#include <wx/sound.h>
#include "local_filesys.h"
#include "statusbar.h"
#include "recursive_operation.h"
#include "auto_ascii_files.h"
#include "dragdropmanager.h"
#if WITH_LIBDBUS
#include "../dbus/desktop_notification.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CQueueViewDropTarget : public wxDropTarget
{
public:
	CQueueViewDropTarget(CQueueView* pQueueView)
		: m_pQueueView(pQueueView), m_pFileDataObject(new wxFileDataObject()),
		m_pRemoteDataObject(new CRemoteDataObject())
	{
		m_pDataObject = new wxDataObjectComposite;
		m_pDataObject->Add(m_pRemoteDataObject, true);
		m_pDataObject->Add(m_pFileDataObject, false);
		SetDataObject(m_pDataObject);
	}

	virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def)
	{
		if (def == wxDragError ||
			def == wxDragNone ||
			def == wxDragCancel)
			return def;

		if (!GetData())
			return wxDragError;

		CDragDropManager* pDragDropManager = CDragDropManager::Get();
		if (pDragDropManager)
			pDragDropManager->pDropTarget = m_pQueueView;

		if (m_pDataObject->GetReceivedFormat() == m_pFileDataObject->GetFormat())
		{
			CState* const pState = CContextManager::Get()->GetCurrentContext();
			if (!pState)
				return wxDragNone;
			const CServer* const pServer = pState->GetServer();
			if (!pServer)
				return wxDragNone;

			const CServerPath& path = pState->GetRemotePath();
			if (path.IsEmpty())
				return wxDragNone;

			pState->UploadDroppedFiles(m_pFileDataObject, path, true);
		}
		else
		{
			if (m_pRemoteDataObject->GetProcessId() != (int)wxGetProcessId())
			{
				wxMessageBox(_("Drag&drop between different instances of FileZilla has not been implemented yet."));
				return wxDragNone;
			}

			CState* const pState = CContextManager::Get()->GetCurrentContext();
			if (!pState)
				return wxDragNone;
			const CServer* const pServer = pState->GetServer();
			if (!pServer)
				return wxDragNone;

			if (!pServer->EqualsNoPass(m_pRemoteDataObject->GetServer()))
			{
				wxMessageBox(_("Drag&drop between different servers has not been implemented yet."));
				return wxDragNone;
			}

			const CLocalPath& target = pState->GetLocalDir();
			if (!target.IsWriteable())
			{
				wxBell();
				return wxDragNone;
			}

			if (!pState->DownloadDroppedFiles(m_pRemoteDataObject, target, true))
				return wxDragNone;
		}

		return def;
	}

	virtual bool OnDrop(wxCoord x, wxCoord y)
	{
		return true;
	}

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
	{
		if (def == wxDragError ||
			def == wxDragNone ||
			def == wxDragCancel)
		{
			return def;
		}

		CDragDropManager* pDragDropManager = CDragDropManager::Get();
		if (pDragDropManager && !pDragDropManager->remoteParent.IsEmpty())
		{
			// Drag from remote to queue, check if local path is writeable
			CState* const pState = CContextManager::Get()->GetCurrentContext();
			if (!pState)
				return wxDragNone;
			if (!pState->GetLocalDir().IsWriteable())
				return wxDragNone;			
		}

		def = wxDragCopy;

		return def;
	}

	virtual void OnLeave()
	{
	}

	virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def)
	{
		return OnDragOver(x, y, def);
	}

protected:
	CQueueView *m_pQueueView;
	wxFileDataObject* m_pFileDataObject;
	CRemoteDataObject* m_pRemoteDataObject;
	wxDataObjectComposite* m_pDataObject;
};

DECLARE_EVENT_TYPE(fzEVT_FOLDERTHREAD_COMPLETE, -1)
DEFINE_EVENT_TYPE(fzEVT_FOLDERTHREAD_COMPLETE)

DECLARE_EVENT_TYPE(fzEVT_FOLDERTHREAD_FILES, -1)
DEFINE_EVENT_TYPE(fzEVT_FOLDERTHREAD_FILES)

DECLARE_EVENT_TYPE(fzEVT_ASKFORPASSWORD, -1)
DEFINE_EVENT_TYPE(fzEVT_ASKFORPASSWORD)

BEGIN_EVENT_TABLE(CQueueView, CQueueViewBase)
EVT_FZ_NOTIFICATION(wxID_ANY, CQueueView::OnEngineEvent)
EVT_COMMAND(wxID_ANY, fzEVT_FOLDERTHREAD_COMPLETE, CQueueView::OnFolderThreadComplete)
EVT_COMMAND(wxID_ANY, fzEVT_FOLDERTHREAD_FILES, CQueueView::OnFolderThreadFiles)

EVT_CONTEXT_MENU(CQueueView::OnContextMenu)
EVT_MENU(XRCID("ID_PROCESSQUEUE"), CQueueView::OnProcessQueue)
EVT_MENU(XRCID("ID_REMOVEALL"), CQueueView::OnStopAndClear)
EVT_MENU(XRCID("ID_REMOVE"), CQueueView::OnRemoveSelected)
EVT_MENU(XRCID("ID_DEFAULT_FILEEXISTSACTION"), CQueueView::OnSetDefaultFileExistsAction)
EVT_MENU(XRCID("ID_ACTIONAFTER_DISABLE"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_CLOSE"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_DISCONNECT"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_RUNCOMMAND"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_SHOWMESSAGE"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_PLAYSOUND"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_REBOOT"), CQueueView::OnActionAfter)
EVT_MENU(XRCID("ID_ACTIONAFTER_SHUTDOWN"), CQueueView::OnActionAfter)

EVT_COMMAND(wxID_ANY, fzEVT_ASKFORPASSWORD, CQueueView::OnAskPassword)

EVT_TIMER(wxID_ANY, CQueueView::OnTimer)
EVT_CHAR(CQueueView::OnChar)

EVT_MENU(XRCID("ID_PRIORITY_HIGHEST"), CQueueView::OnSetPriority)
EVT_MENU(XRCID("ID_PRIORITY_HIGH"), CQueueView::OnSetPriority)
EVT_MENU(XRCID("ID_PRIORITY_NORMAL"), CQueueView::OnSetPriority)
EVT_MENU(XRCID("ID_PRIORITY_LOW"), CQueueView::OnSetPriority)
EVT_MENU(XRCID("ID_PRIORITY_LOWEST"), CQueueView::OnSetPriority)

EVT_COMMAND(wxID_ANY, fzEVT_GRANTEXCLUSIVEENGINEACCESS, CQueueView::OnExclusiveEngineRequestGranted)

EVT_SIZE(CQueueView::OnSize)
END_EVENT_TABLE()

class CFolderProcessingThread : public wxThread
{
	struct t_internalDirPair
	{
		wxString localPath;
		CServerPath remotePath;
	};
public:
	CFolderProcessingThread(CQueueView* pOwner, CFolderScanItem* pFolderItem)
		: wxThread(wxTHREAD_JOINABLE), m_condition(m_sync) {
		m_pOwner = pOwner;
		m_pFolderItem = pFolderItem;

		m_didSendEvent = false;
		m_threadWaiting = false;
		m_throttleWait = false;
		m_processing_entries = false;

		t_internalDirPair* pair = new t_internalDirPair;
		pair->localPath = pFolderItem->GetLocalPath().GetPath().c_str();
		pair->remotePath.SetSafePath(pFolderItem->GetRemotePath().GetSafePath().c_str(), false);
		m_dirsToCheck.push_back(pair);
	}

	virtual ~CFolderProcessingThread()
	{
		for (std::list<CFolderProcessingEntry*>::iterator iter = m_entryList.begin(); iter != m_entryList.end(); iter++)
			delete *iter;
		for (std::list<t_internalDirPair*>::iterator iter = m_dirsToCheck.begin(); iter != m_dirsToCheck.end(); iter++)
			delete *iter;
	}

	void GetFiles(std::list<CFolderProcessingEntry*> &entryList)
	{
		wxASSERT(entryList.empty());
		wxMutexLocker locker(m_sync);
		entryList.swap(m_entryList);

		m_didSendEvent = false;
		m_processing_entries = true;

		if (m_throttleWait)
		{
			m_throttleWait = false;
			m_condition.Signal();
		}
	}

	class t_dirPair : public CFolderProcessingEntry
	{
	public:
		t_dirPair() : CFolderProcessingEntry(CFolderProcessingEntry::dir) {}
		wxString localPath;
		wxString remotePath;
	};

	void ProcessDirectory(const CLocalPath& localPath, CServerPath remotePath, const wxString& name)
	{
		wxMutexLocker locker(m_sync);

		t_internalDirPair* pair = new t_internalDirPair;

		{
			pair->localPath = (localPath.GetPath() + name).c_str();

			remotePath.AddSegment(name);
			pair->remotePath.SetSafePath(remotePath.GetSafePath().c_str(), false);
		}

		m_dirsToCheck.push_back(pair);

		if (m_threadWaiting)
		{
			m_threadWaiting = false;
			m_condition.Signal();
		}
	}

	void CheckFinished()
	{
		m_sync.Lock();
		wxASSERT(m_processing_entries);

		m_processing_entries = false;

		if (m_threadWaiting && (!m_dirsToCheck.empty() || m_entryList.empty()))
		{
			m_threadWaiting = false;
			m_condition.Signal();
		}

		m_sync.Unlock();
	}

	CFolderScanItem* GetFolderScanItem()
	{
		return m_pFolderItem;
	}

protected:

	void AddEntry(CFolderProcessingEntry* entry)
	{
		m_sync.Lock();
		m_entryList.push_back(entry);

		// Wait if there are more than 100 items to queue,
		// don't send notification if there are less than 10.
		// This reduces overhead
		bool send;

		if (m_didSendEvent)
		{
			send = false;
			if (m_entryList.size() >= 100)
			{
				m_throttleWait = true;
				m_condition.Wait();
			}
		}
		else if (m_entryList.size() < 20)
			send = false;
		else
			send = true;

		if (send)
			m_didSendEvent = true;

		m_sync.Unlock();

		if (send)
		{
			// We send the notification after leaving the critical section, else we
			// could get into a deadlock. wxWidgets event system does internal
			// locking.
			wxCommandEvent evt(fzEVT_FOLDERTHREAD_FILES, wxID_ANY);
			wxPostEvent(m_pOwner, evt);
		}
	}

	ExitCode Entry()
	{
#ifdef __WXDEBUG__
		wxMutexGuiEnter();
		wxASSERT(m_pFolderItem->GetTopLevelItem() && m_pFolderItem->GetTopLevelItem()->GetType() == QueueItemType_Server);
		wxMutexGuiLeave();
#endif

		wxASSERT(!m_pFolderItem->Download());

		wxString currentLocalPath;
		CServerPath currentRemotePath;

		CLocalFileSystem localFileSystem;

		while (!TestDestroy() && !m_pFolderItem->m_remove)
		{
			m_sync.Lock();
			if (m_dirsToCheck.empty())
			{
				if (!m_didSendEvent && !m_entryList.empty())
				{
					m_didSendEvent = true;
					m_sync.Unlock();
					wxCommandEvent evt(fzEVT_FOLDERTHREAD_FILES, wxID_ANY);
					wxPostEvent(m_pOwner, evt);
					continue;
				}

				if (!m_didSendEvent && !m_processing_entries)
				{
					m_sync.Unlock();
					break;
				}
				m_threadWaiting = true;
				m_condition.Wait();
				if (m_dirsToCheck.empty())
				{
					m_sync.Unlock();
					break;
				}
				m_sync.Unlock();
				continue;
			}

			const t_internalDirPair *pair = m_dirsToCheck.front();
			m_dirsToCheck.pop_front();

			m_sync.Unlock();

			if (!localFileSystem.BeginFindFiles(pair->localPath, false))
			{
				delete pair;
				continue;
			}

			t_dirPair* pair2 = new t_dirPair;
	
			{
				pair2->localPath = pair->localPath.c_str();
				pair2->remotePath = pair->remotePath.GetSafePath().c_str();
			}

			AddEntry(pair2);

			t_newEntry* entry = new t_newEntry;

			wxString name;
			bool is_link;
			bool is_dir;
			while (localFileSystem.GetNextFile(name, is_link, is_dir, &entry->size, &entry->time, &entry->attributes))
			{
				if (is_link)
					continue;
				entry->name = name.c_str();
				entry->dir = is_dir;

				AddEntry(entry);

				entry = new t_newEntry;
			}
			delete entry;

			delete pair;
		}

		wxCommandEvent evt(fzEVT_FOLDERTHREAD_COMPLETE, wxID_ANY);
		wxPostEvent(m_pOwner, evt);

		return 0;
	}

	std::list<t_internalDirPair*> m_dirsToCheck;

	// Access has to be guarded by m_sync
	std::list<CFolderProcessingEntry*> m_entryList;

	CQueueView* m_pOwner;
	CFolderScanItem* m_pFolderItem;

	wxMutex m_sync;
	wxCondition m_condition;
	bool m_threadWaiting;
	bool m_throttleWait;
	bool m_didSendEvent;
	bool m_processing_entries;
};

CQueueView::CQueueView(CQueue* parent, int index, CMainFrame* pMainFrame, CAsyncRequestQueue *pAsyncRequestQueue)
	: CQueueViewBase(parent, index, _("Queued files")),
	  m_pMainFrame(pMainFrame),
	  m_pAsyncRequestQueue(pAsyncRequestQueue)
{
	if (m_pAsyncRequestQueue)
		m_pAsyncRequestQueue->SetQueue(this);

	m_activeCount = 0;
	m_activeCountDown = 0;
	m_activeCountUp = 0;
	m_activeMode = 0;
	m_quit = 0;
	m_waitStatusLineUpdate = false;
	m_lastTopItem = -1;
	m_pFolderProcessingThread = 0;

	m_totalQueueSize = 0;
	m_filesWithUnknownSize = 0;

	m_actionAfterState = ActionAfterState_Disabled;
#ifdef __WXMSW__
	m_actionAfterWarnDialog = 0;
	m_actionAfterTimer = 0;
	m_actionAfterTimerId = -1;
#endif

	std::list<ColumnId> extraCols;
	extraCols.push_back(colTransferStatus);
	CreateColumns(extraCols);

	//Initialize the engine data
	t_EngineData *pData = new t_EngineData;
	pData->active = false;
	pData->pItem = 0;
	pData->pStatusLineCtrl = 0;
	pData->state = t_EngineData::none;
	pData->pEngine = 0; // TODO: Primary transfer engine data
	pData->m_idleDisconnectTimer = 0;
	m_engineData.push_back(pData);

	SettingsChanged();

	SetDropTarget(new CQueueViewDropTarget(this));

	m_folderscan_item_refresh_timer.SetOwner(this);

	m_line_height = -1;
#ifdef __WXMSW__
	m_header_height = -1;
#endif

	m_resize_timer.SetOwner(this);

#if WITH_LIBDBUS
	m_desktop_notification = 0;
#endif
}

CQueueView::~CQueueView()
{
	if (m_pFolderProcessingThread)
	{
		m_pFolderProcessingThread->Delete();
		m_pFolderProcessingThread->Wait();
		delete m_pFolderProcessingThread;
	}

	DeleteEngines();

	m_resize_timer.Stop();

#if WITH_LIBDBUS
	delete m_desktop_notification;
#endif
}

bool CQueueView::QueueFile(const bool queueOnly, const bool download,
						   const wxString& sourceFile, const wxString& targetFile,
						   const CLocalPath& localPath, const CServerPath& remotePath,
						   const CServer& server, const wxLongLong size, enum CEditHandler::fileType edit /*=CEditHandler::none*/)
{
	CServerItem* pServerItem = CreateServerItem(server);

	CFileItem* fileItem;
	if (sourceFile.empty())
	{
		if (download)
		{
			CLocalPath p(localPath);
			p.AddSegment(targetFile);
			fileItem = new CFolderItem(pServerItem, queueOnly, p);
		}
		else
			fileItem = new CFolderItem(pServerItem, queueOnly, remotePath, targetFile);
		wxASSERT(edit == CEditHandler::none);
	}
	else
	{
		fileItem = new CFileItem(pServerItem, queueOnly, download, sourceFile, targetFile, localPath, remotePath, size);
		if (download)
			fileItem->m_transferSettings.binary = !CAutoAsciiFiles::TransferRemoteAsAscii(sourceFile, remotePath.GetType());
		else
			fileItem->m_transferSettings.binary = !CAutoAsciiFiles::TransferLocalAsAscii(sourceFile, remotePath.GetType());
		fileItem->m_edit = edit;
		if (edit != CEditHandler::none)
			fileItem->m_onetime_action = CFileExistsNotification::overwrite;
	}

	InsertItem(pServerItem, fileItem);

	return true;
}

void CQueueView::QueueFile_Finish(const bool start)
{
	bool need_refresh = false;
	if (m_insertionStart <= GetTopItem() + GetCountPerPage() + 1)
		need_refresh = true;
	CommitChanges();

	if (!m_activeMode && start)
	{
		m_activeMode = 1;
		CContextManager::Get()->NotifyGlobalHandlers(STATECHANGE_QUEUEPROCESSING);
	}

	if (m_activeMode)
	{
		m_waitStatusLineUpdate = true;
		AdvanceQueue(false);
		m_waitStatusLineUpdate = false;
	}

	UpdateStatusLinePositions();

	if (need_refresh)
		RefreshListOnly(false);
}

// Defined in RemoteListView.cpp
extern wxString StripVMSRevision(const wxString& name);

bool CQueueView::QueueFiles(const bool queueOnly, const CLocalPath& localPath, const CRemoteDataObject& dataObject)
{
	CServerItem* pServerItem = CreateServerItem(dataObject.GetServer());

	const std::list<CRemoteDataObject::t_fileInfo>& files = dataObject.GetFiles();

	for (std::list<CRemoteDataObject::t_fileInfo>::const_iterator iter = files.begin(); iter != files.end(); iter++)
	{
		if (iter->dir)
			continue;

		CFileItem* fileItem;
		wxString localFile = ReplaceInvalidCharacters(iter->name);
		if (dataObject.GetServerPath().GetType() == VMS && COptions::Get()->GetOptionVal(OPTION_STRIP_VMS_REVISION))
			localFile = StripVMSRevision(localFile);

		fileItem = new CFileItem(pServerItem, queueOnly, true,
			iter->name, (iter->name != localFile) ? localFile : wxString(),
			localPath, dataObject.GetServerPath(), iter->size);
		fileItem->m_transferSettings.binary = !CAutoAsciiFiles::TransferRemoteAsAscii(iter->name, dataObject.GetServerPath().GetType());

		InsertItem(pServerItem, fileItem);
	}

	QueueFile_Finish(!queueOnly);

	return true;
}

void CQueueView::OnEngineEvent(wxEvent &event)
{
	std::vector<t_EngineData*>::iterator iter;
	for (iter = m_engineData.begin(); iter != m_engineData.end(); iter++)
	{
		if ((wxObject *)(*iter)->pEngine == event.GetEventObject())
			break;
	}
	if (iter == m_engineData.end())
		return;

	t_EngineData* const pEngineData = *iter;

	CNotification *pNotification = pEngineData->pEngine->GetNextNotification();
	while (pNotification)
	{
		ProcessNotification(pEngineData, pNotification);

		if (m_engineData.empty() || !pEngineData->pEngine)
			break;

		pNotification = pEngineData->pEngine->GetNextNotification();
	}
}

void CQueueView::ProcessNotification(CNotification* pNotification)
{
	if (m_engineData.empty())
	{
		delete pNotification;
		return;
	}

	t_EngineData* pEngineData = m_engineData[0];
	if (!pEngineData->active)
	{
		delete pNotification;
		return;
	}

	ProcessNotification(pEngineData, pNotification);
}

void CQueueView::ProcessNotification(t_EngineData* pEngineData, CNotification* pNotification)
{
	switch (pNotification->GetID())
	{
	case nId_logmsg:
		m_pMainFrame->GetStatusView()->AddToLog(reinterpret_cast<CLogmsgNotification *>(pNotification));
		delete pNotification;
		if (COptions::Get()->GetOptionVal(OPTION_MESSAGELOG_POSITION) == 2)
			m_pQueue->Highlight(3);
		break;
	case nId_operation:
		ProcessReply(pEngineData, reinterpret_cast<COperationNotification*>(pNotification));
		delete pNotification;
		break;
	case nId_asyncrequest:
		if (!pEngineData->pItem)
		{
			delete pNotification;
			break;
		}
		switch (reinterpret_cast<CAsyncRequestNotification *>(pNotification)->GetRequestID())
		{
		case reqId_fileexists:
			{
				CFileExistsNotification* pFileExistsNotification = (CFileExistsNotification *)pNotification;
				pFileExistsNotification->overwriteAction = pEngineData->pItem->m_defaultFileExistsAction;

				if (pEngineData->pItem->GetType() == QueueItemType_File)
				{
					CFileItem* pFileItem = (CFileItem*)pEngineData->pItem;

					switch (pFileItem->m_onetime_action)
					{
					case CFileExistsNotification::resume:
						if (pFileExistsNotification->canResume &&
							pFileItem->m_transferSettings.binary)
						{
							pFileExistsNotification->overwriteAction = CFileExistsNotification::resume;
						}
						break;
					case CFileExistsNotification::overwrite:
						pFileExistsNotification->overwriteAction = CFileExistsNotification::overwrite;
						break;
					default:
						// Others are unused
						break;
					}
					pFileItem->m_onetime_action = CFileExistsNotification::unknown;
				}
			}
			break;
		default:
			break;
		}

		m_pAsyncRequestQueue->AddRequest(pEngineData->pEngine, reinterpret_cast<CAsyncRequestNotification *>(pNotification));
		break;
	case nId_active:
		{
			CActiveNotification *pActiveNotification = reinterpret_cast<CActiveNotification *>(pNotification);
			m_pMainFrame->UpdateActivityLed(pActiveNotification->GetDirection());
			delete pNotification;
		}
		break;
	case nId_transferstatus:
		if (pEngineData->pItem && pEngineData->pStatusLineCtrl)
		{
			CTransferStatusNotification *pTransferStatusNotification = reinterpret_cast<CTransferStatusNotification *>(pNotification);
			const CTransferStatus *pStatus = pTransferStatusNotification->GetStatus();

			if (pEngineData->active)
			{
				if (pStatus && pStatus->madeProgress && !pStatus->list && 
					pEngineData->pItem->GetType() == QueueItemType_File)
				{
					CFileItem* pItem = (CFileItem*)pEngineData->pItem;
					pItem->set_made_progress(true);
				}
				pEngineData->pStatusLineCtrl->SetTransferStatus(pStatus);
			}
		}
		delete pNotification;
		break;
	case nId_local_dir_created:
		{
			CLocalDirCreatedNotification *pLocalDirCreatedNotification = reinterpret_cast<CLocalDirCreatedNotification *>(pNotification);
			const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
			for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
				(*iter)->LocalDirCreated(pLocalDirCreatedNotification->dir);
		}
		delete pNotification;
		break;
	default:
		delete pNotification;
		break;
	}
}

bool CQueueView::CanStartTransfer(const CServerItem& server_item, struct t_EngineData *&pEngineData)
{
	const CServer &server = server_item.GetServer();
	const int max_count = server.MaximumMultipleConnections();
	if (!max_count)
		return true;

	int active_count = server_item.m_activeCount;

	bool browsing_on_same = false;
	const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
	CState* pState = 0;
	for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
	{
		pState = *iter;
		const CServer* pBrowsingServer = pState->GetServer();
		if (!pBrowsingServer)
			continue;

		if (*pBrowsingServer == server)
		{
			active_count++;
			browsing_on_same = true;
			break;
		}
	}

	if (active_count < max_count)
		return true;

	// Max count has been reached

	// If we got an idle engine connected to this very server, start the
	// transfer anyhow. Let's not get this connection go to waste.
	pEngineData = GetIdleEngine(&server);
	if (pEngineData->pEngine->IsConnected() && pEngineData->lastServer == server)
		return true;

	if (!browsing_on_same || active_count > 1)
		return false;

	// At this point the following holds:
	// max_count is limited to 1, only connection to server is held by the browsing connection
	pEngineData = m_engineData[0];

	if (pEngineData->active)
		return false;

	if (pEngineData->pEngine != pState->m_pEngine)
	{
		if (pEngineData->pEngine)
			ReleaseExclusiveEngineLock(pEngineData->pEngine);
		pEngineData->state = t_EngineData::waitprimary;
		pEngineData->pEngine = pState->m_pEngine;
	}

	return true;
}

bool CQueueView::TryStartNextTransfer()
{
	if (m_quit || !m_activeMode)
		return false;

	// Check transfer limit
	if (m_activeCount >= COptions::Get()->GetOptionVal(OPTION_NUMTRANSFERS))
		return false;

	// Check limits for concurrent up/downloads
	const int maxDownloads = COptions::Get()->GetOptionVal(OPTION_CONCURRENTDOWNLOADLIMIT);
	const int maxUploads = COptions::Get()->GetOptionVal(OPTION_CONCURRENTUPLOADLIMIT);
	enum TransferDirection wantedDirection;
	if (maxDownloads && m_activeCountDown >= maxDownloads)
	{
		if (maxUploads && m_activeCountUp >= maxUploads)
			return false;
		else
			wantedDirection = upload;
	}
	else if (maxUploads && m_activeCountUp >= maxUploads)
		wantedDirection = download;
	else
		wantedDirection = both;

	struct t_bestMatch
	{
		CFileItem* fileItem;
		CServerItem* serverItem;
		t_EngineData* pEngineData;
	} bestMatch = {0};

	// Find inactive file. Check all servers for
	// the file with the highest priority
	for (std::vector<CServerItem*>::iterator iter = m_serverList.begin(); iter != m_serverList.end(); iter++)
	{
		t_EngineData* pEngineData = 0;
		CServerItem* currentServerItem = *iter;

		if (!CanStartTransfer(*currentServerItem, pEngineData))
			continue;

		CFileItem* newFileItem = currentServerItem->GetIdleChild(m_activeMode == 1, wantedDirection);

		while (newFileItem && newFileItem->Download() && newFileItem->GetType() == QueueItemType_Folder)
		{
			CLocalPath localPath(newFileItem->GetLocalPath());
			localPath.AddSegment(newFileItem->GetLocalFile());
			wxFileName::Mkdir(localPath.GetPath(), 0777, wxPATH_MKDIR_FULL);
			const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
			for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
				(*iter)->RefreshLocalFile(localPath.GetPath());
			if (RemoveItem(newFileItem, true))
			{
				// Server got deleted. Unfortunately we have to start over now
				if (m_serverList.empty())
					return false;

				return true;
			}
			newFileItem = currentServerItem->GetIdleChild(m_activeMode == 1, wantedDirection);
		}

		if (!newFileItem)
			continue;

		if (!bestMatch.fileItem || newFileItem->GetPriority() > bestMatch.fileItem->GetPriority())
		{
			bestMatch.serverItem = currentServerItem;
			bestMatch.fileItem = newFileItem;
			bestMatch.pEngineData = pEngineData;
			if (newFileItem->GetPriority() == priority_highest)
				break;
		}
	}
	if (!bestMatch.fileItem)
		return false;

	// Find idle engine
	t_EngineData* pEngineData;
	if (bestMatch.pEngineData)
		pEngineData = bestMatch.pEngineData;
	else
	{
		pEngineData = GetIdleEngine(&bestMatch.serverItem->GetServer());
		if (!pEngineData)
			return false;
	}

	// Now we have both inactive engine and file.
	// Assign the file to the engine.

	bestMatch.fileItem->SetActive(true);

	pEngineData->pItem = bestMatch.fileItem;
	bestMatch.fileItem->m_pEngineData = pEngineData;
	pEngineData->active = true;
	delete pEngineData->m_idleDisconnectTimer;
	pEngineData->m_idleDisconnectTimer = 0;
	bestMatch.serverItem->m_activeCount++;
	m_activeCount++;
	if (bestMatch.fileItem->Download())
		m_activeCountDown++;
	else
		m_activeCountUp++;

	const CServer oldServer = pEngineData->lastServer;
	pEngineData->lastServer = bestMatch.serverItem->GetServer();

	if (pEngineData->state != t_EngineData::waitprimary)
	{
		if (!pEngineData->pEngine->IsConnected())
		{
			if (pEngineData->lastServer.GetLogonType() == ASK)
			{
				if (CLoginManager::Get().GetPassword(pEngineData->lastServer, true))
					pEngineData->state = t_EngineData::connect;
				else
					pEngineData->state = t_EngineData::askpassword;
			}
			else
				pEngineData->state = t_EngineData::connect;
		}
		else if (oldServer != bestMatch.serverItem->GetServer())
			pEngineData->state = t_EngineData::disconnect;
		else if (pEngineData->pItem->GetType() == QueueItemType_File)
			pEngineData->state = t_EngineData::transfer;
		else
			pEngineData->state = t_EngineData::mkdir;
	}

	if (bestMatch.fileItem->GetType() == QueueItemType_File)
	{
		// Create status line

		m_itemCount++;
		SetItemCount(m_itemCount);
		int lineIndex = GetItemIndex(bestMatch.fileItem);
		UpdateSelections_ItemAdded(lineIndex + 1);

		wxRect rect = GetClientRect();
		rect.y = GetLineHeight() * (lineIndex + 1 - GetTopItem());
#ifdef __WXMSW__
		rect.y += m_header_height;
#endif
		rect.SetHeight(GetLineHeight());
		m_allowBackgroundErase = false;
		if (!pEngineData->pStatusLineCtrl)
			pEngineData->pStatusLineCtrl = new CStatusLineCtrl(this, pEngineData, rect);
		else
		{
			pEngineData->pStatusLineCtrl->SetTransferStatus(0);
			pEngineData->pStatusLineCtrl->SetSize(rect);
			pEngineData->pStatusLineCtrl->Show();
		}
		m_allowBackgroundErase = true;
		m_statusLineList.push_back(pEngineData->pStatusLineCtrl);
	}

	SendNextCommand(*pEngineData);

	return true;
}

void CQueueView::ProcessReply(t_EngineData* pEngineData, COperationNotification* pNotification)
{
	if (pNotification->nReplyCode & FZ_REPLY_DISCONNECTED &&
		pNotification->commandId == cmd_none)
	{
		// Queue is not interested in disconnect notifications
		return;
	}
	wxASSERT(pNotification->commandId != cmd_none);

	// Cancel pending requests
	m_pAsyncRequestQueue->ClearPending(pEngineData->pEngine);

	// Process reply from the engine
	int replyCode = pNotification->nReplyCode;

	if ((replyCode & FZ_REPLY_CANCELED) == FZ_REPLY_CANCELED)
	{
		enum ResetReason reason;
		if (pEngineData->pItem)
		{
			if (pEngineData->pItem->pending_remove())
				reason = remove;
			else
			{
				if (pEngineData->pItem->GetType() == QueueItemType_File && ((CFileItem*)pEngineData->pItem)->made_progress())
				{
					CFileItem* pItem = (CFileItem*)pEngineData->pItem;
					pItem->set_made_progress(false);
					pItem->m_onetime_action = CFileExistsNotification::resume;
				}
				reason = reset;
			}
			pEngineData->pItem->m_statusMessage = _("Interrupted by user");
		}
		else
			reason = reset;
		ResetEngine(*pEngineData, reason);
		return;
	}

	// Cycle through queue states
	switch (pEngineData->state)
	{
	case t_EngineData::disconnect:
		if (pEngineData->active)
		{
			pEngineData->state = t_EngineData::connect;
			pEngineData->pStatusLineCtrl->SetTransferStatus(0);
		}
		else
			pEngineData->state = t_EngineData::none;
		break;
	case t_EngineData::connect:
		if (!pEngineData->pItem)
		{
			ResetEngine(*pEngineData, reset);
			return;
		}
		else if (replyCode == FZ_REPLY_OK)
		{
			if (pEngineData->pItem->GetType() == QueueItemType_File)
				pEngineData->state = t_EngineData::transfer;
			else
				pEngineData->state = t_EngineData::mkdir;
			if (pEngineData->active && pEngineData->pStatusLineCtrl)
				pEngineData->pStatusLineCtrl->SetTransferStatus(0);
		}
		else
		{
			if (replyCode & FZ_REPLY_PASSWORDFAILED)
				CLoginManager::Get().CachedPasswordFailed(pEngineData->lastServer);

			if ((replyCode & FZ_REPLY_CANCELED) == FZ_REPLY_CANCELED)
				pEngineData->pItem->m_statusMessage.clear();
			else if (replyCode & FZ_REPLY_PASSWORDFAILED)
				pEngineData->pItem->m_statusMessage = _("Incorrect password");
			else if ((replyCode & FZ_REPLY_TIMEOUT) == FZ_REPLY_TIMEOUT)
				pEngineData->pItem->m_statusMessage = _("Timeout");
			else if (replyCode & FZ_REPLY_DISCONNECTED)
				pEngineData->pItem->m_statusMessage = _("Disconnected from server");
			else
				pEngineData->pItem->m_statusMessage = _("Connection attempt failed");

			if (replyCode != (FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED) ||
				!IsOtherEngineConnected(pEngineData))
			{
				if (!IncreaseErrorCount(*pEngineData))
					return;
			}

			if (pEngineData != m_engineData[0])
				SwitchEngine(&pEngineData);
		}
		break;
	case t_EngineData::transfer:
		if (!pEngineData->pItem)
		{
			ResetEngine(*pEngineData, reset);
			return;
		}
		if (replyCode == FZ_REPLY_OK)
		{
			ResetEngine(*pEngineData, success);
			return;
		}
		// Increase error count only if item didn't make any progress. This keeps
		// user interaction at a minimum if connection is unstable.

		if (pEngineData->pItem->GetType() == QueueItemType_File && ((CFileItem*)pEngineData->pItem)->made_progress() &&
			(replyCode & FZ_REPLY_WRITEFAILED) != FZ_REPLY_WRITEFAILED)
		{
			// Don't increase error count if there has been progress
			CFileItem* pItem = (CFileItem*)pEngineData->pItem;
			pItem->set_made_progress(false);
			pItem->m_onetime_action = CFileExistsNotification::resume;
		}
		else
		{
			if ((replyCode & FZ_REPLY_CANCELED) == FZ_REPLY_CANCELED)
				pEngineData->pItem->m_statusMessage.clear();
			else if ((replyCode & FZ_REPLY_TIMEOUT) == FZ_REPLY_TIMEOUT)
				pEngineData->pItem->m_statusMessage = _("Timeout");
			else if (replyCode & FZ_REPLY_DISCONNECTED)
				pEngineData->pItem->m_statusMessage = _("Disconnected from server");
			else if ((replyCode & FZ_REPLY_WRITEFAILED) == FZ_REPLY_WRITEFAILED)
			{
				pEngineData->pItem->m_statusMessage = _("Could not write to local file");
				ResetEngine(*pEngineData, failure);
				return;
			}
			else if ((replyCode & FZ_REPLY_CRITICALERROR) == FZ_REPLY_CRITICALERROR)
			{
				pEngineData->pItem->m_statusMessage = _("Could not start transfer");
				ResetEngine(*pEngineData, failure);
				return;
			}
			else
				pEngineData->pItem->m_statusMessage = _("Could not start transfer");
			if (!IncreaseErrorCount(*pEngineData))
				return;

			if (replyCode & FZ_REPLY_DISCONNECTED && pEngineData == m_engineData[0])
			{
				ResetEngine(*pEngineData, retry);
				return;
			}
		}
		if (replyCode & FZ_REPLY_DISCONNECTED)
		{
			if (!SwitchEngine(&pEngineData))
				pEngineData->state = t_EngineData::connect;
		}
		break;
	case t_EngineData::mkdir:
		if (replyCode == FZ_REPLY_OK)
		{
			ResetEngine(*pEngineData, success);
			return;
		}
		if (replyCode & FZ_REPLY_DISCONNECTED)
		{
			if (!IncreaseErrorCount(*pEngineData))
				return;

			if (pEngineData == m_engineData[0])
			{
				ResetEngine(*pEngineData, retry);
				return;
			}

			if (!SwitchEngine(&pEngineData))
				pEngineData->state = t_EngineData::connect;
		}
		else
		{
			// Cannot retry
			ResetEngine(*pEngineData, failure);
			return;
		}

		break;
	case t_EngineData::list:
		ResetEngine(*pEngineData, remove);
		return;
	default:
		return;
	}

	if (pEngineData->state == t_EngineData::connect && pEngineData->lastServer.GetLogonType() == ASK)
	{
		if (!CLoginManager::Get().GetPassword(pEngineData->lastServer, true))
			pEngineData->state = t_EngineData::askpassword;
	}

	if (!m_activeMode)
	{
		enum ResetReason reason;
		if (pEngineData->pItem && pEngineData->pItem->pending_remove())
			reason = remove;
		else
			reason = reset;
		ResetEngine(*pEngineData, reason);
		return;
	}

	SendNextCommand(*pEngineData);
}

void CQueueView::ResetEngine(t_EngineData& data, const enum ResetReason reason)
{
	if (!data.active)
		return;

	m_waitStatusLineUpdate = true;

	if (data.pItem)
	{
		CServerItem* pServerItem = (CServerItem*)data.pItem->GetTopLevelItem();
		if (pServerItem)
		{
			wxASSERT(pServerItem->m_activeCount > 0);
			if (pServerItem->m_activeCount > 0)
				pServerItem->m_activeCount--;
		}

		if (data.pItem->GetType() == QueueItemType_File)
		{
			wxASSERT(data.pStatusLineCtrl);
			for (std::list<CStatusLineCtrl*>::iterator iter = m_statusLineList.begin(); iter != m_statusLineList.end(); iter++)
			{
				if (*iter == data.pStatusLineCtrl)
				{
					m_statusLineList.erase(iter);
					break;
				}
			}
			m_allowBackgroundErase = false;
			data.pStatusLineCtrl->Hide();
			m_allowBackgroundErase = true;

			UpdateSelections_ItemRemoved(GetItemIndex(data.pItem) + 1);

			m_itemCount--;
			SaveSetItemCount(m_itemCount);

			CFileItem* const pFileItem = (CFileItem*)data.pItem;
			if (pFileItem->Download())
			{
				const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
				for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
					(*iter)->RefreshLocalFile(pFileItem->GetLocalPath().GetPath() + pFileItem->GetLocalFile());
			}

			if (pFileItem->m_edit != CEditHandler::none && reason != retry && reason != reset)
			{
				CEditHandler* pEditHandler = CEditHandler::Get();
				wxASSERT(pEditHandler);
				if (pFileItem->m_edit == CEditHandler::remote)
				{
					const CServerItem* pServerItem = (const CServerItem*)pFileItem->GetTopLevelItem();
					pEditHandler->FinishTransfer(reason == success, pFileItem->GetRemoteFile(), pFileItem->GetRemotePath(), pServerItem->GetServer());
				}
				else
					pEditHandler->FinishTransfer(reason == success, pFileItem->GetLocalPath().GetPath() + pFileItem->GetLocalFile());
				if (reason == success)
					pFileItem->m_edit = CEditHandler::none;
			}

			if (reason == failure)
			{
				pFileItem->m_onetime_action = CFileExistsNotification::unknown;
				pFileItem->set_made_progress(false);
			}
		}

		wxASSERT(data.pItem->IsActive());
		wxASSERT(data.pItem->m_pEngineData == &data);
		if (data.pItem->IsActive())
			data.pItem->SetActive(false);
		if (data.pItem->Download())
		{
			wxASSERT(m_activeCountDown > 0);
			if (m_activeCountDown > 0)
				m_activeCountDown--;
		}
		else
		{
			wxASSERT(m_activeCountUp > 0);
			if (m_activeCountUp > 0)
				m_activeCountUp--;
		}

		if (reason == reset)
		{
			if (!data.pItem->queued())
				reinterpret_cast<CServerItem*>(data.pItem->GetTopLevelItem())->QueueImmediateFile(data.pItem);
		}
		else if (reason == failure)
		{
			if (data.pItem->GetType() == QueueItemType_File || data.pItem->GetType() == QueueItemType_Folder)
			{
				const CServer server = ((CServerItem*)data.pItem->GetTopLevelItem())->GetServer();

				RemoveItem(data.pItem, false);

				CQueueViewFailed* pQueueViewFailed = m_pQueue->GetQueueView_Failed();
				CServerItem* pServerItem = pQueueViewFailed->CreateServerItem(server);
				data.pItem->SetParent(pServerItem);
				data.pItem->UpdateTime();
				pQueueViewFailed->InsertItem(pServerItem, data.pItem);
				pQueueViewFailed->CommitChanges();
			}
		}
		else if (reason == success)
		{
			if (data.pItem->GetType() == QueueItemType_File || data.pItem->GetType() == QueueItemType_Folder)
			{
				CQueueViewSuccessful* pQueueViewSuccessful = m_pQueue->GetQueueView_Successful();
				if (pQueueViewSuccessful->AutoClear())
					RemoveItem(data.pItem, true);
				else
				{
					const CServer server = ((CServerItem*)data.pItem->GetTopLevelItem())->GetServer();

					RemoveItem(data.pItem, false);

					CServerItem* pServerItem = pQueueViewSuccessful->CreateServerItem(server);
					data.pItem->UpdateTime();
					data.pItem->SetParent(pServerItem);
					pQueueViewSuccessful->InsertItem(pServerItem, data.pItem);
					pQueueViewSuccessful->CommitChanges();
				}
			}
			else
				RemoveItem(data.pItem, true);
		}
		else if (reason == retry)
		{
		}
		else
			RemoveItem(data.pItem, true);
		data.pItem = 0;
	}
	wxASSERT(m_activeCount > 0);
	if (m_activeCount > 0)
		m_activeCount--;
	data.active = false;

	if (data.state == t_EngineData::waitprimary && data.pEngine)
	{
		const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
		for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
		{
			CState* pState = *iter;
			if (pState->m_pEngine != data.pEngine)
				continue;
			CCommandQueue* pCommandQueue = pState->m_pCommandQueue;
			if (pCommandQueue)
				pCommandQueue->RequestExclusiveEngine(false);
			break;
		}
	}

	data.state = t_EngineData::none;

	AdvanceQueue();

	m_waitStatusLineUpdate = false;
	UpdateStatusLinePositions();
}

bool CQueueView::RemoveItem(CQueueItem* item, bool destroy, bool updateItemCount /*=true*/, bool updateSelections /*=true*/)
{
	// RemoveItem assumes that the item has already been removed from all engines

	if (item->GetType() == QueueItemType_File)
	{
		// Update size information
		const CFileItem* const pFileItem = (const CFileItem* const)item;
		const wxLongLong& size = pFileItem->GetSize();
		if (size < 0)
		{
			m_filesWithUnknownSize--;
			wxASSERT(m_filesWithUnknownSize >= 0);
			if (!m_filesWithUnknownSize && updateItemCount)
				DisplayQueueSize();
		}
		else if (size > 0)
		{
			m_totalQueueSize -= size;
			if (updateItemCount)
				DisplayQueueSize();
			wxASSERT(m_totalQueueSize >= 0);
		}
	}

	bool didRemoveParent = CQueueViewBase::RemoveItem(item, destroy, updateItemCount, updateSelections);

	UpdateStatusLinePositions();

	return didRemoveParent;
}

void CQueueView::SendNextCommand(t_EngineData& engineData)
{
	while (true)
	{
		if (engineData.state == t_EngineData::waitprimary)
		{
			engineData.pItem->m_statusMessage = _("Waiting for browsing connection");

			wxASSERT(engineData.pEngine);
			if (!engineData.pEngine)
			{
				ResetEngine(engineData, retry);
				return;
			}

			CState* pState = 0;
			const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
			for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
			{
				if ((*iter)->m_pEngine != engineData.pEngine)
					continue;
				pState = *iter;
				break;
			}
			if (!pState)
			{
				ResetEngine(engineData, retry);
				return;
			}

			CCommandQueue* pCommandQueue = pState->m_pCommandQueue;
			pCommandQueue->RequestExclusiveEngine(true);
			return;
		}

		if (engineData.state == t_EngineData::disconnect)
		{
			engineData.pItem->m_statusMessage = _("Disconnecting from previous server");
			RefreshItem(engineData.pItem);
			if (engineData.pEngine->Command(CDisconnectCommand()) == FZ_REPLY_WOULDBLOCK)
				return;

			engineData.state = t_EngineData::connect;
			if (engineData.active && engineData.pStatusLineCtrl)
				engineData.pStatusLineCtrl->SetTransferStatus(0);
		}

		if (engineData.state == t_EngineData::askpassword)
		{
			engineData.pItem->m_statusMessage = _("Waiting for password");
			RefreshItem(engineData.pItem);
			if (m_waitingForPassword.empty())
			{
				wxCommandEvent evt(fzEVT_ASKFORPASSWORD);
				wxPostEvent(this, evt);
			}
			m_waitingForPassword.push_back(engineData.pEngine);
			return;
		}

		if (engineData.state == t_EngineData::connect)
		{
			engineData.pItem->m_statusMessage = _("Connecting");
			RefreshItem(engineData.pItem);

			int res = engineData.pEngine->Command(CConnectCommand(engineData.lastServer, false));

			wxASSERT((res & FZ_REPLY_BUSY) != FZ_REPLY_BUSY);
			if (res == FZ_REPLY_WOULDBLOCK)
				return;

			if (res == FZ_REPLY_ALREADYCONNECTED)
			{
				engineData.state = t_EngineData::disconnect;
				continue;
			}

			if (res == FZ_REPLY_OK)
			{
				if (engineData.pItem->GetType() == QueueItemType_File)
				{
					engineData.state = t_EngineData::transfer;
					if (engineData.active)
						engineData.pStatusLineCtrl->SetTransferStatus(0);
				}
				else
					engineData.state = t_EngineData::mkdir;
				break;
			}

			if (!IncreaseErrorCount(engineData))
				return;
			continue;
		}

		if (engineData.state == t_EngineData::transfer)
		{
			CFileItem* fileItem = engineData.pItem;

			fileItem->m_statusMessage = _("Transferring");
			RefreshItem(engineData.pItem);

			int res = engineData.pEngine->Command(CFileTransferCommand(fileItem->GetLocalPath().GetPath() + fileItem->GetLocalFile(), fileItem->GetRemotePath(),
												fileItem->GetRemoteFile(), fileItem->Download(), fileItem->m_transferSettings));
			wxASSERT((res & FZ_REPLY_BUSY) != FZ_REPLY_BUSY);
			if (res == FZ_REPLY_WOULDBLOCK)
				return;

			if (res == FZ_REPLY_NOTCONNECTED)
			{
				if (&engineData == m_engineData[0])
				{
					ResetEngine(engineData, retry);
					return;
				}

				engineData.state = t_EngineData::connect;
				continue;
			}

			if (res == FZ_REPLY_OK)
			{
				ResetEngine(engineData, success);
				return;
			}

			if (!IncreaseErrorCount(engineData))
				return;
			continue;
		}

		if (engineData.state == t_EngineData::mkdir)
		{
			CFileItem* fileItem = engineData.pItem;

			fileItem->m_statusMessage = _("Creating directory");
			RefreshItem(engineData.pItem);

			int res = engineData.pEngine->Command(CMkdirCommand(fileItem->GetRemotePath()));

			wxASSERT((res & FZ_REPLY_BUSY) != FZ_REPLY_BUSY);
			if (res == FZ_REPLY_WOULDBLOCK)
				return;

			if (res == FZ_REPLY_NOTCONNECTED)
			{
				if (&engineData == m_engineData[0])
				{
					ResetEngine(engineData, retry);
					return;
				}

				engineData.state = t_EngineData::connect;
				continue;
			}

			if (res == FZ_REPLY_OK)
			{
				ResetEngine(engineData, success);
				return;
			}

			// Pointless to retry
			ResetEngine(engineData, failure);
			return;
		}
	}
}

bool CQueueView::SetActive(bool active /*=true*/)
{
	if (!active)
	{
		m_activeMode = 0;
		for (std::vector<CServerItem*>::iterator iter = m_serverList.begin(); iter != m_serverList.end(); iter++)
			(*iter)->QueueImmediateFiles();

		const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
		for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
		{
			CState* pState = *iter;

			CRecursiveOperation* pRecursiveOperation = pState->GetRecursiveOperationHandler();
			if (!pRecursiveOperation)
				continue;

			if (pRecursiveOperation->GetOperationMode() == CRecursiveOperation::recursive_download)
				pRecursiveOperation->ChangeOperationMode(CRecursiveOperation::recursive_addtoqueue);
			if (pRecursiveOperation->GetOperationMode() == CRecursiveOperation::recursive_download_flatten)
				pRecursiveOperation->ChangeOperationMode(CRecursiveOperation::recursive_addtoqueue_flatten);
		}

		UpdateStatusLinePositions();

		// Send active engines the cancel command
		unsigned int engineIndex;
		for (engineIndex = 0; engineIndex < m_engineData.size(); engineIndex++)
		{
			t_EngineData* const pEngineData = m_engineData[engineIndex];
			if (!pEngineData->active)
				continue;

			if (pEngineData->state == t_EngineData::waitprimary)
			{
				if (pEngineData->pItem)
					pEngineData->pItem->m_statusMessage = _("Interrupted by user");
				ResetEngine(*pEngineData, reset);
			}
			else
			{
				wxASSERT(pEngineData->pEngine);
				if (!pEngineData->pEngine)
					continue;
				pEngineData->pEngine->Command(CCancelCommand());
			}
		}

		CContextManager::Get()->NotifyGlobalHandlers(STATECHANGE_QUEUEPROCESSING);

		return m_activeCount == 0;
	}
	else
	{
		m_activeMode = 2;

		m_waitStatusLineUpdate = true;
		AdvanceQueue();
		m_waitStatusLineUpdate = false;
		UpdateStatusLinePositions();
	}

	CContextManager::Get()->NotifyGlobalHandlers(STATECHANGE_QUEUEPROCESSING);

	return true;
}

bool CQueueView::Quit()
{
	if (!m_quit)
		m_quit = 1;

#ifdef __WXMSW__
	if (m_actionAfterWarnDialog)
	{
		m_actionAfterWarnDialog->Destroy();
		m_actionAfterWarnDialog = 0;
	}
	delete m_actionAfterTimer;
	m_actionAfterTimer = 0;
#endif

	bool canQuit = true;
	if (!SetActive(false))
		canQuit = false;

	for (unsigned int i = 0; i < 2; i++)
	{
		if (!m_queuedFolders[i].empty())
		{
			canQuit = false;
			for (std::list<CFolderScanItem*>::iterator iter = m_queuedFolders[i].begin(); iter != m_queuedFolders[i].end(); iter++)
				(*iter)->m_remove = true;
		}
	}
	if (m_pFolderProcessingThread)
		canQuit = false;

	if (!canQuit)
		return false;

	DeleteEngines();

	if (m_quit == 1)
	{
		SaveQueue();
		m_quit = 2;
	}

	SaveColumnSettings(OPTION_QUEUE_COLUMN_WIDTHS, -1, -1);

	m_resize_timer.Stop();

	return true;
}

void CQueueView::CheckQueueState()
{
	if (!m_engineData.empty() && !m_engineData[0]->active && m_engineData[0]->pEngine)
	{
		ReleaseExclusiveEngineLock(m_engineData[0]->pEngine);
		m_engineData[0]->pEngine = 0;
	}

	if (m_activeCount)
		return;

	if (m_activeMode)
	{
		m_activeMode = 0;
		/* Users don't seem to like this, so comment it out for now.
		 * maybe make it configureable in future?
		if (!m_pQueue->GetSelection())
		{
			CQueueViewBase* pFailed = m_pQueue->GetQueueView_Failed();
			CQueueViewBase* pSuccessful = m_pQueue->GetQueueView_Successful();
			if (pFailed->GetItemCount())
				m_pQueue->SetSelection(1);
			else if (pSuccessful->GetItemCount())
				m_pQueue->SetSelection(2);
		}
		*/

		TryRefreshListings();

		CContextManager::Get()->NotifyGlobalHandlers(STATECHANGE_QUEUEPROCESSING);

		if (!m_quit)
			ActionAfter();
	}

	if (m_quit)
		m_pMainFrame->Close();
}

bool CQueueView::IncreaseErrorCount(t_EngineData& engineData)
{
	engineData.pItem->m_errorCount++;
	if (engineData.pItem->m_errorCount <= COptions::Get()->GetOptionVal(OPTION_RECONNECTCOUNT))
		return true;

	ResetEngine(engineData, failure);

	return false;
}

void CQueueView::UpdateStatusLinePositions()
{
	if (m_waitStatusLineUpdate)
		return;

	m_lastTopItem = GetTopItem();
	int bottomItem = m_lastTopItem + GetCountPerPage();

	for (std::list<CStatusLineCtrl*>::iterator iter = m_statusLineList.begin(); iter != m_statusLineList.end(); iter++)
	{
		CStatusLineCtrl *pCtrl = *iter;
		int index = GetItemIndex(pCtrl->GetItem()) + 1;
		if (index < m_lastTopItem || index > bottomItem)
		{
			pCtrl->Show(false);
			continue;
		}

		wxRect rect = GetClientRect();
		rect.y = GetLineHeight() * (index - m_lastTopItem);
#ifdef __WXMSW__
		rect.y += m_header_height;
#endif
		rect.SetHeight(GetLineHeight());

		m_allowBackgroundErase = bottomItem + 1 >= m_itemCount;
		pCtrl->SetSize(rect);
		m_allowBackgroundErase = false;
		pCtrl->Show();
		m_allowBackgroundErase = true;
	}
}

void CQueueView::CalculateQueueSize()
{
	// Collect total queue size
	m_totalQueueSize = 0;
	m_fileCount = 0;
	m_folderScanCount = 0;

	m_filesWithUnknownSize = 0;
	for (std::vector<CServerItem*>::const_iterator iter = m_serverList.begin(); iter != m_serverList.end(); iter++)
		m_totalQueueSize += (*iter)->GetTotalSize(m_filesWithUnknownSize, m_fileCount, m_folderScanCount);

	DisplayQueueSize();
	DisplayNumberQueuedFiles();
}

void CQueueView::DisplayQueueSize()
{
	CStatusBar* pStatusBar = m_pMainFrame->GetStatusBar();
	if (!pStatusBar)
		return;
	pStatusBar->DisplayQueueSize(m_totalQueueSize, m_filesWithUnknownSize != 0);
}

bool CQueueView::QueueFolder(bool queueOnly, bool download, const CLocalPath& localPath, const CServerPath& remotePath, const CServer& server)
{
	CServerItem* pServerItem = CreateServerItem(server);

	CFolderScanItem* folderItem = new CFolderScanItem(pServerItem, queueOnly, download, localPath, remotePath);
	InsertItem(pServerItem, folderItem);

	folderItem->m_statusMessage = _("Waiting");

	CommitChanges();

	m_queuedFolders[download ? 0 : 1].push_back(folderItem);
	ProcessFolderItems();

	RefreshListOnly(false);

	return true;
}

bool CQueueView::ProcessFolderItems(int type /*=-1*/)
{
	if (type == -1)
	{
		while (ProcessFolderItems(0));
		ProcessUploadFolderItems();

		return true;
	}

	return false;
}

void CQueueView::ProcessUploadFolderItems()
{
	if (m_queuedFolders[1].empty())
	{
		if (m_quit)
			m_pMainFrame->Close();

		return;
	}

	if (m_pFolderProcessingThread)
		return;

	CFolderScanItem* pItem = m_queuedFolders[1].front();

	if (pItem->queued())
		pItem->m_statusMessage = _("Scanning for files to add to queue");
	else
		pItem->m_statusMessage = _("Scanning for files to upload");
	RefreshItem(pItem);
	pItem->m_active = true;
	m_pFolderProcessingThread = new CFolderProcessingThread(this, pItem);
	m_pFolderProcessingThread->Create();
	m_pFolderProcessingThread->Run();

	RefreshListOnly(false);
}

void CQueueView::OnFolderThreadComplete(wxCommandEvent& event)
{
	if (!m_pFolderProcessingThread)
		return;

	m_folderscan_item_refresh_timer.Stop();

	wxASSERT(!m_queuedFolders[1].empty());
	CFolderScanItem* pItem = m_queuedFolders[1].front();
	if (pItem->m_dir_is_empty)
	{
		CServerItem* pServerItem = (CServerItem*)pItem->GetTopLevelItem();
		CFileItem* fileItem = new CFolderItem(pServerItem, pItem->queued(), pItem->m_current_remote_path, _T(""));
		InsertItem(pServerItem, fileItem);
		QueueFile_Finish(!pItem->queued());
	}
	m_queuedFolders[1].pop_front();

	RemoveItem(pItem, true);

	m_pFolderProcessingThread->Wait();
	delete m_pFolderProcessingThread;
	m_pFolderProcessingThread = 0;

	ProcessUploadFolderItems();
}

int CQueueView::QueueFiles(const std::list<CFolderProcessingEntry*> &entryList, bool queueOnly, bool download, CServerItem* pServerItem, const enum CFileExistsNotification::OverwriteAction defaultFileExistsAction)
{
	wxASSERT(pServerItem);
	
	CFolderScanItem* pFolderScanItem = m_pFolderProcessingThread->GetFolderScanItem();

	int added = 0;

	CFilterManager filters;
	for (std::list<CFolderProcessingEntry*>::const_iterator iter = entryList.begin(); iter != entryList.end(); iter++)
	{
		const CFolderProcessingEntry* entry = *iter;
		if (entry->m_type == CFolderProcessingEntry::dir)
		{
			if (m_pFolderProcessingThread->GetFolderScanItem()->m_dir_is_empty)
			{
				CFileItem* fileItem = new CFolderItem(pServerItem, queueOnly, pFolderScanItem->m_current_remote_path, _T(""));
				InsertItem(pServerItem, fileItem);
				added++;
			}

			const CFolderProcessingThread::t_dirPair* entry = (const CFolderProcessingThread::t_dirPair*)*iter;
			pFolderScanItem->m_current_local_path = CLocalPath(entry->localPath);
			pFolderScanItem->m_current_remote_path.SetSafePath(entry->remotePath);
			pFolderScanItem->m_dir_is_empty = true;
			delete entry;
		}
		else
		{
			const t_newEntry* entry = (const t_newEntry*)*iter;

			if (filters.FilenameFiltered(entry->name, pFolderScanItem->m_current_local_path.GetPath(), entry->dir, entry->size, true, entry->attributes, &entry->time))
			{
				delete entry;
				continue;
			}

			pFolderScanItem->m_dir_is_empty = false;

			if (entry->dir)
			{
				m_pFolderProcessingThread->ProcessDirectory(pFolderScanItem->m_current_local_path, pFolderScanItem->m_current_remote_path, entry->name);
				delete entry;
				continue;
			}

			CFileItem* fileItem = new CFileItem(pServerItem, queueOnly, download, entry->name, wxEmptyString, pFolderScanItem->m_current_local_path, pFolderScanItem->m_current_remote_path, entry->size);

			if (download)
				fileItem->m_transferSettings.binary = !CAutoAsciiFiles::TransferRemoteAsAscii(entry->name, pFolderScanItem->m_current_remote_path.GetType());
			else
				fileItem->m_transferSettings.binary = !CAutoAsciiFiles::TransferLocalAsAscii(entry->name, pFolderScanItem->m_current_remote_path.GetType());

			fileItem->m_defaultFileExistsAction = defaultFileExistsAction;

			delete entry;

			InsertItem(pServerItem, fileItem);

			added++;
		}
	}

	QueueFile_Finish(!queueOnly);

	return added;
}

void CQueueView::SaveQueue()
{
	// Kiosk mode 2 doesn't save queue
	if (COptions::Get()->GetOptionVal(OPTION_DEFAULT_KIOSKMODE) == 2)
		return;

	// While not really needed anymore using sqlite3, we still take the mutex
	// just as extra precaution. Better 'save' than sorry.
	CInterProcessMutex mutex(MUTEX_QUEUE);

	if (!m_queue_storage.SaveQueue(m_serverList))
	{
		wxString msg = wxString::Format(_("An error occurred saving the transfer queue to \"%s\".\nSome queue items might not have been saved."), m_queue_storage.GetDatabaseFilename().c_str());
		wxMessageBox(msg, _("Error saving queue"), wxICON_ERROR);
	}
}

void CQueueView::LoadQueueFromXML()
{
	wxFileName file(COptions::Get()->GetOption(OPTION_DEFAULT_SETTINGSDIR), _T("queue.xml"));
	CXmlFile xml(file);
	TiXmlElement* pDocument = xml.Load(wxFileName(), false);
	if (!pDocument)
	{
		if (!xml.GetError().empty())
		{
			wxString msg = xml.GetError() + _T("\n\n") + _("The queue will not be saved.");
			wxMessageBox(msg, _("Error loading xml file"), wxICON_ERROR);
		}

		return;
	}

	TiXmlElement* pQueue = pDocument->FirstChildElement("Queue");
	if (!pQueue)
		return;

	ImportQueue(pQueue, false);

	pDocument->RemoveChild(pQueue);

	if (COptions::Get()->GetOptionVal(OPTION_DEFAULT_KIOSKMODE) == 2)
		return;

	wxString error;
	if (!xml.Save(&error))
	{
		wxString msg = wxString::Format(_("Could not write \"%s\", the queue could not be saved.\n%s"), file.GetFullPath().c_str(), error.c_str());
		wxMessageBox(msg, _("Error writing xml file"), wxICON_ERROR);
	}
}

void CQueueView::LoadQueue()
{
	// We have to synchronize access to queue.xml so that multiple processed don't write
	// to the same file or one is reading while the other one writes.
	CInterProcessMutex mutex(MUTEX_QUEUE);

	LoadQueueFromXML();

	bool error = false;

	if (!m_queue_storage.BeginTransaction())
		error = true;
	else
	{
		CServer server;
		wxLongLong_t id;
		for (id = m_queue_storage.GetServer(server, true); id > 0; id = m_queue_storage.GetServer(server, false))
		{
			m_insertionStart = -1;
			m_insertionCount = 0;
			CServerItem *pServerItem = CreateServerItem(server);

			CFileItem* fileItem = 0;
			wxLongLong_t fileId;
			for (fileId = m_queue_storage.GetFile(&fileItem, id); fileItem; fileId = m_queue_storage.GetFile(&fileItem, 0))
			{
				fileItem->SetParent(pServerItem);
				fileItem->SetPriority(fileItem->GetPriority());
				InsertItem(pServerItem, fileItem);
			}
			if (fileId < 0)
				error = true;

			if (!pServerItem->GetChild(0))
			{
				m_itemCount--;
				m_serverList.pop_back();
				delete pServerItem;
			}
		}
		if (id < 0)
			error = true;

		if (COptions::Get()->GetOptionVal(OPTION_DEFAULT_KIOSKMODE) != 2)
			if (!m_queue_storage.Clear())
				error = true;

		if (!m_queue_storage.EndTransaction())
			error = true;

		if (!m_queue_storage.Vacuum())
			error = true;
	}
	
	m_insertionStart = -1;
	m_insertionCount = 0;
	CommitChanges();

	if (error)
	{
		wxString file = CQueueStorage::GetDatabaseFilename();
		wxString msg = wxString::Format(_("An error occurred loading the transfer queue from \"%s\".\nSome queue items might not have been restored."), file.c_str());
		wxMessageBox(msg, _("Error loading queue"), wxICON_ERROR);
	}
}

void CQueueView::ImportQueue(TiXmlElement* pElement, bool updateSelections)
{
	TiXmlElement* pServer = pElement->FirstChildElement("Server");
	while (pServer)
	{
		CServer server;
		if (GetServer(pServer, server))
		{
			m_insertionStart = -1;
			m_insertionCount = 0;
			CServerItem *pServerItem = CreateServerItem(server);

			CLocalPath previousLocalPath;
			CServerPath previousRemotePath;

			for (TiXmlElement* pFile = pServer->FirstChildElement("File"); pFile; pFile = pFile->NextSiblingElement("File"))
			{
				wxString localFile = GetTextElement(pFile, "LocalFile");
				wxString remoteFile = GetTextElement(pFile, "RemoteFile");
				wxString safeRemotePath = GetTextElement(pFile, "RemotePath");
				bool download = GetTextElementInt(pFile, "Download") != 0;
				wxLongLong size = GetTextElementLongLong(pFile, "Size", -1);
				unsigned int errorCount = GetTextElementInt(pFile, "ErrorCount");
				unsigned int priority = GetTextElementInt(pFile, "Priority", priority_normal);

				int dataType = GetTextElementInt(pFile, "DataType", -1);
				if (dataType == -1)
					dataType = GetTextElementInt(pFile, "TransferMode", 1); 
				bool binary = dataType != 0;
				int overwrite_action = GetTextElementInt(pFile, "OverwriteAction", CFileExistsNotification::unknown);

				CServerPath remotePath;
				if (localFile != _T("") && remoteFile != _T("") && remotePath.SetSafePath(safeRemotePath) &&
					size >= -1 && priority < PRIORITY_COUNT)
				{
					wxString localFileName;
					CLocalPath localPath(localFile, &localFileName);

					if (localFileName.empty())
						continue;

					// CServerPath and wxString are reference counted.
					// Save some memory here by re-using the old copy
					if (localPath != previousLocalPath)
						previousLocalPath = localPath;
					if (previousRemotePath != remotePath)
						previousRemotePath = remotePath;

					CFileItem* fileItem = new CFileItem(pServerItem, true, download,
						download ? remoteFile : localFileName,
						(remoteFile != localFileName) ? (download ? localFileName : remoteFile) : wxString(),
						previousLocalPath, previousRemotePath, size);
					fileItem->m_transferSettings.binary = binary;
					fileItem->SetPriorityRaw((enum QueuePriority)priority);
					fileItem->m_errorCount = errorCount;
					InsertItem(pServerItem, fileItem);

					if (overwrite_action > 0 && overwrite_action < CFileExistsNotification::ACTION_COUNT)
						fileItem->m_defaultFileExistsAction = (CFileExistsNotification::OverwriteAction)overwrite_action;
				}
			}
			for (TiXmlElement* pFolder = pServer->FirstChildElement("Folder"); pFolder; pFolder = pFolder->NextSiblingElement("Folder"))
			{
				CFolderItem* folderItem;

				bool download = GetTextElementInt(pFolder, "Download") != 0;
				if (download)
				{
					wxString localFile = GetTextElement(pFolder, "LocalFile");
					if (localFile == _T(""))
						continue;
					folderItem = new CFolderItem(pServerItem, true, CLocalPath(localFile));
				}
				else
				{
					wxString remoteFile = GetTextElement(pFolder, "RemoteFile");
					wxString safeRemotePath = GetTextElement(pFolder, "RemotePath");
					if (safeRemotePath == _T(""))
						continue;

					CServerPath remotePath;
					if (!remotePath.SetSafePath(safeRemotePath))
						continue;
					folderItem = new CFolderItem(pServerItem, true, remotePath, remoteFile);
				}

				unsigned int priority = GetTextElementInt(pFolder, "Priority", priority_normal);
				if (priority >= PRIORITY_COUNT)
				{
					delete folderItem;
					continue;
				}
				folderItem->SetPriority((enum QueuePriority)priority);

				InsertItem(pServerItem, folderItem);
			}

			if (!pServerItem->GetChild(0))
			{
				m_itemCount--;
				m_serverList.pop_back();
				delete pServerItem;
			}
			else if (updateSelections)
				CommitChanges();
		}

		pServer = pServer->NextSiblingElement("Server");
	}

	if (!updateSelections)
	{
		m_insertionStart = -1;
		m_insertionCount = 0;
		CommitChanges();
	}
	else
		RefreshListOnly();
}

void CQueueView::SettingsChanged()
{
	// Create missing engines if needed
	const int engineCount = m_engineData.size();
	const int newEngineCount = COptions::Get()->GetOptionVal(OPTION_NUMTRANSFERS);
	if (newEngineCount >= engineCount)
	{
		for (int i = 0; i < (newEngineCount - engineCount + 1); i++)
		{
			t_EngineData *pData = new t_EngineData;
			pData->active = false;
			pData->pItem = 0;
			pData->pStatusLineCtrl = 0;
			pData->state = t_EngineData::none;
			pData->m_idleDisconnectTimer = 0;

			pData->pEngine = new CFileZillaEngine();
			pData->pEngine->Init(this, COptions::Get());

			m_engineData.push_back(pData);
		}
	}

	if (m_activeMode)
		AdvanceQueue();
}

void CQueueView::OnPostScroll()
{
	if (GetTopItem() != m_lastTopItem)
		UpdateStatusLinePositions();
}

void CQueueView::OnContextMenu(wxContextMenuEvent& event)
{
	wxMenu* pMenu = wxXmlResource::Get()->LoadMenu(_T("ID_MENU_QUEUE"));
	if (!pMenu)
		return;

	bool has_selection = HasSelection();

	pMenu->Check(XRCID("ID_PROCESSQUEUE"), IsActive() ? true : false);
	pMenu->Check(XRCID("ID_ACTIONAFTER_DISABLE"), IsActionAfter(ActionAfterState_Disabled));
	pMenu->Check(XRCID("ID_ACTIONAFTER_CLOSE"), IsActionAfter(ActionAfterState_Close));
	pMenu->Check(XRCID("ID_ACTIONAFTER_DISCONNECT"), IsActionAfter(ActionAfterState_Disconnect));
	pMenu->Check(XRCID("ID_ACTIONAFTER_RUNCOMMAND"), IsActionAfter(ActionAfterState_RunCommand));
	pMenu->Check(XRCID("ID_ACTIONAFTER_SHOWMESSAGE"), IsActionAfter(ActionAfterState_ShowMessage));
	pMenu->Check(XRCID("ID_ACTIONAFTER_PLAYSOUND"), IsActionAfter(ActionAfterState_PlaySound));
#ifdef __WXMSW__
	pMenu->Check(XRCID("ID_ACTIONAFTER_REBOOT"), IsActionAfter(ActionAfterState_Reboot));
	pMenu->Check(XRCID("ID_ACTIONAFTER_SHUTDOWN"), IsActionAfter(ActionAfterState_Shutdown));
#endif
	pMenu->Enable(XRCID("ID_REMOVE"), has_selection);

	pMenu->Enable(XRCID("ID_PRIORITY"), has_selection);
	pMenu->Enable(XRCID("ID_DEFAULT_FILEEXISTSACTION"), has_selection);
#ifdef __WXMSW__
	pMenu->Enable(XRCID("ID_ACTIONAFTER"), m_actionAfterWarnDialog == NULL);
#endif

	PopupMenu(pMenu);
	delete pMenu;
}

void CQueueView::OnProcessQueue(wxCommandEvent& event)
{
	SetActive(event.IsChecked());
}

void CQueueView::OnStopAndClear(wxCommandEvent& event)
{
	SetActive(false);
	RemoveAll();
}

void CQueueView::OnActionAfter(wxCommandEvent& event)
{
	if (!event.IsChecked() || event.GetId() == XRCID("ID_ACTIONAFTER_DISABLE"))
	{ // Goes from checked to non-checked or disable is pressed
		m_actionAfterState = ActionAfterState_Disabled;
		m_actionAfterRunCommand = _T("");

#ifdef __WXMSW__
		if (m_actionAfterWarnDialog)
		{
			m_actionAfterWarnDialog->Destroy();
			m_actionAfterWarnDialog = 0;
		}
		delete m_actionAfterTimer;
		m_actionAfterTimer = 0;
#endif
	}
	else if (event.GetId() == XRCID("ID_ACTIONAFTER_DISABLE"))
		m_actionAfterState = ActionAfterState_Disabled;

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_CLOSE"))
		m_actionAfterState = ActionAfterState_Close;

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_DISCONNECT"))
		m_actionAfterState = ActionAfterState_Disconnect;

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_SHOWMESSAGE"))
		m_actionAfterState = ActionAfterState_ShowMessage;

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_PLAYSOUND"))
		m_actionAfterState = ActionAfterState_PlaySound;

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_RUNCOMMAND"))
	{
		m_actionAfterState = ActionAfterState_RunCommand;
		wxTextEntryDialog dlg(m_pMainFrame, _("Please enter a path and executable to run.\nE.g. c:\\somePath\\file.exe under MS Windows or /somePath/file under Unix.\nYou can also optionally specify program arguments."), _("Enter command"));

		if (dlg.ShowModal() != wxID_OK)
		{
			m_actionAfterState = ActionAfterState_Disabled;
			return;
		}
		const wxString &command = dlg.GetValue();

		if (command == _T(""))
		{
			wxMessageBox(_("No command given, aborting."), _("Empty command"), wxICON_ERROR, m_pMainFrame);
			m_actionAfterState = ActionAfterState_Disabled;
			return;
		}
		m_actionAfterRunCommand = command;
	}

#ifdef __WXMSW__

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_REBOOT"))
		m_actionAfterState = ActionAfterState_Reboot;

	else if (event.GetId() == XRCID("ID_ACTIONAFTER_SHUTDOWN"))
		m_actionAfterState = ActionAfterState_Shutdown;

#endif

}

void CQueueView::RemoveAll()
{
	// This function removes all inactive items and queues active items
	// for removal

	// First, clear all selections
#ifndef __WXMSW__
	// GetNextItem is O(n) if nothing is selected, GetSelectedItemCount() is O(1)
	if (GetSelectedItemCount())
#endif
	{
		int item;
		while ((item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
			SetItemState(item, 0, wxLIST_STATE_SELECTED);
	}

	std::vector<CServerItem*> newServerList;
	m_itemCount = 0;
	for (std::vector<CServerItem*>::iterator iter = m_serverList.begin(); iter != m_serverList.end(); iter++)
	{
		if ((*iter)->TryRemoveAll())
			delete *iter;
		else
		{
			newServerList.push_back(*iter);
			m_itemCount += 1 + (*iter)->GetChildrenCount(true);
		}
	}

	// Clear list of queued directories that aren't busy
	for (unsigned int i = 0; i < 2; i++)
	{
		std::list<CFolderScanItem*>::iterator begin = m_queuedFolders[i].begin();
		std::list<CFolderScanItem*>::iterator end = m_queuedFolders[i].end();
		if (begin != end && (*begin)->m_active)
			++begin;
		m_queuedFolders[i].erase(begin, end);
	}

	SaveSetItemCount(m_itemCount);
	m_actionAfterState = ActionAfterState_Disabled;

	m_serverList = newServerList;
	UpdateStatusLinePositions();

	CalculateQueueSize();

	CheckQueueState();
	RefreshListOnly();
}

void CQueueView::RemoveQueuedFolderItem(CFolderScanItem* pFolder)
{
	for (unsigned int i = 0; i < 2; i++)
	{
		for (std::list<CFolderScanItem*>::iterator iter = m_queuedFolders[i].begin(); iter != m_queuedFolders[i].end(); iter++)
		{
			if (*iter != pFolder)
				continue;

			m_queuedFolders[i].erase(iter);
			return;
		}
	}
}

void CQueueView::OnRemoveSelected(wxCommandEvent& event)
{
#ifndef __WXMSW__
	// GetNextItem is O(n) if nothing is selected, GetSelectedItemCount() is O(1)
	if (!GetSelectedItemCount())
		return;
#endif

	std::list<CQueueItem*> selectedItems;
	long item = -1;
	while (true)
	{
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		selectedItems.push_front(GetQueueItem(item));
		SetItemState(item, 0, wxLIST_STATE_SELECTED);
	}

	m_waitStatusLineUpdate = true;

	while (!selectedItems.empty())
	{
		CQueueItem* pItem = selectedItems.front();
		selectedItems.pop_front();

		if (pItem->GetType() == QueueItemType_Status)
			continue;
		else if (pItem->GetType() == QueueItemType_FolderScan)
		{
			CFolderScanItem* pFolder = (CFolderScanItem*)pItem;
			if (pFolder->m_active)
			{
				pFolder->m_remove = true;
				continue;
			}
			else
				RemoveQueuedFolderItem(pFolder);
		}
		else if (pItem->GetType() == QueueItemType_Server)
		{
			CServerItem* pServer = (CServerItem*)pItem;
			StopItem(pServer);

			// Server items get deleted automatically if all children are gone
			continue;
		}
		else if (pItem->GetType() == QueueItemType_File ||
				 pItem->GetType() == QueueItemType_Folder)
		{
			CFileItem* pFile = (CFileItem*)pItem;
			if (pFile->IsActive())
			{
				pFile->set_pending_remove(true);
				StopItem(pFile);
				continue;
			}
		}

		CQueueItem* pTopLevelItem = pItem->GetTopLevelItem();
		if (!pTopLevelItem->GetChild(1))
		{
			// Parent will get deleted
			// If next selected item is parent, remove it from list
			if (!selectedItems.empty() && selectedItems.front() == pTopLevelItem)
				selectedItems.pop_front();
		}
		RemoveItem(pItem, true, false, false);
	}
	DisplayNumberQueuedFiles();
	DisplayQueueSize();
	SaveSetItemCount(m_itemCount);

	m_waitStatusLineUpdate = false;
	UpdateStatusLinePositions();

	RefreshListOnly();
}

bool CQueueView::StopItem(CFileItem* item)
{
	if (!item->IsActive())
		return true;

	((CServerItem*)item->GetTopLevelItem())->QueueImmediateFile(item);

	if (item->m_pEngineData->state == t_EngineData::waitprimary)
	{
		enum ResetReason reason;
		if (item->m_pEngineData->pItem && item->m_pEngineData->pItem->pending_remove())
			reason = remove;
		else
			reason = reset;
		if (item->m_pEngineData->pItem)
			item->m_pEngineData->pItem->m_statusMessage.clear();
		ResetEngine(*item->m_pEngineData, reason);
		return true;
	}
	else
	{
		item->m_pEngineData->pEngine->Command(CCancelCommand());
		return false;
	}
}

bool CQueueView::StopItem(CServerItem* pServerItem)
{
	std::list<CQueueItem*> items;
	for (unsigned int i = 0; i < pServerItem->GetChildrenCount(false); i++)
		items.push_back(pServerItem->GetChild(i, false));

	for (std::list<CQueueItem*>::reverse_iterator iter = items.rbegin(); iter != items.rend(); iter++)
	{
		CQueueItem* pItem = *iter;
		if (pItem->GetType() == QueueItemType_FolderScan)
		{
			CFolderScanItem* pFolder = (CFolderScanItem*)pItem;
			if (pFolder->m_active)
			{
				pFolder->m_remove = true;
				continue;
			}
		}
		else if (pItem->GetType() == QueueItemType_File ||
				 pItem->GetType() == QueueItemType_Folder)
		{
			CFileItem* pFile = (CFileItem*)pItem;
			if (pFile->IsActive())
			{
				pFile->set_pending_remove(true);
				StopItem(pFile);
				continue;
			}
		}
		else
			// Unknown type, shouldn't be here.
			wxASSERT(false);

		if (RemoveItem(pItem, true, false))
		{
			DisplayNumberQueuedFiles();
			SaveSetItemCount(m_itemCount);
			return true;
		}
	}
	DisplayNumberQueuedFiles();
	SaveSetItemCount(m_itemCount);

	return false;
}

void CQueueView::OnFolderThreadFiles(wxCommandEvent& event)
{
	if (!m_pFolderProcessingThread)
		return;

	wxASSERT(!m_queuedFolders[1].empty());
	CFolderScanItem* pItem = m_queuedFolders[1].front();

	std::list<CFolderProcessingEntry*> entryList;
	m_pFolderProcessingThread->GetFiles(entryList);
	int added = QueueFiles(entryList, pItem->queued(), false, (CServerItem*)pItem->GetTopLevelItem(), pItem->m_defaultFileExistsAction);
	m_pFolderProcessingThread->CheckFinished();

	pItem->m_count += added;

	if (!m_folderscan_item_refresh_timer.IsRunning())
		m_folderscan_item_refresh_timer.Start(200, true);
}

void CQueueView::SetDefaultFileExistsAction(enum CFileExistsNotification::OverwriteAction action, const enum TransferDirection direction)
{
	for (std::vector<CServerItem*>::iterator iter = m_serverList.begin(); iter != m_serverList.end(); iter++)
		(*iter)->SetDefaultFileExistsAction(action, direction);
}

void CQueueView::OnSetDefaultFileExistsAction(wxCommandEvent &event)
{
	if (!HasSelection())
		return;

	CDefaultFileExistsDlg dlg;
	if (!dlg.Load(this, true))
		return;

	// Get current default action for the item
	enum CFileExistsNotification::OverwriteAction downloadAction = CFileExistsNotification::unknown;
	enum CFileExistsNotification::OverwriteAction uploadAction = CFileExistsNotification::unknown;
	bool has_upload = false;
	bool has_download = false;
	bool download_unknown = false;
	bool upload_unknown = false;

	long item = -1;
	while (true)
	{
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		CQueueItem* pItem = GetQueueItem(item);
		if (!pItem)
			continue;

		switch (pItem->GetType())
		{
		case QueueItemType_FolderScan:
			if (uploadAction == CFileExistsNotification::unknown)
				uploadAction = ((CFolderScanItem*)pItem)->m_defaultFileExistsAction;
			else if (((CFolderScanItem*)pItem)->m_defaultFileExistsAction != uploadAction)
				upload_unknown = true;
			has_upload = true;
			break;
		case QueueItemType_File:
			{
				CFileItem *pFileItem = (CFileItem*)pItem;
				if (pFileItem->Download())
				{
					if (downloadAction == CFileExistsNotification::unknown)
						downloadAction = pFileItem->m_defaultFileExistsAction;
					else if (pFileItem->m_defaultFileExistsAction != downloadAction)
						download_unknown = true;
					has_download = true;
				}
				else
				{
					if (uploadAction == CFileExistsNotification::unknown)
						uploadAction = pFileItem->m_defaultFileExistsAction;
					else if (pFileItem->m_defaultFileExistsAction != uploadAction)
						upload_unknown = true;
					has_upload = true;
				}
			}
			break;
		case QueueItemType_Server:
			{
				download_unknown = true;
				upload_unknown = true;
				has_download = true;
				has_upload = true;
			}
			break;
		default:
			break;
		}
	}
	if (download_unknown)
		downloadAction = CFileExistsNotification::unknown;
	if (upload_unknown)
		uploadAction = CFileExistsNotification::unknown;

	if (!dlg.Run(has_download ? &downloadAction : 0, has_upload ? &uploadAction : 0))
		return;

	item = -1;
	while (true)
	{
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1)
			break;

		CQueueItem* pItem = GetQueueItem(item);
		if (!pItem)
			continue;

		switch (pItem->GetType())
		{
		case QueueItemType_FolderScan:
			if (!has_upload)
				break;
			((CFolderScanItem*)pItem)->m_defaultFileExistsAction = uploadAction;
			break;
		case QueueItemType_File:
			{
				CFileItem *pFileItem = (CFileItem*)pItem;
				if (pFileItem->Download())
				{
					if (!has_download)
						break;
					pFileItem->m_defaultFileExistsAction = downloadAction;
				}
				else
				{
					if (!has_upload)
						break;
					pFileItem->m_defaultFileExistsAction = uploadAction;
				}
			}
			break;
		case QueueItemType_Server:
			{
				CServerItem *pServerItem = (CServerItem*)pItem;
				if (has_download)
					pServerItem->SetDefaultFileExistsAction(downloadAction, download);
				if (has_upload)
					pServerItem->SetDefaultFileExistsAction(uploadAction, upload);
			}
			break;
		default:
			break;
		}
	}
}

t_EngineData* CQueueView::GetIdleEngine(const CServer* pServer /*=0*/)
{
	t_EngineData* pFirstIdle = 0;

	// We start with 1, since 0 is the primary connection
	for (unsigned int i = 1; i < m_engineData.size(); i++)
	{
		if (m_engineData[i]->active)
			continue;

		if (!pServer)
			return m_engineData[i];

		if (m_engineData[i]->pEngine->IsConnected() && m_engineData[i]->lastServer == *pServer)
			return m_engineData[i];

		if (!pFirstIdle)
			pFirstIdle = m_engineData[i];
	}

	return pFirstIdle;
}

void CQueueView::TryRefreshListings()
{
	if (m_quit)
		return;

	const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
	for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
	{
		CState* pState = *iter;

		const CServer* const pServer = pState->GetServer();
		if (!pServer)
			continue;

		const CDirectoryListing* const pListing = pState->GetRemoteDir().Value();
		if (!pListing)
			continue;

		// See if there's an engine that is already listing
		unsigned int i;
		for (i = 1; i < m_engineData.size(); i++)
		{
			if (!m_engineData[i]->active || m_engineData[i]->state != t_EngineData::list)
				continue;

			if (m_engineData[i]->lastServer != *pServer)
				continue;

			// This engine is already listing a directory on the current server
			break;
		}
		if (i != m_engineData.size())
			continue;

		if (m_last_refresh_server == *pServer && m_last_refresh_path == pListing->path &&
			m_last_refresh_listing_time == pListing->m_firstListTime)
		{
			// Do not try to refresh same directory multiple times
			continue;
		}

		t_EngineData* pEngineData = GetIdleEngine(pServer);
		if (!pEngineData)
			continue;

		if (!pEngineData->pEngine->IsConnected() || pEngineData->lastServer != *pServer)
			continue;

		m_last_refresh_server = *pServer;
		m_last_refresh_path = pListing->path;
		m_last_refresh_listing_time = pListing->m_firstListTime;

		CListCommand command(pListing->path, _T(""), LIST_FLAG_AVOID);
		int res = pEngineData->pEngine->Command(command);
		if (res != FZ_REPLY_WOULDBLOCK)
			continue;

		pEngineData->active = true;
		pEngineData->state = t_EngineData::list;
		m_activeCount++;

		break;
	}
}

void CQueueView::OnAskPassword(wxCommandEvent& event)
{
	while (!m_waitingForPassword.empty())
	{
		const CFileZillaEngine* const pEngine = m_waitingForPassword.front();

		std::vector<t_EngineData*>::iterator iter;
		for (iter = m_engineData.begin(); iter != m_engineData.end(); iter++)
		{
			if ((*iter)->pEngine == pEngine)
				break;
		}
		if (iter == m_engineData.end())
		{
			m_waitingForPassword.pop_front();
			continue;
		}

		t_EngineData* pEngineData = *iter;

		if (pEngineData->state != t_EngineData::askpassword)
		{
			m_waitingForPassword.pop_front();
			continue;
		}

		if (CLoginManager::Get().GetPassword(pEngineData->lastServer, false))
		{
			pEngineData->state = t_EngineData::connect;
			SendNextCommand(*pEngineData);
		}
		else
			ResetEngine(*pEngineData, remove);

		m_waitingForPassword.pop_front();
	}
}

void CQueueView::UpdateItemSize(CFileItem* pItem, wxLongLong size)
{
	wxASSERT(pItem);

	const wxLongLong oldSize = pItem->GetSize();
	if (size == oldSize)
		return;

	if (oldSize == -1)
	{
		wxASSERT(m_filesWithUnknownSize);
		if (m_filesWithUnknownSize)
			m_filesWithUnknownSize--;
	}
	else
	{
		wxASSERT(m_totalQueueSize >= oldSize);
		if (m_totalQueueSize > oldSize)
			m_totalQueueSize -= oldSize;
		else
			m_totalQueueSize = 0;
	}

	if (size == -1)
		m_filesWithUnknownSize++;
	else
		m_totalQueueSize += size;

	pItem->SetSize(size);

	DisplayQueueSize();
}

void CQueueView::AdvanceQueue(bool refresh /*=true*/)
{
	static bool insideAdvanceQueue = false;
	if (insideAdvanceQueue)
		return;

	insideAdvanceQueue = true;
	while (TryStartNextTransfer())
	{
	}

	// Set timer for connected, idle engines
	for (unsigned int i = 1; i < m_engineData.size(); i++)
	{
		if (m_engineData[i]->active)
			continue;

		if (m_engineData[i]->m_idleDisconnectTimer)
		{
			if (m_engineData[i]->pEngine->IsConnected())
				continue;

			delete m_engineData[i]->m_idleDisconnectTimer;
			m_engineData[i]->m_idleDisconnectTimer = 0;
		}
		else
		{
			if (!m_engineData[i]->pEngine->IsConnected())
				continue;

			m_engineData[i]->m_idleDisconnectTimer = new wxTimer(this);
			m_engineData[i]->m_idleDisconnectTimer->Start(60000, true);
		}
	}

	if (refresh)
		RefreshListOnly(false);

	insideAdvanceQueue = false;

	CheckQueueState();
}

void CQueueView::InsertItem(CServerItem* pServerItem, CQueueItem* pItem)
{
	CQueueViewBase::InsertItem(pServerItem, pItem);

	if (pItem->GetType() == QueueItemType_File)
	{
		CFileItem* pFileItem = (CFileItem*)pItem;

		const wxLongLong& size = pFileItem->GetSize();
		if (size < 0)
			m_filesWithUnknownSize++;
		else if (size > 0)
			m_totalQueueSize += size;
	}
}

void CQueueView::CommitChanges()
{
	CQueueViewBase::CommitChanges();

	DisplayQueueSize();
}

void CQueueView::OnTimer(wxTimerEvent& event)
{
	const int id = event.GetId();
	if (id == -1)
		return;
#ifdef __WXMSW__
	if (id == m_actionAfterTimerId)
	{
		OnActionAfterTimerTick();
		return;
	}
#endif

	if (id == m_resize_timer.GetId())
	{
		UpdateStatusLinePositions();
		return;
	}

	if (id == m_folderscan_item_refresh_timer.GetId())
	{
		if (m_queuedFolders[1].empty())
			return;

		CFolderScanItem* pItem = m_queuedFolders[1].front();
		pItem->m_statusMessage = wxString::Format(_("%d files added to queue"), pItem->GetCount());
		RefreshItem(pItem);
		return;
	}

	for (unsigned int i = 1; i < m_engineData.size(); i++)
	{
		t_EngineData* pData = m_engineData[i];
		if (pData->m_idleDisconnectTimer && !pData->m_idleDisconnectTimer->IsRunning())
		{
			delete pData->m_idleDisconnectTimer;
			pData->m_idleDisconnectTimer = 0;

			if (pData->pEngine->IsConnected())
				pData->pEngine->Command(CDisconnectCommand());
		}
	}

	event.Skip();
}

void CQueueView::DeleteEngines()
{
	for (unsigned int engineIndex = 1; engineIndex < m_engineData.size(); engineIndex++)
	{
		t_EngineData* const data = m_engineData[engineIndex];
		wxASSERT(!data->active);
		delete data->pEngine;
		data->pEngine = 0;
		delete data->m_idleDisconnectTimer;
		data->m_idleDisconnectTimer = 0;
	}
	for (unsigned int engineIndex = 0; engineIndex < m_engineData.size(); engineIndex++)
		delete m_engineData[engineIndex];
	m_engineData.clear();
}

void CQueueView::WriteToFile(TiXmlElement* pElement) const
{
	TiXmlElement* pQueue = pElement->FirstChildElement("Queue");
	if (!pQueue)
	{
		pQueue = pElement->LinkEndChild(new TiXmlElement("Queue"))->ToElement();
	}

	wxASSERT(pQueue);

	for (std::vector<CServerItem*>::const_iterator iter = m_serverList.begin(); iter != m_serverList.end(); iter++)
		(*iter)->SaveItem(pQueue);
}

void CQueueView::OnSetPriority(wxCommandEvent& event)
{
#ifndef __WXMSW__
	// GetNextItem is O(n) if nothing is selected, GetSelectedItemCount() is O(1)
	if (!GetSelectedItemCount())
		return;
#endif

	enum QueuePriority priority;

	const int id = event.GetId();
	if (id == XRCID("ID_PRIORITY_LOWEST"))
		priority = priority_lowest;
	else if (id == XRCID("ID_PRIORITY_LOW"))
		priority = priority_low;
	else if (id == XRCID("ID_PRIORITY_HIGH"))
		priority = priority_high;
	else if (id == XRCID("ID_PRIORITY_HIGHEST"))
		priority = priority_highest;
	else
		priority = priority_normal;


	CQueueItem* pSkip = 0;
	long item = -1;
	while (-1 != (item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)))
	{
		CQueueItem* pItem = GetQueueItem(item);
		if (!pItem)
			continue;

		if (pItem->GetType() == QueueItemType_Server)
			pSkip = pItem;
		else if (pItem->GetTopLevelItem() == pSkip)
			continue;
		else
			pSkip = 0;

		pItem->SetPriority(priority);
	}

	RefreshListOnly();
}

void CQueueView::OnExclusiveEngineRequestGranted(wxCommandEvent& event)
{
	t_EngineData* pEngineData = m_engineData[0];

	CFileZillaEngine* pEngine = 0;
	CState* pState;
	CCommandQueue* pCommandQueue;
	const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
	for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
	{
		pState = *iter;
		pCommandQueue = pState->m_pCommandQueue;
		if (!pCommandQueue)
			continue;

		pEngine = pCommandQueue->GetEngineExclusive(event.GetId());
		if (!pEngine)
			continue;

		if (pEngine != pEngineData->pEngine)
		{
			pCommandQueue->ReleaseEngine();
			pEngine = 0;
			continue;
		}

		break;
	}

	if (!pEngine)
		return;

	if (!pEngineData->active)
	{
		pCommandQueue->ReleaseEngine();
		return;
	}

	wxASSERT(pEngineData->state == t_EngineData::waitprimary);
	if (pEngineData->state != t_EngineData::waitprimary)
		return;

	CServerItem* pServerItem = (CServerItem*)pEngineData->pItem->GetParent();

	const CServer* pCurrentServer = pState->GetServer();

	wxASSERT(pServerItem);

	if (!pCurrentServer || *pCurrentServer != pServerItem->GetServer())
	{
		if (pState->m_pCommandQueue)
			pState->m_pCommandQueue->ReleaseEngine();
		ResetEngine(*pEngineData, retry);
		return;
	}

	if (pEngineData->pItem->GetType() == QueueItemType_File)
		pEngineData->state = t_EngineData::transfer;
	else
		pEngineData->state = t_EngineData::mkdir;

	pEngineData->pEngine = pEngine;

	SendNextCommand(*pEngineData);
}

enum ActionAfterState CQueueView::GetActionAfterState() const
{
	return m_actionAfterState;
}

bool CQueueView::IsActionAfter(enum ActionAfterState state)
{
	return m_actionAfterState == state;
}

void CQueueView::ActionAfter(bool warned /*=false*/)
{
	// Need to check all contexts whether there's a recursive
	// download operation still in progress
	const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
	for (unsigned int i = 0; i < pStates->size(); i++)
	{
		CState *pState = (*pStates)[i];
		CRecursiveOperation *pRecursiveOperationHandler;
		if (!pState || !(pRecursiveOperationHandler = pState->GetRecursiveOperationHandler()))
			continue;

		if (pRecursiveOperationHandler->GetOperationMode() == CRecursiveOperation::recursive_download ||
			pRecursiveOperationHandler->GetOperationMode() == CRecursiveOperation::recursive_download_flatten)
		{
			return;
		}
	}

#if WITH_LIBDBUS
	if (!m_pMainFrame->IsActive())
	{
		if (!m_desktop_notification)
			m_desktop_notification = new CDesktopNotification;
		int failed_count = m_pQueue->GetQueueView_Failed()->GetFileCount();
		if (failed_count != 0)
		{
			wxString fmt = wxPLURAL("All transfers have finished. %d file could not be transferred.", "All transfers have finished. %d files could not be transferred.", failed_count);
			m_desktop_notification->Notify(_("Transfers finished"), wxString::Format(fmt, failed_count), _T("transfer.error"));
		}
		else
			m_desktop_notification->Notify(_("Transfers finished"), _("All files have been successfully transferred"), _T("transfer.complete"));
	}
#endif

	switch (m_actionAfterState)
	{
		case ActionAfterState_Close:
		{
			m_pMainFrame->Close();
			break;
		}
		case ActionAfterState_Disconnect:
		{
			const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
			for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
			{
				CState* pState = *iter;
				if (pState->IsRemoteConnected() && pState->IsRemoteIdle())
					pState->Disconnect();
			}
			break;
		}
		case ActionAfterState_RunCommand:
		{
			wxExecute(m_actionAfterRunCommand);
			break;
		}
		case ActionAfterState_ShowMessage:
		{
			wxMessageDialog* dialog = new wxMessageDialog(m_pMainFrame, _("No more files in the queue!"), _T("Queue completion"), wxOK | wxICON_INFORMATION);
			m_pMainFrame->RequestUserAttention(wxUSER_ATTENTION_ERROR);
			dialog->ShowModal();
			dialog->Destroy();
			break;
		}
		case ActionAfterState_PlaySound:
		{
			wxSound sound(wxGetApp().GetResourceDir() + _T("finished.wav"));
			sound.Play(wxSOUND_ASYNC);
			break;
		}
#ifdef __WXMSW__
		case ActionAfterState_Reboot:
		{
			if (!warned)
			{
				ActionAfterWarnUser(false);
				return;
			}
			else
				wxShutdown(wxSHUTDOWN_REBOOT);
			break;
		}

		case ActionAfterState_Shutdown:
		{
			if (!warned)
			{
				ActionAfterWarnUser(true);
				return;
			}
			else
				wxShutdown(wxSHUTDOWN_POWEROFF);
			break;
		}
#endif
		default:
			break;

	}
	m_actionAfterState = ActionAfterState_Disabled; // Resetting the state.
}

#ifdef __WXMSW__
void CQueueView::ActionAfterWarnUser(bool shutdown)
{
	if (m_actionAfterWarnDialog != NULL)
		return;

	wxString message;
	if (shutdown)
		message = _("The system will soon shut down unless you click Cancel.");
	else
		message = _("The system will soon reboot unless you click Cancel.");

	m_actionAfterWarnDialog = new wxProgressDialog(_("Queue has been fully processed"), message, 150, m_pMainFrame, wxPD_CAN_ABORT | wxPD_AUTO_HIDE | wxPD_CAN_SKIP | wxPD_APP_MODAL);

	// Magic id from wxWidgets' src/generic/propdlgg.cpp
	wxWindow* pSkip = m_actionAfterWarnDialog->FindWindow(32000);
	if (pSkip)
	{
		if (!shutdown)
			pSkip->SetLabel(_("Reboot now"));
		else
			pSkip->SetLabel(_("Shutdown now"));
	}

	CWrapEngine engine;
	engine.WrapRecursive(m_actionAfterWarnDialog, 2);
	m_actionAfterWarnDialog->CentreOnParent();
	m_actionAfterWarnDialog->SetFocus();
	m_pMainFrame->RequestUserAttention(wxUSER_ATTENTION_ERROR);

	wxASSERT(!m_actionAfterTimer);
	m_actionAfterTimer = new wxTimer(this, m_actionAfterTimerId);
	m_actionAfterTimerId = m_actionAfterTimer->GetId();
	m_actionAfterTimerCount = 0;
	m_actionAfterTimer->Start(100, wxTIMER_CONTINUOUS);
}

void CQueueView::OnActionAfterTimerTick()
{
	if (!m_actionAfterWarnDialog)
	{
		delete m_actionAfterTimer;
		m_actionAfterTimer = 0;
		return;
	}

	bool skipped = false;
	if (m_actionAfterTimerCount > 150)
	{
		m_actionAfterWarnDialog->Destroy();
		m_actionAfterWarnDialog = 0;
		delete m_actionAfterTimer;
		m_actionAfterTimer = 0;
		ActionAfter(true);
	}
	else if (!m_actionAfterWarnDialog->Update(m_actionAfterTimerCount++, _T(""), &skipped))
	{
		// User has pressed cancel!
		m_actionAfterState = ActionAfterState_Disabled; // resetting to disabled
		m_actionAfterWarnDialog->Destroy();
		m_actionAfterWarnDialog = 0;
		delete m_actionAfterTimer;
		m_actionAfterTimer = 0;
	}
	else if (skipped)
	{
		m_actionAfterWarnDialog->Destroy();
		m_actionAfterWarnDialog = 0;
		delete m_actionAfterTimer;
		m_actionAfterTimer = 0;
		ActionAfter(true);
	}
}
#endif //__WXMSW__

bool CQueueView::SwitchEngine(t_EngineData** ppEngineData)
{
	if (m_engineData.size() < 2)
		return false;

	t_EngineData* pEngineData = *ppEngineData;

	std::vector<t_EngineData*>::iterator iter = m_engineData.begin();
	iter++;
	for (; iter != m_engineData.end(); iter++)
	{
		t_EngineData* pNewEngineData = *iter;
		if (pNewEngineData == pEngineData)
			continue;

		if (pNewEngineData->active)
			continue;

		if (pNewEngineData->lastServer != pEngineData->lastServer)
			continue;

		if (!pNewEngineData->pEngine->IsConnected())
			continue;

		wxASSERT(!pNewEngineData->pItem);
		pNewEngineData->pItem = pEngineData->pItem;
		pNewEngineData->pItem->m_pEngineData = pNewEngineData;
		pEngineData->pItem = 0;

		pNewEngineData->active = true;
		pEngineData->active = false;

		delete pNewEngineData->m_idleDisconnectTimer;
		pNewEngineData->m_idleDisconnectTimer = 0;

		CStatusLineCtrl* pOldStatusLineCtrl = pNewEngineData->pStatusLineCtrl;
		pNewEngineData->pStatusLineCtrl = pEngineData->pStatusLineCtrl;
		pNewEngineData->pStatusLineCtrl->SetEngineData(pNewEngineData);
		if (pOldStatusLineCtrl)
		{
			pEngineData->pStatusLineCtrl = pOldStatusLineCtrl;
			pEngineData->pStatusLineCtrl->SetEngineData(pEngineData);
		}

		if (pNewEngineData->pItem->GetType() == QueueItemType_File)
			pNewEngineData->state = t_EngineData::transfer;
		else
			pNewEngineData->state = t_EngineData::mkdir;
		if (pNewEngineData->pStatusLineCtrl)
			pNewEngineData->pStatusLineCtrl->SetTransferStatus(0);

		pEngineData->state = t_EngineData::none;

		*ppEngineData = pNewEngineData;
		return true;
	}

	return false;
}

bool CQueueView::IsOtherEngineConnected(t_EngineData* pEngineData)
{
	for (std::vector<t_EngineData*>::iterator iter = m_engineData.begin(); iter != m_engineData.end(); iter++)
	{
		t_EngineData* current = *iter;

		if (current == pEngineData)
			continue;

		if (!current->pEngine)
			continue;

		if (current->lastServer != pEngineData->lastServer)
			continue;

		if (current->pEngine->IsConnected())
			return true;
	}

	return false;
}

void CQueueView::OnChar(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DELETE)
	{
		wxCommandEvent cmdEvt;
		OnRemoveSelected(cmdEvt);
	}
	else
		event.Skip();
}

int CQueueView::GetLineHeight()
{
	if (m_line_height != -1)
		return m_line_height;

	if (!GetItemCount())
		return 20;

	wxRect rect;
	if (!GetItemRect(0, rect))
		return 20;

	m_line_height = rect.GetHeight();

#ifdef __WXMSW__
	m_header_height = rect.y + GetScrollPos(wxVERTICAL) * m_line_height;
#endif

	return m_line_height;
}

void CQueueView::OnSize(wxSizeEvent& event)
{
	if (!m_resize_timer.IsRunning())
		m_resize_timer.Start(250, true);

	event.Skip();
}

void CQueueView::RenameFileInTransfer(CFileZillaEngine *pEngine, const wxString& newName, bool local)
{
	std::vector<t_EngineData*>::iterator iter;
	for (iter = m_engineData.begin(); iter != m_engineData.end(); iter++)
	{
		if ((*iter)->pEngine == pEngine)
			break;
	}
	if (iter == m_engineData.end())
		return;

	t_EngineData* const pEngineData = *iter;
	if (!pEngineData->pItem)
		return;
	
	if (pEngineData->pItem->GetType() != QueueItemType_File)
		return;

	CFileItem* pFile = (CFileItem*)pEngineData->pItem;
	if (local)
	{
		wxFileName fn(pFile->GetLocalPath().GetPath(), pFile->GetLocalFile());
		fn.SetFullName(newName);
		pFile->SetTargetFile(fn.GetFullName());
	}
	else
		pFile->SetTargetFile(newName);

	RefreshItem(pFile);
}

wxString CQueueView::ReplaceInvalidCharacters(const wxString& filename)
{
	if (!COptions::Get()->GetOptionVal(OPTION_INVALID_CHAR_REPLACE_ENABLE))
		return filename;

	const wxChar replace = COptions::Get()->GetOption(OPTION_INVALID_CHAR_REPLACE)[0];

	wxString result;

	wxChar* start = result.GetWriteBuf(filename.Len() + 1);
	wxChar* buf = start;

	const wxChar* p = filename.c_str();
	while (*p)
	{
		const wxChar c = *p;
		switch (c)
		{
		case '/':
#ifdef __WXMSW__
		case '\\':
		case ':':
		case '*':
		case '?':
		case '"':
		case '<':
		case '>':
		case '|':
#endif
			if (replace)
				*buf++ = replace;
			break;
		default:
#ifdef __WXMSW__
			if (c < 0x20)
				*buf++ = replace;
			else
#endif
			{
				*buf++ = c;
			}
		}
		p++;
	}
	*buf = 0;

	result.UngetWriteBuf( buf - start );

	return result;
}

void CQueueView::ReleaseExclusiveEngineLock(CFileZillaEngine* pEngine)
{
	wxASSERT(pEngine);
	if (!pEngine)
		return;

	const std::vector<CState*> *pStates = CContextManager::Get()->GetAllStates();
	for (std::vector<CState*>::const_iterator iter = pStates->begin(); iter != pStates->end(); iter++)
	{
		CState* pState = *iter;
		if (pState->m_pEngine != pEngine)
			continue;
		CCommandQueue *pCommandQueue = pState->m_pCommandQueue;
		if (pCommandQueue)
			pCommandQueue->ReleaseEngine();

		break;
	}
}

#ifdef __WXMSW__

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED        0x031E
#endif // WM_DWMCOMPOSITIONCHANGED

WXLRESULT CQueueView::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
	if (nMsg == WM_DWMCOMPOSITIONCHANGED || nMsg == WM_THEMECHANGED)
	{
		m_line_height = -1;
		if (!m_resize_timer.IsRunning())
			m_resize_timer.Start(250, true);
	}
	else if (nMsg == WM_LBUTTONDOWN)
	{
		// If clicking a partially selected item, Windows starts an internal timer with the double-click interval (as seen in the 
		// disassembly). After the timer expires, the given item is selected. But there's a huge bug in Windows: We don't get
		// notified about this change in scroll position in any way (verified using Spy++), so on left button down, start our
		// own timer with a slightly higher interval.
		if (!m_resize_timer.IsRunning())
			m_resize_timer.Start(GetDoubleClickTime() + 5, true);
	}
	return CQueueViewBase::MSWWindowProc(nMsg, wParam, lParam);
}

#endif
