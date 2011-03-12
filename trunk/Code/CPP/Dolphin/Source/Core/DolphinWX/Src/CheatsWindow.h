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

#ifndef __CHEATSWINDOW_H__
#define __CHEATSWINDOW_H__

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/filepicker.h>
#include <wx/statbmp.h>
#include <wx/imaglist.h>
#include <wx/treectrl.h>
#include <wx/gbsizer.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <wx/mimetype.h>
#include <wx/colour.h>
#include <wx/listbox.h>

#include <string>
#include <vector>

#include "ActionReplay.h"

#include "GeckoCodeDiag.h"

#include "Filesystem.h"
#include "IniFile.h"

class CreateCodeDialog : public wxDialog
{
public:
	CreateCodeDialog(wxWindow* const parent, const u32 address);

protected:

	const u32 code_address;

	wxTextCtrl *textctrl_name, *textctrl_code, *textctrl_value;
	wxCheckBox *checkbox_use_hex;

	void PressOK(wxCommandEvent&);
	void PressCancel(wxCommandEvent&);
};

class CheatSearchTab : public wxPanel
{
public:
	CheatSearchTab(wxWindow* const parent);

protected:

	class CheatSearchResult
	{
	public:
		CheatSearchResult() : address(0), old_value(0) {}

		u32 address;
		u32 old_value;
	};

	std::vector<CheatSearchResult>	search_results;
	unsigned int search_type_size;

	wxChoice* search_type;
	wxListBox*	lbox_search_results;
	wxStaticText* label_results_count;
	wxTextCtrl*	textctrl_value_x;
	wxButton *btnInitScan, *btnNextScan;

	struct
	{
		wxRadioButton *rad_8, *rad_16, *rad_32;

	} size_radiobtn;

	struct
	{
		wxRadioButton *rad_oldvalue, *rad_uservalue;

	} value_x_radiobtn;

	void UpdateCheatSearchResultsList();
	void StartNewSearch(wxCommandEvent& event);
	void FilterCheatSearchResults(wxCommandEvent& event);
	void CreateARCode(wxCommandEvent&);
	void ApplyFocus(wxCommandEvent&);
};

class wxCheatsWindow : public wxFrame
{
	friend class CreateCodeDialog;

	public:
		wxCheatsWindow(wxWindow* const parent);
		~wxCheatsWindow();

	protected:

		struct ARCodeIndex {
			u32 uiIndex;
			size_t index;
		};

		// Event Table
		//DECLARE_EVENT_TABLE();

		// --- GUI Controls ---
		wxNotebook *m_Notebook_Main;

		wxPanel *m_Tab_Cheats;
		wxPanel *m_Tab_Log;

		wxCheckBox *m_CheckBox_LogAR;

		wxStaticText *m_Label_Codename;
		wxStaticText *m_Label_NumCodes;

		wxCheckListBox *m_CheckListBox_CheatsList;

		wxTextCtrl *m_TextCtrl_Log;

		wxListBox *m_ListBox_CodesList;

		wxStaticBox *m_GroupBox_Info;

		wxArrayString m_CheatStringList;

		std::vector<ARCodeIndex> indexList;

		Gecko::CodeConfigPanel *m_geckocode_panel;
		IniFile m_gameini;
		std::string m_gameini_path;

		void Init_ChildControls();

		void Load_ARCodes();

		// --- Wx Events Handlers ---

		// $ Close Button
		void OnEvent_ButtonClose_Press(wxCommandEvent& event);

		// $ Cheats List
		void OnEvent_CheatsList_ItemSelected(wxCommandEvent& event);
		void OnEvent_CheatsList_ItemToggled(wxCommandEvent& event);

		// $ Apply Changes Button
		void OnEvent_ApplyChanges_Press(wxCommandEvent& event);

		// $ Update Log Button
		void OnEvent_ButtonUpdateLog_Press(wxCommandEvent& event);

		// $ Enable Logging Checkbox
		void OnEvent_CheckBoxEnableLogging_StateChange(wxCommandEvent& event);
};

#endif

