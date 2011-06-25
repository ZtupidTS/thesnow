#ifndef __REMOTETREEVIEW_H__
#define __REMOTETREEVIEW_H__

#include "systemimagelist.h"
#include "state.h"
#include "filter.h"
#include "treectrlex.h"

class CQueueView;
class CRemoteTreeView : public wxTreeCtrlEx, CSystemImageList, CStateEventHandler
{
	DECLARE_CLASS(CRemoteTreeView)

	friend class CRemoteTreeViewDropTarget;

public:
	CRemoteTreeView(wxWindow* parent, wxWindowID id, CState* pState, CQueueView* pQueue);
	virtual ~CRemoteTreeView();

protected:
	wxTreeItemId MakeParent(CServerPath path, bool select);
	void SetDirectoryListing(const CSharedPointer<const CDirectoryListing> &pListing, bool modified);
	virtual void OnStateChange(CState* pState, enum t_statechange_notifications notification, const wxString& data, const void* data2);

	void DisplayItem(wxTreeItemId parent, const CDirectoryListing& listing);
	void RefreshItem(wxTreeItemId parent, const CDirectoryListing& listing, bool will_select_parent);

	void SetItemImages(wxTreeItemId item, bool unknown);

	bool HasSubdirs(const CDirectoryListing& listing, const CFilterManager& filter);

	virtual int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2);

	CServerPath GetPathFromItem(const wxTreeItemId& item) const;

	bool ListExpand(wxTreeItemId item);

	void ApplyFilters();

	CQueueView* m_pQueue;

	void CreateImageList();
	wxBitmap CreateIcon(int index, const wxString& overlay = _T(""));
	wxImageList* m_pImageList;

	// Set to true in SetDirectoryListing.
	// Used to suspends event processing in OnItemExpanding for example
	bool m_busy;

	wxTreeItemId m_ExpandAfterList;

	wxTreeItemId m_dropHighlight;

	DECLARE_EVENT_TABLE()
	void OnItemExpanding(wxTreeEvent& event);
	void OnSelectionChanged(wxTreeEvent& event);
	void OnItemActivated(wxTreeEvent& event);
	void OnBeginDrag(wxTreeEvent& event);
#ifndef __WXMSW__
	void OnKeyDown(wxKeyEvent& event);
#endif
	void OnContextMenu(wxTreeEvent& event);
	void OnMenuChmod(wxCommandEvent& event);
	void OnMenuDownload(wxCommandEvent& event);
	void OnMenuDelete(wxCommandEvent& event);
	void OnMenuRename(wxCommandEvent& event);
	void OnBeginLabelEdit(wxTreeEvent& event);
	void OnEndLabelEdit(wxTreeEvent& event);
	void OnMkdir(wxCommandEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnMenuGeturl(wxCommandEvent& event);

	wxTreeItemId m_contextMenuItem;
};

#endif
