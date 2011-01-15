// Copyright (C) 2004 TightVNC Development Team. All Rights Reserved.
//
//  TightVNC is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC homepage on the Web: http://www.tightvnc.com/

// ControlPanel.h: interface for the ControlPanel class.

#ifndef _WINVNC_CONTROLPANEL_H
#define _WINVNC_CONTROLPANEL_H
#pragma once

#include "resource.h"
#include "vncServer.h"
#include "commctrl.h"

class ControlPanel  
{
public:
	ControlPanel(vncServer* server, HWND hwndMenu);
	virtual ~ControlPanel();
	static BOOL CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void UpdateListView();
	bool showDialog();
    void initDialog();
	void setDisable();
protected:
    inline void Enable(int id, BOOL enable) {
		EnableWindow(GetDlgItem(m_hwnd, id), enable);
	}
	inline void SetChecked(int id, BOOL checked) {
		SendDlgItemMessage(m_hwnd, id, BM_SETCHECK, checked, 0);
	}
	inline BOOL IsChecked(int id) {
		return (SendDlgItemMessage(m_hwnd, id, BM_GETCHECK, 0, 0) == BST_CHECKED);
	}
    void getSelectedConn(vncClientList* selconn);
    BOOL InsertListViewItem(int Numbe, TCHAR ItemString[3][80]);
    BOOL InitListViewColumns();

	HWND m_hwnd, m_hwndMenu;
    vncServer* m_server;
    vncClientList m_clients;
};

#endif
