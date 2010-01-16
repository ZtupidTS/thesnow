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

#include "Common.h" // Common
#include <iostream> // System
#include <fstream>
#include <sstream>

#include "DSPDebugWindow.h"
#include "DSPRegisterView.h"
#include "CodeView.h"
#include "../DSPSymbols.h"

// Define these here to avoid undefined symbols while still saving functionality
void Host_NotifyMapLoaded() {}
void Host_UpdateBreakPointView() {}

// Event table and class
BEGIN_EVENT_TABLE(DSPDebuggerLLE, wxFrame)	
	EVT_CLOSE(DSPDebuggerLLE::OnClose)

	EVT_MENU_RANGE(ID_RUNTOOL, ID_STEPTOOL, DSPDebuggerLLE::OnChangeState)
	EVT_MENU(ID_SHOWPCTOOL, DSPDebuggerLLE::OnShowPC)
	EVT_TEXT(ID_ADDRBOX,    DSPDebuggerLLE::OnAddrBoxChange)
    EVT_LISTBOX(ID_SYMBOLLIST,     DSPDebuggerLLE::OnSymbolListChange)
END_EVENT_TABLE()

DSPDebuggerLLE::DSPDebuggerLLE(wxWindow *parent, wxWindowID id, const wxString &title,
							   const wxPoint &position, const wxSize& size, long style)
							   : wxFrame(parent, id, title, position, size, style)
							   , m_CachedStepCounter(-1)
{
	CreateGUIControls();
}

DSPDebuggerLLE::~DSPDebuggerLLE()
{
}

void DSPDebuggerLLE::CreateGUIControls()
{
	// Basic settings
	SetSize(700, 800);
	this->SetSizeHints(700, 800);
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

	m_Toolbar = CreateToolBar(wxTB_NODIVIDER|wxTB_NOICONS|wxTB_HORZ_TEXT|wxTB_DOCKABLE, ID_TOOLBAR); 
	m_Toolbar->AddTool(ID_RUNTOOL, wxT("Run"), wxNullBitmap, wxEmptyString, wxITEM_NORMAL);
	m_Toolbar->AddTool(ID_STEPTOOL, wxT("Step"), wxNullBitmap, wxT("Step Code "), wxITEM_NORMAL);
	m_Toolbar->AddTool(ID_SHOWPCTOOL, wxT("Show Pc"), wxNullBitmap, wxT("Show where PC is"), wxITEM_NORMAL);
	m_Toolbar->AddTool(ID_JUMPTOTOOL, wxT("Jump"), wxNullBitmap, wxT("Jump to a specific Address"), wxITEM_NORMAL);
	m_Toolbar->AddSeparator();

	m_Toolbar->AddControl(new wxTextCtrl(m_Toolbar, ID_ADDRBOX, _T("")));

	m_Toolbar->Realize();

	wxBoxSizer* sMain = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerLeft  = new wxBoxSizer(wxVERTICAL);
	sizerLeft->Add(m_SymbolList = new wxListBox(this, ID_SYMBOLLIST, wxDefaultPosition, wxSize(140, 100), 0, NULL, wxLB_SORT),
		1, wxEXPAND);

	m_CodeView = new CCodeView(&debug_interface, &DSPSymbols::g_dsp_symbol_db, this, ID_CODEVIEW);
	m_CodeView->SetPlain();

	sMain->Add(sizerLeft, 0, wxEXPAND, 0);

	sMain->Add(m_CodeView, 4, wxEXPAND, 0);

	wxStaticLine* m_staticline = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
	sMain->Add(m_staticline, 0, wxEXPAND|wxALL, 0);

	m_Regs = new DSPRegisterView(this, ID_DSP_REGS);
	sMain->Add(m_Regs, 0, wxEXPAND|wxALL, 5);

	this->SetSizer(sMain);
	this->Layout();

	UpdateState();
}

void DSPDebuggerLLE::OnClose(wxCloseEvent& event)
{
	Hide();
}

void DSPDebuggerLLE::OnChangeState(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	case ID_RUNTOOL:
		if (DSPCore_GetState() == DSPCORE_RUNNING)
			DSPCore_SetState(DSPCORE_STEPPING);
		else
			DSPCore_SetState(DSPCORE_RUNNING);
		break;

	case ID_STEPTOOL:
		if (DSPCore_GetState() == DSPCORE_STEPPING)
		{
			DSPCore_Step();
			Refresh();
		}
		break;

	case ID_SHOWPCTOOL:
		FocusOnPC();
		break;
	}

	UpdateState();
}

void DSPDebuggerLLE::OnShowPC(wxCommandEvent& event)
{
	// UpdateDisAsmListView will focus on PC
	Refresh();
}

void DSPDebuggerLLE::Refresh()
{
	UpdateSymbolMap();
	UpdateDisAsmListView();
	UpdateRegisterFlags();
	UpdateState();
}

void DSPDebuggerLLE::FocusOnPC()
{
	JumpToAddress(g_dsp.pc);
}

void DSPDebuggerLLE::UpdateState()
{
	if (DSPCore_GetState() == DSPCORE_RUNNING) {
		m_Toolbar->FindById(ID_RUNTOOL)->SetLabel(wxT("Pause"));
		m_Toolbar->FindById(ID_STEPTOOL)->Enable(false);
	}
	else {
		m_Toolbar->FindById(ID_RUNTOOL)->SetLabel(wxT("Run"));
		m_Toolbar->FindById(ID_STEPTOOL)->Enable(true);
	}
	m_Toolbar->Realize();
}

void DSPDebuggerLLE::UpdateDisAsmListView()
{
	if (m_CachedStepCounter == g_dsp.step_counter)
		return;

	// show PC
	FocusOnPC();
	m_CachedStepCounter = g_dsp.step_counter;
	m_Regs->Update();
}

void DSPDebuggerLLE::UpdateSymbolMap()
{
	if (g_dsp.dram == NULL)
		return;

	m_SymbolList->Freeze();	// HyperIris: wx style fast filling
	m_SymbolList->Clear();
	for (SymbolDB::XFuncMap::iterator iter = DSPSymbols::g_dsp_symbol_db.GetIterator();
		 iter != DSPSymbols::g_dsp_symbol_db.End(); iter++)
	{
		int idx = m_SymbolList->Append(wxString::FromAscii(iter->second.name.c_str()));
		m_SymbolList->SetClientData(idx, (void*)&iter->second);
	}
	m_SymbolList->Thaw();
}

void DSPDebuggerLLE::OnSymbolListChange(wxCommandEvent& event)
{
	int index = m_SymbolList->GetSelection();
	if (index >= 0) {
		Symbol* pSymbol = static_cast<Symbol *>(m_SymbolList->GetClientData(index));
		if (pSymbol != NULL)
		{
			if (pSymbol->type == Symbol::SYMBOL_FUNCTION)
			{
				JumpToAddress(pSymbol->address);
			}
		}
	}
}

void DSPDebuggerLLE::UpdateRegisterFlags()
{

}

void DSPDebuggerLLE::OnAddrBoxChange(wxCommandEvent& event)
{
	wxTextCtrl* pAddrCtrl = (wxTextCtrl*)GetToolBar()->FindControl(ID_ADDRBOX);
	wxString txt = pAddrCtrl->GetValue();

	std::string text(txt.mb_str());
	text = StripSpaces(text);
	if (text.size())
	{
		u32 addr;
		sscanf(text.c_str(), "%04x", &addr);
		if (JumpToAddress(addr))
			pAddrCtrl->SetBackgroundColour(*wxWHITE);
		else
			pAddrCtrl->SetBackgroundColour(*wxRED);
	}
	event.Skip(1);
}

bool DSPDebuggerLLE::JumpToAddress(u16 addr)
{
	int new_line = DSPSymbols::Addr2Line(addr);
	if (new_line >= 0) {
		m_CodeView->Center(new_line);
		return true;
	} else {
		return false;
	}
}
