﻿// Copyright (C) 2003 Dolphin Project.

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
#include "AboutDolphin.h"
#include "CPUDetect.h"
#include "../resources/dolphin_logo.cpp"

BEGIN_EVENT_TABLE(AboutDolphin, wxDialog)
	EVT_CLOSE(AboutDolphin::OnClose)
	EVT_BUTTON(wxID_CLOSE, AboutDolphin::CloseClick)
END_EVENT_TABLE()

AboutDolphin::AboutDolphin(wxWindow *parent, wxWindowID id,
				const wxString &title, const wxPoint &position,
				const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

AboutDolphin::~AboutDolphin()
{
}

void AboutDolphin::CreateGUIControls()
{
	m_Close = new wxButton(this, wxID_CLOSE, _("关闭"),
		wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	wxMemoryInputStream istream(dolphin_logo_png, sizeof dolphin_logo_png);
	wxImage iDolphinLogo(istream, wxBITMAP_TYPE_PNG);
	sbDolphinLogo = new wxStaticBitmap(this, ID_LOGO,
		wxBitmap(iDolphinLogo), wxDefaultPosition, wxDefaultSize, 0);

	std::string Text = std::string("") +
		"Dolphin SVN revision " + svn_rev_str + "\n"
		"Copyright (c) 2003-2010+ Dolphin Team\n"
		"\n"
		"Dolphin is a Gamecube/Wii emulator, which was\n"
		"originally written by F|RES and ector.\n"
		"Today Dolphin is an open source project with many\n"
		"contributors, too many to list.\n"
		"If interested, just go check out the project page at\n"
		"http://code.google.com/p/dolphin-emu/ .\n"
		"\n"
		"Special thanks to Bushing, Costis, CrowTRobo,\n"
		"Marcan, Segher, Titanik, or9 and Hotquik for their\n"
		"reverse engineering and docs/demos.\n"
		"\n"
		"Big thanks to Gilles Mouchard whose Microlib PPC\n"
		"emulator gave our development a kickstart.\n"
		"\n"
		"Thanks to Frank Wille for his PowerPC disassembler,\n"
		"which or9 and we modified to include Gekko specifics.\n"
		"\n"
		"Thanks to hcs/destop for their GC ADPCM decoder.\n"
		"\n"
		"We are not affiliated with Nintendo in any way.\n"
		"Gamecube and Wii are trademarks of Nintendo.\n"
		"The emulator is for educational purposes only\n"
		"and should not be used to play games you do\n"
		"not legally own.\n\n\n\nThis Chinese version was based on Dolphin";
	Message = new wxStaticText(this, ID_MESSAGE,
		wxString::FromAscii(Text.c_str()),
		wxDefaultPosition, wxDefaultSize, 0);
	Message->Wrap(this->GetSize().GetWidth());

	sMain = new wxBoxSizer(wxVERTICAL);
	sMainHor = new wxBoxSizer(wxHORIZONTAL);
	sMainHor->Add(sbDolphinLogo);

	sInfo = new wxBoxSizer(wxVERTICAL);
	sInfo->Add(Message, 1, wxEXPAND|wxALL, 5);
	sMainHor->Add(sInfo);
	sMain->Add(sMainHor, 1, wxEXPAND);

	sButtons = new wxBoxSizer(wxHORIZONTAL);
	sButtons->Add(0, 0, 1, wxEXPAND, 5);
	sButtons->Add(m_Close, 0, wxALL, 5);
	sMain->Add(sButtons, 0, wxEXPAND);

	this->SetSizer(sMain);
	sMain->Layout();

	Fit();
	CenterOnParent();
}

void AboutDolphin::OnClose(wxCloseEvent& WXUNUSED (event))
{
	EndModal(wxID_OK);
}

void AboutDolphin::CloseClick(wxCommandEvent& WXUNUSED (event))
{
	Close();
}
