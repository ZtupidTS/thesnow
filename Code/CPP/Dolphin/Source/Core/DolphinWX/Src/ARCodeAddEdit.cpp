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

#include "ARCodeAddEdit.h"
#include "ARDecrypt.h"

extern std::vector<ActionReplay::ARCode> arCodes;

BEGIN_EVENT_TABLE(CARCodeAddEdit, wxDialog)
	EVT_CLOSE(CARCodeAddEdit::OnClose)
	EVT_BUTTON(wxID_OK, CARCodeAddEdit::SaveCheatData)
	EVT_SPIN(ID_ENTRY_SELECT, CARCodeAddEdit::ChangeEntry)
END_EVENT_TABLE()

CARCodeAddEdit::CARCodeAddEdit(int _selection, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	selection  = _selection;
	CreateGUIControls(selection);
}

CARCodeAddEdit::~CARCodeAddEdit()
{
}

void CARCodeAddEdit::CreateGUIControls(int _selection)
{
	ActionReplay::ARCode tempEntries;
	wxString currentName = _("Insert name here..");
	
	if (_selection == -1)
	{
		tempEntries.name = "";
	}
	else
	{
	    currentName = wxString(arCodes.at(_selection).name.c_str(), *wxConvCurrent);
	    tempEntries = arCodes.at(_selection);
	}

	wxBoxSizer* sEditCheat = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sEditCheatButtons = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer* sbEntry = new wxStaticBoxSizer(wxVERTICAL, this, _("Cheat Code"));
	wxGridBagSizer* sgEntry = new wxGridBagSizer(0, 0);

	wxStaticText* EditCheatNameText = new wxStaticText(this, ID_EDITCHEAT_NAME_TEXT, _("名称:"), wxDefaultPosition, wxDefaultSize);
	EditCheatName = new wxTextCtrl(this, ID_EDITCHEAT_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	EditCheatName->SetValue(currentName);
	EntrySelection = new wxSpinButton(this, ID_ENTRY_SELECT, wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	EntrySelection->SetRange(1, ((int)arCodes.size()) > 0 ? (int)arCodes.size() : 1); 
	EntrySelection->SetValue((int)(arCodes.size() - _selection));
	EditCheatCode = new wxTextCtrl(this, ID_EDITCHEAT_CODE, wxEmptyString, wxDefaultPosition, wxSize(300, 100), wxTE_MULTILINE);
	UpdateTextCtrl(tempEntries);
	wxButton* bOK = new wxButton(this, wxID_OK, _("确定"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	wxButton* bCancel = new wxButton(this, wxID_CANCEL, _("取消"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	sgEntry->Add(EditCheatNameText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER|wxALL, 5);
	sgEntry->Add(EditCheatName,		wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sgEntry->Add(EntrySelection,	wxGBPosition(0, 2), wxGBSpan(2, 1), wxEXPAND|wxALL, 5);
	sgEntry->Add(EditCheatCode,		wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND|wxALL, 5);
	sgEntry->AddGrowableCol(1);
	sgEntry->AddGrowableRow(1);
	sbEntry->Add(sgEntry, 1, wxEXPAND|wxALL);

	sEditCheatButtons->AddStretchSpacer();
	sEditCheatButtons->Add(bOK, 0, wxALL, 5);
	sEditCheatButtons->Add(bCancel, 0, wxALL, 5);

	sEditCheat->Add(sbEntry, 1, wxEXPAND|wxALL, 5);
	sEditCheat->Add(sEditCheatButtons, 0, wxEXPAND, 5);

	SetSizerAndFit(sEditCheat);
}

void CARCodeAddEdit::OnClose(wxCloseEvent& WXUNUSED (event))
{
	Destroy();
}

void CARCodeAddEdit::ChangeEntry(wxSpinEvent& event)
{
	ActionReplay::ARCode currentCode = arCodes.at((int)arCodes.size() - event.GetPosition());
	EditCheatName->SetValue(wxString(currentCode.name.c_str(), *wxConvCurrent));
	UpdateTextCtrl(currentCode);
}

void CARCodeAddEdit::SaveCheatData(wxCommandEvent& WXUNUSED (event))
{
	std::vector<ActionReplay::AREntry> tempEntries;
	std::vector<std::string> encryptedLine;
	std::string cheatValues = std::string(EditCheatCode->GetValue().mb_str());
	size_t line = 0;

	// there's no newline, or newline is imcomplete, stop here.
	while (line != std::string::npos)
	{
		if (line > 0) line++;
		std::vector<std::string> pieces;
		std::string line_str = cheatValues.substr(line, cheatValues.find('\n', line) - line);

		SplitString(line_str, ' ', pieces);
		
		if (pieces.size() == 2 && pieces[0].size() == 8 && pieces[1].size() == 8)
		{
			u32 addr = strtoul(pieces[0].c_str(), NULL, 16);
			u32 value = strtoul(pieces[1].c_str(), NULL, 16);
			// Decrypted code
			tempEntries.push_back(ActionReplay::AREntry(addr, value));
		}
		else
		{
			SplitString(line_str, '-', pieces);
			
			if (pieces.size() == 3 && pieces[0].size() == 4 && pieces[1].size() == 4 && pieces[2].size() == 5) 
			{
				// Encrypted code
				encryptedLine.push_back(pieces[0]+pieces[1]+pieces[2]);
			}
		}
		
		if ((line = cheatValues.find('\n', line)) == std::string::npos && encryptedLine.size())
		{
			ActionReplay::DecryptARCode(encryptedLine, tempEntries);
		}
	}

	if (selection == -1)
	{
		ActionReplay::ARCode newCheat;

		newCheat.name = std::string(EditCheatName->GetValue().mb_str());
		newCheat.ops = tempEntries;
		newCheat.active = true;

		arCodes.push_back(newCheat);
	}
	else
	{
		arCodes.at(selection).name = std::string(EditCheatName->GetValue().mb_str());
		arCodes.at(selection).ops = tempEntries;
	}

	AcceptAndClose();
}

void CARCodeAddEdit::UpdateTextCtrl(ActionReplay::ARCode arCode)
{
	EditCheatCode->Clear();

	if (arCode.name != "")
		for (u32 i = 0; i < arCode.ops.size(); i++)
			EditCheatCode->AppendText(wxString::Format(wxT("%08X %08X\n"), arCode.ops.at(i).cmd_addr, arCode.ops.at(i).value));
	else
		EditCheatCode->SetValue(_("Insert Encrypted or Decrypted code here..."));
}
