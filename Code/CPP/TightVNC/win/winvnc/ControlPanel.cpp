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

// ControlPanel.cpp: implementation of the ControlPanel class.

#include <stdlib.h>
#include "ControlPanel.h"
#include "WinVNC.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ControlPanel::ControlPanel(vncServer* server, HWND hwndMenu)
{
	m_server = server;
	m_hwnd = NULL;
	m_hwndMenu = hwndMenu;
	m_clients.clear();
}
void ControlPanel::initDialog() {
	InitListViewColumns();
	setDisable();
}

void ControlPanel::setDisable() {
	if (m_hwnd != NULL) 
		SetChecked(IDC_DISABLE_CLIENTS, m_server->ClientsDisabled());
}

BOOL ControlPanel::InitListViewColumns() 
{ 
	ListView_SetExtendedListViewStyle(GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS), 
		LVS_EX_FULLROWSELECT);
	wchar_t szText[256];      
	LVCOLUMN lvc; 
	int iCol;
    
	TCHAR *ColumnsStrings[] = {
		L"IP address",
		L"Time connected",
		L"Status"
	};
	
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
	
	for (iCol = 0; iCol < 3; iCol++) { 
		lvc.iSubItem = iCol;
		lvc.pszText = szText;	
		lvc.cx = 96;           
		lvc.fmt = LVCFMT_LEFT;
		
		wcscpy(szText, ColumnsStrings[iCol]); 
		if (ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS), iCol, &lvc) == -1) 
			return FALSE; 
	} 
	m_clients = m_server->ClientList();
	UpdateListView();
	return TRUE; 
} 
BOOL ControlPanel::InsertListViewItem(int Numbe, TCHAR ItemString[3][80])
{
	int i;
	LVITEM lvI;
	lvI.mask = LVIF_TEXT| LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	lvI.iItem = Numbe; 
	lvI.iSubItem = 0; 
	lvI.pszText = ItemString[0]; 									  
	
	if(ListView_InsertItem(GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS), &lvI) == -1)
		return NULL;
	
	for (i =1;i < 3; i++) {	
		ListView_SetItemText(
			GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS), 
			Numbe, i, ItemString[i]);
	}
	return TRUE;
}

void ControlPanel::UpdateListView() 
{
	int i = 0;
	TCHAR ItemString[3][80];
	vncClientList selconn;
	getSelectedConn(&selconn);
	m_clients = m_server->ClientList();
	ListView_DeleteAllItems(GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS));
	vncClientList::iterator ci;
	for (ci = m_clients.begin(); ci != m_clients.end(); ci++) {	
		
		wcscpy(ItemString[0], (LPWSTR)m_server->GetClient(*ci)->GetClientName());
		wcscpy(ItemString[1], (LPWSTR)m_server->GetClient(*ci)->getStartTime());
		ItemString[1][24] = '\0';
		if (m_server->GetClient(*ci)->getStopUpdate()) {
			wcscpy(ItemString[2], L"Stop updating");
		} else {
			if (m_server->GetClient(*ci)->IsInputEnabled()) {
				wcscpy(ItemString[2], L"Full Control");
			} else {
				wcscpy(ItemString[2], L"View-only");
			}
		}
		InsertListViewItem(i, ItemString);
		vncClientList::iterator si;
		for (si = selconn.begin(); si != selconn.end(); si++) {
			if ((*ci) == (*si))
				ListView_SetItemState(GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS),
				i, LVIS_SELECTED, LVIS_SELECTED);
		}
		i++;
	}
}

void ControlPanel::getSelectedConn(vncClientList* selconn)
{
	HWND hListView = GetDlgItem(m_hwnd, IDC_LIST_CONNECTIONS);
	selconn->clear();
	int i = 0;
    vncClientList::iterator ci;
    for (ci = m_clients.begin(); ci != m_clients.end(); ci++) {
		if (ListView_GetItemState(hListView, i, LVIS_SELECTED) == LVIS_SELECTED)
			selconn->push_back(*ci);
		i++;
	}
}

bool ControlPanel::showDialog() {
	int resalt = 1;
	if (m_hwnd == NULL) {
		resalt = DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_CONTROL_PANEL), NULL,
								(DLGPROC)DialogProc, (LONG)this);
	} else {
		SetForegroundWindow(m_hwnd);
	}
	return resalt == 1;
}

BOOL CALLBACK ControlPanel::DialogProc(HWND hwnd, UINT uMsg,
												   WPARAM wParam, LPARAM lParam)
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	ControlPanel *_this = (ControlPanel *) GetWindowLong(hwnd, GWL_USERDATA);
	
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			ControlPanel *_this = (ControlPanel *) lParam;
			_this->m_hwnd = hwnd;
			_this->initDialog();
			return 0;
		}
	case WM_HELP:	
		help.Popup(lParam);
		return 0;
    case WM_COMMAND:		
		switch (LOWORD(wParam))
		{
		case IDC_PROPERTIES:
			PostMessage(_this->m_hwndMenu, WM_COMMAND, MAKELONG(ID_PROPERTIES, 0), 0);			
			return false;
		case IDC_ADD_CLIENT:
			PostMessage(_this->m_hwndMenu, WM_COMMAND, MAKELONG(ID_OUTGOING_CONN, 0), 0);				
			return false;
		case IDC_KILL_SEL_CLIENT:
			{
				vncClientList selconn;
				_this->getSelectedConn(&selconn);
				vncClientList::iterator ci;
				for (ci = selconn.begin(); ci != selconn.end(); ci++) {
					_this->m_server->KillClient(*ci);
				}
				return false;
			}
		case IDC_KILL_ALL:
			PostMessage(_this->m_hwndMenu, WM_COMMAND, MAKELONG(ID_KILLCLIENTS, 0), 0);				
			return false;
		case IDC_DISABLE_CLIENTS:
			PostMessage(_this->m_hwndMenu, WM_COMMAND, MAKELONG(ID_DISABLE_CONN, 0), 0);				
			return false;
		case IDC_VIEW_ONLY:
		case IDC_FULL_CONTROL:
			{
				vncClientList selconn;
				_this->getSelectedConn(&selconn);
				vncClientList::iterator ci;
				BOOL control = (LOWORD(wParam)) == IDC_FULL_CONTROL;
				for (ci = selconn.begin(); ci != selconn.end(); ci++) {
					_this->m_server->GetClient(*ci)->EnableKeyboard(control);
					_this->m_server->GetClient(*ci)->EnablePointer(control);
					_this->m_server->GetClient(*ci)->setStopUpdate(FALSE);
				}
				_this->UpdateListView();
			}   				
			return false;
		case IDC_STOP_UPDATE:
			{
				vncClientList selconn;
				_this->getSelectedConn(&selconn);
				EndDialog(hwnd, 0);
				DestroyWindow(hwnd);
				_this->m_hwnd = NULL;				
				vncClientList::iterator ci;
				for (ci = selconn.begin(); ci != selconn.end(); ci++) {
					_this->m_server->GetClient(*ci)->EnableKeyboard(FALSE);
					_this->m_server->GetClient(*ci)->EnablePointer(FALSE);
					_this->m_server->GetClient(*ci)->setStopUpdate(TRUE);
				}			
			} 
			return false;
		case IDCANCEL:
			_this->m_hwnd = NULL;
			EndDialog(hwnd, 0);			
			return TRUE;
		}
			
		return 0;
	}
	return 0;	
}

ControlPanel::~ControlPanel()
{

}
