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

#ifndef __BASICCONFIGDIALOG_h__
#define __BASICCONFIGDIALOG_h__

#include <iostream>
#include <vector>

#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/gbsizer.h>
#include "wiimote_hid.h"

class WiimoteBasicConfigDialog : public wxDialog
{
	public:
		WiimoteBasicConfigDialog(wxWindow *parent,
			wxWindowID id = wxID_ANY,
			const wxString &title = wxT("Wii Remote Plugin Configuration"),
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS);
		virtual ~WiimoteBasicConfigDialog(){;}

		// General open, close and event functions
		void UpdateGUI();
		void UpdateBasicConfigDialog(bool state);
		void ButtonClick(wxCommandEvent& event);
		void ShutDown(wxTimerEvent& WXUNUSED(event));
		void UpdateOnce(wxTimerEvent& event);
		
		// Timers
		wxTimer	*m_TimeoutOnce,
				*m_ShutDownTimer;

	private:
		DECLARE_EVENT_TABLE();

		bool ControlsCreated;
		int m_Page;

		wxNotebook *m_Notebook;
		wxPanel *m_Controller[MAX_WIIMOTES];

		wxButton *m_OK,
				 *m_Cancel,
				 *m_ButtonMapping,
				 *m_Recording,
				 *m_PairUpRealWiimote[MAX_WIIMOTES],
				 *m_ConnectRealWiimote[MAX_WIIMOTES];

		wxChoice *m_InputSource[MAX_WIIMOTES],
				 *m_Extension[MAX_WIIMOTES];

		wxSlider *m_SliderWidth[MAX_WIIMOTES],
				 *m_SliderHeight[MAX_WIIMOTES],
				 *m_SliderLeft[MAX_WIIMOTES],
				 *m_SliderTop[MAX_WIIMOTES],
				 *m_WiimoteTimeout[MAX_WIIMOTES],
				 *m_SliderIrLevel[MAX_WIIMOTES];

		wxCheckBox  *m_SidewaysWiimote[MAX_WIIMOTES],
					*m_UprightWiimote[MAX_WIIMOTES],
					*m_WiiMotionPlusConnected[MAX_WIIMOTES],
					*m_CheckAR43[MAX_WIIMOTES],
					*m_CheckAR169[MAX_WIIMOTES],
					*m_Crop[MAX_WIIMOTES],
					*m_WiiAutoReconnect[MAX_WIIMOTES],
					*m_WiiAutoUnpair[MAX_WIIMOTES],
					*m_WiiExtendedPairUp[MAX_WIIMOTES],
					*m_UDPWiiEnable[MAX_WIIMOTES],
					*m_UDPWiiAccel[MAX_WIIMOTES],
					*m_UDPWiiButt[MAX_WIIMOTES],
					*m_UDPWiiIR[MAX_WIIMOTES],
					*m_UDPWiiNun[MAX_WIIMOTES];

		wxStaticText *m_TextWiimoteTimeout[MAX_WIIMOTES],
					 *m_TextScreenWidth[MAX_WIIMOTES],
					 *m_TextScreenHeight[MAX_WIIMOTES],
					 *m_TextScreenLeft[MAX_WIIMOTES],
					 *m_TextScreenTop[MAX_WIIMOTES],
					 *m_TextScreenIrLevel[MAX_WIIMOTES],
					 *m_TextAR[MAX_WIIMOTES],
					 *m_TextFoundRealWiimote[MAX_WIIMOTES],
					 *m_TextUDPWiiPort[MAX_WIIMOTES];

		wxBoxSizer  *m_MainSizer,
					*m_SizeBasicGeneral[MAX_WIIMOTES],
					*m_SizeBasicGeneralLeft[MAX_WIIMOTES],
					*m_SizeBasicGeneralRight[MAX_WIIMOTES],
					*m_SizeRealTimeout[MAX_WIIMOTES],
					*m_SizeRealRefreshPair[MAX_WIIMOTES],
					*m_SizerIRPointerWidth[MAX_WIIMOTES],
					*m_SizerIRPointerHeight[MAX_WIIMOTES],
					*m_SizerIRPointerScreen[MAX_WIIMOTES],
					*m_SizerIRPointerSensitivity[MAX_WIIMOTES],
					*m_SizeUDPWiiPort[MAX_WIIMOTES];

		wxStaticBoxSizer *m_SizeBasic[MAX_WIIMOTES],
						 *m_SizeEmu[MAX_WIIMOTES],
						 *m_SizeReal[MAX_WIIMOTES],
						 *m_SizeRealAuto[MAX_WIIMOTES],
						 *m_SizeExtensions[MAX_WIIMOTES],
						 *m_SizerIRPointer[MAX_WIIMOTES],
						 *m_SizeUDPWii[MAX_WIIMOTES];
		
		wxTextCtrl *m_UDPWiiPort[MAX_WIIMOTES];

		enum
		{
			ID_BUTTONMAPPING,
			ID_BUTTONRECORDING,
			IDTM_SHUTDOWN,
			IDTM_UPDATE,
			IDTM_UPDATE_ONCE,
			
			ID_NOTEBOOK,
			ID_CONTROLLERPAGE1,
			ID_CONTROLLERPAGE2,
			ID_CONTROLLERPAGE3,
			ID_CONTROLLERPAGE4,
			
			// Emulated/Real Wiimote
			IDC_INPUT_SOURCE,
			IDC_SIDEWAYSWIIMOTE,
			IDC_UPRIGHTWIIMOTE,
			IDC_MOTIONPLUSCONNECTED,
			IDC_WIIAUTORECONNECT,
			IDC_WIIAUTOUNPAIR,
			IDC_WIIAUTOPAIR,
			IDC_EXTCONNECTED,

			//UDPWii
			IDC_UDPWIIENABLE,
			IDC_UDPWIIACCEL,
			IDC_UDPWIIBUTT,
			IDC_UDPWIIIR,
			IDC_UDPWIINUN,
			IDT_UDPWIIPORT,

			// Real
			IDB_PAIRUP_REAL,
			IDB_REFRESH_REAL,

			IDS_WIDTH,			
			IDS_HEIGHT, 
			IDS_LEFT,
			IDS_TOP,
			IDS_LEVEL,
			IDS_TIMEOUT,
		};

		void CreateGUIControls();
		void OnClose(wxCloseEvent& event);
		void NotebookPageChanged(wxNotebookEvent& event);
		void GeneralSettingsChanged(wxCommandEvent& event);
		void IRCursorChanged(wxScrollEvent& event);
		void UDPWiiSettingsChanged(wxCommandEvent& event);

		void DoRefreshReal();	// Real
		void DoUseReal();

		void DoExtensionConnectedDisconnected(int Extension = -1); // Emulated
};

extern WiimoteBasicConfigDialog *m_BasicConfigFrame;
#endif
