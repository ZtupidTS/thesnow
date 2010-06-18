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

#ifndef _NETWINDOW_H_
#define _NETWINDOW_H_

#include "CommonTypes.h"

#include <queue>
#include <string>

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
#include <wx/spinctrl.h>
//#include <wx/thread.h>

#include "GameListCtrl.h"

// just leaving these here so i can find something later if i need it
//#include "Frame.h"
//#include "Globals.h"
//#include "BootManager.h"
//#include "Common.h"
//#include "NetStructs.h"
//#include "Core.h"
//#include "HW/SI.h"
//#include "HW/SI_Device.h"
//#include "HW/SI_DeviceGCController.h"
//#include "Timer.h"

#include "LockingQueue.h"

enum
{
	NP_GUI_EVT_CHANGE_GAME = 45,
	NP_GUI_EVT_START_GAME,
	NP_GUI_EVT_STOP_GAME,
};

class NetPlaySetupDiag : public wxFrame
{
public:
	NetPlaySetupDiag(wxWindow* const parent, const CGameListCtrl* const game_list);
	~NetPlaySetupDiag();
private:
	void OnJoin(wxCommandEvent& event);
	void OnHost(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	wxTextCtrl		*m_nickname_text,
		*m_host_port_text,
		*m_connect_port_text,
		*m_connect_ip_text;

	wxListBox*		m_game_lbox;

	const CGameListCtrl* const m_game_list;
};

class NetPlayDiag : public wxFrame
{
public:
	NetPlayDiag(wxWindow* const parent, const CGameListCtrl* const game_list
		, const std::string& game, const bool is_hosting = false);
	~NetPlayDiag();

	LockingQueue<std::string>	chat_msgs;
	//std::string				chat_msg;

	void OnStart(wxCommandEvent& event);
	void OnStop(wxCommandEvent& event);

private:
    DECLARE_EVENT_TABLE()

	void OnChat(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnPadBuffHelp(wxCommandEvent& event);
	void OnThread(wxCommandEvent& event);
	void OnChangeGame(wxCommandEvent& event);
	void OnAdjustBuffer(wxCommandEvent& event);
	void OnConfigPads(wxCommandEvent& event);

	wxListBox*		m_player_lbox;
	wxTextCtrl*		m_chat_text;
	wxTextCtrl*		m_chat_msg_text;

	std::string		m_selected_game;
	wxButton*		m_game_btn;

	std::vector<int>	m_playerids;

	const CGameListCtrl* const m_game_list;
	//NetPlay* const	m_netplay;
};

DECLARE_EVENT_TYPE(wxEVT_THREAD, -1)

class ChangeGameDiag : public wxDialog
{
public:
	ChangeGameDiag(wxWindow* const parent, const CGameListCtrl* const game_list, wxString& game_name);

private:
	void OnPick(wxCommandEvent& event);

	wxListBox*		m_game_lbox;
	wxString&		m_game_name;
};

class PadMapDiag : public wxDialog
{
public:
	PadMapDiag(wxWindow* const parent, int map[]);

private:
	void OnAdjust(wxCommandEvent& event);

	wxChoice*	m_map_cbox[4];
	int* const	m_mapping;
};

#endif // _NETWINDOW_H_

