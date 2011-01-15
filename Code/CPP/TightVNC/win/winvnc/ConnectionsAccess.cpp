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

// ConnectionsAccess.cpp: implementation of the ConnectionsAccess class.

#include "ConnectionsAccess.h"
#include "WinVNC.h"
#include "VNCHelp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ConnectionsAccess::ConnectionsAccess(vncServer * server, HWND hwnd)
{
	m_server = server;
	m_hwnd = hwnd;
	Init();
}

void ConnectionsAccess::Apply()
{
	int count = GetItemCount();
	if (count < 1) {
		m_server->SetAuthHosts(0);
		return;
	}

	// Allocate 17 bytes per each pattern (IP, action and separator)
	wchar_t *auth_hosts = (wchar_t *)malloc(count * 17);
	auth_hosts[0] = '\0';

	for (int i = count - 1; i >= 0; i--) {
		GetListViewItem(i, ItemString);
		if (!FormatPattern(FALSE, ItemString[0], NULL))
			continue;

		if (wcscmp(ItemString[1], L"Allow") == 0) {
			wcscat(auth_hosts, L"+");
		} else if (wcscmp(ItemString[1], L"Deny") == 0) {
			wcscat(auth_hosts, L"-");
		} else if (wcscmp(ItemString[1], L"Query") == 0) {
			wcscat(auth_hosts, L"?");
		}

		wcscat(auth_hosts, ItemString[0]);
		if (i != 0)
			wcscat(auth_hosts, L":");
	}

	m_server->SetAuthHosts((LPSTR)auth_hosts);
	free(auth_hosts);
}

void ConnectionsAccess::Init()
{
	InitListViewColumns();
	char *auth_hosts = m_server->AuthHosts();

	char *pattern = auth_hosts;
	for (;;) {
		int len = strcspn(pattern, ":");
		if (strspn(pattern, "+-?") == 1 && len <= 256) {
			memcpy(ItemString[0], &pattern[1], len - 1);
			ItemString[0][len - 1] = '\0';
			switch (pattern[0]) {
			case '+': wcscpy(ItemString[1], L"Allow"); break;
			case '-': wcscpy(ItemString[1], L"Deny"); break;
			case '?': wcscpy(ItemString[1], L"Query"); break;
			}
			if (FormatPattern(TRUE, ItemString[0], NULL))
				InsertListViewItem(0, ItemString);
		}
		if (pattern[len] == '\0')
			break;
		pattern += len + 1;
	}

	free(auth_hosts);
}

void ConnectionsAccess::MoveUp()
{
	int number = GetSelectedItem();
	if (number <= 0)
		return;
	GetListViewItem(number, ItemString);
	InsertListViewItem(number - 1, ItemString);
	DeleteItem(number + 1);
	SetSelectedItem(number - 1);
}

void ConnectionsAccess::MoveDown()
{
	int number = GetSelectedItem();
	if (number == -1 || number == (GetItemCount() - 1))
		return;
	GetListViewItem(number, ItemString);
	InsertListViewItem(number + 2, ItemString);
	DeleteItem(number);
	SetSelectedItem(number + 1);
}

void ConnectionsAccess::Remove()
{
	int number = GetSelectedItem();
	if (number == -1)
		return;
	DeleteItem(number);
}

void ConnectionsAccess::Add()
{
	m_edit = FALSE;
	if (DoEditDialog() != IDOK)
		return;
	InsertListViewItem(0, ItemString);
	SetSelectedItem(0);
}

void ConnectionsAccess::Edit()
{
	int numbersel = GetSelectedItem();
	if (numbersel == -1)
		return;
	GetListViewItem(numbersel, ItemString);
	m_edit = TRUE;
	if (DoEditDialog() != IDOK)
		return;
	DeleteItem(numbersel);
	InsertListViewItem(numbersel, ItemString);
	SetSelectedItem(numbersel);
}

DWORD ConnectionsAccess::DoEditDialog()
{
	return DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_CONN_HOST), 
		m_hwnd, (DLGPROC) EditDlgProc, (LONG) this);
}

int ConnectionsAccess::GetItemCount()
{
	return ListView_GetItemCount(GetDlgItem(m_hwnd, IDC_LIST_HOSTS));
}

void ConnectionsAccess::SetSelectedItem(int number)
{
	ListView_SetItemState(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
				number, LVIS_SELECTED, LVIS_SELECTED);
}

void ConnectionsAccess::DeleteItem(int number)
{
	ListView_DeleteItem(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), number);
}

int ConnectionsAccess::GetSelectedItem()
{
	int count = ListView_GetItemCount(GetDlgItem(m_hwnd, IDC_LIST_HOSTS));
	if (count < 1)
		return -1;
	int numbersel = -1;
	for (int i = 0; i < count; i++) {
		if (ListView_GetItemState(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
									i, LVIS_SELECTED) == LVIS_SELECTED)
			numbersel = i;
	}
		return numbersel;
}

void ConnectionsAccess::GetListViewItem(int Numbe, TCHAR ItemString[2][256])
{
	for (int i = 0; i < 2; i++) {
		wcscpy(ItemString[i], L"");
		ListView_GetItemText(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
							Numbe, i, ItemString[i], 256);							
	}
}

BOOL ConnectionsAccess::InsertListViewItem(int Numbe, TCHAR ItemString[2][256])
{
	LVITEM lvI;
	lvI.mask = LVIF_TEXT| LVIF_STATE; 
	lvI.state = 0; 
	lvI.stateMask = 0; 
	lvI.iItem = Numbe; 
	lvI.iSubItem = 0; 
	lvI.pszText = ItemString[0]; 									  
	
	if(ListView_InsertItem(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), &lvI) == -1)
		return NULL;
		
	ListView_SetItemText(
			GetDlgItem(m_hwnd, IDC_LIST_HOSTS), 
			Numbe, 1, ItemString[1]);
	
	return TRUE;
}

BOOL ConnectionsAccess::InitListViewColumns()
{
	ListView_SetExtendedListViewStyle(GetDlgItem(m_hwnd, IDC_LIST_HOSTS),
									  LVS_EX_FULLROWSELECT);
	TCHAR *ColumnsStrings[] = {
		L"IP pattern",
		L"Action"
	};

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (int iCol = 0; iCol < 2; iCol++) {
		lvc.iSubItem = iCol;
		lvc.pszText = ColumnsStrings[iCol];
		lvc.cx = 97;
		lvc.fmt = LVCFMT_LEFT;

		if (ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_LIST_HOSTS), iCol, &lvc) == -1)
			return FALSE;
	}
	return TRUE;
}

BOOL ConnectionsAccess::FormatPattern(BOOL toList, TCHAR strpattern[256], 
									  char buf_parts[4][5])
{
	char parts[4][5] = {"*", "*", "*", "*"};
	int num_components = 0;
	int num_numbers = 0;

	if (strpattern[0] != '\0') {
		wchar_t *component = strpattern;
		for (;;) {
			int len = wcscspn(component, L".");
			if (len == 1 && component[0] == '*') {
				// The component is single "*" - it's ok, do nothing.
			} else if (num_components != num_numbers) {
				return FALSE;		// non-'*' found after there was '*'
			} else {
				wcsncpy((LPWSTR)parts[num_numbers], component, len);
				if (!MatchPatternComponent((LPWSTR)parts[num_numbers]))
					return FALSE;
				num_numbers++;
			}
			if (component[len] == '\0')
				break;
			if (++num_components > 4)
				return FALSE;		// too many components in IP pattern
			component += len + 1;
		}
	}

	// Now we are sure the format is correct.

	int num_parts = (toList) ? 4 : num_numbers;
	strpattern[0] = '\0';
	for (int i = 0; i < num_parts; i++) {
		wcscat(strpattern, (i == 0) ? L"" : L".");
		wcscat(strpattern, (LPWSTR)parts[i]);
	}
	if (buf_parts != NULL) {	
		for (int i = 0; i < 4; i++)
			strcpy(buf_parts[i], parts[i]);
	}

	return TRUE;
}

BOOL ConnectionsAccess::MatchPatternComponent(TCHAR component[5])
{
	if (wcscmp(component, L"*") == 0 || wcscmp(component, L"") == 0)
		return TRUE;
	int len = wcslen(component);
	if (len < 1 || wcsspn(component, L"0123456789") != len)
		return FALSE;	// not a non-negative number
	int value = _wtoi(component);
	if (value > 255)
		return FALSE;	// not a number in the range 0..255
	wsprintf(component, L"%d", value);
	return TRUE;
}

void ConnectionsAccess::MatchEdit(HWND hwnd, DWORD idedit)
{
	int num_comp;
	switch (idedit)
	{
	case IDC_PATTERN_COMP1:
		num_comp = 0;
		break;
	case IDC_PATTERN_COMP2:
		num_comp = 1;
		break;
	case IDC_PATTERN_COMP3:
		num_comp = 2;
		break;
	case IDC_PATTERN_COMP4:
		num_comp = 3;
		break;
	}

	TCHAR buffer[5];
	GetDlgItemText(hwnd, idedit, buffer, 5);
	if (!MatchPatternComponent(buffer)) {
		SetDlgItemTextA(hwnd, idedit, IPComponent[num_comp]);
		int poscursor = strlen(IPComponent[num_comp]);
		SendMessage(GetDlgItem(hwnd, idedit), EM_SETSEL,
					poscursor, poscursor);
		MessageBoxA(hwnd,
				"Element of pattern should be an unsigned\n"
				"number from the range 0..255, or *.",
				"Error", MB_ICONSTOP | MB_OK);
	} else {
		strcpy(IPComponent[num_comp], (LPSTR)buffer);
	}
}

BOOL CALLBACK ConnectionsAccess::EditDlgProc(HWND hwnd,
						  UINT uMsg,
						  WPARAM wParam,
						  LPARAM lParam )
{
	// We use the dialog-box's USERDATA to store a _this pointer
	// This is set only once WM_INITDIALOG has been recieved, though!
	ConnectionsAccess *_this = (ConnectionsAccess *) GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg)
	{

	case WM_INITDIALOG:
		{
			// Retrieve the Dialog box parameter and use it as a pointer
			// to the calling vncProperties object
			SetWindowLong(hwnd, GWL_USERDATA, lParam);
			ConnectionsAccess *_this = (ConnectionsAccess *) lParam;

			SendMessage(GetDlgItem(hwnd, IDC_PATTERN_COMP1), EM_LIMITTEXT, (WPARAM)3, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PATTERN_COMP2), EM_LIMITTEXT, (WPARAM)3, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PATTERN_COMP3), EM_LIMITTEXT, (WPARAM)3, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PATTERN_COMP4), EM_LIMITTEXT, (WPARAM)3, 0);

			if (_this->m_edit) {
				SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_SETCHECK,
					(wcscmp(_this->ItemString[1], L"Allow") == 0), 0);
				SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_SETCHECK,
					(wcscmp(_this->ItemString[1], L"Deny") == 0), 0);
				SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_SETCHECK,
					(wcscmp(_this->ItemString[1], L"Query") == 0), 0);
				_this->FormatPattern(FALSE, _this->ItemString[0], _this->IPComponent);
			} else {
				for (int i = 0; i < 4; i++)
					strcpy(_this->IPComponent[i], "*");
				SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_SETCHECK, TRUE, 0);
			}

			SetDlgItemTextA(hwnd, IDC_PATTERN_COMP1, _this->IPComponent[0]);
			SetDlgItemTextA(hwnd, IDC_PATTERN_COMP2, _this->IPComponent[1]);
			SetDlgItemTextA(hwnd, IDC_PATTERN_COMP3, _this->IPComponent[2]);
			SetDlgItemTextA(hwnd, IDC_PATTERN_COMP4, _this->IPComponent[3]);

			return TRUE;
		}
	case WM_HELP:	
		help.Popup(lParam);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_PATTERN_COMP1:
		case IDC_PATTERN_COMP2:
		case IDC_PATTERN_COMP3:
		case IDC_PATTERN_COMP4:
			{
				if (HIWORD(wParam) == EN_UPDATE)
					_this->MatchEdit(hwnd, LOWORD(wParam));
			}
			return 0;
		case IDOK:
			{
				_this->ItemString[0][0] = '\0';
				for (int i = 0; i < 4; i++) {
					wcscat(_this->ItemString[0], (i == 0) ? L"" : L".");
					wcscat(_this->ItemString[0], (LPWSTR)_this->IPComponent[i]);
				}

				if (!_this->FormatPattern(TRUE, _this->ItemString[0], NULL)) {
					MessageBox(hwnd,
							L"The pattern format is incorrect. It should be entered\n"
							 L"as A.B.C.D or A.B.C or A.B or A, where each element\n"
							 L"should be an unsigned number from the range 0..255",
							L"Error", MB_ICONSTOP | MB_OK);
					return TRUE;
				}
				if (SendDlgItemMessage(hwnd, IDC_RADIO_ALLOW, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					wcscpy(_this->ItemString[1], L"Allow");
				} else if (SendDlgItemMessage(hwnd, IDC_RADIO_DENY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					wcscpy(_this->ItemString[1], L"Deny");
				} else if (SendDlgItemMessage(hwnd, IDC_RADIO_QUERY, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					wcscpy(_this->ItemString[1], L"Query");
				}
				EndDialog(hwnd, IDOK);
				return TRUE;
			}
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			return TRUE;		
		}
		return 0;
	}
	return 0;
}

ConnectionsAccess::~ConnectionsAccess()
{
}
