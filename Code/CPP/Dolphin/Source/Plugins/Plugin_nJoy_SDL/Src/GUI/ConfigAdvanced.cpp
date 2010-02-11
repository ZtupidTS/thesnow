
// Project description
// -------------------
// Name: nJoy 
// Description: A Dolphin Compatible Input Plugin
//
// Author: Falcon4ever (nJoy@falcon4ever.com)
// Site: www.multigesture.net
// Copyright (C) 2003 Dolphin Project.
//

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

// Include
// ---------
#include "ConfigBox.h"
#include "../nJoy.h"
#include "Images/controller.xpm"

extern bool g_EmulatorRunning;

// Set PAD status
// --------------
void PADConfigDialognJoy::PadGetStatus()
{
	// Return if it's not detected. The ID should never be less than zero here, it can only be that
	// because of a manual ini file change, but we make that check anway.
	if(PadMapping[notebookpage].ID < 0 || PadMapping[notebookpage].ID >= SDL_NumJoysticks())
	{
		m_TStatusIn[notebookpage]->SetLabel(wxT("Not connected")); m_TStatusOut[notebookpage]->SetLabel(wxT("Not connected"));
		m_TStatusInC[notebookpage]->SetLabel(wxT("Not connected")); m_TStatusOutC[notebookpage]->SetLabel(wxT("Not connected"));
		m_TStatusTriggers[notebookpage]->SetLabel(wxT("Not connected"));
		return;
	}

	// Get physical device status
	int PhysicalDevice = PadMapping[notebookpage].ID;
	int TriggerType = PadMapping[notebookpage].triggertype;

	// Get original values
	int main_x = PadState[notebookpage].axis[InputCommon::CTL_MAIN_X];
	int main_y = PadState[notebookpage].axis[InputCommon::CTL_MAIN_Y];
    int sub_x = PadState[notebookpage].axis[InputCommon::CTL_SUB_X];
	int sub_y = PadState[notebookpage].axis[InputCommon::CTL_SUB_Y];

	// Get adjusted values
	int main_x_after = main_x, main_y_after = main_y;
	if(PadMapping[notebookpage].bSquareToCircle) InputCommon::Square2Circle(main_x_after, main_y_after, notebookpage, PadMapping[notebookpage].SDiagonal);
	// Adjust radius
	if(PadMapping[notebookpage].bRadiusOnOff) InputCommon::RadiusAdjustment(main_x_after, main_y_after, notebookpage, PadMapping[notebookpage].SRadius);
	// C-stick
	int sub_x_after = sub_x, sub_y_after = sub_y;
	if(PadMapping[notebookpage].bSquareToCircleC) InputCommon::Square2Circle(sub_x_after, sub_y_after, notebookpage, PadMapping[notebookpage].SDiagonalC);
	if(PadMapping[notebookpage].bRadiusOnOffC) InputCommon::RadiusAdjustment(sub_x_after, sub_y_after, notebookpage, PadMapping[notebookpage].SRadiusC);

	// Convert values
	float f_x = main_x / 32767.0;
	float f_y = main_y / 32767.0;
	float f_x_aft = main_x_after / 32767.0;
	float f_y_aft = main_y_after / 32767.0;
	// C-stick
	float f_x_c = sub_x / 32767.0;
	float f_y_c = sub_y / 32767.0;
	float f_x_aft_c = sub_x_after / 32767.0;
	float f_y_aft_c = sub_y_after / 32767.0;
	// Print values
	m_TStatusIn[notebookpage]->SetLabel(wxString::Format(wxT("x:%1.2f y:%1.2f"), f_x, f_y));
	m_TStatusOut[notebookpage]->SetLabel(wxString::Format(wxT("x:%1.2f y:%1.2f"), f_x_aft, f_y_aft));
	m_TStatusInC[notebookpage]->SetLabel(wxString::Format(wxT("x:%1.2f y:%1.2f"), f_x_c, f_y_c));
	m_TStatusOutC[notebookpage]->SetLabel(wxString::Format(wxT("x:%1.2f y:%1.2f"), f_x_aft_c, f_y_aft_c));

	// Adjust the values for the plot
	// Border adjustment
	int BoxW_ = BoxW - 2; int BoxH_ = BoxH - 2;
	main_x = (BoxW_ / 2) + (main_x * BoxW_ / (32767 * 2));
	main_y = (BoxH_ / 2) + (main_y * BoxH_ / (32767 * 2));
	int main_x_out = (BoxW_ / 2) + (main_x_after * BoxW_ / (32767 * 2));
	int main_y_out = (BoxH_ / 2) + (main_y_after * BoxH_ / (32767 * 2));	
	// Adjust the dot
	m_bmpDot[notebookpage]->SetPosition(wxPoint(main_x, main_y));
	m_bmpDotOut[notebookpage]->SetPosition(wxPoint(main_x_out, main_y_out));
	// C-stick
	sub_x = (BoxW_ / 2) + (sub_x * BoxW_ / (32767 * 2));
	sub_y = (BoxH_ / 2) + (sub_y * BoxH_ / (32767 * 2));
	int sub_x_out = (BoxW_ / 2) + (sub_x_after * BoxW_ / (32767 * 2));
	int sub_y_out = (BoxH_ / 2) + (sub_y_after * BoxH_ / (32767 * 2));	
	m_bmpDotC[notebookpage]->SetPosition(wxPoint(sub_x, sub_y));
	m_bmpDotOutC[notebookpage]->SetPosition(wxPoint(sub_x_out, sub_y_out));
	///////////////////// Analog stick


	
	// Triggers
	// -----------------
	int TriggerValue = 255;
	if (PadState[notebookpage].halfpress) TriggerValue = 100;

	// Get the selected keys
	long Left, Right;
	m_JoyShoulderL[notebookpage]->GetValue().ToLong(&Left);
	m_JoyShoulderR[notebookpage]->GetValue().ToLong(&Right);

	// Get the trigger values
	int TriggerLeft = PadState[notebookpage].axis[InputCommon::CTL_L_SHOULDER];
	int TriggerRight = PadState[notebookpage].axis[InputCommon::CTL_R_SHOULDER];

	// Convert the triggers values
	if (PadMapping[notebookpage].triggertype == InputCommon::CTL_TRIGGER_SDL)
	{
		TriggerLeft = InputCommon::Pad_Convert(TriggerLeft);
		TriggerRight = InputCommon::Pad_Convert(TriggerRight);
	}

	// If we don't have any axis selected for the shoulder buttons
	if(Left < 1000) TriggerLeft = 0;
	if(Right < 1000) TriggerRight = 0;

	// Get the digital values
	if(Left < 1000 && PadState[notebookpage].buttons[InputCommon::CTL_L_SHOULDER]) TriggerLeft = TriggerValue;
	if(Right < 1000 && PadState[notebookpage].buttons[InputCommon::CTL_R_SHOULDER]) TriggerRight = TriggerValue;

	m_TStatusTriggers[notebookpage]->SetLabel(wxString::Format(
		wxT("Left:%03i  Right:%03i"),
		TriggerLeft, TriggerRight	
		));
	///////////////////// Triggers
}

// Show the current pad status
// -----------------
std::string ShowStatus(int VirtualController)
{
	// Save the physical device
	int PhysicalDevice = PadMapping[VirtualController].ID;

	// Make local shortcut
	SDL_Joystick *joy = PadState[VirtualController].joy;

	// Make shortcuts for all pads
	SDL_Joystick *joy0 = PadState[0].joy;
	SDL_Joystick *joy1 = PadState[1].joy;
	SDL_Joystick *joy2 = PadState[2].joy;
	SDL_Joystick *joy3 = PadState[3].joy;

	// Temporary storage
	std::string StrAxes, StrHats, StrBut;
	int value;

	// Get status
	int Axes = joyinfo[PhysicalDevice].NumAxes;
	int Balls = joyinfo[PhysicalDevice].NumBalls;
	int Hats = joyinfo[PhysicalDevice].NumHats;
	int Buttons = joyinfo[PhysicalDevice].NumButtons;

	// Update the internal values
	SDL_JoystickUpdate();

	// Go through all axes and read out their values
	for(int i = 0; i < Axes; i++)
	{	
		value = SDL_JoystickGetAxis(joy, i);
		StrAxes += StringFromFormat(" %i:%06i", i, value);
	}
	for(int i = 0;i < Hats; i++)
	{	
		value = SDL_JoystickGetHat(joy, i);
		StrHats += StringFromFormat(" %i:%i", i, value);
	}
	for(int i = 0;i < Buttons; i++)
	{	
		value = SDL_JoystickGetButton(joy, i);
		StrBut += StringFromFormat(" %i:%i", i+1, value);
	}

	return StringFromFormat(
		"All pads:\n"
		"ID: %i %i %i %i\n"
		"Controllertype: %i %i %i %i\n"
		"SquareToCircle: %i %i %i %i\n\n"	
		#ifdef _WIN32
			"Handles: %i %i %i %i\n"
			"XInput: %i %i %i\n"
		#endif

		"This pad:\n"
		"Axes: %s\n"
		"Hats: %s\n"
		"But: %s\n"
		"Device: Ax: %i Balls:%i Hats:%i But:%i",
		PadMapping[0].ID, PadMapping[1].ID, PadMapping[2].ID, PadMapping[3].ID,
		PadMapping[0].controllertype, PadMapping[1].controllertype, PadMapping[2].controllertype, PadMapping[3].controllertype,
		PadMapping[0].bSquareToCircle, PadMapping[1].bSquareToCircle, PadMapping[2].bSquareToCircle, PadMapping[3].bSquareToCircle,
		#ifdef _WIN32
			joy0, joy1, joy2, joy3,
			//PadState[PadMapping[0].ID].joy, PadState[PadMapping[1].ID].joy, PadState[PadMapping[2].ID].joy, PadState[PadMapping[3].ID].joy,
			XInput::IsConnected(0), XInput::GetXI(0, InputCommon::XI_TRIGGER_L), XInput::GetXI(0, InputCommon::XI_TRIGGER_R),
		#endif
		StrAxes.c_str(), StrHats.c_str(), StrBut.c_str(),
		Axes, Balls, Hats, Buttons
		);
}

// Populate the advanced tab
// -----------------
void PADConfigDialognJoy::Update()
{
	// Check that Dolphin is in focus, otherwise don't update the pad status
	/* If the emulator is running and unpaused GetJoyState() is run a little more often than needed,
	   but I allow that since it can confuse the user if the input status in the configuration window
	   is not update when the emulator is paused. */
	if (g_Config.bCheckFocus || IsFocus()) // && !g_EmulatorRunning)
	{
		for (int i = 0; (u32)i < joyinfo.size(); i++)
			InputCommon::GetJoyState(PadState[i], PadMapping[i], i, joyinfo[PadMapping[i].ID].NumButtons);
	}

	// Show the current status in a window in the wxPanel
	#ifdef SHOW_PAD_STATUS
		m_pStatusBar->SetLabel(wxString::Format(
			"%s", ShowStatus(notebookpage).c_str()
			));
	#endif
}


// Populate the advanced tab
// -----------------
void PADConfigDialognJoy::CreateAdvancedControls(int i)
{
	// Main-stick
	m_TStatusIn[i] = new wxStaticText(m_Controller[i], IDT_STATUS_IN, wxT("In"));
	m_TStatusOut[i] = new wxStaticText(m_Controller[i], IDT_STATUS_OUT, wxT("Out"));
	m_gStatusIn[i] = new wxStaticBoxSizer( wxHORIZONTAL, m_Controller[i], wxT("Main-stick (In) (Out)"));
	// C-stick
	m_TStatusInC[i] = new wxStaticText(m_Controller[i], IDT_STATUS_IN, wxT("In"));
	m_TStatusOutC[i] = new wxStaticText(m_Controller[i], IDT_STATUS_OUT, wxT("Out"));
	m_gStatusInC[i] = new wxStaticBoxSizer( wxHORIZONTAL, m_Controller[i], wxT("C-stick (In) (Out)"));

	// Main-stick In-Out
	m_pInStatus[i] = new wxPanel(m_Controller[i], ID_INSTATUS1 + i, wxDefaultPosition, wxDefaultSize);
	m_bmpSquare[i] = new wxStaticBitmap(m_pInStatus[i], ID_STATUSBMP1 + i, CreateBitmap(),
		//wxPoint(4, 15), wxSize(70,70));
		//wxPoint(4, 20), wxDefaultSize);
		wxDefaultPosition, wxDefaultSize);
	m_bmpDot[i] = new wxStaticBitmap(m_pInStatus[i], ID_STATUSDOTBMP1 + i, CreateBitmapDot(),
		wxPoint(BoxW / 2, BoxH / 2), wxDefaultSize);

	m_pOutStatus[i] = new wxPanel(m_Controller[i], ID_INSTATUS1 + i, wxDefaultPosition, wxDefaultSize);
	m_bmpSquareOut[i] = new wxStaticBitmap(m_pOutStatus[i], ID_STATUSBMP1 + i, CreateBitmap(),
		//wxPoint(4, 15), wxSize(70,70));
		//wxPoint(4, 20), wxDefaultSize);
		wxDefaultPosition, wxDefaultSize);
	// Yes the diagonals for the original GC controller are this narrow (i.e. around 80% of the full radius),
	// it's not a perfect octagon. Some third party GC controllers has a diagonal at 90% however,
	// i.e. at around 63,63 rather than 55,55.
	m_bmpAreaOut[i] = new wxStaticBitmap(m_pOutStatus[i], wxID_ANY, CreateBitmapArea(100,55),
		wxPoint(1, 1), wxDefaultSize);
	m_bmpDotOut[i] = new wxStaticBitmap(m_pOutStatus[i], ID_STATUSDOTBMP1 + i, CreateBitmapDot(),
		wxPoint(BoxW / 2, BoxH / 2), wxDefaultSize);

	// C-stick In-Out
	m_pInStatusC[i] = new wxPanel(m_Controller[i], ID_INSTATUS1 + i, wxDefaultPosition, wxDefaultSize);
	m_bmpSquareC[i] = new wxStaticBitmap(m_pInStatusC[i], wxID_ANY, CreateBitmap(),
		//wxPoint(4, 15), wxSize(70,70));
		//wxPoint(4, 20), wxDefaultSize);
		wxDefaultPosition, wxDefaultSize);
	m_bmpDotC[i] = new wxStaticBitmap(m_pInStatusC[i], wxID_ANY, CreateBitmapDot(),
		wxPoint(BoxW / 2, BoxH / 2), wxDefaultSize);

	m_pOutStatusC[i] = new wxPanel(m_Controller[i], ID_INSTATUS1 + i, wxDefaultPosition, wxDefaultSize);
	m_bmpSquareOutC[i] = new wxStaticBitmap(m_pOutStatusC[i], wxID_ANY, CreateBitmap(),
		//wxPoint(4, 15), wxSize(70,70));
		//wxPoint(4, 20), wxDefaultSize);
		wxDefaultPosition, wxDefaultSize);
	m_bmpAreaOutC[i] = new wxStaticBitmap(m_pOutStatusC[i], wxID_ANY, CreateBitmapArea(85,46),
		wxPoint(1, 1), wxDefaultSize);
	m_bmpDotOutC[i] = new wxStaticBitmap(m_pOutStatusC[i], wxID_ANY, CreateBitmapDot(),
		wxPoint(BoxW / 2, BoxH / 2), wxDefaultSize);
}


wxBitmap PADConfigDialognJoy::CreateBitmap() // Create box
{
	BoxW = 70, BoxH = 70;
	wxBitmap bitmap(BoxW, BoxH);
	wxMemoryDC dc;
	dc.SelectObject(bitmap);

	// Set outline and fill colors
	//wxBrush LightBlueBrush(_T("#0383f0"));
	//wxPen LightBluePen(_T("#80c5fd"));
	//wxPen LightGrayPen(_T("#909090"));
	wxPen LightBluePen(_T("#7f9db9")); // Windows XP color	
	dc.SetPen(LightBluePen);
	dc.SetBrush(*wxWHITE_BRUSH);

	dc.Clear();
	dc.DrawRectangle(0, 0, BoxW, BoxH);
	dc.SelectObject(wxNullBitmap);
	return bitmap;
}

// Create dot
wxBitmap PADConfigDialognJoy::CreateBitmapDot()
{
	int w = 2, h = 2;
	wxBitmap bitmap(w, h);
	wxMemoryDC dc;
	dc.SelectObject(bitmap);

	// Set outline and fill colors
	//wxBrush RedBrush(_T("#0383f0"));	
	//wxPen RedPen(_T("#80c5fd"));
	//wxPen LightGrayPen(_T("#909090"));
	dc.SetPen(*wxRED_PEN);
	dc.SetBrush(*wxRED_BRUSH);

	dc.Clear();
	dc.DrawRectangle(0, 0, w, h);
	dc.SelectObject(wxNullBitmap);
	return bitmap;
}
wxBitmap PADConfigDialognJoy::CreateBitmapArea(int Max, int Diagonal)
{
	wxBitmap bitmap(BoxW - 2, BoxH - 2);
	wxMemoryDC dc;
	dc.SelectObject(bitmap);

	// Set outline and fill colors
	// wxMEDIUM_GREY_PEN, wxLIGHT_GREY_BRUSH
	wxBrush LightGrayBrush(_T("#dddddd"));
	wxPen LightGrayPen(_T("#bfbfbf"));
	dc.SetBrush(LightGrayBrush);
	dc.SetPen(LightGrayPen);
	
	// Clear bitmap
	dc.Clear();

	// Create offset for polygon
	float Adj = (float)(BoxW-2) / 256.0;
	float iAdj = 127.0 * Adj;
	// The polygon corners
    wxPoint Points[8];
    Points[0].x = (int)(0.0 * Adj + iAdj); Points[0].y = (int)(Max * Adj + iAdj);
    Points[1].x = (int)(Diagonal * Adj + iAdj); Points[1].y = (int)(Diagonal * Adj + iAdj);
    Points[2].x = (int)(Max * Adj + iAdj); Points[2].y = (int)(0.0 * Adj + iAdj);
	Points[3].x = (int)(Diagonal * Adj + iAdj); Points[3].y = (int)(-Diagonal * Adj + iAdj);
	Points[4].x = (int)(0.0 * Adj + iAdj); Points[4].y = (int)(-Max * Adj + iAdj);
	Points[5].x = (int)(-Diagonal * Adj + iAdj); Points[5].y = (int)(-Diagonal * Adj + iAdj);
	Points[6].x = (int)(-Max * Adj + iAdj); Points[6].y = (int)(0.0 * Adj + iAdj);
	Points[7].x = (int)(-Diagonal * Adj + iAdj); Points[7].y = (int)(Diagonal * Adj + iAdj);
	// Draw polygon 
	dc.DrawPolygon(8, Points);

	return bitmap;
}
