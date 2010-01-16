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

#ifndef _DSP_DEBUGGER_LLE_H
#define _DSP_DEBUGGER_LLE_H

// general things
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include <wx/wx.h>
#include <wx/frame.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/statline.h>

#include "disassemble.h"
#include "DSPInterpreter.h"
#include "DSPMemoryMap.h"
#include "../DSPDebugInterface.h"

class DSPRegisterView;
class CCodeView;

class DSPDebuggerLLE : public wxFrame
{
public:
	DSPDebuggerLLE(wxWindow *parent,
		wxWindowID id = wxID_ANY,
		const wxString &title = wxT("DSP LLE Debugger"),
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_FRAME_STYLE);

	virtual ~DSPDebuggerLLE();

	void Refresh();

private:
	DECLARE_EVENT_TABLE();

	enum
	{
		// Toolbar
		ID_TOOLBAR = 1000,
		ID_RUNTOOL,
		ID_STEPTOOL,
		ID_SHOWPCTOOL,
		ID_ADDRBOX,
		ID_JUMPTOTOOL,
		ID_DISASMDUMPTOOL,
		ID_CHECK_ASSERTINT,
		ID_CHECK_HALT,
		ID_CHECK_INIT,
		ID_SYMBOLLIST,

		// Code view
		ID_CODEVIEW,

		// Register View
		ID_DSP_REGS,
	};

	// Disasm listctrl columns
	enum
	{
		COLUMN_BP,
		COLUMN_FUNCTION,
		COLUMN_ADDRESS,
		COLUMN_MNEMONIC,
		COLUMN_OPCODE,
		COLUMN_EXT,
		COLUMN_PARAM,
	};

	DSPDebugInterface debug_interface;
	u64 m_CachedStepCounter;

	// GUI updaters
	void UpdateDisAsmListView();
	void UpdateRegisterFlags();
	void UpdateSymbolMap();
	void UpdateState();

	// GUI items
	wxToolBar* m_Toolbar;
	CCodeView* m_CodeView;
	DSPRegisterView* m_Regs;
	wxListBox* m_SymbolList;

	void OnClose(wxCloseEvent& event);
	void OnChangeState(wxCommandEvent& event);
	void OnShowPC(wxCommandEvent& event);
	void OnRightClick(wxListEvent& event);
	void OnDoubleClick(wxListEvent& event);
	void OnAddrBoxChange(wxCommandEvent& event);
	void OnSymbolListChange(wxCommandEvent& event);

	bool JumpToAddress(u16 addr);

	void CreateGUIControls();
	void FocusOnPC();
	void UnselectAll();
};

#endif //_DSP_DEBUGGER_LLE_H
