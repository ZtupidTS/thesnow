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

#ifndef __ARCODE_ADDEDIT_h__
#define __ARCODE_ADDEDIT_h__

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "ISOProperties.h"

class CARCodeAddEdit : public wxDialog
{
	public:
		CARCodeAddEdit(int _selection, wxWindow* parent,
			wxWindowID id = 1,
			const wxString& title = _("Edit ActionReplay Code"),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxDEFAULT_DIALOG_STYLE);
		virtual ~CARCodeAddEdit();

	private:
		DECLARE_EVENT_TABLE();

		wxTextCtrl *EditCheatName;
		wxSpinButton *EntrySelection;
		wxTextCtrl *EditCheatCode;

		enum {
			ID_EDITCHEAT_NAME_TEXT = 4550,
			ID_EDITCHEAT_NAME,
			ID_ENTRY_SELECT,
			ID_EDITCHEAT_CODE
		};

		void CreateGUIControls(int selection);
		void OnClose(wxCloseEvent& event);
		void SaveCheatData(wxCommandEvent& event);
		void ChangeEntry(wxSpinEvent& event);
		void UpdateTextCtrl(ActionReplay::ARCode arCode);

		int selection;

};
#endif // __PATCH_ADDEDIT_h__
