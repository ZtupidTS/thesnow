#ifndef __FILEZILLAENGINEPRIVATE_H__
#define __FILEZILLAENGINEPRIVATE_H__

#include "timeex.h"

enum EngineNotificationType
{
	engineCancel,
	engineTransferEnd
};

class wxFzEngineEvent;
class CControlSocket;
class CLogging;
class CRateLimiter;
class CFileZillaEnginePrivate : public wxEvtHandler
{
public:
	int ResetOperation(int nErrorCode);	
	void SetActive(int direction);

	// Add new pending notification
	void AddNotification(CNotification *pNotification);

	unsigned int GetNextAsyncRequestNumber();

	// Event handling
	bool SendEvent(enum EngineNotificationType eventType, int data = 0);

	bool IsBusy() const;
	bool IsConnected() const;

	const CCommand *GetCurrentCommand() const;
	enum Command GetCurrentCommandId() const;

	COptionsBase *GetOptions() { return m_pOptions; }

	void SendDirectoryListingNotification(const CServerPath& path, bool onList, bool modified, bool failed);

	// If deleting or renaming a directory, it could be possible that another
	// engine's CControlSocket instance still has that directory as
	// current working directory (m_CurrentPath)
	// Since this would cause problems, this function interate over all engines
	// connected ot the same server and invalidates the current working 
	// directories if they match or if it is a subdirectory of the changed
	// directory.
	void InvalidateCurrentWorkingDirs(const CServerPath& path);

	int GetEngineId() const {return m_engine_id; }

protected:
	CFileZillaEnginePrivate();
	virtual ~CFileZillaEnginePrivate();

	// Command handlers, only called by CFileZillaEngine::Command
	int Connect(const CConnectCommand &command);
	int Disconnect(const CDisconnectCommand &command);
	int Cancel(const CCancelCommand &command);
	int List(const CListCommand &command);
	int FileTransfer(const CFileTransferCommand &command);
	int RawCommand(const CRawCommand& command);
	int Delete(const CDeleteCommand& command);
	int RemoveDir(const CRemoveDirCommand& command);
	int Mkdir(const CMkdirCommand& command);
	int Rename(const CRenameCommand& command);
	int Chmod(const CChmodCommand& command);

	int ContinueConnect();

	DECLARE_EVENT_TABLE()
	void OnEngineEvent(wxFzEngineEvent &event);
	void OnTimer(wxTimerEvent& event);

	wxEvtHandler *m_pEventHandler;

	int m_engine_id;
	static std::list<CFileZillaEnginePrivate*> m_engineList;

	// Indicicates if data has been received/sent and whether to send any notifications
	static int m_activeStatus[2];
	
	// Remember last path used in a dirlisting.
	CServerPath m_lastListDir;
	CTimeEx m_lastListTime;

	CControlSocket *m_pControlSocket;

	CCommand *m_pCurrentCommand;

	std::list<CNotification*> m_NotificationList;
	bool m_maySendNotificationEvent;

	bool m_bIsInCommand; //true if Command is on the callstack
	int m_nControlSocketError;

	COptionsBase *m_pOptions;

	unsigned int m_asyncRequestCounter;

	// Used to synchronize access to the notification list
	wxCriticalSection m_lock;

	CLogging* m_pLogging;

	// Everything related to the retry code
	// ------------------------------------

	void RegisterFailedLoginAttempt(const CServer& server, bool critical);

	// Get the amount of time to wait till next reconnection attempt in milliseconds
	unsigned int GetRemainingReconnectDelay(const CServer& server);

	struct t_failedLogins
	{
		CServer server;
		wxDateTime time;
		bool critical;
	};
	static std::list<t_failedLogins> m_failedLogins;
	int m_retryCount;
	wxTimer m_retryTimer;

	CRateLimiter* m_pRateLimiter;
};

#endif //__FILEZILLAENGINEPRIVATE_H__
