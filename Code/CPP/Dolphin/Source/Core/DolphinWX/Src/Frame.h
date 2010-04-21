// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/


#ifndef __FRAME_H_
#define __FRAME_H_

#include <wx/wx.h> // wxWidgets
#include <wx/busyinfo.h>
#include <wx/mstream.h>
#include <wx/listctrl.h>
#include <wx/artprov.h>
#if defined(__APPLE__)
//id is an objective-c++ type, wx team need to change this
#define id toolid
#endif
#include <wx/aui/aui.h>
#include <string>
#include <vector>

#include "CDUtils.h"
#include "CodeWindow.h"
#include "LogWindow.h"
#if defined(HAVE_X11) && HAVE_X11
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#endif

// A shortcut to access the bitmaps
#define wxGetBitmapFromMemory(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap _wxGetBitmapFromMemory(const unsigned char* data, int length)
{
	wxMemoryInputStream is(data, length);
	return(wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1));
}

// Class declarations
class CGameListCtrl;
class CLogWindow;

// The CPanel class to receive MSWWindowProc messages from the video plugin.
class CPanel : public wxPanel
{
	public:
		CPanel(
			wxWindow* parent,
			wxWindowID id = wxID_ANY
			);

	private:
		DECLARE_EVENT_TABLE();

		#ifdef _WIN32
			// Receive WndProc messages
			WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
		#endif
};

class CFrame : public wxFrame
{
	public:
		CFrame(wxFrame* parent,
			wxWindowID id = wxID_ANY,
#ifdef NO_MOD
			const wxString& title = wxT("Dolphin"),
#else
			const wxString& title = wxT("Dolphin (MOD)"),
#endif
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			bool _UseDebugger = false,
			bool ShowLogWindow = false,
			long style = wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE);

		void* GetRenderHandle()
		{
			#ifdef _WIN32
				return (void *)m_RenderParent->GetHandle();
			#elif defined(HAVE_X11) && HAVE_X11
				return (void *)GDK_WINDOW_XID(GTK_WIDGET(m_RenderParent->GetHandle())->window);
			#else
				return m_RenderParent;
			#endif
		}

		virtual ~CFrame();

		// These have to be public		
		CCodeWindow* g_pCodeWindow;
		wxMenuBar* m_MenuBar;
		wxBitmap aNormalFile;
		void InitBitmaps();
		void DoPause();
		void DoStop();
		bool bRenderToMain;
#ifdef _WIN32
		bool bRendererHasFocus;
#endif
		bool bNoWiimoteMsg;
		void UpdateGUI();
		void ToggleLogWindow(bool, int i = -1);
		void ToggleConsole(bool, int i = -1);		
		void PostEvent(wxCommandEvent& event);
		void PostMenuEvent(wxMenuEvent& event);
		void PostUpdateUIEvent(wxUpdateUIEvent& event);
		void StatusBarMessage(const char * Text, ...);
		void ClearStatusBar();
		void OnCustomHostMessage(int Id);
		void OnSizeRequest(int& x, int& y, int& width, int& height);
		void StartGame(const std::string& filename);
		void OnRenderParentClose(wxCloseEvent& event);
		void OnRenderParentMove(wxMoveEvent& event);
		bool RendererHasFocus();
		void DoFullscreen(bool _F);

		// AUI
		wxAuiManager *m_Mgr;
		wxAuiToolBar *m_ToolBar, *m_ToolBarDebug, *m_ToolBarAui;
		long NOTEBOOK_STYLE, TOOLBAR_STYLE;
		int iLeftWidth[2], iMidWidth[2];
		bool bFloatLogWindow;
		bool bFloatConsoleWindow;

		// Utility
		wxWindow * GetWxWindow(wxString);
		#ifdef _WIN32
			wxWindow * GetWxWindowHwnd(HWND);
		#endif
		wxWindow * GetFloatingPage(int Id);
		wxWindow * GetNootebookPage(wxString);
		wxWindow * GetNootebookPageFromId(wxWindowID Id);
		wxAuiNotebook * GetNotebookFromId(u32);
		wxWindowID WindowParentIdFromChildId(int Id);
		wxString WindowNameFromId(int Id);
		int GetNotebookCount();
		int Limit(int,int,int);
		int PercentageToPixels(int,int);
		int PixelsToPercentage(int,int);
		void ListChildren();
		void ListTopWindows();
		const wxChar * GetMenuLabel(int Id);

		// Perspectives
		void AddRemoveBlankPage();
		void OnNotebookPageClose(wxAuiNotebookEvent& event);
		void OnAllowNotebookDnD(wxAuiNotebookEvent& event);
		void OnNotebookPageChanged(wxAuiNotebookEvent& event);
		void OnFloatWindow(wxCommandEvent& event);
		void OnTab(wxAuiNotebookEvent& event);
		int GetNootebookAffiliation(wxString Name);
		void ClosePages();
		void DoToggleWindow(int,bool);
		void ShowAllNotebooks(bool Window = false);
		void HideAllNotebooks(bool Window = false);
		void CloseAllNotebooks();
		void DoAddPage(wxWindow *, int, wxString, bool);
		void DoRemovePage(wxWindow *, bool Hide = true);
		void DoRemovePageId(wxWindowID Id, bool Hide = true, bool Destroy = false);
		void DoRemovePageString(wxString, bool Hide = true, bool Destroy = false);
		void TogglePane();
		void SetSimplePaneSize();
		void SetPaneSize();
		void ResetToolbarStyle();
		void TogglePaneStyle(bool On, int EventId);
		void ToggleNotebookStyle(bool On, long Style);
		void ResizeConsole();
		// Float window
		void DoUnfloatPage(int Id);
		void OnFloatingPageClosed(wxCloseEvent& event);
		void OnFloatingPageSize(wxSizeEvent& event);
		void DoFloatNotebookPage(wxWindowID Id);
		wxFrame * CreateParentFrame(wxWindowID Id = wxID_ANY, const wxString& title = wxT(""), wxWindow * = NULL);
		// User perspectives. Should find a way to make these private.
		struct SPerspectives
		{
			std::string Name;
			wxString Perspective;
			std::vector<int> Width, Height;
		};
		std::vector<SPerspectives> Perspectives;
		wxString AuiFullscreen, AuiCurrent;
		wxArrayString AuiPerspective;
		u32 ActivePerspective;	
		void NamePanes();
		void AddPane();
		void Save();
		void SaveLocal();
		void OnPaneClose(wxAuiManagerEvent& evt);
		void ReloadPanes();
		void DoLoadPerspective();
		void OnDropDownToolbarSelect(wxCommandEvent& event);		
		void OnDropDownSettingsToolbar(wxAuiToolBarEvent& event);
		void OnDropDownToolbarItem(wxAuiToolBarEvent& event);
		void OnSelectPerspective(wxCommandEvent& event);		

	private:
		wxStatusBar* m_pStatusBar;
		wxBoxSizer* sizerPanel;
		wxBoxSizer* sizerFrame;
		CGameListCtrl* m_GameListCtrl;
		wxPanel* m_Panel;
		wxFrame* m_RenderFrame;
		wxPanel* m_RenderParent;
		wxToolBarToolBase* m_ToolPlay;
		CLogWindow* m_LogWindow;
		bool UseDebugger;
		bool m_bEdit;
		bool m_bTabSplit;
		bool m_bNoDocking;
		bool m_bControlsCreated;
		char newDiscpath[2048];
		wxMessageDialog *m_StopDlg;

		std::vector<std::string> drives;

		enum EToolbar
		{
			Toolbar_FileOpen,
			Toolbar_Refresh,
			Toolbar_Browse,
			Toolbar_Play,
			Toolbar_Stop,
			Toolbar_Pause,
			Toolbar_Screenshot,
			Toolbar_FullScreen,
			Toolbar_PluginOptions,
			Toolbar_PluginGFX,
			Toolbar_PluginDSP,
			Toolbar_PluginPAD,
			Toolbar_Wiimote,			
			Toolbar_Help,
			EToolbar_Max
		};

		enum EBitmapsThemes
		{
			BOOMY,
			VISTA,
			XPLASTIK,
			KDE,
			THEMES_MAX
		};

		wxBitmap m_Bitmaps[EToolbar_Max];
		wxBitmap m_BitmapsMenu[EToolbar_Max];

		void PopulateToolbar(wxAuiToolBar* toolBar);
		void PopulateToolbarAui(wxAuiToolBar* toolBar);
		void RecreateToolbar();
		void CreateMenu();
		wxPanel *CreateEmptyPanel(wxWindowID Id = wxID_ANY);
		wxAuiNotebook *CreateEmptyNotebook();

#ifdef _WIN32
		// Override window proc for tricks like screensaver disabling
		WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif
		// Event functions
		void OnQuit(wxCommandEvent& event);
		void OnHelp(wxCommandEvent& event);
		void OnToolBar(wxCommandEvent& event);
		void OnAuiToolBar(wxAuiToolBarEvent& event);

		void OnOpen(wxCommandEvent& event); // File menu
		void DoOpen(bool Boot);
		void OnRefresh(wxCommandEvent& event);
		void OnBrowse(wxCommandEvent& event);
		void OnBootDrive(wxCommandEvent& event);

		void OnPlay(wxCommandEvent& event); // Emulation
		void OnStop(wxCommandEvent& event);
		void OnReset(wxCommandEvent& event);
		void OnRecord(wxCommandEvent& event);
		void OnPlayRecording(wxCommandEvent& event);
		void OnChangeDisc(wxCommandEvent& event);
		void OnScreenshot(wxCommandEvent& event);
		void OnActive(wxActivateEvent& event);
		void OnClose(wxCloseEvent &event);	
		void OnLoadState(wxCommandEvent& event);
		void OnSaveState(wxCommandEvent& event);
		void OnLoadStateFromFile(wxCommandEvent& event);
		void OnSaveStateToFile(wxCommandEvent& event);
		void OnLoadLastState(wxCommandEvent& event);
		void OnUndoLoadState(wxCommandEvent& event);
		void OnUndoSaveState(wxCommandEvent& event);
		
		void OnFrameSkip(wxCommandEvent& event);
		void OnFrameStep(wxCommandEvent& event);
		
		void OnConfigMain(wxCommandEvent& event); // Options
		void OnPluginGFX(wxCommandEvent& event);
		void OnPluginDSP(wxCommandEvent& event);
		void OnPluginPAD(wxCommandEvent& event);
		void OnPluginWiimote(wxCommandEvent& event);

		void OnToggleFullscreen(wxCommandEvent& event);
		void OnToggleDualCore(wxCommandEvent& event);
		void OnToggleSkipIdle(wxCommandEvent& event);
		void OnToggleThrottle(wxCommandEvent& event);
		void OnManagerResize(wxAuiManagerEvent& event);
		void OnMove(wxMoveEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnToggleToolbar(wxCommandEvent& event);
		void DoToggleToolbar(bool);
		void OnToggleStatusbar(wxCommandEvent& event);
		void OnToggleLogWindow(wxCommandEvent& event);
		void OnToggleConsole(wxCommandEvent& event);
		void OnKeyDown(wxKeyEvent& event);
		void OnKeyUp(wxKeyEvent& event);
		void OnDoubleClick(wxMouseEvent& event);
		
		void OnHostMessage(wxCommandEvent& event);

		void OnMemcard(wxCommandEvent& event); // Misc
		void OnImportSave(wxCommandEvent& event);
		void OnOpenLuaWindow(wxCommandEvent& event);

		void OnNetPlay(wxCommandEvent& event);

		void OnShow_CheatsWindow(wxCommandEvent& event);
		void OnLoadWiiMenu(wxCommandEvent& event);
		void OnConnectWiimote(wxCommandEvent& event);
		void GameListChanged(wxCommandEvent& event);

		void OnGameListCtrl_ItemActivated(wxListEvent& event);
		void OnRenderParentResize(wxSizeEvent& event);
		bool RendererIsFullscreen();
#if defined HAVE_X11 && HAVE_X11
		void X11_SendClientEvent(const char *message,
				int data1 = 0, int data2 = 0, int data3 = 0, int data4 = 0);
#endif

		// MenuBar
		// File - Drive
		wxMenuItem* m_pSubMenuDrive;

		// Emulation
		wxMenuItem* m_pSubMenuLoad;
		wxMenuItem* m_pSubMenuSave;
		wxMenuItem* m_pSubMenuFrameSkipping;

		void BootGame(const std::string& filename);

#if wxUSE_TIMER
		// Used to process command events
		void OnTimer(wxTimerEvent& WXUNUSED(event));
		wxTimer m_timer;
#endif

		// Event table
		DECLARE_EVENT_TABLE();
};


#endif  // __FRAME_H_

