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

#ifndef _VIDEOSOFTWARE_CONFIG_DIAG_H_
#define _VIDEOSOFTWARE_CONFIG_DIAG_H_

#include <vector>
#include <string>

#include "SWVideoConfig.h"

#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>

class VideoConfigDialog : public wxDialog
{
public:
	VideoConfigDialog(wxWindow* parent, const std::string &title, const std::string& ininame);

protected:
	void Event_ClickClose(wxCommandEvent&);
	void Event_Close(wxCloseEvent&);

	SWVideoConfig& vconfig;
	std::string ininame;
};

#endif
