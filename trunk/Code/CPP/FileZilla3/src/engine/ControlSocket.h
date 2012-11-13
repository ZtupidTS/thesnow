#ifndef __CONTROLSOCKET_H__
#define __CONTROLSOCKET_H__

#include "socket.h"
#include "logging_private.h"
#include "backend.h"

class COpData
{
public:
	COpData(enum Command op_Id);
	virtual ~COpData();

	int opState;
	const enum Command opId;

	bool waitForAsyncRequest;
	bool holdsLock;

	COpData *pNextOpData;
};

class CConnectOpData : public COpData
{
public:
	CConnectOpData()
		: COpData(cmd_connect),
		port(0)
	{
	}

	wxString host;
	unsigned int port;
};

class CFileTransferOpData : public COpData
{
public:
	CFileTransferOpData(bool is_download, const wxString& local_file, const wxString& remote_file, const CServerPath& remote_path);
	virtual ~CFileTransferOpData();
	// Transfer data
	wxString localFile, remoteFile;
	CServerPath remotePath;
	const bool download;

	wxDateTime fileTime;
	wxFileOffset localFileSize;
	wxFileOffset remoteFileSize;

	bool tryAbsolutePath;
	bool resume;

	CFileTransferCommand::t_transferSettings transferSettings;

	// Set to true when sending the command which
	// starts the actual transfer
	bool transferInitiated;
};

class CMkdirOpData : public COpData
{
public:
	CMkdirOpData()
		: COpData(cmd_mkdir)
	{
	}

	virtual ~CMkdirOpData()
	{
	}

	CServerPath path;
	CServerPath currentPath;
	CServerPath commonParent;
	std::list<wxString> segments;
};

class CChangeDirOpData : public COpData
{
public:
	CChangeDirOpData()
		: COpData(cmd_cwd)
	{
		tryMkdOnFail = false;
		link_discovery = false;
	}

	virtual ~CChangeDirOpData()
	{
	}

	CServerPath path;
	wxString subDir;
	bool tryMkdOnFail;
	CServerPath target;

	bool link_discovery;
};

enum TransferEndReason
{
	none,
	successful,
	timeout,
	transfer_failure,					// Error during transfer, like lost connection. Retry automatcally
	transfer_failure_critical,			// Error during transfer like lack of diskspace. Needs user interaction
	pre_transfer_command_failure,		// If a command fails prior to sending the transfer command
	transfer_command_failure_immediate,	// Used if server does not send the 150 reply after the transfer command
	transfer_command_failure,			// Used if the transfer command fails, but after receiving a 150 first
	failure,							// Other unspecific failure
	failed_resumetest
};

class CTransferStatus;
class CControlSocket: public wxEvtHandler, public CLogging
{
public:
	CControlSocket(CFileZillaEnginePrivate *pEngine);
	virtual ~CControlSocket();

	virtual int Connect(const CServer &server) = 0;
	virtual int Disconnect();
	virtual void Cancel();
	virtual int List(CServerPath path = CServerPath(), wxString subDir = _T(""), int flags = 0) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int FileTransfer(const wxString localFile, const CServerPath &remotePath,
							 const wxString &remoteFile, bool download,
							 const CFileTransferCommand::t_transferSettings& transferSettings) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int RawCommand(const wxString& command = _T("")) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int Delete(const CServerPath& path, const std::list<wxString>& files) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int RemoveDir(const CServerPath& path = CServerPath(), const wxString& subDir = _T("")) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int Mkdir(const CServerPath& path) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int Rename(const CRenameCommand& command) { return FZ_REPLY_NOTSUPPORTED; }
	virtual int Chmod(const CChmodCommand& command) { return FZ_REPLY_NOTSUPPORTED; }
	virtual bool Connected() = 0;

	// If m_pCurrentOpData is zero, this function returns the current command
	// from the engine.
	enum Command GetCurrentCommandId() const;

	virtual void TransferEnd() {}

	void SendAsyncRequest(CAsyncRequestNotification* pNotification);
	virtual bool SetAsyncRequestReply(CAsyncRequestNotification *pNotification) = 0;
	bool SetFileExistsAction(CFileExistsNotification *pFileExistsNotification);

	void InitTransferStatus(wxFileOffset totalSize, wxFileOffset startOffset, bool list);
	void SetTransferStatusStartTime();
	void UpdateTransferStatus(wxFileOffset transferredBytes);
	void ResetTransferStatus();
	bool GetTransferStatus(CTransferStatus &status, bool &changed);
	void SetTransferStatusMadeProgress();

	const CServer* GetCurrentServer() const;

	// Conversion function which convert between local and server charset.
	wxString ConvToLocal(const char* buffer);
	wxChar* ConvToLocalBuffer(const char* buffer);
	wxChar* ConvToLocalBuffer(const char* buffer, wxMBConv& conv);
	wxCharBuffer ConvToServer(const wxString& str, bool force_utf8 = false);

	// ---
	// The following two functions control the timeout behaviour:
	// ---

	// Call this if data could be sent or retrieved
	void SetAlive();

	// Set to true if waiting for data
	void SetWait(bool waiting);

	CFileZillaEnginePrivate* GetEngine() { return m_pEngine; }

	// Only called from the engine, see there for description
	void InvalidateCurrentWorkingDir(const CServerPath& path);

protected:
	wxTimeSpan GetTimezoneOffset();

	virtual int DoClose(int nErrorCode = FZ_REPLY_DISCONNECTED);
	bool m_closed;

	virtual int ResetOperation(int nErrorCode);

	virtual int SendNextCommand();

	void LogTransferResultMessage(int nErrorCode, CFileTransferOpData *pData);

	// Called by ResetOperation if there's a queued operation
	virtual int ParseSubcommandResult(int prevResult);

	wxString ConvertDomainName(wxString domain);

	int CheckOverwriteFile();

	void CreateLocalDir(const wxString &local_file);

	bool ParsePwdReply(wxString reply, bool unquoted = false, const CServerPath& defaultPath = CServerPath());

	COpData *m_pCurOpData;
	int m_nOpState;
	CFileZillaEnginePrivate *m_pEngine;
	CServer *m_pCurrentServer;

	CServerPath m_CurrentPath;

	CTransferStatus *m_pTransferStatus;
	int m_transferStatusSendState;

	wxCSConv *m_pCSConv;
	bool m_useUTF8;

	// Timeout data
	wxTimer m_timer;
	wxStopWatch m_stopWatch;

	// -------------------------
	// Begin cache locking stuff
	// -------------------------

	enum locking_reason
	{
		lock_unknown = -1,
		lock_list,
		lock_mkdir
	};

	// Tries to obtain lock. Returns true on success.
	// On failure, caller has to pass control.
	// SendNextCommand will be called once the lock gets available
	// and engine could obtain it.
	// Lock is recursive. Lock counter increases on suboperations.
	bool TryLockCache(enum locking_reason reason, const CServerPath& directory);
	bool IsLocked(enum locking_reason reason, const CServerPath& directory);

	// Unlocks the cache. Can be called if not holding the lock
	// Doesn't need reason as one engine can at most hold one lock
	void UnlockCache();

	// Called from the fzOBTAINLOCK event.
	// Returns reason != unknown iff engine is the first waiting engine
	// and obtains the lock.
	// On failure, the engine was not waiting for a lock.
	enum locking_reason ObtainLockFromEvent();

	bool IsWaitingForLock();

#ifdef __VISUALC__
	// Retarded compiler does not like my code
	public:
#endif
	struct t_lockInfo
	{
		CControlSocket* pControlSocket;
		CServerPath directory;
		enum locking_reason reason;
		bool waiting;
		int lockcount;
	};
	static std::list<t_lockInfo> m_lockInfoList;
#ifdef __VISUALC__
	protected:
#endif

	const std::list<t_lockInfo>::iterator GetLockStatus();

	// -----------------------
	// End cache locking stuff
	// -----------------------

	bool m_invalidateCurrentPath;

	DECLARE_EVENT_TABLE();
	void OnTimer(wxTimerEvent& event);
	void OnObtainLock(wxCommandEvent& event);
};

class CProxySocket;
class CRealControlSocket : public CControlSocket, public CSocketEventHandler
{
public:
	CRealControlSocket(CFileZillaEnginePrivate *pEngine);
	virtual ~CRealControlSocket();

	virtual int Connect(const CServer &server);
	virtual int ContinueConnect();

	virtual bool Connected() { return m_pSocket->GetState() == CSocket::connected; }

protected:
	virtual int DoClose(int nErrorCode = FZ_REPLY_DISCONNECTED);
	void ResetSocket();

	virtual void OnSocketEvent(CSocketEvent &event);
	virtual void OnConnect();
	virtual void OnReceive();
	virtual void OnSend();
	virtual void OnClose(int error);

	virtual bool Send(const char *buffer, int len);

	CSocket* m_pSocket;

	CBackend* m_pBackend;
	CProxySocket* m_pProxyBackend;

	char *m_pSendBuffer;
	int m_nSendBufferLen;
};

#endif
