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


#ifndef CODEWINDOW_H_
#define CODEWINDOW_H_

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/artprov.h>
#include <wx/aui/aui.h>

#include "Thread.h"
#include "CoreParameter.h"

// GUI global
#include "../../DolphinWX/Src/Globals.h"
#include "../../DolphinWX/Src/Frame.h"

class CFrame;
class CRegisterWindow;
class CBreakPointWindow;
class CMemoryWindow;
class CJitWindow;
class CCodeView;

class CCodeWindow
	: public wxPanel
{
	public:

		CCodeWindow(const SCoreStartupParameter& _LocalCoreStartupParameter,
			CFrame * parent,
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxTAB_TRAVERSAL | wxNO_BORDER,
			const wxString& name = wxT("Dolphin-Debugger")
			);
		/*
		CCodeWindow(const SCoreStartupParameter& _LocalCoreStartupParameter, wxWindow* parent,
			wxWindowID id = wxID_ANY,
			const wxString& title = _T("Dolphin-Debugger"),
		const wxPoint& pos = wxPoint(950, 100),
		const wxSize& size = wxSize(400, 500),
		long style = wxDEFAULT_FRAME_STYLE | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE);
		*/

		~CCodeWindow();
		void Load();
		void Save();

		// Parent interaction
		CFrame *Parent;
		wxMenuBar * GetMenuBar();
		wxAuiToolBar * GetToolBar();		
		int GetNootebookAffiliation(wxString);
		wxBitmap m_Bitmaps[ToolbarDebugBitmapMax];

		bool UseInterpreter();
		bool BootToPause();
		bool AutomaticStart();
		bool UnlimitedJITCache();
		bool JITBlockLinking();
        void JumpToAddress(u32 _Address);

		void Update();
		void NotifyMapLoaded();
		void CreateMenu(const SCoreStartupParameter& _LocalCoreStartupParameter, wxMenuBar * pMenuBar);
		void CreateMenuView(wxMenuBar * pMenuBar, wxMenu*);
		void CreateMenuOptions(wxMenuBar * pMenuBar, wxMenu*);
		void CreateMenuSymbols();
		void RecreateToolbar(wxAuiToolBar*);
		void PopulateToolbar(wxAuiToolBar* toolBar);
		void UpdateButtonStates();
		void OpenPages();
		void UpdateManager();
		
		// Menu bar
		// -------------------
		void OnCPUMode(wxCommandEvent& event); // CPU Mode menu	
		void OnJITOff(wxCommandEvent& event);

		void OnToggleWindow(wxCommandEvent& event);
		void OnToggleCodeWindow(bool,int);
		void OnToggleRegisterWindow(bool,int);
		void OnToggleBreakPointWindow(bool,int);
		void OnToggleMemoryWindow(bool,int);
		void OnToggleJitWindow(bool,int);
		void OnToggleDLLWindow(int,bool,int);
		void OnChangeFont(wxCommandEvent& event);

		void OnCodeStep(wxCommandEvent& event);
		void OnAddrBoxChange(wxCommandEvent& event);
		void OnSymbolsMenu(wxCommandEvent& event);
		void OnJitMenu(wxCommandEvent& event);
		void OnProfilerMenu(wxCommandEvent& event);

		// Sub dialogs
		wxMenuBar* pMenuBar;
		CRegisterWindow* m_RegisterWindow;
		CBreakPointWindow* m_BreakpointWindow;
		CMemoryWindow* m_MemoryWindow;
		CJitWindow* m_JitWindow;

		// Settings
		bool bAutomaticStart; bool bBootToPause;
		bool bLogWindow; int iLogWindow;
		bool bConsoleWindow; int iConsoleWindow;
		bool bCodeWindow; int iCodeWindow; bool bFloatCodeWindow;
		bool bRegisterWindow; int iRegisterWindow; bool bFloatRegisterWindow;
		bool bBreakpointWindow; int iBreakpointWindow; bool bFloatBreakpointWindow;
		bool bMemoryWindow; int iMemoryWindow; bool bFloatMemoryWindow;
		bool bJitWindow; int iJitWindow; bool bFloatJitWindow;
		bool bSoundWindow; int iSoundWindow; bool bFloatSoundWindow;
		bool bVideoWindow; int iVideoWindow; bool bFloatVideoWindow;

	private:

		enum
		{			
			// Debugger GUI Objects
			ID_CODEVIEW,
			ID_CALLSTACKLIST,
			ID_CALLERSLIST,
			ID_CALLSLIST,
			ID_SYMBOLLIST
		};

		void OnSymbolListChange(wxCommandEvent& event);
		void OnSymbolListContextMenu(wxContextMenuEvent& event);
		void OnCallstackListChange(wxCommandEvent& event);
		void OnCallersListChange(wxCommandEvent& event);
		void OnCallsListChange(wxCommandEvent& event);
		void OnCodeViewChange(wxCommandEvent &event);
		void SingleCPUStep();		
		void OnHostMessage(wxCommandEvent& event);

		void UpdateLists();
		void UpdateCallstack();
		void OnStatusBar(wxMenuEvent& event); void OnStatusBar_(wxUpdateUIEvent& event);
		void DoTip(wxString text);
		void OnKeyDown(wxKeyEvent& event);

		void InitBitmaps();
		void CreateGUIControls(const SCoreStartupParameter& _LocalCoreStartupParameter);	

		wxMenuItem* jitblocklinking, *jitunlimited, *jitoff;
		wxMenuItem* jitlsoff, *jitlslxzoff, *jitlslwzoff, *jitlslbzxoff;
		wxMenuItem* jitlspoff;
		wxMenuItem* jitlsfoff;
		wxMenuItem* jitfpoff;
		wxMenuItem* jitioff;
		wxMenuItem* jitpoff;
		wxMenuItem* jitsroff;

		CCodeView* codeview;
		wxListBox* callstack;
		wxListBox* symbols;
		wxListBox* callers;
		wxListBox* calls;
		Common::Event sync_event;

		DECLARE_EVENT_TABLE()	
};

#endif /*CODEWINDOW_*/
