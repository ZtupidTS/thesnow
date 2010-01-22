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


// Include
#include "Common.h"

#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/textdlg.h>
#include <wx/listctrl.h>
#include <wx/thread.h>
#include <wx/mstream.h>
#include <wx/tipwin.h>

#include "Host.h"

#include "Debugger.h"

#include "RegisterWindow.h"
#include "BreakpointWindow.h"
#include "MemoryWindow.h"
#include "JitWindow.h"

#include "CodeWindow.h"
#include "CodeView.h"

#include "FileUtil.h"
#include "Core.h"
#include "HW/Memmap.h"
#include "HLE/HLE.h"
#include "Boot/Boot.h"
#include "LogManager.h"
#include "HW/CPU.h"
#include "PowerPC/PowerPC.h"
#include "Debugger/PPCDebugInterface.h"
#include "Debugger/Debugger_SymbolMap.h"
#include "PowerPC/PPCAnalyst.h"
#include "PowerPC/Profiler.h"
#include "PowerPC/PPCSymbolDB.h"
#include "PowerPC/SignatureDB.h"
#include "PowerPC/PPCTables.h"
#include "PowerPC/JitCommon/JitBase.h"
#include "PowerPC/JitCommon/JitCache.h" // for ClearCache()

#include "PluginManager.h"
#include "ConfigManager.h"


extern "C"  // Bitmaps
{
	#include "../resources/toolbar_play.c"
	#include "../resources/toolbar_pause.c"
	#include "../resources/toolbar_add_memorycheck.c"
	#include "../resources/toolbar_delete.c"
	#include "../resources/toolbar_add_breakpoint.c"
}

class CPluginInfo;
class CPluginManager;


// -------
// Main

BEGIN_EVENT_TABLE(CCodeWindow, wxPanel)   

	// Menu bar
	EVT_MENU(IDM_AUTOMATICSTART,       CCodeWindow::OnCPUMode) // Options
	EVT_MENU(IDM_BOOTTOPAUSE,       CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_FONTPICKER,		CCodeWindow::OnChangeFont)

	EVT_MENU(IDM_INTERPRETER,       CCodeWindow::OnCPUMode) // Jit
	EVT_MENU(IDM_JITUNLIMITED,	 CCodeWindow::OnCPUMode)
#ifdef JIT_OFF_OPTIONS
	EVT_MENU(IDM_JITOFF,			CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITLSOFF,			CCodeWindow::OnCPUMode)
		EVT_MENU(IDM_JITLSLXZOFF,		CCodeWindow::OnCPUMode)
			EVT_MENU(IDM_JITLSLWZOFF,		CCodeWindow::OnCPUMode)
		EVT_MENU(IDM_JITLSLBZXOFF,		CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITLSFOFF,			CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITLSPOFF,			CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITFPOFF,			CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITIOFF,			CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITPOFF,			CCodeWindow::OnCPUMode)
	EVT_MENU(IDM_JITSROFF,			CCodeWindow::OnCPUMode)
#endif
	EVT_MENU(IDM_CLEARCODECACHE,    CCodeWindow::OnJitMenu)
	EVT_MENU(IDM_LOGINSTRUCTIONS,   CCodeWindow::OnJitMenu)
	EVT_MENU(IDM_SEARCHINSTRUCTION, CCodeWindow::OnJitMenu)

	EVT_MENU(IDM_REGISTERWINDOW,    CCodeWindow::OnToggleWindow) // View
	EVT_MENU(IDM_BREAKPOINTWINDOW,  CCodeWindow::OnToggleWindow)
	EVT_MENU(IDM_MEMORYWINDOW,      CCodeWindow::OnToggleWindow)
	EVT_MENU(IDM_JITWINDOW,			CCodeWindow::OnToggleWindow)
	EVT_MENU(IDM_SOUNDWINDOW,		CCodeWindow::OnToggleWindow)
	EVT_MENU(IDM_VIDEOWINDOW,		CCodeWindow::OnToggleWindow)

	EVT_MENU(IDM_CLEARSYMBOLS,      CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_LOADMAPFILE,       CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_SCANFUNCTIONS,     CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_SAVEMAPFILE,       CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_SAVEMAPFILEWITHCODES, CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_CREATESIGNATUREFILE, CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_USESIGNATUREFILE,  CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_PATCHHLEFUNCTIONS, CCodeWindow::OnSymbolsMenu)
	EVT_MENU(IDM_RENAME_SYMBOLS, CCodeWindow::OnSymbolsMenu)

	EVT_MENU(IDM_PROFILEBLOCKS,     CCodeWindow::OnProfilerMenu)
	EVT_MENU(IDM_WRITEPROFILE,      CCodeWindow::OnProfilerMenu)

	// Menu tooltips
	//EVT_MENU_HIGHLIGHT_ALL(	CCodeWindow::OnStatusBar)
	// Do this to to avoid that the ToolTips get stuck when only the wxMenu is changed
	// and not any wxMenuItem that is required by EVT_MENU_HIGHLIGHT_ALL
	//EVT_UPDATE_UI(wxID_ANY, CCodeWindow::OnStatusBar_)

	// Toolbar
	EVT_MENU(IDM_STEP,				CCodeWindow::OnCodeStep)
	EVT_MENU(IDM_STEPOVER,			CCodeWindow::OnCodeStep)
	EVT_MENU(IDM_SKIP,				CCodeWindow::OnCodeStep)
	EVT_MENU(IDM_SETPC,				CCodeWindow::OnCodeStep)
	EVT_MENU(IDM_GOTOPC,			CCodeWindow::OnCodeStep)
	EVT_TEXT(IDM_ADDRBOX,           CCodeWindow::OnAddrBoxChange)

	// Other
	EVT_LISTBOX(ID_SYMBOLLIST,     CCodeWindow::OnSymbolListChange)
	EVT_LISTBOX(ID_CALLSTACKLIST,  CCodeWindow::OnCallstackListChange)
	EVT_LISTBOX(ID_CALLERSLIST,    CCodeWindow::OnCallersListChange)
	EVT_LISTBOX(ID_CALLSLIST,      CCodeWindow::OnCallsListChange)

	EVT_HOST_COMMAND(wxID_ANY,      CCodeWindow::OnHostMessage)	

	//EVT_COMMAND(ID_CODEVIEW, wxEVT_CODEVIEW_CHANGE, CCodeWindow::OnCodeViewChange)

END_EVENT_TABLE()


// Class
CCodeWindow::CCodeWindow(const SCoreStartupParameter& _LocalCoreStartupParameter, CFrame *parent,
	wxWindowID id, const wxPoint& position, const wxSize& size, long style, const wxString& name)
	: wxPanel((wxWindow*)parent, id, position, size, style, name)
	, Parent(parent)
	, m_RegisterWindow(NULL)
	, m_BreakpointWindow(NULL)
	, m_MemoryWindow(NULL)
	, m_JitWindow(NULL)
	, codeview(NULL)
{
	InitBitmaps();

	CreateGUIControls(_LocalCoreStartupParameter);

	// Connect keyboard
	wxTheApp->Connect(wxID_ANY, wxEVT_KEY_DOWN,
		wxKeyEventHandler(CCodeWindow::OnKeyDown),
		(wxObject*)0, this);
}
CCodeWindow::~CCodeWindow()
{
	//if (Parent) Parent->g_pCodeWindow = NULL;
	//ConsoleListener* Console = LogManager::GetInstance()->getConsoleListener();
	//Console->Log(LogTypes::LERROR, StringFromFormat(" >>> CCodeWindow Destroyed\n").c_str());
}
// Redirect old wxFrame calls
wxMenuBar *CCodeWindow::GetMenuBar()
{
	return Parent->GetMenuBar();
}
wxAuiToolBar *CCodeWindow::GetToolBar()
{
	return Parent->m_ToolBarDebug;
}

// ----------
// Events

void CCodeWindow::OnKeyDown(wxKeyEvent& event)
{
	event.Skip();

	if ((event.GetKeyCode() == WXK_SPACE) && Parent->IsActive()) SingleCPUStep();		
}

void CCodeWindow::OnHostMessage(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	    case IDM_NOTIFYMAPLOADED:
		    NotifyMapLoaded();
		    break;
		// Is this still necessary ?
		//case IDM_UPDATELOGDISPLAY:
		//	if (m_LogWindow) m_LogWindow->NotifyUpdate();
		//	break;

	    case IDM_UPDATEDISASMDIALOG:
		    Update();
			if (codeview) codeview->Center(PC);
		    if (m_RegisterWindow) m_RegisterWindow->NotifyUpdate();
		    break;

		case IDM_UPDATEBREAKPOINTS:
            Update();
			if (m_BreakpointWindow) m_BreakpointWindow->NotifyUpdate();
			break;
		case IDM_UPDATESTATUSBAR:
			//if (main_frame->m_pStatusBar != NULL)
			{
				//this->GetParent()->m_p
				//this->GetParent()->
				//parent->m_pStatusBar->SetStatusText(wxT("Hi"), 0);
				//m_pStatusBar->SetStatusText(event.GetString(), event.GetInt());
				//this->GetParent()->m_pStatusBar->SetStatusText(event.GetString(), event.GetInt());
				//main_frame->m_pStatusBar->SetStatusText(event.GetString(), event.GetInt());
			}
			break;
	}
}

// The Play, Stop, Step, Skip, Go to PC and Show PC buttons go here
void CCodeWindow::OnCodeStep(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	    case IDM_STEP:
			SingleCPUStep();
		    break;

	    case IDM_STEPOVER:
		    CCPU::EnableStepping(true);   // TODO: Huh?
		    break;

	    case IDM_SKIP:
		    PC += 4;
		    Update();
		    break;

	    case IDM_SETPC:
		    PC = codeview->GetSelection();
		    Update();
		    break;

	    case IDM_GOTOPC:
		    JumpToAddress(PC);
		    break;
	}

	UpdateButtonStates();	
	// Update all toolbars in the aui manager
	Parent->UpdateGUI();
}


void CCodeWindow::JumpToAddress(u32 _Address)
{
    codeview->Center(_Address);
	UpdateLists();
}


void CCodeWindow::OnCodeViewChange(wxCommandEvent &event)
{
	UpdateLists();
}

void CCodeWindow::OnAddrBoxChange(wxCommandEvent& event)
{
	if (!GetToolBar()) return;

	wxTextCtrl* pAddrCtrl = (wxTextCtrl*)GetToolBar()->FindControl(IDM_ADDRBOX);
	wxString txt = pAddrCtrl->GetValue();

	std::string text(txt.mb_str());
	text = StripSpaces(text);
	if (text.size() == 8)
	{
		u32 addr;
		sscanf(text.c_str(), "%08x", &addr);
		JumpToAddress(addr);
	}

	event.Skip(1);
}

void CCodeWindow::OnCallstackListChange(wxCommandEvent& event)
{
	int index   = callstack->GetSelection();
	if (index >= 0) {
		u32 address = (u32)(u64)(callstack->GetClientData(index));
		if (address)
			JumpToAddress(address);
	}
}

void CCodeWindow::OnCallersListChange(wxCommandEvent& event)
{
	int index = callers->GetSelection();
	if (index >= 0) {
		u32 address = (u32)(u64)(callers->GetClientData(index));
		if (address)
			JumpToAddress(address);
	}
}

void CCodeWindow::OnCallsListChange(wxCommandEvent& event)
{
	int index = calls->GetSelection();
	if (index >= 0) {
		u32 address = (u32)(u64)(calls->GetClientData(index));
		if (address)
			JumpToAddress(address);
	}
}

void CCodeWindow::SingleCPUStep()
{
	CCPU::StepOpcode(&sync_event);
	//            if (CCPU::IsStepping())
	//	            sync_event.Wait();
	wxThread::Sleep(20);
	// need a short wait here
	JumpToAddress(PC);
	Update();
	Host_UpdateLogDisplay();
}

void CCodeWindow::UpdateLists()
{
	callers->Clear();
	u32 addr = codeview->GetSelection();
	Symbol *symbol = g_symbolDB.GetSymbolFromAddr(addr);
	if (!symbol)
		return;
	for (int i = 0; i < (int)symbol->callers.size(); i++)
	{
		u32 caller_addr = symbol->callers[i].callAddress;
		Symbol *caller_symbol = g_symbolDB.GetSymbolFromAddr(caller_addr);
		if (caller_symbol) {
			int idx = callers->Append(wxString::FromAscii(StringFromFormat("< %s (%08x)", caller_symbol->name.c_str(), caller_addr).c_str()));
			callers->SetClientData(idx, (void*)caller_addr);
		}
	}

	calls->Clear();
	for (int i = 0; i < (int)symbol->calls.size(); i++)
	{
		u32 call_addr = symbol->calls[i].function;
		Symbol *call_symbol = g_symbolDB.GetSymbolFromAddr(call_addr);
		if (call_symbol) {
			int idx = calls->Append(wxString::FromAscii(StringFromFormat("> %s (%08x)", call_symbol->name.c_str(), call_addr).c_str()));
			calls->SetClientData(idx, (void*)call_addr);
		}
	}
}

void CCodeWindow::UpdateCallstack()
{
	//if (PowerPC::GetState() == PowerPC::CPU_POWERDOWN) return;
	if (Core::GetState() == Core::CORE_STOPPING) return;

	callstack->Clear();

	std::vector<Dolphin_Debugger::CallstackEntry> stack;

	bool ret = Dolphin_Debugger::GetCallstack(stack);

	for (size_t i = 0; i < stack.size(); i++)
	{
		int idx = callstack->Append(wxString::FromAscii(stack[i].Name.c_str()));
		callstack->SetClientData(idx, (void*)(u64)stack[i].vAddress);
	}

	if (!ret)
		callstack->Append(wxString::FromAscii("invalid callstack"));
}

void CCodeWindow::CreateGUIControls(const SCoreStartupParameter& _LocalCoreStartupParameter)
{	
	// Configure the code window
	wxBoxSizer* sizerBig   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerLeft  = new wxBoxSizer(wxVERTICAL);

	DebugInterface* di = &PowerPC::debug_interface;

	codeview = new CCodeView(di, &g_symbolDB, this, ID_CODEVIEW);
	sizerBig->Add(sizerLeft, 2, wxEXPAND);
	sizerBig->Add(codeview, 5, wxEXPAND);

	sizerLeft->Add(callstack = new wxListBox(this, ID_CALLSTACKLIST, wxDefaultPosition, wxSize(90, 100)), 0, wxEXPAND);
	sizerLeft->Add(symbols = new wxListBox(this, ID_SYMBOLLIST, wxDefaultPosition, wxSize(90, 100), 0, NULL, wxLB_SORT), 1, wxEXPAND);
	sizerLeft->Add(calls = new wxListBox(this, ID_CALLSLIST, wxDefaultPosition, wxSize(90, 100), 0, NULL, wxLB_SORT), 0, wxEXPAND);
	sizerLeft->Add(callers = new wxListBox(this, ID_CALLERSLIST, wxDefaultPosition, wxSize(90, 100), 0, NULL, wxLB_SORT), 0, wxEXPAND);

	SetSizer(sizerBig);

	sizerLeft->SetSizeHints(this);
	sizerLeft->Fit(this);
	sizerBig->SetSizeHints(this);
	sizerBig->Fit(this);

	sync_event.Init();
}


// -------
// Menus

// Create CPU Mode menus
void CCodeWindow::CreateMenu(const SCoreStartupParameter& _LocalCoreStartupParameter, wxMenuBar * _pMenuBar)
{
	// Create menu
	//pMenuBar = new wxMenuBar(wxMB_DOCKABLE);
	pMenuBar = _pMenuBar;

	// Add separator to mark beginning of debugging menus

	// CPU Mode
	wxMenu* pCoreMenu = new wxMenu;

	wxMenuItem* interpreter = pCoreMenu->Append(IDM_INTERPRETER, _T("&Interpreter core")
		, wxString::FromAscii("This is nessesary to get break points"
		" and stepping to work as explained in the Developer Documentation. But it can be very"
		" slow, perhaps slower than 1 fps.")
		, wxITEM_CHECK);
	interpreter->Check(_LocalCoreStartupParameter.iCPUCore == 0);
	pCoreMenu->AppendSeparator();

	jitblocklinking = pCoreMenu->Append(IDM_JITBLOCKLINKING, _T("&JIT Block Linking off"),
		_T("Provide safer execution by not linking the JIT blocks."),
		wxITEM_CHECK);

	jitunlimited = pCoreMenu->Append(IDM_JITUNLIMITED, _T("&Unlimited JIT Cache"),
		_T("Avoid any involuntary JIT cache clearing, this may prevent Zelda TP from crashing.")
		_T(" [This option must be selected before a game is started.]"),
		wxITEM_CHECK);
	pCoreMenu->Append(IDM_CLEARCODECACHE, _T("&Clear JIT cache"));

	pCoreMenu->AppendSeparator();
	pCoreMenu->Append(IDM_LOGINSTRUCTIONS, _T("&Log JIT instruction coverage"));
	pCoreMenu->Append(IDM_SEARCHINSTRUCTION, _T("&Search for an op"));

	#ifdef JIT_OFF_OPTIONS
		pCoreMenu->AppendSeparator();
		jitoff = pCoreMenu->Append(IDM_JITOFF, _T("&JIT off (JIT core)"),
			_T("Turn off all JIT functions, but still use the JIT core from Jit.cpp"),
			wxITEM_CHECK);
		jitlsoff = pCoreMenu->Append(IDM_JITLSOFF, _T("&JIT LoadStore off"), wxEmptyString, wxITEM_CHECK);
			jitlslbzxoff = pCoreMenu->Append(IDM_JITLSLBZXOFF, _T("    &JIT LoadStore lbzx off"), wxEmptyString, wxITEM_CHECK);
			jitlslxzoff = pCoreMenu->Append(IDM_JITLSLXZOFF, _T("    &JIT LoadStore lXz off"), wxEmptyString, wxITEM_CHECK);
				jitlslwzoff = pCoreMenu->Append(IDM_JITLSLWZOFF, _T("        &JIT LoadStore lwz off"), wxEmptyString, wxITEM_CHECK);
		jitlspoff = pCoreMenu->Append(IDM_JITLSFOFF, _T("&JIT LoadStore Floating off"), wxEmptyString, wxITEM_CHECK);
		jitlsfoff = pCoreMenu->Append(IDM_JITLSPOFF, _T("&JIT LoadStore Paired off"), wxEmptyString, wxITEM_CHECK);
		jitfpoff = pCoreMenu->Append(IDM_JITFPOFF, _T("&JIT FloatingPoint off"), wxEmptyString, wxITEM_CHECK);
		jitioff = pCoreMenu->Append(IDM_JITIOFF, _T("&JIT Integer off"), wxEmptyString, wxITEM_CHECK);
		jitpoff = pCoreMenu->Append(IDM_JITPOFF, _T("&JIT Paired off"), wxEmptyString, wxITEM_CHECK);
		jitsroff = pCoreMenu->Append(IDM_JITSROFF, _T("&JIT SystemRegisters off"), wxEmptyString, wxITEM_CHECK);
	#endif

	pMenuBar->Append(pCoreMenu, _T("&JIT"));

	CreateMenuSymbols();
}
// Create View menu
void CCodeWindow::CreateMenuView(wxMenuBar * _pMenuBar, wxMenu* _pMenu)
{
	//wxMenu* pDebugDialogs = new wxMenu;

	wxMenuItem* pRegister = _pMenu->Append(IDM_REGISTERWINDOW, _T("&Registers"), wxEmptyString, wxITEM_CHECK);
	pRegister->Check(bRegisterWindow);
	wxMenuItem* pBreakPoints = _pMenu->Append(IDM_BREAKPOINTWINDOW, _T("&BreakPoints"), wxEmptyString, wxITEM_CHECK);
	pBreakPoints->Check(bBreakpointWindow);
	wxMenuItem* pMemory = _pMenu->Append(IDM_MEMORYWINDOW, _T("&Memory"), wxEmptyString, wxITEM_CHECK);
	pMemory->Check(bMemoryWindow);
	wxMenuItem* pJit = _pMenu->Append(IDM_JITWINDOW, _T("&Jit"), wxEmptyString, wxITEM_CHECK);
	pJit->Check(bJitWindow);
	wxMenuItem* pSound = _pMenu->Append(IDM_SOUNDWINDOW, _T("&Sound"), wxEmptyString, wxITEM_CHECK);
	pSound->Check(bSoundWindow);
	wxMenuItem* pVideo = _pMenu->Append(IDM_VIDEOWINDOW, _T("&Video"), wxEmptyString, wxITEM_CHECK);
	pVideo->Check(bVideoWindow);

	//pMenuBar->Append(pDebugDialogs, _T("&Views"));	
}

void CCodeWindow::CreateMenuOptions(wxMenuBar * _pMenuBar, wxMenu* _pMenu)
{
	wxMenuItem* boottopause = _pMenu->Append(IDM_BOOTTOPAUSE, _T("Boot to pause"),
		wxT("Start the game directly instead of booting to pause"), wxITEM_CHECK);
	boottopause->Check(bBootToPause);

	wxMenuItem* automaticstart = _pMenu->Append(IDM_AUTOMATICSTART, _T("&Automatic start")
		, wxString::FromAscii(
		"Automatically load the Default ISO when Dolphin starts, or the last game you loaded,"
		" if you have not given it an elf file with the --elf command line. [This can be"
		" convenient if you are bugtesting with a certain game and want to rebuild"
		" and retry it several times, either with changes to Dolphin or if you are"
		" developing a homebrew game.]")
		, wxITEM_CHECK);
	automaticstart->Check(bAutomaticStart);	

	_pMenu->Append(IDM_FONTPICKER, _T("&Font..."), wxEmptyString, wxITEM_NORMAL);
}

// CPU Mode and JIT Menu
void CCodeWindow::OnCPUMode(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case IDM_INTERPRETER:
			PowerPC::SetMode(UseInterpreter() ? PowerPC::MODE_INTERPRETER : PowerPC::MODE_JIT); break;

		case IDM_BOOTTOPAUSE:
			bBootToPause = !bBootToPause; return;
		case IDM_AUTOMATICSTART:
			bAutomaticStart = !bAutomaticStart; return;

#ifdef JIT_OFF_OPTIONS
		case IDM_JITOFF:
			Core::g_CoreStartupParameter.bJITOff = event.IsChecked(); break;
		case IDM_JITLSOFF:
			Core::g_CoreStartupParameter.bJITLoadStoreOff = event.IsChecked(); break;
			case IDM_JITLSLXZOFF:
				Core::g_CoreStartupParameter.bJITLoadStorelXzOff = event.IsChecked(); break;
				case IDM_JITLSLWZOFF:
					Core::g_CoreStartupParameter.bJITLoadStorelwzOff = event.IsChecked(); break;
			case IDM_JITLSLBZXOFF:
				Core::g_CoreStartupParameter.bJITLoadStorelbzxOff = event.IsChecked(); break;
		case IDM_JITLSFOFF:
			Core::g_CoreStartupParameter.bJITLoadStoreFloatingOff = event.IsChecked(); break;
		case IDM_JITLSPOFF:
			Core::g_CoreStartupParameter.bJITLoadStorePairedOff = event.IsChecked(); break;
		case IDM_JITFPOFF:
			Core::g_CoreStartupParameter.bJITFloatingPointOff = event.IsChecked(); break;
		case IDM_JITIOFF:
			Core::g_CoreStartupParameter.bJITIntegerOff = event.IsChecked(); break;
		case IDM_JITPOFF:
			Core::g_CoreStartupParameter.bJITPairedOff = event.IsChecked(); break;
		case IDM_JITSROFF:
			Core::g_CoreStartupParameter.bJITSystemRegistersOff = event.IsChecked(); break;	
#endif
	}

	// Clear the JIT cache to enable these changes
	jit->ClearCache();
	// Update
	UpdateButtonStates();
}
void CCodeWindow::OnJitMenu(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case IDM_LOGINSTRUCTIONS:
			PPCTables::LogCompiledInstructions(); break;

		case IDM_CLEARCODECACHE:
			jit->ClearCache(); break;

		case IDM_SEARCHINSTRUCTION:
		{
			wxString str;
			str = wxGetTextFromUser(_(""), wxT("Op?"), wxEmptyString, this);
			for (u32 addr = 0x80000000; addr < 0x80100000; addr += 4) {
				const char *name = PPCTables::GetInstructionName(Memory::ReadUnchecked_U32(addr));
				if (name && !strcmp((const char *)str.mb_str(), name))
					NOTICE_LOG(POWERPC, "Found %s at %08x", str.c_str(), addr);
			}
			break;
		}
	}
}


// Shortcuts
bool CCodeWindow::UseInterpreter()
{
	return GetMenuBar()->IsChecked(IDM_INTERPRETER);
}
bool CCodeWindow::BootToPause()
{
	return GetMenuBar()->IsChecked(IDM_BOOTTOPAUSE);
}
bool CCodeWindow::AutomaticStart()
{
	return GetMenuBar()->IsChecked(IDM_AUTOMATICSTART);
}
bool CCodeWindow::UnlimitedJITCache()
{
	return GetMenuBar()->IsChecked(IDM_JITUNLIMITED);
}
bool CCodeWindow::JITBlockLinking()
{
	return GetMenuBar()->IsChecked(IDM_JITBLOCKLINKING);
}


// Toolbar
void CCodeWindow::InitBitmaps()
{
	// load original size 48x48
	m_Bitmaps[Toolbar_DebugGo] = wxGetBitmapFromMemory(toolbar_play_png);
	m_Bitmaps[Toolbar_Step] = wxGetBitmapFromMemory(toolbar_add_breakpoint_png);
	m_Bitmaps[Toolbar_StepOver] = wxGetBitmapFromMemory(toolbar_add_memcheck_png);
	m_Bitmaps[Toolbar_Skip] = wxGetBitmapFromMemory(toolbar_add_memcheck_png);
	m_Bitmaps[Toolbar_GotoPC] = wxGetBitmapFromMemory(toolbar_add_memcheck_png);
	m_Bitmaps[Toolbar_SetPC] = wxGetBitmapFromMemory(toolbar_add_memcheck_png);
	m_Bitmaps[Toolbar_DebugPause] = wxGetBitmapFromMemory(toolbar_pause_png);

	// scale to 24x24 for toolbar
	for (size_t n = Toolbar_DebugGo; n < ToolbarDebugBitmapMax; n++)
	{
		m_Bitmaps[n] = wxBitmap(m_Bitmaps[n].ConvertToImage().Scale(24, 24));
	}
}


void CCodeWindow::PopulateToolbar(wxAuiToolBar* toolBar)
{
	int w = m_Bitmaps[Toolbar_DebugGo].GetWidth(),
		h = m_Bitmaps[Toolbar_DebugGo].GetHeight();

	toolBar->SetToolBitmapSize(wxSize(w, h));
	toolBar->AddTool(IDM_STEP,		_T("Step"),			m_Bitmaps[Toolbar_Step]);
	toolBar->AddTool(IDM_STEPOVER,	_T("Step Over"),    m_Bitmaps[Toolbar_StepOver]);
	toolBar->AddTool(IDM_SKIP,		_T("Skip"),			m_Bitmaps[Toolbar_Skip]);
	toolBar->AddSeparator();
	toolBar->AddTool(IDM_GOTOPC,    _T("Show PC"),		m_Bitmaps[Toolbar_GotoPC]);
	toolBar->AddTool(IDM_SETPC,		_T("Set PC"),		m_Bitmaps[Toolbar_SetPC]);
	toolBar->AddSeparator();
	toolBar->AddControl(new wxTextCtrl(toolBar, IDM_ADDRBOX, _T("")));

	// after adding the buttons to the toolbar, must call Realize() to reflect
	// the changes
	toolBar->Realize();
}

// Update GUI
void CCodeWindow::Update()
{
	if (!codeview) return;

	codeview->Refresh();
	UpdateCallstack();
	UpdateButtonStates();

	/* DO NOT Automatically show the current PC position when a breakpoint is hit or
	   when we pause SINCE THIS CAN BE CALLED AT OTHER TIMES TOO */
	//codeview->Center(PC);
}

void CCodeWindow::UpdateButtonStates()
{
	bool Initialized = (Core::GetState() != Core::CORE_UNINITIALIZED);
	//	bool Running = (Core::GetState() == Core::CORE_RUN);
	bool Pause = (Core::GetState() == Core::CORE_PAUSE);
	bool Stepping = CCPU::IsStepping();
	wxAuiToolBar* ToolBar = GetToolBar();

	// Toolbar
	// ------------------
	if (!ToolBar) return;

	if (!Initialized)
	{
		ToolBar->EnableTool(IDM_STEPOVER, false);
		ToolBar->EnableTool(IDM_SKIP, false);
	}
	else
	{
		if (!Stepping)
		{
			ToolBar->EnableTool(IDM_STEPOVER, false);
			ToolBar->EnableTool(IDM_SKIP, false);
		}
		else
		{
			ToolBar->EnableTool(IDM_STEPOVER, true);
			ToolBar->EnableTool(IDM_SKIP, true);
		}
	}

	ToolBar->EnableTool(IDM_STEP, Initialized && Stepping && UseInterpreter());	
	
	if (ToolBar) ToolBar->Realize();

	// Menu bar
	// ------------------
	GetMenuBar()->Enable(IDM_INTERPRETER, Pause); // CPU Mode

	GetMenuBar()->Enable(IDM_JITUNLIMITED, !Initialized);
#ifdef JIT_OFF_OPTIONS
	GetMenuBar()->Enable(IDM_JITOFF, Pause);
	GetMenuBar()->Enable(IDM_JITLSOFF, Pause);
	GetMenuBar()->Enable(IDM_JITLSLXZOFF, Pause);
	GetMenuBar()->Enable(IDM_JITLSLWZOFF, Pause);
	GetMenuBar()->Enable(IDM_JITLSLBZXOFF, Pause);
	GetMenuBar()->Enable(IDM_JITLSFOFF, Pause);
	GetMenuBar()->Enable(IDM_JITLSPOFF, Pause);
	GetMenuBar()->Enable(IDM_JITFPOFF, Pause);
	GetMenuBar()->Enable(IDM_JITIOFF, Pause);
	GetMenuBar()->Enable(IDM_JITPOFF, Pause);
	GetMenuBar()->Enable(IDM_JITSROFF, Pause);
#endif

	GetMenuBar()->Enable(IDM_CLEARCODECACHE, Pause); // JIT Menu
	GetMenuBar()->Enable(IDM_SEARCHINSTRUCTION, Initialized);

	GetMenuBar()->Enable(IDM_CLEARSYMBOLS, Initialized); // Symbols menu
	GetMenuBar()->Enable(IDM_SCANFUNCTIONS, Initialized);
	GetMenuBar()->Enable(IDM_LOADMAPFILE, Initialized);
	GetMenuBar()->Enable(IDM_SAVEMAPFILE, Initialized);
	GetMenuBar()->Enable(IDM_SAVEMAPFILEWITHCODES, Initialized);
	GetMenuBar()->Enable(IDM_CREATESIGNATUREFILE, Initialized);
    GetMenuBar()->Enable(IDM_RENAME_SYMBOLS, Initialized);
	GetMenuBar()->Enable(IDM_USESIGNATUREFILE, Initialized);
	GetMenuBar()->Enable(IDM_PATCHHLEFUNCTIONS, Initialized);

	// Update Fonts
	callstack->SetFont(DebuggerFont);
	symbols->SetFont(DebuggerFont);
	callers->SetFont(DebuggerFont);
	calls->SetFont(DebuggerFont);
}


// Show Tool Tip for menu items
void CCodeWindow::DoTip(wxString text)
{
	// Create a blank tooltip to clear the eventual old one
	static wxTipWindow *tw = NULL;
	if (tw)
	{
		tw->SetTipWindowPtr(NULL);
		tw->Close();
	}
	tw = NULL;

	// Don't make a new one for blank text
	if(text.empty()) return;

	tw = new wxTipWindow(this, text, 175, &tw);

	// Move it to the right
	#ifdef _WIN32
		POINT point;
		GetCursorPos(&point);		
		tw->SetPosition(wxPoint(point.x + 25, point.y));
	#endif
}

// See the comment under BEGIN_EVENT_TABLE for an explanation of why we use both these events.
void CCodeWindow::OnStatusBar(wxMenuEvent& event)
{
	event.Skip(); // This doesn't hurt right?

	/* We assume the debug build user don't need to see this all the time. And these tooltips
	   may not be entirely stable. So we leave them out of debug builds. I could for example
	   get it to crash at wxWindowBase::DoHitTest(), that may be fixed in wxWidgets 2.9.0. */
	#if !defined(_DEBUG) || defined(DEBUGFAST)
		//DoTip(pMenuBar->GetHelpString(event.GetId()));
	#endif
}
void CCodeWindow::OnStatusBar_(wxUpdateUIEvent& event)
{
	event.Skip();

	#if !defined(_DEBUG) || defined(DEBUGFAST)
		// The IDM_ADDRBOX id seems to come with this outside the toolbar
		//if(event.GetId() != IDM_ADDRBOX) DoTip(wxEmptyString);
	#endif
}

