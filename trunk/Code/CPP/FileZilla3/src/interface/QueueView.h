#ifndef __QUEUEVIEW_H__
#define __QUEUEVIEW_H__

#include "dndobjects.h"
#include "queue.h"

#include <libfilezilla.h>

#include <set>
#include <wx/progdlg.h>

#include "queue_storage.h"

class CFolderProcessingEntry
{
public:
	enum t_type
	{
		dir,
		file
	};
	const enum t_type m_type;

	CFolderProcessingEntry(enum t_type type) : m_type(type) {}
};

class t_newEntry : public CFolderProcessingEntry
{
public:
	t_newEntry() 
		: CFolderProcessingEntry(CFolderProcessingEntry::file)
		, attributes()
		, dir()
	{}

	wxString name;
	wxLongLong size;
	wxDateTime time;
	int attributes;
	bool dir;
};

enum ActionAfterState
{
	ActionAfterState_Disabled,
	ActionAfterState_Close,
	ActionAfterState_Disconnect,
	ActionAfterState_RunCommand,
	ActionAfterState_ShowMessage,
	ActionAfterState_PlaySound
// On Windows, wx can reboot or shutdown the system as well.
#ifdef __WXMSW__
	,
	ActionAfterState_Reboot,
	ActionAfterState_Shutdown
#endif
};

class CStatusLineCtrl;
class CFileItem;
struct t_EngineData
{
	CFileZillaEngine* pEngine;
	bool active;

	enum EngineDataState
	{
		none,
		cancel,
		disconnect,
		connect,
		transfer,
		list,
		mkdir,
		askpassword,
		waitprimary
	} state;

	CFileItem* pItem;
	CServer lastServer;
	CStatusLineCtrl* pStatusLineCtrl;
	wxTimer* m_idleDisconnectTimer;
};

class CMainFrame;
class CStatusLineCtrl;
class CFolderProcessingThread;
class CAsyncRequestQueue;
class CQueue;
#if WITH_LIBDBUS
class CDesktopNotification;
#endif

class CQueueView : public CQueueViewBase
{
	friend class CFolderProcessingThread;
	friend class CQueueViewDropTarget;
	friend class CQueueViewFailed;
public:
	CQueueView(CQueue* parent, int index, CMainFrame* pMainFrame, CAsyncRequestQueue* pAsyncRequestQueue);
	virtual ~CQueueView();

	bool QueueFile(const bool queueOnly, const bool download,
		const wxString& localFile, const wxString& remoteFile,
		const CLocalPath& localPath, const CServerPath& remotePath,
		const CServer& server, const wxLongLong size, enum CEditHandler::fileType edit = CEditHandler::none);

	void QueueFile_Finish(const bool start); // Need to be called after QueueFile
	bool QueueFiles(const bool queueOnly, const CLocalPath& localPath, const CRemoteDataObject& dataObject);
	int QueueFiles(const std::list<CFolderProcessingEntry*> &entryList, bool queueOnly, bool download, CServerItem* pServerItem, const enum CFileExistsNotification::OverwriteAction defaultFileExistsAction);
	bool QueueFolder(bool queueOnly, bool download, const CLocalPath& localPath, const CServerPath& remotePath, const CServer& server);

	bool IsEmpty() const;
	int IsActive() const { return m_activeMode; }
	bool SetActive(bool active = true);
	bool Quit();

	// If the settings are changed, this function will recalculate some
	// data like the list of ascii file types
	void SettingsChanged();

	// This sets the default file exists action for all files currently in queue.
	// This includes queued folders which are yet to be processed
	void SetDefaultFileExistsAction(enum CFileExistsNotification::OverwriteAction action, const enum TransferDirection direction);

	void UpdateItemSize(CFileItem* pItem, wxLongLong size);

	void RemoveAll();

	void LoadQueue();
	void LoadQueueFromXML();
	void ImportQueue(TiXmlElement* pElement, bool updateSelections);

	virtual void InsertItem(CServerItem* pServerItem, CQueueItem* pItem);

	virtual void CommitChanges();

	void WriteToFile(TiXmlElement* pElement) const;

	void ProcessNotification(CNotification* pNotification);

	void RenameFileInTransfer(CFileZillaEngine *pEngine, const wxString& newName, bool local);

	static wxString ReplaceInvalidCharacters(const wxString& filename);

protected:

#ifdef __WXMSW__
	WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

	void AdvanceQueue(bool refresh = true);
	bool TryStartNextTransfer();

	// Called from TryStartNextTransfer(), checks
	// whether it is allowed to start another transfer on that server item
	bool CanStartTransfer(const CServerItem& server_item, struct t_EngineData *&pEngineData);

	bool ProcessFolderItems(int type = -1);
	void ProcessUploadFolderItems();

	void ProcessReply(t_EngineData* pEngineData, COperationNotification* pNotification);
	void SendNextCommand(t_EngineData& engineData);

	enum ResetReason
	{
		success,
		failure,
		reset,
		retry,
		remove
	};

	enum ActionAfterState GetActionAfterState() const;

	void ResetEngine(t_EngineData& data, const enum ResetReason reason);
	void DeleteEngines();

	virtual bool RemoveItem(CQueueItem* item, bool destroy, bool updateItemCount = true, bool updateSelections = true);

	// Stops processing of given item
	// Returns true on success, false if it would block
	bool StopItem(CFileItem* item);
	bool StopItem(CServerItem* pServerItem);

	void CheckQueueState();
	bool IncreaseErrorCount(t_EngineData& engineData);
	void UpdateStatusLinePositions();
	void CalculateQueueSize();
	void DisplayQueueSize();
	void SaveQueue();

	bool IsActionAfter(enum ActionAfterState);
	void ActionAfter(bool warned = false);
#ifdef __WXMSW__
	void ActionAfterWarnUser(bool shutdown);
#endif

	void ProcessNotification(t_EngineData* pEngineData, CNotification* pNotification);

	// Tries to refresh the current remote directory listing
	// if there's an idle engine connected to the current server of
	// the primary connection.
	void TryRefreshListings();
	CServer m_last_refresh_server;
	CServerPath m_last_refresh_path;
	CTimeEx m_last_refresh_listing_time;

	// Called from Process Reply.
	// After a disconnect, check if there's another idle engine that
	// is already connected.
	bool SwitchEngine(t_EngineData** ppEngineData);

	bool IsOtherEngineConnected(t_EngineData* pEngineData);

	t_EngineData* GetIdleEngine(const CServer* pServer = 0);

	std::vector<t_EngineData*> m_engineData;
	std::list<CStatusLineCtrl*> m_statusLineList;

	/*
	 * List of queued folders used to populate the queue.
	 * Index 0 for downloads, index 1 for uploads.
	 * For each type, only the first one can be active at any given time.
	 */
	std::list<CFolderScanItem*> m_queuedFolders[2];

	void RemoveQueuedFolderItem(CFolderScanItem* pFolder);

	CFolderProcessingThread *m_pFolderProcessingThread;

	/*
	 * Don't update status line positions if m_waitStatusLineUpdate is true.
	 * This assures we are updating the status line positions only once,
	 * and not multiple times (for example inside a loop).
	 */
	bool m_waitStatusLineUpdate;

	// Remember last top item in UpdateStatusLinePositions()
	int m_lastTopItem;

	int m_activeCount;
	int m_activeCountDown;
	int m_activeCountUp;
	int m_activeMode; // 0 inactive, 1 only immediate transfers, 2 all
	int m_quit;

	enum ActionAfterState m_actionAfterState;
	wxString m_actionAfterRunCommand;
#ifdef __WXMSW__
	wxTimer* m_actionAfterTimer;
	wxProgressDialog* m_actionAfterWarnDialog;
	int m_actionAfterTimerCount;
	int m_actionAfterTimerId;
#endif

	wxLongLong m_totalQueueSize;
	int m_filesWithUnknownSize;

	CMainFrame* m_pMainFrame;
	CAsyncRequestQueue* m_pAsyncRequestQueue;

	std::list<CFileZillaEngine*> m_waitingForPassword;

	virtual void OnPostScroll();

	wxTimer m_folderscan_item_refresh_timer;

	int GetLineHeight();
	int m_line_height;
#ifdef __WXMSW__
	int m_header_height;
#endif

	wxTimer m_resize_timer;

	void ReleaseExclusiveEngineLock(CFileZillaEngine* pEngine);

#if WITH_LIBDBUS
	CDesktopNotification* m_desktop_notification;
#endif

	CQueueStorage m_queue_storage;

	DECLARE_EVENT_TABLE();
	void OnEngineEvent(wxEvent &event);
	void OnFolderThreadComplete(wxCommandEvent& event);
	void OnFolderThreadFiles(wxCommandEvent& event);
	void OnChar(wxKeyEvent& event);

	// Context menu handlers
	void OnContextMenu(wxContextMenuEvent& event);
	void OnProcessQueue(wxCommandEvent& event);
	void OnStopAndClear(wxCommandEvent& event);
	void OnRemoveSelected(wxCommandEvent& event);
	void OnSetDefaultFileExistsAction(wxCommandEvent& event);

	void OnAskPassword(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& evnet);

	void OnSetPriority(wxCommandEvent& event);

	void OnExclusiveEngineRequestGranted(wxCommandEvent& event);

	void OnActionAfter(wxCommandEvent& event);
	void OnActionAfterTimerTick();

	void OnSize(wxSizeEvent& event);
};

#endif
