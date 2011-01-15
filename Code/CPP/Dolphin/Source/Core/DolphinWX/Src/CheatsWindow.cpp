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

#include "Globals.h"
#include "CheatsWindow.h"
#include "ActionReplay.h"
#include "Core.h"
#include "ConfigManager.h"
#include "VolumeHandler.h"
#include "ISOProperties.h"
#include "HW/Memmap.h"

#define _connect_macro_(b, f, c, s)	(b)->Connect(wxID_ANY, (c), wxCommandEventHandler(f), (wxObject*)0, (wxEvtHandler*)s)

#define MAX_CHEAT_SEARCH_RESULTS_DISPLAY	256

extern std::vector<ActionReplay::ARCode> arCodes;

// meh
static wxCheatsWindow *g_cheat_window;

wxCheatsWindow::wxCheatsWindow(wxWindow* const parent)
	: wxFrame(parent, wxID_ANY, _("Cheats Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE)
{
	::g_cheat_window = this;

	// Create the GUI controls
	Init_ChildControls();

	// Load Data
	Load_ARCodes();

	// Load Gecko Codes :/
	{
	const DiscIO::IVolume* const vol = VolumeHandler::GetVolume();
	if (vol)
	{
		m_gameini_path = std::string(File::GetUserPath(D_GAMECONFIG_IDX)) + vol->GetUniqueID() + ".ini";
		m_gameini.Load(m_gameini_path);
		m_geckocode_panel->LoadCodes(m_gameini);
	}
	}

	Center();
	Show();
}

void wxCheatsWindow::Init_ChildControls()
{
	wxPanel* const panel = new wxPanel(this);

	// Main Notebook
	m_Notebook_Main = new wxNotebook(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	// --- Tabs ---
	// $ Cheats List Tab
	m_Tab_Cheats = new wxPanel(m_Notebook_Main, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	m_CheckListBox_CheatsList = new wxCheckListBox(m_Tab_Cheats, wxID_ANY, wxDefaultPosition, wxSize(300, 0), m_CheatStringList, wxLB_HSCROLL, wxDefaultValidator);
	_connect_macro_(m_CheckListBox_CheatsList, wxCheatsWindow::OnEvent_CheatsList_ItemSelected, wxEVT_COMMAND_LISTBOX_SELECTED, this);
	_connect_macro_(m_CheckListBox_CheatsList, wxCheatsWindow::OnEvent_CheatsList_ItemToggled, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, this);

	m_Label_Codename = new wxStaticText(m_Tab_Cheats, wxID_ANY, _("Name: "), wxDefaultPosition, wxDefaultSize);
	m_GroupBox_Info = new wxStaticBox(m_Tab_Cheats, wxID_ANY, _("Code Info"), wxDefaultPosition, wxDefaultSize);

	m_Label_NumCodes = new wxStaticText(m_Tab_Cheats, wxID_ANY, _("Number Of Codes: "),  wxDefaultPosition, wxDefaultSize);
	m_ListBox_CodesList = new wxListBox(m_Tab_Cheats, wxID_ANY, wxDefaultPosition, wxSize(120, 150), 0, 0, wxLB_HSCROLL);

	wxStaticBoxSizer* sGroupBoxInfo = new wxStaticBoxSizer(m_GroupBox_Info, wxVERTICAL);
	sGroupBoxInfo->Add(m_Label_Codename, 0, wxALL, 5);
	sGroupBoxInfo->Add(m_Label_NumCodes, 0, wxALL, 5);
	sGroupBoxInfo->Add(m_ListBox_CodesList, 1, wxALL, 5);

	wxBoxSizer* sizer_tab_cheats = new wxBoxSizer(wxHORIZONTAL);
	sizer_tab_cheats->Add(m_CheckListBox_CheatsList, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT, 10);
	sizer_tab_cheats->Add(sGroupBoxInfo, 0, wxALIGN_LEFT | wxEXPAND | wxALL, 5);

	m_Tab_Cheats->SetSizerAndFit(sizer_tab_cheats);	

	// $ Cheat Search Tab
	wxPanel* const tab_cheat_search = new CheatSearchTab(m_Notebook_Main);

	// $ Log Tab
	m_Tab_Log = new wxPanel(m_Notebook_Main, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	wxButton* const button_updatelog = new wxButton(m_Tab_Log, wxID_ANY, _("Update"));
	_connect_macro_(button_updatelog, wxCheatsWindow::OnEvent_ButtonUpdateLog_Press, wxEVT_COMMAND_BUTTON_CLICKED, this);

	m_CheckBox_LogAR = new wxCheckBox(m_Tab_Log, wxID_ANY, _("Enable AR Logging"));
	_connect_macro_(m_CheckBox_LogAR, wxCheatsWindow::OnEvent_CheckBoxEnableLogging_StateChange, wxEVT_COMMAND_CHECKBOX_CLICKED, this);

	m_CheckBox_LogAR->SetValue(ActionReplay::IsSelfLogging());
	m_TextCtrl_Log = new wxTextCtrl(m_Tab_Log, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(100, -1), wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);

	wxBoxSizer *HStrip1 = new wxBoxSizer(wxHORIZONTAL);
	HStrip1->Add(m_CheckBox_LogAR, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
	HStrip1->Add(button_updatelog, 0, wxALL, 5);

	wxBoxSizer *sTabLog = new wxBoxSizer(wxVERTICAL);
	sTabLog->Add(HStrip1, 0, wxALL, 5);
	sTabLog->Add(m_TextCtrl_Log, 1, wxALL|wxEXPAND, 5);

	m_Tab_Log->SetSizerAndFit(sTabLog);

	// Add Tabs to Notebook
	m_Notebook_Main->AddPage(m_Tab_Cheats, _("AR Codes"));
	m_geckocode_panel = new Gecko::CodeConfigPanel(m_Notebook_Main);
	m_Notebook_Main->AddPage(m_geckocode_panel, _("Gecko Codes"));
	m_Notebook_Main->AddPage(tab_cheat_search, _("Cheat Search"));
	m_Notebook_Main->AddPage(m_Tab_Log, _("记录"));

	// Button Strip
	wxButton* const button_apply = new wxButton(panel, wxID_ANY, _("应用"), wxDefaultPosition, wxDefaultSize);
	_connect_macro_(button_apply, wxCheatsWindow::OnEvent_ApplyChanges_Press, wxEVT_COMMAND_BUTTON_CLICKED, this);
	wxButton* const button_close = new wxButton(panel, wxID_ANY, _("关闭"), wxDefaultPosition, wxDefaultSize);
	_connect_macro_(button_close, wxCheatsWindow::OnEvent_ButtonClose_Press, wxEVT_COMMAND_BUTTON_CLICKED, this);
	wxBoxSizer* sButtons = new wxBoxSizer(wxHORIZONTAL);
	sButtons->Add(button_apply, 1, wxRIGHT, 5);
	sButtons->Add(button_close, 1, 0, 0);

	wxBoxSizer* const sMain = new wxBoxSizer(wxVERTICAL);
	sMain->Add(m_Notebook_Main, 1, wxEXPAND|wxALL, 5);
	sMain->Add(sButtons, 0, wxRIGHT | wxBOTTOM | wxALIGN_RIGHT, 5);
	panel->SetSizerAndFit(sMain);

	wxBoxSizer* const frame_szr = new wxBoxSizer(wxVERTICAL);
	frame_szr->Add(panel, 1, wxEXPAND);
	SetSizerAndFit(frame_szr);
}

CheatSearchTab::CheatSearchTab(wxWindow* const parent)
	: wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize)
{
	// first scan button
	btnInitScan = new wxButton(this, -1, _("New Scan"));
	_connect_macro_(btnInitScan, CheatSearchTab::StartNewSearch, wxEVT_COMMAND_BUTTON_CLICKED, this);

	// next scan button
	btnNextScan = new wxButton(this, -1, _("Next Scan"));
	_connect_macro_(btnNextScan, CheatSearchTab::FilterCheatSearchResults, wxEVT_COMMAND_BUTTON_CLICKED, this);
	btnNextScan->Disable();

	// data size radio buttons
	size_radiobtn.rad_8 = new wxRadioButton(this, -1, _("8 bit"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	size_radiobtn.rad_16 = new wxRadioButton(this, -1, _("16 bit"));
	size_radiobtn.rad_32 = new wxRadioButton(this, -1, _("32 bit"));
	size_radiobtn.rad_8->SetValue(true);

	// data sizes groupbox
	wxStaticBoxSizer* const sizer_cheat_new_search = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Data Size"));
	sizer_cheat_new_search->Add(size_radiobtn.rad_8, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 5);
	sizer_cheat_new_search->Add(size_radiobtn.rad_16, 0, wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 5);
	sizer_cheat_new_search->Add(size_radiobtn.rad_32, 0, wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 5);

	// result controls
	lbox_search_results = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize);
	label_results_count = new wxStaticText(this, -1, _("Count:"));

	// create AR code button
	wxButton* const button_cheat_search_copy_address = new wxButton(this, -1, _("Create AR Code"));
	_connect_macro_(button_cheat_search_copy_address, CheatSearchTab::CreateARCode, wxEVT_COMMAND_BUTTON_CLICKED, this);

	// results groupbox
	wxStaticBoxSizer* const sizer_cheat_search_results = new wxStaticBoxSizer(wxVERTICAL, this, _("Results"));
	sizer_cheat_search_results->Add(label_results_count, 0, wxALIGN_LEFT | wxALL, 5);
	sizer_cheat_search_results->Add(lbox_search_results, 1, wxEXPAND | wxALL, 5);
	sizer_cheat_search_results->Add(button_cheat_search_copy_address, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

	// Search value radio buttons
	value_x_radiobtn.rad_oldvalue = new wxRadioButton(this, -1, _("Previous Value"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	value_x_radiobtn.rad_uservalue = new wxRadioButton(this, -1, wxT(""));
	value_x_radiobtn.rad_oldvalue->SetValue(true);

	// search value textbox
	textctrl_value_x = new wxTextCtrl(this, -1, wxT("0x0"), wxDefaultPosition, wxSize(96,-1));
	_connect_macro_(textctrl_value_x, CheatSearchTab::ApplyFocus, wxEVT_SET_FOCUS, this);

	wxBoxSizer* const sizer_cheat_filter_text = new wxBoxSizer(wxHORIZONTAL);
	sizer_cheat_filter_text->Add(value_x_radiobtn.rad_uservalue, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
	sizer_cheat_filter_text->Add(textctrl_value_x, 1, wxALIGN_CENTER_VERTICAL, 5);

	// value groupbox
	wxStaticBoxSizer* const sizer_cheat_search_filter_x = new wxStaticBoxSizer(wxVERTICAL, this, _("Value"));
	sizer_cheat_search_filter_x->Add(value_x_radiobtn.rad_oldvalue, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sizer_cheat_search_filter_x->Add(sizer_cheat_filter_text, 0, wxALL | wxEXPAND, 5);

	// filter types in the compare dropdown
	static const wxString searches[] = {
		_("Unknown"),
		_("Not Equal"),
		_("Equal"),
		_("Greater Than"),
		_("Less Than"),
	// TODO: Implement between search.
		//_("Between"),
	};

	search_type = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, sizeof(searches)/sizeof(*searches), searches);
	search_type->Select(0);

	wxStaticBoxSizer* const sizer_cheat_search_filter = new wxStaticBoxSizer(wxVERTICAL, this, _("Search Filter"));
	sizer_cheat_search_filter->Add(sizer_cheat_search_filter_x, 0, wxALL | wxEXPAND, 5);
	sizer_cheat_search_filter->Add(search_type, 0, wxALL, 5);

	// left sizer
	wxBoxSizer* const sizer_left = new wxBoxSizer(wxVERTICAL);
	sizer_left->Add(sizer_cheat_search_results, 1, wxEXPAND, 5);

	// button sizer
	wxBoxSizer* boxButtons = new wxBoxSizer(wxHORIZONTAL);
	boxButtons->Add(btnInitScan, 1, wxRIGHT, 5);
	boxButtons->Add(btnNextScan, 1);

	// right sizer
	wxBoxSizer* const sizer_right = new wxBoxSizer(wxVERTICAL);
	sizer_right->Add(sizer_cheat_new_search, 0, wxBOTTOM, 5);
	sizer_right->Add(sizer_cheat_search_filter, 0, wxEXPAND | wxBOTTOM, 5);
	sizer_right->AddStretchSpacer(1);
	sizer_right->Add(boxButtons, 0, wxTOP | wxEXPAND, 5);

	// main sizer
	wxBoxSizer* const sizer_main = new wxBoxSizer(wxHORIZONTAL);
	sizer_main->Add(sizer_left, 1, wxEXPAND | wxALL, 5);
	sizer_main->Add(sizer_right, 0, wxEXPAND | wxALL, 5);

	SetSizerAndFit(sizer_main);
}

void wxCheatsWindow::OnEvent_ButtonClose_Press(wxCommandEvent& WXUNUSED (event))
{
	Destroy();
}

void wxCheatsWindow::Load_ARCodes()
{
	using namespace ActionReplay;

	m_CheckListBox_CheatsList->Clear();

	indexList.clear();
	size_t size = GetCodeListSize();
	for (size_t i = 0; i < size; i++)
	{
		ARCode code = GetARCode(i);
		ARCodeIndex ind;
		u32 index = m_CheckListBox_CheatsList->Append(wxString(code.name.c_str(), *wxConvCurrent));
		m_CheckListBox_CheatsList->Check(index, code.active);
		ind.index = i;
		ind.uiIndex = index;
		indexList.push_back(ind);
	}
}

void wxCheatsWindow::OnEvent_CheatsList_ItemSelected(wxCommandEvent& WXUNUSED (event))
{
	using namespace ActionReplay;

	int index = m_CheckListBox_CheatsList->GetSelection();
	for (size_t i = 0; i < indexList.size(); i++)
	{
		if ((int)indexList[i].uiIndex == index)
		{
			ARCode code = GetARCode(i);
			m_Label_Codename->SetLabel(_("名称: ") + wxString(code.name.c_str(), *wxConvCurrent));
			char text[CHAR_MAX];
			char* numcodes = text;
			sprintf(numcodes, "Number of Codes: %lu", (unsigned long)code.ops.size());
			m_Label_NumCodes->SetLabel(wxString::FromAscii(numcodes));
			m_ListBox_CodesList->Clear();
			for (size_t j = 0; j < code.ops.size(); j++)
			{
				char text2[CHAR_MAX];
			    char* ops = text2;
				sprintf(ops, "%08x %08x", code.ops[j].cmd_addr, code.ops[j].value);
				m_ListBox_CodesList->Append(wxString::FromAscii(ops));
			}
		}
	}
}

void wxCheatsWindow::OnEvent_CheatsList_ItemToggled(wxCommandEvent& WXUNUSED (event))
{
	int index = m_CheckListBox_CheatsList->GetSelection();
	for (size_t i = 0; i < indexList.size(); i++)
	{
		if ((int)indexList[i].uiIndex == index)
		{
			ActionReplay::SetARCode_IsActive(m_CheckListBox_CheatsList->IsChecked(index), indexList[i].index);
		}
	}
}

void wxCheatsWindow::OnEvent_ApplyChanges_Press(wxCommandEvent& WXUNUSED (event))
{
	// Appply AR Code changes
	for (size_t i = 0; i < indexList.size(); i++)
	{
		ActionReplay::SetARCode_IsActive(m_CheckListBox_CheatsList->IsChecked(indexList[i].uiIndex), indexList[i].index);
	}

	// Apply Gecko Code changes
	Gecko::SetActiveCodes(m_geckocode_panel->GetCodes());

	// save gameini, with changed gecko codes
	if (m_gameini_path.size())
	{
		Gecko::SaveCodes(m_gameini, m_geckocode_panel->GetCodes());
		m_gameini.Save(m_gameini_path);
	}
}

void wxCheatsWindow::OnEvent_ButtonUpdateLog_Press(wxCommandEvent& WXUNUSED (event))
{
	m_TextCtrl_Log->Clear();
	const std::vector<std::string> &arLog = ActionReplay::GetSelfLog();
	for (u32 i = 0; i < arLog.size(); i++)
	{
		m_TextCtrl_Log->AppendText(wxString::FromAscii(arLog[i].c_str()));
	}
}

void wxCheatsWindow::OnEvent_CheckBoxEnableLogging_StateChange(wxCommandEvent& WXUNUSED (event))
{
	ActionReplay::EnableSelfLogging(m_CheckBox_LogAR->IsChecked());
}

void CheatSearchTab::StartNewSearch(wxCommandEvent& WXUNUSED (event))
{
	search_results.clear();

	const u8* const memptr = Memory::GetPointer(0);
	if (NULL == memptr)
	{
		PanicAlertT("A game is not currently running.");
	}
	else
	{
		// enable the next scan button
		btnNextScan->Enable();

		// determine the search data size
		search_type_size =
			size_radiobtn.rad_8->GetValue() +
			(size_radiobtn.rad_16->GetValue() << 1) +
			(size_radiobtn.rad_32->GetValue() << 2);

		CheatSearchResult r;
		// can I assume cheatable values will be aligned like this?
		for (u32 addr = 0; addr != Memory::RAM_SIZE; addr += search_type_size)
		{
			r.address = addr;
			memcpy(&r.old_value, memptr + addr, search_type_size);
			search_results.push_back(r);
		}
	}

	UpdateCheatSearchResultsList();
}

void CheatSearchTab::FilterCheatSearchResults(wxCommandEvent&)
{
	const u8* const memptr = Memory::GetPointer(0);
	if (NULL == memptr)
	{
		PanicAlertT("A game is not currently running.");
	}
	else
	{
		std::vector<CheatSearchResult>::iterator
			i = search_results.begin(),
			e = search_results.end();
		std::vector<CheatSearchResult>	filtered_results;

		int filter_mask = 0;
		// determine the selected filter
		// 1 : equal
		// 2 : greater-than
		// 4 : less-than

		const int filters[] = {7, 6, 1, 2, 4};
		filter_mask = filters[search_type->GetSelection()];

		if (value_x_radiobtn.rad_oldvalue->GetValue())	// using old value comparison
		{
			for (; i!=e; ++i)
			{
				// with big endian, can just use memcmp for ><= comparison
				int cmp_result = memcmp(memptr + i->address, &i->old_value, search_type_size);
				if (cmp_result < 0)
					cmp_result = 4;
				else
					cmp_result = cmp_result ? 2 : 1;

				if (cmp_result & filter_mask)
				{
					memcpy(&i->old_value, memptr + i->address, search_type_size);
					filtered_results.push_back(*i);
				}
			}
		}
		else	// using user entered x value comparison
		{
			u32 user_x_val;

			// parse the user entered x value
			if (filter_mask != 7) // don't need the value for the "None" filter
			{
				long parsed_x_val = 0;
				int val_base = 10;

				wxString x_val = textctrl_value_x->GetLabel();
				if (wxT("0x") == x_val.substr(0,2))
				{
					//x_val = x_val.substr(2);	// wxwidgets seems fine parsing a "0x0000" string
					val_base = 16;
				}

				if (!x_val.ToLong(&parsed_x_val, val_base))
				{
					PanicAlertT("You must enter a valid decimal or hex value.");
					return;
				}

				user_x_val = (u32)parsed_x_val;

				// #ifdef LIL_ENDIAN :p
				switch (search_type_size)
				{
				case 1 :
					break;
				case 2 :
					*(u16*)&user_x_val = Common::swap16((u8*)&user_x_val);
					break;
				case 4 :
					user_x_val = Common::swap32(user_x_val);
					break;
				}
				// #elseif BIG_ENDIAN
				// would have to move <u32 vals (8/16bit) to start of the user_x_val for the comparisons i use below
				// #endif
			}

			for (; i!=e; ++i)
			{
				// with big endian, can just use memcmp for ><= comparison
				int cmp_result = memcmp(memptr + i->address, &user_x_val, search_type_size);
				if (cmp_result < 0)
					cmp_result = 4;
				else if (cmp_result)
					cmp_result = 2;
				else
					cmp_result = 1;

				if (cmp_result & filter_mask)
				{
					memcpy(&i->old_value, memptr + i->address, search_type_size);
					filtered_results.push_back(*i);
				}
			}
		}

		search_results.swap(filtered_results);

		UpdateCheatSearchResultsList();
	}
}

void CheatSearchTab::ApplyFocus(wxCommandEvent&)
{
	value_x_radiobtn.rad_uservalue->SetValue(true);
}

void CheatSearchTab::UpdateCheatSearchResultsList()
{
	lbox_search_results->Clear();

	wxString count_label = _("Count:") + wxString::Format(wxT(" %i"), search_results.size());
	if (search_results.size() > MAX_CHEAT_SEARCH_RESULTS_DISPLAY)
	{
		count_label += _(" (too many to display)");
	}
	else
	{
		std::vector<CheatSearchResult>::const_iterator
			i = search_results.begin(),
			e = search_results.end();
		for (; i!=e; ++i)
		{
			u32 display_value = i->old_value;

			// #ifdef LIL_ENDIAN :p
			switch (search_type_size)
			{
			case 1 :
				break;
			case 2 :
				*(u16*)&display_value = Common::swap16((u8*)&display_value);
				break;
			case 4 :
				display_value = Common::swap32(display_value);
				break;
			}
			// #elseif BIG_ENDIAN
			// need to do some stuff in here (for 8 and 16bit) for bigendian
			// #endif

			static wxChar rowfmt[] = wxT("0x%08x    0x%0|x    %u/%i");
			rowfmt[14] = (wxChar)(wxT('0') + search_type_size*2);

			lbox_search_results->Append(
				wxString::Format(rowfmt, i->address, display_value, display_value, display_value));
		}
	}

	label_results_count->SetLabel(count_label);
}

void CheatSearchTab::CreateARCode(wxCommandEvent&)
{
	const int sel = lbox_search_results->GetSelection();
	if (sel >= 0)
	{
		const u32 address = search_results[sel].address | ((search_type_size & ~1) << 24);

		CreateCodeDialog* const arcode_dlg = new CreateCodeDialog(this, address);
		arcode_dlg->ShowModal();
	}
}

CreateCodeDialog::CreateCodeDialog(wxWindow* const parent, const u32 address)
	: wxDialog(parent, -1, _("Create AR Code"), wxDefaultPosition)
	, code_address(address)
{
	wxPanel* const panel = new wxPanel(this);

	wxStaticText* const label_name = new wxStaticText(panel, -1, _("Name: "));
	textctrl_name = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxSize(256,-1));

	wxStaticText* const label_code = new wxStaticText(panel, -1, _("Code: "));
	textctrl_code = new wxTextCtrl(panel, -1, wxString::Format(wxT("0x%08x"), address));
	textctrl_code->Disable();

	wxStaticText* const label_value = new wxStaticText(panel, -1, _("Value: "));
	textctrl_value = new wxTextCtrl(panel, -1, wxT("0"));

	checkbox_use_hex = new wxCheckBox(panel, -1, _("Use Hex"));
	checkbox_use_hex->SetValue(true);

	wxBoxSizer* const sizer_value_label = new wxBoxSizer(wxHORIZONTAL);
	sizer_value_label->Add(label_value, 0, wxRIGHT, 5);
	sizer_value_label->Add(checkbox_use_hex, 0, 0, 0);

	wxButton* const btn_ok = new wxButton(panel, -1, _("OK"));
	_connect_macro_(btn_ok, CreateCodeDialog::PressOK, wxEVT_COMMAND_BUTTON_CLICKED, this);
	wxButton* const btn_cancel = new wxButton(panel, -1, _("Cancel"));
	_connect_macro_(btn_cancel, CreateCodeDialog::PressCancel, wxEVT_COMMAND_BUTTON_CLICKED, this);

	// button sizer
	wxBoxSizer* const sizer_buttons = new wxBoxSizer(wxHORIZONTAL);
	sizer_buttons->Add(btn_ok, 0, wxRIGHT, 5);
	sizer_buttons->Add(btn_cancel, 0, 0, 0);

	// main sizer
	wxBoxSizer* const sizer_panel = new wxBoxSizer(wxVERTICAL);
	sizer_panel->Add(label_name, 0, wxALL, 5);
	sizer_panel->Add(textctrl_name, 0, wxALL, 5);
	sizer_panel->Add(label_code, 0, wxALL, 5);
	sizer_panel->Add(textctrl_code, 0, wxALL, 5);
	sizer_panel->Add(sizer_value_label, 0, wxALL, 5);
	sizer_panel->Add(textctrl_value, 0, wxALL, 5);
	sizer_panel->Add(sizer_buttons, 0, wxALL | wxALIGN_RIGHT, 5);

	panel->SetSizerAndFit(sizer_panel);

	// panel sizer
	wxBoxSizer* const sizer_main = new wxBoxSizer(wxHORIZONTAL);
	sizer_main->Add(panel, 1, wxEXPAND, 5);

	SetSizerAndFit(sizer_main);
}

void CreateCodeDialog::PressOK(wxCommandEvent&)
{
	const wxString code_name = textctrl_name->GetValue();
	if (code_name.empty())
	{
		PanicAlertT("You must enter a name!");
		return;
	}

	long code_value;
	if (!textctrl_value->GetValue().ToLong(&code_value, 10 + checkbox_use_hex->GetValue()*6))
	{
		PanicAlertT("Invalid Value!");
		return;
	}

	//wxString full_code = textctrl_code->GetValue();
	//full_code += wxT(' ');
	//full_code += wxString::Format(wxT("0x%08x"), code_value);

	// create the new code
	ActionReplay::ARCode new_cheat;
	new_cheat.active = false;
	new_cheat.name = std::string(code_name.ToAscii());
	const ActionReplay::AREntry new_entry(code_address, code_value);
	new_cheat.ops.push_back(new_entry);

	// pretty hacky - add the code to the gameini
	{
	CISOProperties isoprops(SConfig::GetInstance().m_LastFilename, this);
	// add the code to the isoproperties arcode list
	arCodes.push_back(new_cheat);
	// save the gameini
	isoprops.SaveGameConfig();
	isoprops.ActionReplayList_Load();	// loads the new arcodes
	//ActionReplay::UpdateActiveList();
	}

	// refresh arcode list in other tab
	::g_cheat_window->Load_ARCodes();

	Destroy();
}

void CreateCodeDialog::PressCancel(wxCommandEvent&)
{
	Destroy();
}
