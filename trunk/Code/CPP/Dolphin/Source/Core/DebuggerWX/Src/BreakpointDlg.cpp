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

#include "Common.h"
#include "Host.h"
#include "Debugger.h"
#include "StringUtil.h"
#include "PowerPC/PowerPC.h"
#include "BreakpointWindow.h"
#include "BreakpointDlg.h"

BEGIN_EVENT_TABLE(BreakPointDlg,wxDialog)
	EVT_CLOSE(BreakPointDlg::OnClose)
	EVT_BUTTON(ID_OK, BreakPointDlg::OnOK)
	EVT_BUTTON(ID_CANCEL, BreakPointDlg::OnCancel)
END_EVENT_TABLE()

class CBreakPointWindow;

BreakPointDlg::BreakPointDlg(CBreakPointWindow *_Parent, wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxDialog(parent, id, title, position, size, style)
, Parent(_Parent)
{
	CreateGUIControls();
}


BreakPointDlg::~BreakPointDlg()
{
} 


void BreakPointDlg::CreateGUIControls()
{
	SetIcon(wxNullIcon);
	SetSize(8,8,279,121);
	Center();	


	m_pButtonOK = new wxButton(this, ID_OK, wxT("OK"), wxPoint(192,64), wxSize(73,25), 0, wxDefaultValidator, wxT("OK"));

	m_pButtonCancel = new wxButton(this, ID_CANCEL, wxT("Cancel"), wxPoint(112,64), wxSize(73,25), 0, wxDefaultValidator, wxT("Cancel"));

	m_pEditAddress = new wxTextCtrl(this, ID_ADDRESS, wxT("80000000"), wxPoint(56,24), wxSize(197,20), 0, wxDefaultValidator, wxT("WxEdit1"));
}


void BreakPointDlg::OnClose(wxCloseEvent& /*event*/)
{
	Destroy();
}

void BreakPointDlg::OnOK(wxCommandEvent& /*event*/)
{
	wxString AddressString = m_pEditAddress->GetLineText(0);
	u32 Address = 0;
	if (AsciiToHex(AddressString.mb_str(), Address))
	{
		PowerPC::breakpoints.Add(Address);
		Parent->NotifyUpdate();
		//Host_UpdateBreakPointView();
		Close();		
	}
}

void BreakPointDlg::OnCancel(wxCommandEvent& /*event*/)
{
	Close();
}
