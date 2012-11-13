#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "aui_notebook_ex.h"
#include "listctrlex.h"
#include "edithandler.h"

#define PRIORITY_COUNT 5
enum QueuePriority
{
	priority_lowest = 0,
	priority_low = 1,
	priority_normal = 2,
	priority_high = 3,
	priority_highest = 4
};

enum QueueItemType
{
	QueueItemType_Server,
	QueueItemType_File,
	QueueItemType_Folder,
	QueueItemType_FolderScan,
	QueueItemType_Status
};

enum TransferDirection
{
	both,
	download,
	upload
};

class TiXmlElement;
class CQueueItem
{
public:
	virtual ~CQueueItem();

	virtual void SetPriority(enum QueuePriority priority);

	virtual void AddChild(CQueueItem* pItem);
	unsigned int GetChildrenCount(bool recursive);
	CQueueItem* GetChild(unsigned int item, bool recursive = true);
	CQueueItem* GetParent() { return m_parent; }
	const CQueueItem* GetParent() const { return m_parent; }
	void SetParent(CQueueItem* parent) { m_parent = parent; }
	virtual bool RemoveChild(CQueueItem* pItem, bool destroy = true); // Removes a child item with is somewhere in the tree of children.
	virtual bool TryRemoveAll(); // Removes a inactive childrens, queues active children for removal.
								 // Returns true if item can be removed itself
	CQueueItem* GetTopLevelItem();
	const CQueueItem* GetTopLevelItem() const;
	int GetItemIndex() const; // Return the visible item index relative to the topmost parent item.
	virtual void SaveItem(TiXmlElement* pElement) const {}

	virtual enum QueueItemType GetType() const = 0;

	wxDateTime GetTime() const { return m_time; }
	void UpdateTime() { m_time = wxDateTime::Now(); }

	const wxString& GetIndent() const;

	const std::vector<CQueueItem*>& GetChildren() const { return m_children; }
	int GetRemovedAtFront() const { return m_removed_at_front; }

protected:
	CQueueItem(CQueueItem* parent = 0);

	CQueueItem* m_parent;

	int m_visibleOffspring; // Visible offspring over all sublevels
	int m_indent;

	int m_maxCachedIndex;

	struct t_cacheItem
	{
		int index;
		int child;
	};
	std::vector<t_cacheItem> m_lookupCache;

	friend class CServerItem;

	wxDateTime m_time;

private:
	std::vector<CQueueItem*> m_children;

	// Number of items removed at front of list
	// Increased instead of calling slow m_children.erase(0),
	// resetted on insert.
	int m_removed_at_front;
};

class CFileItem;
class CServerItem : public CQueueItem
{
public:
	CServerItem(const CServer& server);
	virtual ~CServerItem();
	virtual enum QueueItemType GetType() const { return QueueItemType_Server; }

	const CServer& GetServer() const;
	wxString GetName() const;

	virtual void AddChild(CQueueItem* pItem);

	CFileItem* GetIdleChild(bool immadiateOnly, enum TransferDirection direction);
	virtual bool RemoveChild(CQueueItem* pItem, bool destroy = true); // Removes a child item with is somewhere in the tree of children
	wxLongLong GetTotalSize(int& filesWithUnknownSize, int& queuedFiles, int& folderScanCount) const;

	void QueueImmediateFiles();
	void QueueImmediateFile(CFileItem* pItem);

	virtual void SaveItem(TiXmlElement* pElement) const;

	void SetDefaultFileExistsAction(enum CFileExistsNotification::OverwriteAction action, const enum TransferDirection direction);

	virtual bool TryRemoveAll();

	void DetachChildren();

	virtual void SetPriority(enum QueuePriority priority);

	void SetChildPriority(CFileItem* pItem, enum QueuePriority oldPriority, enum QueuePriority newPriority);

	int m_activeCount;

protected:
	void AddFileItemToList(CFileItem* pItem);
	void RemoveFileItemFromList(CFileItem* pItem);

	CServer m_server;

	// array of item lists, sorted by priority. Used by scheduler to find
	// next file to transfer
	// First index specifies whether the item is queued (0) or immediate (1)
	std::list<CFileItem*> m_fileList[2][PRIORITY_COUNT];
};

struct t_EngineData;

class CFileItem : public CQueueItem
{
public:
	CFileItem(CServerItem* parent, bool queued, bool download,
		const wxString& sourceFile, const wxString& targetFile,
		const CLocalPath& localPath, const CServerPath& remotePath, wxLongLong size);

	virtual ~CFileItem();

	virtual void SetPriority(enum QueuePriority priority);
	void SetPriorityRaw(enum QueuePriority priority);
	enum QueuePriority GetPriority() const;

	const wxString& GetLocalFile() const { return !Download() ? GetSourceFile() : (m_targetFile.empty() ? m_sourceFile : m_targetFile); }
	const wxString& GetRemoteFile() const { return Download() ? GetSourceFile() : (m_targetFile.empty() ? m_sourceFile : m_targetFile); }
	const wxString& GetSourceFile() const { return m_sourceFile; }
	const wxString& GetTargetFile() const { return m_targetFile; }
	const CLocalPath& GetLocalPath() const { return m_localPath; }
	const CServerPath& GetRemotePath() const { return m_remotePath; }
	const wxLongLong& GetSize() const { return m_size; }
	void SetSize(wxLongLong size) { m_size = size; }
	inline bool Download() const { return flags & flag_download; }

	inline bool queued() const { return (flags & flag_queued) != 0; }
	inline void set_queued(bool q)
	{
		if (q)
			flags |= flag_queued;
		else
			flags &= ~flag_queued;
	}

	inline bool pending_remove() const { return (flags & flag_remove) != 0; }
	inline void set_pending_remove(bool remove)
	{
		if (remove)
			flags |= flag_remove;
		else
			flags &= ~flag_remove;
	}

	virtual enum QueueItemType GetType() const { return QueueItemType_File; }

	bool IsActive() const { return (flags & flag_active) != 0; }
	virtual void SetActive(bool active);

	virtual void SaveItem(TiXmlElement* pElement) const;

	virtual bool TryRemoveAll(); // Removes a inactive childrens, queues active children for removal.
								 // Returns true if item can be removed itself

	void SetTargetFile(const wxString &file);

	int m_errorCount;
	CEditHandler::fileType m_edit;

	wxString m_statusMessage;

	CFileTransferCommand::t_transferSettings m_transferSettings;

	t_EngineData* m_pEngineData;

	enum CFileExistsNotification::OverwriteAction m_defaultFileExistsAction;

	inline bool made_progress() const { return (flags & flag_made_progress) != 0; }
	inline void set_made_progress(bool made_progress)
	{
		if (made_progress)
			flags |= flag_made_progress;
		else
			flags &= ~flag_made_progress;
	}

	enum CFileExistsNotification::OverwriteAction m_onetime_action;

protected:
	enum
	{
		flag_download = 0x01,
		flag_active = 0x02,
		flag_made_progress = 0x04,
		flag_queued = 0x08,
		flag_remove = 0x10
	};
	int flags;

	enum QueuePriority m_priority;

	wxString m_sourceFile;
	wxString m_targetFile;
	const CLocalPath m_localPath;
	const CServerPath m_remotePath;
	wxLongLong m_size;
};

class CFolderItem : public CFileItem
{
public:
	CFolderItem(CServerItem* parent, bool queued, const CLocalPath& localPath);
	CFolderItem(CServerItem* parent, bool queued, const CServerPath& remotePath, const wxString& remoteFile);

	virtual enum QueueItemType GetType() const { return QueueItemType_Folder; }

	virtual void SaveItem(TiXmlElement* pElement) const;

	virtual void SetActive(bool active);
};

class CFolderScanItem : public CQueueItem
{
public:
	CFolderScanItem(CServerItem* parent, bool queued, bool download, const CLocalPath& localPath, const CServerPath& remotePath);
	virtual ~CFolderScanItem() {}

	virtual enum QueueItemType GetType() const { return QueueItemType_FolderScan; }
	CLocalPath GetLocalPath() const { return m_localPath; }
	CServerPath GetRemotePath() const { return m_remotePath; }
	bool Download() const { return m_download; }
	int GetCount() const { return m_count; }
	virtual bool TryRemoveAll();

	bool queued() const { return m_queued; }
	void set_queued(bool q) { m_queued = q; }

	wxString m_statusMessage;

	volatile bool m_remove;
	bool m_active;

	int m_count;

	enum CFileExistsNotification::OverwriteAction m_defaultFileExistsAction;

	bool m_dir_is_empty;
	CLocalPath m_current_local_path;
	CServerPath m_current_remote_path;

protected:
	bool m_queued;
	CLocalPath m_localPath;
	CServerPath m_remotePath;
	bool m_download;
};

class CStatusItem : public CQueueItem
{
public:
	CStatusItem() {}
	virtual ~CStatusItem() {}

	virtual enum QueueItemType GetType() const { return QueueItemType_Status; }
};

class CQueue;
class CQueueViewBase : public wxListCtrlEx
{
public:

	enum ColumnId
	{
		colLocalName,
		colDirection,
		colRemoteName,
		colSize,
		colPriority,
		colTime,
		colTransferStatus,
		colErrorReason
	};

	CQueueViewBase(CQueue* parent, int index, const wxString& title);
	virtual ~CQueueViewBase();

	// Gets item for given server or creates new if it doesn't exist
	CServerItem* CreateServerItem(const CServer& server);

	virtual void InsertItem(CServerItem* pServerItem, CQueueItem* pItem);
	virtual bool RemoveItem(CQueueItem* pItem, bool destroy, bool updateItemCount = true, bool updateSelections = true);

	// Has to be called after adding or removing items. Also updates
	// item count and selections.
	virtual void CommitChanges();

	wxString GetTitle() const { return m_title; }

	int GetFileCount() const { return m_fileCount; }

protected:

	void CreateColumns(std::list<ColumnId> const& extraColumns = std::list<ColumnId>());
	void AddQueueColumn(ColumnId id);

	// Gets item for given server
	CServerItem* GetServerItem(const CServer& server);

	// Gets item with given index
	CQueueItem* GetQueueItem(unsigned int item);

	// Get index for given queue item
	int GetItemIndex(const CQueueItem* item);

	virtual wxString OnGetItemText(long item, long column) const;
	virtual wxString OnGetItemText(CQueueItem* pItem, ColumnId column) const;
	virtual int OnGetItemImage(long item) const;

	void RefreshItem(const CQueueItem* pItem);

	void DisplayNumberQueuedFiles();

	// Position at which insertions start and number of insertions
	int m_insertionStart;
	unsigned int m_insertionCount;

	int m_fileCount;
	int m_folderScanCount;
	bool m_fileCountChanged;
	bool m_folderScanCountChanged;

	// Selection management.
	void UpdateSelections_ItemAdded(int added);
	void UpdateSelections_ItemRangeAdded(int added, int count);
	void UpdateSelections_ItemRemoved(int removed);
	void UpdateSelections_ItemRangeRemoved(int removed, int count);

	int m_itemCount;
	bool m_allowBackgroundErase;

	std::vector<CServerItem*> m_serverList;

	CQueue* m_pQueue;

	const int m_pageIndex;

	const wxString m_title;

	wxTimer m_filecount_delay_timer;

	std::vector<ColumnId> m_columns;

	DECLARE_EVENT_TABLE();
	void OnEraseBackground(wxEraseEvent& event);
	void OnNavigationKey(wxNavigationKeyEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnEndColumnDrag(wxListEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnKeyDown(wxKeyEvent& event);
};

class CQueueView;
class CQueueViewFailed;
class CQueueViewSuccessful;

class CMainFrame;
class CAsyncRequestQueue;
class CQueue : public wxAuiNotebookEx
{
public:
	CQueue(wxWindow* parent, CMainFrame* pMainFrame, CAsyncRequestQueue* pAsyncRequestQueue);
	virtual ~CQueue() {}

	inline CQueueView* GetQueueView() { return m_pQueueView; }
	inline CQueueViewFailed* GetQueueView_Failed() { return m_pQueueView_Failed; }
	inline CQueueViewSuccessful* GetQueueView_Successful() { return m_pQueueView_Successful; }

	virtual void SetFocus();
protected:

	CQueueView* m_pQueueView;
	CQueueViewFailed* m_pQueueView_Failed;
	CQueueViewSuccessful* m_pQueueView_Successful;
};

#include "QueueView.h"

#endif //__QUEUE_H__
