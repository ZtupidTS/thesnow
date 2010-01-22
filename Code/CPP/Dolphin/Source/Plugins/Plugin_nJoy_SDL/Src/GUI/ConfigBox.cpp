// Project description
// -------------------
// Name: nJoy
// Description: A Dolphin Compatible Input Plugin
//
// Author: Falcon4ever (nJoy@falcon4ever.com)
// Site: www.multigesture.net
// Copyright (C) 2003 Dolphin Project.
//
// Licensetype: GNU General Public License (GPL)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.
//
// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/
//
// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/
//

#include "math.h" // System

#include "ConfigBox.h" // Local
#include "../nJoy.h"
#include "Images/controller.xpm"

extern bool g_EmulatorRunning;

// D-Pad type
static const char* DPadType[] =
{
	"Hat",
	"Custom",
};

// Trigger type
static const char* TriggerType[] =
{
	"SDL", // -0x8000 to 0x7fff
	"XInput", // 0x00 to 0xff
};


// The wxWidgets class
BEGIN_EVENT_TABLE(PADConfigDialognJoy,wxDialog)
	EVT_CLOSE(PADConfigDialognJoy::OnClose)
	EVT_BUTTON(ID_ABOUT, PADConfigDialognJoy::AboutClick)
	EVT_BUTTON(ID_OK, PADConfigDialognJoy::OKClick)
	EVT_BUTTON(ID_CANCEL, PADConfigDialognJoy::CancelClick)
	EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, PADConfigDialognJoy::NotebookPageChanged)

	// Change gamepad
	EVT_COMBOBOX(IDC_JOYNAME, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDC_ENABLE, PADConfigDialognJoy::ChangeSettings)

	 // Other settings
	EVT_CHECKBOX(IDC_SAVEBYID, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDC_SHOWADVANCED, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDCB_CHECKFOCUS, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDCB_MAINSTICK_DIAGONAL, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDC_CONTROLTYPE, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDC_TRIGGERTYPE, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDC_DEADZONE, PADConfigDialognJoy::ChangeSettings)	

	// Rumble settings
	EVT_CHECKBOX(IDC_ENABLERUMBLE, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDC_RUMBLESTRENGTH, PADConfigDialognJoy::ChangeSettings)

	// Advanced settings
	EVT_COMBOBOX(IDCB_MAINSTICK_RADIUS, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDCB_MAINSTICK_CB_RADIUS, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDCB_MAINSTICK_DIAGONAL, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDCB_MAINSTICK_S_TO_C, PADConfigDialognJoy::ChangeSettings)
	// C-stick
	EVT_COMBOBOX(IDCB_CSTICK_RADIUS, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDCB_CSTICK_CB_RADIUS, PADConfigDialognJoy::ChangeSettings)
	EVT_COMBOBOX(IDCB_CSTICK_DIAGONAL, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDCB_CSTICK_S_TO_C, PADConfigDialognJoy::ChangeSettings)
	EVT_CHECKBOX(IDCB_FILTER_SETTINGS, PADConfigDialognJoy::ChangeSettings)

	EVT_BUTTON(IDB_SHOULDER_L, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_SHOULDER_R, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTON_A, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTON_B, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTON_X, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTON_Y, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTON_Z, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTONSTART, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_BUTTONHALFPRESS, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_DPAD_UP, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_DPAD_DOWN, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_DPAD_LEFT, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_DPAD_RIGHT, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_ANALOG_MAIN_X, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_ANALOG_MAIN_Y, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_ANALOG_SUB_X, PADConfigDialognJoy::GetButtons)
	EVT_BUTTON(IDB_ANALOG_SUB_Y, PADConfigDialognJoy::GetButtons)

	#if wxUSE_TIMER
		EVT_TIMER(IDTM_CONSTANT, PADConfigDialognJoy::OnTimer)
		EVT_TIMER(IDTM_BUTTON, PADConfigDialognJoy::OnButtonTimer)
	#endif
END_EVENT_TABLE()

PADConfigDialognJoy::PADConfigDialognJoy(wxWindow *parent, wxWindowID id, const wxString &title,
					 const wxPoint &position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	// Define values
	notebookpage = 0;
	g_Pressed = 0;
	Debugging = false;
	m_TCDebugging = NULL;
	ControlsCreated = false;

	// Create controls
	CreateGUIControls();

	#if wxUSE_TIMER
		m_ConstantTimer = new wxTimer(this, IDTM_CONSTANT);
		m_ButtonMappingTimer = new wxTimer(this, IDTM_BUTTON);

		// Reset values
		GetButtonWaitingID = 0; GetButtonWaitingTimer = 0;
		if (NumGoodPads)
		{
			// Start the constant timer
			int TimesPerSecond = 10;
			m_ConstantTimer->Start(1000 / TimesPerSecond);
		}
	#endif

	// wxEVT_KEY_DOWN is blocked for enter, tab and the directional keys
	wxTheApp->Connect(wxID_ANY, wxEVT_KEY_UP,
			wxKeyEventHandler(PADConfigDialognJoy::OnKeyDown),
			(wxObject*)0, this);
}

PADConfigDialognJoy::~PADConfigDialognJoy()
{
// The statbar sample has this so I add this to
#if wxUSE_TIMER
    if (m_ConstantTimer->IsRunning()) m_ConstantTimer->Stop();
#endif
}

void PADConfigDialognJoy::OnKeyDown(wxKeyEvent& event)
{
	g_Pressed = event.GetKeyCode();
}

// Close window
void PADConfigDialognJoy::OnClose(wxCloseEvent& event)
{
	// Allow wxWidgets to close the window
	//event.Skip();

	// Stop the timer
	m_ConstantTimer->Stop();

	// Close pads, unless we are running a game
	//if (!g_EmulatorRunning)	Shutdown();

	EndModal(wxID_CLOSE);
}

// Call about dialog
void PADConfigDialognJoy::AboutClick(wxCommandEvent& event)
{
	#ifdef _WIN32
		wxWindow win;
		win.SetHWND((WXHWND)this->GetHWND());
		win.Enable(false);  

		AboutBox frame(&win);
		frame.ShowModal();

		win.Enable(true);
		win.SetHWND(0);
	#else
		AboutBox frame(NULL);
		frame.ShowModal();
	#endif
}

// Click OK
void PADConfigDialognJoy::OKClick(wxCommandEvent& event)
{
	if (event.GetId() == ID_OK)
	{
		DoSave();	// Save settings
		if (Debugging) PanicAlert("Done");
		Close(); // Call OnClose()
	}
}

// Click Cancel
void PADConfigDialognJoy::CancelClick(wxCommandEvent& event)
{
	if (event.GetId() == ID_CANCEL)
	{
		// Forget all potential changes to PadMapping by loading the last saved settings
		g_Config.Load();
		Close(); // Call OnClose()
	}
}

// Debugging
void PADConfigDialognJoy::LogMsg(const char* format, ...)
{
	#ifdef _WIN32
	if (Debugging)
	{
		const int MaxMsgSize = 1024*2;
		char buffer[MaxMsgSize];
		va_list args;
		va_start(args, format);
		CharArrayFromFormatV(buffer, MaxMsgSize, format, args);
		va_end(args);

		// Add timestamp
		std::string StrTmp = buffer;
		//StrTmp += Common::Timer::GetTimeFormatted();

		if(m_TCDebugging) m_TCDebugging->AppendText(wxString::FromAscii(StrTmp.c_str()));
	}	
	#endif
}



// Save Settings
/*

   Saving is currently done when:

   1. Closing the configuration window
   2. Changing the gamepad
   3. When the gamepad is enabled or disbled
   4. When we change saving mode (by Id or by slot)

   Input: ChangePad needs to be used when we change the pad for a slot. Slot needs to be used when
   we only want to save changes to one slot.
*/
void PADConfigDialognJoy::DoSave(bool ChangePad, int Slot)
{
	// Replace "" with "-1" before we are saving
	ToBlank(false);

	if (ChangePad)
	{
		// Since we are selecting the pad to save to by the Id we can't update it when we change the pad
		for(int i = 0; i < 4; i++)
			SaveButtonMapping(i, true);
		
		// Now we can update the ID
		PadMapping[notebookpage].ID = m_Joyname[notebookpage]->GetSelection();
		PadState[notebookpage].joy = joyinfo.at(PadMapping[notebookpage].ID).joy;

		g_Config.Save(Slot);
	}
	else
	{
		// Update PadMapping[] from the GUI controls
		for(int i = 0; i < 4; i++)
			SaveButtonMapping(i);

		g_Config.Save(Slot);
	}		

	// Then change it back to ""
	ToBlank();
}

// On changing the SaveById option we update all pages
void PADConfigDialognJoy::OnSaveById()
{
	// Save current settings
	DoSave(false, notebookpage);

	// Update the SaveByID setting and load the settings
	g_Config.bSaveByID = m_CBSaveByID[notebookpage]->IsChecked();
	g_Config.Load(false, true);

	// Update the GUI from the now updated PadMapping[]
	UpdateGUIAll(-1);
}

// Change Joystick
/* Function: When changing the joystick we save and load the settings and update the PadMapping
   and PadState array. PadState[].joy is the gamepad handle that is used to access the pad throughout
   the plugin. Joyinfo[].joy is only used the first time the pads are checked. */
void PADConfigDialognJoy::DoChangeJoystick()
{
	// Before changing the pad we save potential changes to the current pad (to support SaveByID)
	DoSave(true);
	
	// Load the settings for the new Id
	g_Config.Load(true);
	UpdateGUI(notebookpage); // Update the GUI
}

// Notebook page changed
void PADConfigDialognJoy::NotebookPageChanged(wxNotebookEvent& event)
{	
	// Save current settings now, don't wait for OK
	if (ControlsCreated && !g_Config.bSaveByID)
		DoSave(false, notebookpage);

	// Update the global variable
	notebookpage = event.GetSelection();

	// Update GUI
	if (ControlsCreated)
		UpdateGUI(notebookpage);
}

// Replace the harder to understand -1 with "" for the sake of user friendliness
void PADConfigDialognJoy::ToBlank(bool ToBlank)
{
	if (!ControlsCreated) return;

	for (int j = 0; j < 4; j++)
	{
		if(ToBlank)
		{
			for(int i = IDB_ANALOG_MAIN_X; i <= IDB_BUTTONHALFPRESS; i++)
				#ifndef _WIN32
					if(!strcmp(GetButtonText(i, j).mb_str(), "-1")) SetButtonText(i, "", j);
				#else
					if(GetButtonText(i, j) == wxT("-1")) SetButtonText(i, "", j);
				#endif
		}
		else
		{
			for(int i = IDB_ANALOG_MAIN_X; i <= IDB_BUTTONHALFPRESS; i++)
				if(GetButtonText(i, j).IsEmpty()) SetButtonText(i, "-1", j);
		}
	}	
}


// Change settings
void PADConfigDialognJoy::SetButtonTextAll(int id, const char *text)
{
	for (int i = 0; i < 4; i++)
	{
		// Safety check to avoid crash
		if (joyinfo.size() > (u32)PadMapping[i].ID)
			if (joyinfo[PadMapping[i].ID].Name == joyinfo[PadMapping[notebookpage].ID].Name)
				SetButtonText(id, text, i);
	};
}

void PADConfigDialognJoy::SaveButtonMappingAll(int Slot)
{
	for (int i = 0; i < 4; i++)
	{
		// This can occur when no gamepad is detected
		if (joyinfo.size() > (u32)PadMapping[i].ID)
			if (joyinfo[PadMapping[i].ID].Name == joyinfo[PadMapping[Slot].ID].Name)
				SaveButtonMapping(i, false, Slot);
	}
}

void PADConfigDialognJoy::UpdateGUIAll(int Slot)
{
	if (Slot == -1)
	{
		for (int i = 0; i < 4; i++)
			UpdateGUI(i);
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			// Safety check to avoid crash
			if (joyinfo.size() > (u32)PadMapping[i].ID)
				if (joyinfo[PadMapping[i].ID].Name == joyinfo[PadMapping[Slot].ID].Name)
					UpdateGUI(i);
		}
	}
}

void PADConfigDialognJoy::ChangeSettings( wxCommandEvent& event )
{
	switch(event.GetId())
	{
		case IDC_SAVEBYID:
			OnSaveById();
			break;		

		case IDC_SHOWADVANCED:
			g_Config.bShowAdvanced = m_CBShowAdvanced[notebookpage]->IsChecked();			
			for(int i = 0; i < 4; i++)
				m_CBShowAdvanced[i]->SetValue(g_Config.bShowAdvanced);
			UpdateGUI(notebookpage);
			// Resize the window without the need of any weird hack 
			SetSizerAndFit(m_MainSizer);
			break;

		// Advanced settings
		case IDCB_CHECKFOCUS:
			g_Config.bCheckFocus = m_CBCheckFocus[notebookpage]->IsChecked();
			for(int i = 0; i < 4; i++)
				m_CBCheckFocus[i]->SetValue(g_Config.bCheckFocus);
			break;
		case IDCB_FILTER_SETTINGS:
			g_Config.bNoTriggerFilter = m_AdvancedMapFilter[notebookpage]->IsChecked();
			for(int i = 0; i < 4; i++)
				m_AdvancedMapFilter[i]->SetValue(g_Config.bNoTriggerFilter);
			break;

		case IDC_CONTROLTYPE:
			if(!g_Config.bSaveByID)
			{
				PadMapping[notebookpage].controllertype = m_ControlType[notebookpage]->GetSelection();
				UpdateGUI(notebookpage);
			}
		case IDC_TRIGGERTYPE:
			if(!g_Config.bSaveByID)
			{
				PadMapping[notebookpage].triggertype = m_TriggerType[notebookpage]->GetSelection();
				UpdateGUI(notebookpage);
			}
			break;
		case IDC_ENABLERUMBLE:
			PadMapping[notebookpage].rumble = m_Rumble[notebookpage]->IsChecked();
			break;
		case IDC_RUMBLESTRENGTH:
			g_Config.RumbleStrength = m_RStrength[notebookpage]->GetSelection();
			break;
		case IDC_JOYNAME: 
			DoChangeJoystick();
			break;
		case IDC_ENABLE:
			PadMapping[notebookpage].enable = m_Enable[notebookpage]->IsChecked();
			UpdateGUI(notebookpage);
			// Resize the window without the need of any weird hack 
			SetSizerAndFit(m_MainSizer);
			break;
	}

	// Update all slots that use this device
	if(g_Config.bSaveByID) SaveButtonMappingAll(notebookpage);
	//if(g_Config.bSaveByID) UpdateGUIAll(notebookpage);
}


// Update GUI
// Called from: CreateGUIControls(), ChangeControllertype()
void PADConfigDialognJoy::UpdateGUI(int _notebookpage)
{
	// If there are no good pads disable the entire notebook
	if (NumGoodPads == 0)
	{
		m_Notebook->Enable(false);
		return;
	}

	// Update the GUI from PadMapping[]
	UpdateGUIButtonMapping(_notebookpage);

	// Collect status
	bool Hat = (PadMapping[_notebookpage].controllertype == InputCommon::CTL_DPAD_HAT);
	long Left, Right;
	m_JoyShoulderL[_notebookpage]->GetValue().ToLong(&Left);
	m_JoyShoulderR[_notebookpage]->GetValue().ToLong(&Right);
	bool AnalogTrigger = (Left >= 1000 || Right >= 1000);
	#ifdef _WIN32
		bool XInput = XInput::IsConnected(PadMapping[_notebookpage].ID);
	#endif

	// Hat type selection
	m_JoyDpadUp[_notebookpage]->Show(!Hat);
	m_JoyDpadLeft[_notebookpage]->Show(!Hat);
	m_JoyDpadRight[_notebookpage]->Show(!Hat);

	m_bJoyDpadUp[_notebookpage]->Show(!Hat);
	m_bJoyDpadLeft[_notebookpage]->Show(!Hat);
	m_bJoyDpadRight[_notebookpage]->Show(!Hat);
	
	m_textDpadUp[_notebookpage]->Show(!Hat);
	m_textDpadLeft[_notebookpage]->Show(!Hat);
	m_textDpadRight[_notebookpage]->Show(!Hat);	

	m_textDpadDown[_notebookpage]->SetLabel(Hat ? wxT("Select hat") : wxT("Down"));
	m_bJoyDpadDown[_notebookpage]->SetToolTip(Hat ?
		wxT("Select a hat by pressing the hat in any direction") : wxT(""));

	// General settings
	m_CBSaveByID[_notebookpage]->SetValue(g_Config.bSaveByID);
	m_CBShowAdvanced[_notebookpage]->SetValue(g_Config.bShowAdvanced);
	m_CBCheckFocus[_notebookpage]->SetValue(g_Config.bCheckFocus);
	m_AdvancedMapFilter[_notebookpage]->SetValue(g_Config.bNoTriggerFilter);
	m_RStrength[_notebookpage]->SetSelection(g_Config.RumbleStrength);

	// Replace the harder to understand -1 with "" for the sake of user friendliness
	ToBlank();

	// Advanced settings
	if (g_Config.bShowAdvanced)
	{
		if (PadMapping[_notebookpage].bRadiusOnOff) m_CoBRadius[_notebookpage]->Enable(true);
			else m_CoBRadius[_notebookpage]->Enable(false);
		if (PadMapping[_notebookpage].bSquareToCircle) m_CoBDiagonal[_notebookpage]->Enable(true);
			else m_CoBDiagonal[_notebookpage]->Enable(false);
		if (PadMapping[_notebookpage].bRadiusOnOffC) m_CoBRadiusC[_notebookpage]->Enable(true);
			else m_CoBRadiusC[_notebookpage]->Enable(false);
		if (PadMapping[_notebookpage].bSquareToCircleC) m_CoBDiagonalC[_notebookpage]->Enable(true);
			else m_CoBDiagonalC[_notebookpage]->Enable(false);
	}

	m_sKeys[_notebookpage]->Show(PadMapping[_notebookpage].enable);
	m_sSettings[_notebookpage]->Show(PadMapping[_notebookpage].enable);
	m_sMainRight[_notebookpage]->Show(g_Config.bShowAdvanced && PadMapping[_notebookpage].enable);

	// Repaint the background
	m_Controller[_notebookpage]->Refresh();
}


// Paint the background
void PADConfigDialognJoy::OnPaint(wxPaintEvent &event)
{
	event.Skip();

	wxPaintDC dcWin(m_pKeys[notebookpage]);
	PrepareDC( dcWin );
	dcWin.DrawBitmap( WxStaticBitmap1_BITMAP, 94, 0, true );
}

// Populate the config window
void PADConfigDialognJoy::CreateGUIControls()
{
	INFO_LOG(PAD, "CreateGUIControls()");

	#ifndef _DEBUG		
		SetTitle(wxT("Configure: nJoy Input Plugin"));
	#else			
		SetTitle(wxT("Configure: nJoy (Debug) Input Plugin"));
	#endif

	SetIcon(wxNullIcon);

#ifndef _WIN32
	// Force a 8pt font so that it looks more or less "correct" regardless of the default font setting
	wxFont f(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	SetFont(f);
#endif

	// Buttons
	m_About = new wxButton(this, ID_ABOUT, wxT("����"), wxDefaultPosition, wxSize(75, 25), 0, wxDefaultValidator, wxT("About"));
	m_OK = new wxButton(this, ID_OK, wxT("ȷ��"), wxDefaultPosition, wxSize(75, 25), 0, wxDefaultValidator, wxT("OK"));
	m_Cancel = new wxButton(this, ID_CANCEL, wxT("ȡ��"), wxDefaultPosition, wxSize(75, 25), 0, wxDefaultValidator, wxT("Cancel"));
	m_OK->SetToolTip(
		wxT("Save your settings and close this window.")
		);
	m_Cancel->SetToolTip(
		wxT("Close this window without saving your changes.")
		);

	// Notebook
	m_Notebook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);

	// Controller pages
	m_Controller[0] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE1, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[0], wxT("������ 1"));
	m_Controller[1] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE2, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[1], wxT("������ 2"));
	m_Controller[2] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE3, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[2], wxT("������ 3"));
	m_Controller[3] = new wxPanel(m_Notebook, ID_CONTROLLERPAGE4, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Controller[3], wxT("������ 4"));


	// Define bitmap for EVT_PAINT
	WxStaticBitmap1_BITMAP = wxBitmap(ConfigBox_WxStaticBitmap1_XPM);

	// Search for devices and add them to the device list
	wxArrayString arrayStringFor_Joyname; // The string array
	if (NumGoodPads > 0)
	{
		for(int x = 0; (u32)x < joyinfo.size(); x++)
		{
			arrayStringFor_Joyname.Add(wxString::FromAscii(joyinfo[x].Name.c_str()));
		}
	}
	else
	{
		arrayStringFor_Joyname.Add(wxString::FromAscii("<No Gamepad Detected>"));
	}

	// Populate the DPad type and Trigger type list
	wxArrayString wxAS_DPadType;
	wxAS_DPadType.Add(wxString::FromAscii(DPadType[InputCommon::CTL_DPAD_HAT]));
	wxAS_DPadType.Add(wxString::FromAscii(DPadType[InputCommon::CTL_DPAD_CUSTOM]));

	wxArrayString wxAS_TriggerType;
	wxAS_TriggerType.Add(wxString::FromAscii(TriggerType[InputCommon::CTL_TRIGGER_SDL]));
	wxAS_TriggerType.Add(wxString::FromAscii(TriggerType[InputCommon::CTL_TRIGGER_XINPUT]));
	
	// Populate the deadzone list and the Rumble Strength

	char buffer[8];

	wxArrayString wxAS_RumbleStrength;
	for (int i = 1; i < 11; i++) 
	{
		sprintf (buffer, "%d %%", i*10);
		wxAS_RumbleStrength.Add(wxString::FromAscii(buffer));
	}

	wxArrayString arrayStringFor_Deadzone;
	for(int x = 1; x <= 100; x++)
	{		
		sprintf (buffer, "%d %%", x);
		arrayStringFor_Deadzone.Add(wxString::FromAscii(buffer));
	}

	// Populate all four pages
	for(int i = 0; i < 4; i++)
	{
		// Populate keys sizer
		// Set relative values for the keys
		int t = -75; // Top
		int l = -4; // Left
		m_sKeys[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Keys"));
		m_pKeys[i] = new wxPanel(m_Controller[i], ID_KEYSPANEL1 + i, wxDefaultPosition, wxSize(600, 400), 0);
		//m_sKeys[i] = new wxStaticBox (m_Controller[i], IDG_JOYSTICK, wxT("Keys"), wxDefaultPosition, wxSize(608, 500));
		m_sKeys[i]->Add(m_pKeys[i], 0, (wxALL), 0); // margin = 0

		// GameCube controller picture
		// TODO: Controller image
		// Placeholder instead of bitmap
		// m_PlaceholderBMP[i] = new wxTextCtrl(m_Controller[i], ID_CONTROLLERPICTURE, wxT("BITMAP HERE PLZ KTHX!"), wxPoint(98, 75), wxSize(423, 306), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("BITMAP HERE PLZ KTHX!"));
		// m_PlaceholderBMP[i]->Enable(false);
		
		/* You can enable the bitmap here. But it loads �berslow on init... (only in windows, linux
		seems to load it fast!) AAaaand the XPM file (256 colours) looks crappier than the real bitmap...
		so maybe we can find a way to use a bitmap?	*/
		//m_controllerimage[i] = new wxStaticBitmap(m_pKeys[i], ID_CONTROLLERPICTURE, WxStaticBitmap1_BITMAP, wxPoint(l + 98, t + 75), wxSize(421,304));		
		//m_controllerimage[i] = new wxBitmap( WxStaticBitmap1_BITMAP );		

		 // Paint background. This allows objects to be visible on top of the picture
		m_pKeys[i]->Connect(wxID_ANY, wxEVT_PAINT,
			wxPaintEventHandler(PADConfigDialognJoy::OnPaint),
			(wxObject*)0, this);


		// Keys objects
		// Left and right shoulder buttons
		m_JoyShoulderL[i] = new wxTextCtrl(m_pKeys[i], ID_SHOULDER_L, wxT("0"), wxPoint(l + 6, t + 80), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyShoulderL[i]->Enable(false);
		m_bJoyShoulderL[i] = new wxButton(m_pKeys[i], IDB_SHOULDER_L, wxEmptyString, wxPoint(l + 70, t + 82), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_JoyShoulderR[i] = new wxTextCtrl(m_pKeys[i], ID_SHOULDER_R, wxT("0"), wxPoint(l + 552, t + 106), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyShoulderR[i]->Enable(false);
		m_bJoyShoulderR[i] = new wxButton(m_pKeys[i], IDB_SHOULDER_R, wxEmptyString, wxPoint(l + 526, t + 108), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		// Left analog
		int ALt = 169; int ALw = ALt + 14; int ALb = ALw + 2; // Set offset
		m_JoyAnalogMainX[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_MAIN_X, wxT("0"), wxPoint(l + 6, t + ALw), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogMainX[i]->Enable(false);
		m_JoyAnalogMainY[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_MAIN_Y, wxT("0"), wxPoint(l + 6, t + ALw + 36), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogMainY[i]->Enable(false);		
		m_bJoyAnalogMainX[i] = new wxButton(m_pKeys[i], IDB_ANALOG_MAIN_X, wxEmptyString, wxPoint(l + 70, t + ALb), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyAnalogMainY[i] = new wxButton(m_pKeys[i], IDB_ANALOG_MAIN_Y, wxEmptyString, wxPoint(l + 70, t + ALb + 36), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_textMainX[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_MAIN_X, wxT("X-axis"), wxPoint(l + 6, t + ALt), wxDefaultSize, 0, wxT("X-axis"));
		m_textMainY[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_MAIN_Y, wxT("Y-axis"), wxPoint(l + 6, t + ALt + 36), wxDefaultSize, 0, wxT("Y-axis"));

		// D-Pad
		int DPt = 250; int DPw = DPt + 14; int DPb = DPw + 2; // Set offset
		m_JoyDpadUp[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_UP, wxT("0"), wxPoint(l + 6, t + DPw), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadDown[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_DOWN, wxT("0"), wxPoint(l + 6, t + DPw + 36*1), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadLeft[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_LEFT, wxT("0"), wxPoint(l + 6, t + DPw + 36*2), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadRight[i] = new wxTextCtrl(m_pKeys[i], ID_DPAD_RIGHT, wxT("0"), wxPoint(l + 6, t + DPw + 36*3), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyDpadUp[i]->Enable(false);
		m_JoyDpadDown[i]->Enable(false);
		m_JoyDpadLeft[i]->Enable(false);
		m_JoyDpadRight[i]->Enable(false);
		m_bJoyDpadUp[i] = new wxButton(m_pKeys[i], IDB_DPAD_UP, wxEmptyString, wxPoint(l + 70, t + DPb + 36*0), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyDpadDown[i] = new wxButton(m_pKeys[i], IDB_DPAD_DOWN, wxEmptyString, wxPoint(l + 70, t + DPb + 36*1), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyDpadLeft[i] = new wxButton(m_pKeys[i], IDB_DPAD_LEFT, wxEmptyString, wxPoint(l + 70, t + DPb + 36*2), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyDpadRight[i] = new wxButton(m_pKeys[i], IDB_DPAD_RIGHT, wxEmptyString, wxPoint(l + 70, t + DPb + 36*3), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_textDpadUp[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_UP, wxT("��"), wxPoint(l + 6, t + DPt + 36*0), wxDefaultSize, 0, wxT("��"));
		m_textDpadDown[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_DOWN, wxT("��"), wxPoint(l + 6, t + DPt + 36*1), wxDefaultSize, 0, wxT("��"));
		m_textDpadLeft[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_LEFT, wxT("��"), wxPoint(l + 6, t + DPt + 36*2), wxDefaultSize, 0, wxT("��"));
		m_textDpadRight[i] = new wxStaticText(m_pKeys[i], IDT_DPAD_RIGHT, wxT("��"), wxPoint(l + 6, t + DPt + 36*3), wxDefaultSize, 0, wxT("��"));

		// Buttons
		m_JoyButtonA[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_A, wxT("0"), wxPoint(l + 552, t + 280), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonA[i]->Enable(false);
		m_JoyButtonB[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_B, wxT("0"), wxPoint(l + 552, t + 80), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonB[i]->Enable(false);
		m_JoyButtonX[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_X, wxT("0"), wxPoint(l + 552, t + 242), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonX[i]->Enable(false);
		m_JoyButtonY[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_Y, wxT("0"), wxPoint(l + 552, t + 171), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonY[i]->Enable(false);
		m_JoyButtonZ[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTON_Z, wxT("0"), wxPoint(l + 552, t + 145), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonZ[i]->Enable(false);
		m_bJoyButtonA[i] = new wxButton(m_pKeys[i], IDB_BUTTON_A, wxEmptyString, wxPoint(l + 526, t + 282), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonB[i] = new wxButton(m_pKeys[i], IDB_BUTTON_B, wxEmptyString, wxPoint(l + 526, t + 82), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonX[i] = new wxButton(m_pKeys[i], IDB_BUTTON_X, wxEmptyString, wxPoint(l + 526, t + 244), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonY[i] = new wxButton(m_pKeys[i], IDB_BUTTON_Y, wxEmptyString, wxPoint(l + 526, t + 173), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyButtonZ[i] = new wxButton(m_pKeys[i], IDB_BUTTON_Z, wxEmptyString, wxPoint(l + 526, t + 147), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);

		// C-buttons
		m_JoyAnalogSubX[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_SUB_X, wxT("0"), wxPoint(l + 552, t + 336), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogSubX[i]->Enable(false);
		m_JoyAnalogSubY[i] = new wxTextCtrl(m_pKeys[i], ID_ANALOG_SUB_Y, wxT("0"), wxPoint(l + 552, t + 373), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyAnalogSubY[i]->Enable(false);
		m_bJoyAnalogSubX[i] = new wxButton(m_pKeys[i], IDB_ANALOG_SUB_X, wxEmptyString, wxPoint(l + 526, t + 338), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_bJoyAnalogSubY[i] = new wxButton(m_pKeys[i], IDB_ANALOG_SUB_Y, wxEmptyString, wxPoint(l + 526, t + 375), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_textSubX[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_SUB_X, wxT("X-axis"), wxPoint(l + 552, t + 321), wxDefaultSize, 0, wxT("X-axis"));
		m_textSubY[i] = new wxStaticText(m_pKeys[i], IDT_ANALOG_SUB_Y, wxT("Y-axis"), wxPoint(l + 552, t + 358), wxDefaultSize, 0, wxT("Y-axis"));
		
		// Start button
		m_bJoyButtonStart[i] = new wxButton(m_pKeys[i], IDB_BUTTONSTART, wxEmptyString, wxPoint(l + 284, t + 365), wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		m_JoyButtonStart[i] = new wxTextCtrl(m_pKeys[i], ID_BUTTONSTART, wxT("0"), wxPoint(l + 220, t + 363), wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonStart[i]->Enable(false);
		
		// Website text
		#ifdef _WIN32
		m_textWebsite[i] = new wxStaticText(m_pKeys[i], IDT_WEBSITE, wxT("www.multigesture.net"), wxPoint(l + 400, t + 380), wxDefaultSize, 0, wxT("www.multigesture.net"));
		#else
		m_textWebsite[i] = new wxStaticText(m_Controller[i], IDT_WEBSITE, wxT("www.multigesture.net"), wxPoint(l + 480, t + 418), wxDefaultSize, 0, wxT("www.multigesture.net"));
		#endif


		// Populate Controller sizer
		// Groups
		#ifdef _WIN32
		m_Joyname[i] = new wxComboBox(m_Controller[i], IDC_JOYNAME, arrayStringFor_Joyname[0], wxDefaultPosition, wxSize(300, 25), arrayStringFor_Joyname, wxCB_READONLY);
		#else
		m_Joyname[i] = new wxComboBox(m_Controller[i], IDC_JOYNAME, arrayStringFor_Joyname[0], wxDefaultPosition, wxSize(300, 25), arrayStringFor_Joyname, 0, wxDefaultValidator, wxT("m_Joyname"));
		#endif
		m_Joyname[i]->SetToolTip(wxT("Save your settings and configure another joypad"));

		m_Enable[i] = new wxCheckBox(m_Controller[i], IDC_ENABLE, wxT("Pad Enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
		m_Enable[i]->SetToolTip(wxT("Enable this pad to send input events to game"));

		m_gJoyname[i] = new wxStaticBoxSizer (wxHORIZONTAL, m_Controller[i], wxT("������"));
		m_gJoyname[i]->Add(m_Joyname[i], 0, wxEXPAND | (wxLEFT | wxRIGHT), 5);
		m_gJoyname[i]->Add(m_Enable[i], 0, wxEXPAND | (wxLEFT | wxRIGHT), 5);

		// General settings

		// General settings 1
		m_JoyButtonHalfpress[i] = new wxTextCtrl(m_Controller[i], ID_BUTTONHALFPRESS, wxT("0"), wxDefaultPosition, wxSize(59, 19), wxTE_READONLY | wxTE_CENTRE, wxDefaultValidator, wxT("0"));
		m_JoyButtonHalfpress[i]->Enable(false);
		m_bJoyButtonHalfpress[i] = new wxButton(m_Controller[i], IDB_BUTTONHALFPRESS, wxEmptyString, wxDefaultPosition, wxSize(21, 14), 0, wxDefaultValidator, wxEmptyString);
		#ifdef _WIN32
		m_Deadzone[i] = new wxComboBox(m_Controller[i], IDC_DEADZONE, wxEmptyString, wxDefaultPosition, wxSize(59, 21), arrayStringFor_Deadzone, wxCB_READONLY, wxDefaultValidator, wxT("m_Deadzone"));
		m_textDeadzone[i] = new wxStaticText(m_Controller[i], IDT_DEADZONE, wxT("Deadzone"));		
		m_textHalfpress[i] = new wxStaticText(m_Controller[i], IDT_BUTTONHALFPRESS, wxT("Half press"));
		#else
		m_Deadzone[i] = new wxComboBox(m_Controller[i], IDC_DEADZONE, wxEmptyString, wxPoint(167, 398), wxSize(80, 25), arrayStringFor_Deadzone, wxCB_READONLY, wxDefaultValidator, wxT("m_Deadzone"));
		m_textDeadzone[i] = new wxStaticText(m_Controller[i], IDT_DEADZONE, wxT("Deadzone"), wxPoint(105, 404), wxDefaultSize, 0, wxT("Deadzone"));		
		m_textHalfpress[i] = new wxStaticText(m_Controller[i], IDT_BUTTONHALFPRESS, wxT("Half press"), wxPoint(105, 428), wxDefaultSize, 0, wxT("Half press"));
		#endif

		// Populate general settings 1		
		m_gExtrasettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("��������"));
		m_gGBExtrasettings[i] = new wxGridBagSizer(0, 0);
		m_gGBExtrasettings[i]->Add(m_textDeadzone[i], wxGBPosition(0, 0), wxGBSpan(1, 1), (wxRIGHT | wxTOP), 3);
		m_gGBExtrasettings[i]->Add(m_Deadzone[i], wxGBPosition(0, 1), wxGBSpan(1, 1), (wxBOTTOM), 2);
		m_gGBExtrasettings[i]->Add(m_textHalfpress[i], wxGBPosition(1, 0), wxGBSpan(1, 1), (wxRIGHT | wxTOP), 3);
		m_gGBExtrasettings[i]->Add(m_JoyButtonHalfpress[i], wxGBPosition(1, 1), wxGBSpan(1, 1), (wxALL), 0);
		m_gGBExtrasettings[i]->Add(m_bJoyButtonHalfpress[i], wxGBPosition(1, 2), wxGBSpan(1, 1), (wxLEFT | wxTOP), 2);
		m_gExtrasettings[i]->Add(m_gGBExtrasettings[i], 0, wxEXPAND | wxALL, 3);

		// Create general settings 2 (controller typ)
		m_TSControltype[i] = new wxStaticText(m_Controller[i], IDT_DPADTYPE, wxT("D-Pad"));		
		m_TSTriggerType[i] = new wxStaticText(m_Controller[i], IDT_TRIGGERTYPE, wxT("Trigger"));
		m_ControlType[i] = new wxComboBox(m_Controller[i], IDC_CONTROLTYPE, wxAS_DPadType[0], wxDefaultPosition, wxDefaultSize, wxAS_DPadType, wxCB_READONLY);
		m_TriggerType[i] = new wxComboBox(m_Controller[i], IDC_TRIGGERTYPE, wxAS_TriggerType[0], wxDefaultPosition, wxDefaultSize, wxAS_TriggerType, wxCB_READONLY);

		// Populate general settings 2 (controller typ)
		m_gGenSettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("D-pad and trigger"));
		m_gGBGenSettings[i] = new wxGridBagSizer(0, 0);
		m_gGBGenSettings[i]->Add(m_TSControltype[i], wxGBPosition(0, 0), wxGBSpan(1, 1), (wxTOP), 4);
		m_gGBGenSettings[i]->Add(m_ControlType[i], wxGBPosition(0, 1), wxGBSpan(1, 1), (wxBOTTOM | wxLEFT), 2);
		m_gGBGenSettings[i]->Add(m_TSTriggerType[i], wxGBPosition(1, 0), wxGBSpan(1, 1), (wxTOP), 4);
		m_gGBGenSettings[i]->Add(m_TriggerType[i], wxGBPosition(1, 1), wxGBSpan(1, 1), (wxLEFT), 2);
		m_gGenSettings[i]->Add(m_gGBGenSettings[i], 0, wxEXPAND | wxALL, 3);		

		// Create objects for general settings 3
		m_gGenSettingsID[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("����") );
		m_CBSaveByID[i] = new wxCheckBox(m_Controller[i], IDC_SAVEBYID, wxT("�� ID ����"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
		m_CBSaveByID[i]->Enable(false);
		m_CBShowAdvanced[i] = new wxCheckBox(m_Controller[i], IDC_SHOWADVANCED, wxT("��ʾ�߼�����"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
		
		// Populate general settings 3
		m_gGenSettingsID[i]->Add(m_CBSaveByID[i], 0, wxEXPAND | wxALL, 3);
		m_gGenSettingsID[i]->Add(m_CBShowAdvanced[i], 0, wxEXPAND | wxALL, 3);
		
		// Create objects for Rumble settings (general 4)	
		m_RStrength[i] = new wxComboBox(m_Controller[i], IDC_RUMBLESTRENGTH, wxAS_RumbleStrength[0], wxDefaultPosition, wxSize(85, 20), wxAS_RumbleStrength, wxCB_READONLY);
		m_Rumble[i] = new wxCheckBox(m_Controller[i], IDC_ENABLERUMBLE, wxT("Enable Rumble"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
#if !SDL_VERSION_ATLEAST(1, 3, 0) && !defined(_WIN32)
		m_Rumble[i]->Disable();
#endif

		// Populate general settings 4
		m_gRumble[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Rumble settings"));
		m_gGBRumble[i] = new wxGridBagSizer(0, 0);
		m_gGBRumble[i]->Add(m_Rumble[i], wxGBPosition(0, 0), wxGBSpan(1, 1), (wxTOP), 1);
		m_gGBRumble[i]->Add(m_RStrength[i], wxGBPosition(1, 0), wxGBSpan(1, 1), (wxTOP), 6);
		m_gRumble[i]->Add(m_gGBRumble[i], 0, wxEXPAND | wxALL, 3);

		// Create tooltips	
		m_ControlType[i]->SetToolTip(wxT(
			"Use a 'hat' on your gamepad or configure a custom button for each direction."
			));
		m_TriggerType[i]->SetToolTip(wxT(
			"Select XInput if you want the triggers to work with the XBox 360 pad."
			));
		m_CBSaveByID[i]->SetToolTip(wxString::Format(
			wxT("Map these settings to the selected controller device instead of to the")
			wxT("\nselected slot (1, 2, 3 or 4). This may be a more convenient way")
			wxT("\nto save your settings if you have multiple controllers.")
			, i+1
			));	

		// Populate settings
		m_sSettings[i] = new wxBoxSizer ( wxHORIZONTAL );
		m_sSettings[i]->Add(m_gExtrasettings[i], 0, wxEXPAND | wxALL, 0);
		m_sSettings[i]->Add(m_gGenSettings[i], 0, wxEXPAND | wxLEFT, 5);
		m_sSettings[i]->Add(m_gGenSettingsID[i], 0, wxEXPAND | wxLEFT, 5);
		m_sSettings[i]->Add(m_gRumble[i], 0, wxEXPAND | wxLEFT, 5);

		// Advanced settings

		// Input status controls
		
		// Input status text
		CreateAdvancedControls(i);

		// Main-stick sizers
		m_GBAdvancedMainStick[i] = new wxGridBagSizer(0, 0);
		m_GBAdvancedMainStick[i]->Add(m_pInStatus[i], wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL, 0);
		m_GBAdvancedMainStick[i]->Add(m_pOutStatus[i], wxGBPosition(0, 1), wxGBSpan(1, 1), wxLEFT, 5);
		m_GBAdvancedMainStick[i]->Add(m_TStatusIn[i], wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 0);
		m_GBAdvancedMainStick[i]->Add(m_TStatusOut[i], wxGBPosition(1, 1), wxGBSpan(1, 1), wxLEFT, 5);
		// Cstick sizers
		m_GBAdvancedCStick[i] = new wxGridBagSizer(0, 0);
		m_GBAdvancedCStick[i]->Add(m_pInStatusC[i], wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL, 0);
		m_GBAdvancedCStick[i]->Add(m_pOutStatusC[i], wxGBPosition(0, 1), wxGBSpan(1, 1), wxLEFT, 5);
		m_GBAdvancedCStick[i]->Add(m_TStatusInC[i], wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL, 0);
		m_GBAdvancedCStick[i]->Add(m_TStatusOutC[i], wxGBPosition(1, 1), wxGBSpan(1, 1), wxLEFT, 5);
		// Add sizers
		m_gStatusIn[i]->Add(m_GBAdvancedMainStick[i], 0, wxLEFT, 5);
		m_gStatusInC[i]->Add(m_GBAdvancedCStick[i], 0, wxLEFT, 5);

		// Populate input status settings

		// The drop down menu
		m_gStatusInSettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Main-stick ����"));
		m_gStatusInSettingsRadiusH[i] = new wxBoxSizer(wxHORIZONTAL);
		m_gStatusInSettingsC[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("C-stick ����"));
		m_gStatusInSettingsRadiusHC[i] = new wxBoxSizer(wxHORIZONTAL);
		wxArrayString asRadius;
			asRadius.Add(wxT("100%"));
			asRadius.Add(wxT("90%"));
			asRadius.Add(wxT("80%"));
			asRadius.Add(wxT("70%"));
			asRadius.Add(wxT("60%"));
			asRadius.Add(wxT("50%"));
			asRadius.Add(wxT("40%"));
		m_CoBRadius[i] = new wxComboBox(m_Controller[i], IDCB_MAINSTICK_RADIUS, asRadius[0], wxDefaultPosition, wxDefaultSize, asRadius, wxCB_READONLY);
		m_CoBRadiusC[i] = new wxComboBox(m_Controller[i], IDCB_CSTICK_RADIUS, asRadius[0], wxDefaultPosition, wxDefaultSize, asRadius, wxCB_READONLY);

		// The checkbox
		m_CBRadius[i] = new wxCheckBox(m_Controller[i], IDCB_MAINSTICK_CB_RADIUS, wxT("Radius"));
		m_CBRadiusC[i] = new wxCheckBox(m_Controller[i], IDCB_CSTICK_CB_RADIUS, wxT("Radius"));
		wxString CBRadiusToolTip = wxT("This will reduce the stick radius.");
		m_CBRadius[i]->SetToolTip(CBRadiusToolTip);
		m_CBRadiusC[i]->SetToolTip(CBRadiusToolTip);

		// The drop down menu);
		m_gStatusInSettingsH[i] = new wxBoxSizer(wxHORIZONTAL);
		m_gStatusInSettingsHC[i] = new wxBoxSizer(wxHORIZONTAL);
		wxArrayString asStatusInSet;
			asStatusInSet.Add(wxT("100%"));
			asStatusInSet.Add(wxT("95%"));
			asStatusInSet.Add(wxT("90%"));
			asStatusInSet.Add(wxT("85%"));
			asStatusInSet.Add(wxT("80%"));
			asStatusInSet.Add(wxT("75%"));
		m_CoBDiagonal[i] = new wxComboBox(m_Controller[i], IDCB_MAINSTICK_DIAGONAL, asStatusInSet[0], wxDefaultPosition, wxDefaultSize, asStatusInSet, wxCB_READONLY);
		m_CoBDiagonalC[i] = new wxComboBox(m_Controller[i], IDCB_CSTICK_DIAGONAL, asStatusInSet[0], wxDefaultPosition, wxDefaultSize, asStatusInSet, wxCB_READONLY);

		// The checkbox
		m_CBS_to_C[i] = new wxCheckBox(m_Controller[i], IDCB_MAINSTICK_S_TO_C, wxT("Diagonal"));
		m_CBS_to_CC[i] = new wxCheckBox(m_Controller[i], IDCB_CSTICK_S_TO_C, wxT("Diagonal"));
		wxString CBS_to_CToolTip = 
			wxT("This will convert a square stick radius to a circle stick radius similar to the octagonal area that the original GameCube pad produce.")
			wxT(" To produce a smooth circle in the 'Out' window you have to manually set")
			wxT(" your diagonal values from the 'In' window in the drop down menu.");
		m_CBS_to_C[i]->SetToolTip(CBS_to_CToolTip);
		m_CBS_to_CC[i]->SetToolTip(CBS_to_CToolTip);

		// Populate sizers
		m_gStatusInSettings[i]->Add(m_gStatusInSettingsRadiusH[i], 0, (wxLEFT | wxRIGHT | wxBOTTOM), 4);
		m_gStatusInSettings[i]->Add(m_gStatusInSettingsH[i], 0, (wxLEFT | wxRIGHT | wxBOTTOM), 4);	
		// C-stick
		m_gStatusInSettingsC[i]->Add(m_gStatusInSettingsRadiusHC[i], 0, (wxLEFT | wxRIGHT | wxBOTTOM), 4);
		m_gStatusInSettingsC[i]->Add(m_gStatusInSettingsHC[i], 0, (wxLEFT | wxRIGHT | wxBOTTOM), 4);		

		m_gStatusInSettingsRadiusH[i]->Add(m_CBRadius[i], 0, wxLEFT | wxTOP, 3);
		m_gStatusInSettingsRadiusH[i]->Add(m_CoBRadius[i], 0, wxLEFT, 3);
		m_gStatusInSettingsH[i]->Add(m_CBS_to_C[i], 0, wxLEFT | wxTOP, 3);
		m_gStatusInSettingsH[i]->Add(m_CoBDiagonal[i], 0, wxLEFT, 3);
		// C-stick
		m_gStatusInSettingsRadiusHC[i]->Add(m_CBRadiusC[i], 0, wxLEFT | wxTOP, 3);
		m_gStatusInSettingsRadiusHC[i]->Add(m_CoBRadiusC[i], 0, wxLEFT, 3);
		m_gStatusInSettingsHC[i]->Add(m_CBS_to_CC[i], 0, wxLEFT | wxTOP, 3);
		m_gStatusInSettingsHC[i]->Add(m_CoBDiagonalC[i], 0, wxLEFT, 3);

		// The trigger values
		m_gStatusTriggers[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("Trigger values"));
		m_TStatusTriggers[i] = new wxStaticText(m_Controller[i], IDT_TRIGGERS, wxT("Left:  Right:"));
		m_gStatusTriggers[i]->Add(m_TStatusTriggers[i], 0, (wxALL), 4);

		m_gStatusAdvancedSettings[i] = new wxStaticBoxSizer( wxVERTICAL, m_Controller[i], wxT("�߼�����"));
		m_CBCheckFocus[i] = new wxCheckBox(m_Controller[i], IDCB_CHECKFOCUS, wxT("Allow out of focus input"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
		m_AdvancedMapFilter[i] = new wxCheckBox(m_Controller[i], IDCB_FILTER_SETTINGS , wxT("No trigger filter"));
		m_gStatusAdvancedSettings[i]->Add(m_CBCheckFocus[i], 0, (wxALL), 4);
		m_gStatusAdvancedSettings[i]->Add(m_AdvancedMapFilter[i], 0, (wxALL), 4);

		// Tool tips
		m_CBCheckFocus[i]->SetToolTip(wxT(
			"Allow gamepad input even when Dolphin is not in focus. Out of focus keyboard input is never allowed."));
		m_AdvancedMapFilter[i]->SetToolTip(
			wxT("This will allow you to map a digital axis to the main stick or the C-stick. If you don't have")
			wxT(" any analog triggers that will be automatically set when the trigger filter is off.")
			);
		

		// Populate sizers

		// Populate main left sizer
		m_sMainLeft[i] = new wxBoxSizer(wxVERTICAL);
		m_sMainLeft[i]->Add(m_gJoyname[i], 0, wxEXPAND | (wxALL), 5);
		m_sMainLeft[i]->Add(m_sKeys[i], 1, wxEXPAND | (wxLEFT | wxRIGHT), 5);
		m_sMainLeft[i]->Add(m_sSettings[i], 0, wxEXPAND | (wxALL), 5);

		// Populate main right sizer
		m_sMainRight[i] = new wxBoxSizer(wxVERTICAL);
		m_sMainRight[i]->Add(m_gStatusIn[i], 0, wxEXPAND | (wxLEFT), 2);
		m_sMainRight[i]->Add(m_gStatusInSettings[i], 0, wxEXPAND | (wxLEFT | wxTOP), 2);	
		m_sMainRight[i]->Add(m_gStatusInC[i], 0, wxEXPAND | (wxLEFT), 2);
		m_sMainRight[i]->Add(m_gStatusInSettingsC[i], 0, wxEXPAND | (wxLEFT | wxTOP), 2);
		m_sMainRight[i]->Add(m_gStatusTriggers[i], 0, wxEXPAND | (wxLEFT | wxTOP), 2);
		m_sMainRight[i]->Add(m_gStatusAdvancedSettings[i], 0, wxEXPAND | (wxLEFT | wxTOP), 2);
#ifdef RERECORDING
		m_sMainRight[i]->Add(m_SizeRecording[i], 0, wxEXPAND | (wxLEFT | wxTOP), 2);
#endif

		// Populate main sizer
		m_sMain[i] = new wxBoxSizer(wxHORIZONTAL);
		m_sMain[i]->Add(m_sMainLeft[i], 0, wxEXPAND | (wxALL), 0);
		m_sMain[i]->Add(m_sMainRight[i], 0, wxEXPAND | (wxRIGHT | wxTOP), 5);
		m_Controller[i]->SetSizerAndFit(m_sMain[i]); // Set the main sizer

		// Show or hide it. We have to do this after we add it to its sizer
		m_sMainRight[i]->Show(g_Config.bShowAdvanced);

		// Update GUI
		UpdateGUI(i);
	} // end of loop

	// Populate buttons sizer.
	wxBoxSizer * m_sButtons = new wxBoxSizer(wxHORIZONTAL);
	m_sButtons->Add(m_About, 0, (wxBOTTOM), 0);
	m_sButtons->AddStretchSpacer(1);
	m_sButtons->Add(m_OK, 0, wxALIGN_RIGHT | (wxBOTTOM), 0);
	m_sButtons->Add(m_Cancel, 0, wxALIGN_RIGHT | (wxLEFT), 5);


	// Populate master sizer.
	m_MainSizer = new wxBoxSizer(wxVERTICAL);
	m_MainSizer->Add(m_Notebook, 0, wxEXPAND | wxALL, 5);
	m_MainSizer->Add(m_sButtons, 1, wxEXPAND | ( wxLEFT | wxRIGHT | wxBOTTOM), 5);
	SetSizerAndFit(m_MainSizer);

	// Debugging
	#ifdef SHOW_PAD_STATUS
		m_pStatusBar = new wxStaticText(this, IDT_DEBUGGING, wxT("Debugging"), wxPoint(135, 100), wxDefaultSize);
	#endif

	// Set window size
	SetSizer(m_MainSizer);
	Layout();
	Fit();
	// Center the window if there is room for it
	#ifdef _WIN32
		if (GetSystemMetrics(SM_CYFULLSCREEN) > 600)
			Center();
	#endif

	// All done
	ControlsCreated = true;

	// Replace the harder to understand -1 with "" for the sake of user friendliness
	ToBlank();
}
