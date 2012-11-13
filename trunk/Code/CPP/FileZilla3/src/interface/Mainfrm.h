#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#ifndef __WXMAC__
#include <wx/taskbar.h>
#endif

class CStatusView;
class CQueueView;
class CState;
class CAsyncRequestQueue;
class CLed;
class CThemeProvider;
class CQuickconnectBar;
#if FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK
class CUpdateWizard;
#endif //FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK
class CSiteManagerItemData_Site;
class CQueue;
class CWindowStateManager;
class CStatusBar;
class CMainFrameStateEventHandler;
class CSplitterWindowEx;
class CContextControl;
class CToolBar;
class CMenuBar;

class CMainFrame : public wxFrame
{
	friend class CMainFrameStateEventHandler;
public:
	CMainFrame();
	virtual ~CMainFrame();

	void UpdateActivityLed(int direction);

	void AddToRequestQueue(CFileZillaEngine* pEngine, CAsyncRequestNotification* pNotification);
	CStatusView* GetStatusView() { return m_pStatusView; }
	CQueueView* GetQueue() { return m_pQueueView; }
	CQuickconnectBar* GetQuickconnectBar() { return m_pQuickconnectBar; }

	void UpdateLayout(int layout = -1, int swap = -1, int messagelog_position = -1);

	// Window size and position as well as pane sizes
	void RememberSplitterPositions();
	bool RestoreSplitterPositions();
	void SetDefaultSplitterPositions();

	void CheckChangedSettings();

	void ConnectNavigationHandler(wxEvtHandler* handler);

	CStatusBar* GetStatusBar() { return m_pStatusBar; }

	void ProcessCommandLine();

	void PostInitialize();

	bool ConnectToServer(const CServer& server, const CServerPath& path = CServerPath());

	CContextControl* GetContextControl() { return m_pContextControl; }

	bool ConnectToSite(CSiteManagerItemData_Site* const pData, bool newTab = false);

protected:
	bool CloseDialogsAndQuit(wxCloseEvent &event);
	bool CreateMenus();
	bool CreateQuickconnectBar();
	bool CreateToolBar();
	void OpenSiteManager(const CServer* pServer = 0);

	void FocusNextEnabled(std::list<wxWindow*>& windowOrder, std::list<wxWindow*>::iterator iter, bool skipFirst, bool forward);

	void SetBookmarksFromPath(const wxString& path);

	CStatusBar* m_pStatusBar;
	CMenuBar* m_pMenuBar;
	CToolBar* m_pToolBar;
	CQuickconnectBar* m_pQuickconnectBar;

	CSplitterWindowEx* m_pTopSplitter; // If log position is 0, splits message log from rest of panes
	CSplitterWindowEx* m_pBottomSplitter; // Top contains view splitter, bottom queue (or queuelog splitter if in position 1)
	CSplitterWindowEx* m_pQueueLogSplitter;

	CContextControl* m_pContextControl;

	CStatusView* m_pStatusView;
	CQueueView* m_pQueueView;
	CLed* m_pActivityLed[2];
	wxTimer m_transferStatusTimer;
	CThemeProvider* m_pThemeProvider;
#if FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK
	CUpdateWizard* m_pUpdateWizard;
#endif //FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK

	void ShowLocalTree();
	void ShowRemoteTree();

	void ShowDropdownMenu(wxMenu* pMenu, wxToolBar* pToolBar, wxCommandEvent& event);

#ifdef __WXMSW__
	virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

	void HandleResize();

	// Event handlers
	DECLARE_EVENT_TABLE()
	void OnSize(wxSizeEvent& event);
	void OnMenuHandler(wxCommandEvent& event);
	void OnEngineEvent(wxEvent& event);
	void OnDisconnect(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnReconnect(wxCommandEvent& event);
	void OnRefresh(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnSiteManager(wxCommandEvent& event);
	void OnProcessQueue(wxCommandEvent& event);
	void OnMenuEditSettings(wxCommandEvent& event);
	void OnToggleToolBar(wxCommandEvent& event);
	void OnToggleLogView(wxCommandEvent& event);
	void OnToggleLocalTreeView(wxCommandEvent& event);
	void OnToggleRemoteTreeView(wxCommandEvent& event);
	void OnToggleQueueView(wxCommandEvent& event);
	void OnMenuHelpAbout(wxCommandEvent& event);
	void OnFilter(wxCommandEvent& event);
	void OnFilterRightclicked(wxCommandEvent& event);
#if FZ_MANUALUPDATECHECK
	void OnCheckForUpdates(wxCommandEvent& event);
#endif //FZ_MANUALUPDATECHECK
	void OnSitemanagerDropdown(wxCommandEvent& event);
	void OnNavigationKeyEvent(wxNavigationKeyEvent& event);
	void OnGetFocus(wxFocusEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnActivate(wxActivateEvent& event);
	void OnToolbarComparison(wxCommandEvent& event);
	void OnToolbarComparisonDropdown(wxCommandEvent& event);
	void OnDropdownComparisonMode(wxCommandEvent& event);
	void OnDropdownComparisonHide(wxCommandEvent& event);
	void OnSyncBrowse(wxCommandEvent& event);
#ifndef __WXMAC__
	void OnIconize(wxIconizeEvent& event);
	void OnTaskBarClick(wxTaskBarIconEvent& event);
#endif
#ifdef __WXGTK__
	void OnTaskBarClick_Delayed(wxCommandEvent& event);
#endif
	void OnSearch(wxCommandEvent& event);
	void OnMenuNewTab(wxCommandEvent& event);
	void OnMenuCloseTab(wxCommandEvent& event);

	bool m_bInitDone;
	bool m_bQuit;
	wxEventType m_closeEvent;
	wxTimer m_closeEventTimer;

	CAsyncRequestQueue* m_pAsyncRequestQueue;
	CMainFrameStateEventHandler* m_pStateEventHandler;

	CWindowStateManager* m_pWindowStateManager;

	CQueue* m_pQueuePane;

#ifndef __WXMAC__
	wxTaskBarIcon* m_taskBarIcon;
#endif
#ifdef __WXGTK__
	// There is a bug in KDE, causing the window to toggle iconized state
	// several times a second after uniconizing it from taskbar icon.
	// Set m_taskbar_is_uniconizing in OnTaskBarClick and unset the
	// next time the pending event processing runs and calls OnTaskBarClick_Delayed.
	// While set, ignore iconize events.
	bool m_taskbar_is_uniconizing;
#endif

	int m_comparisonToggleAcceleratorId;
};

#endif
