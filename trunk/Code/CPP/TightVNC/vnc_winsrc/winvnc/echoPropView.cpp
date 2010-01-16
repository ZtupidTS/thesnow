//  Copyright (C) 2005 Dennis Syrovatsky. All Rights Reserved.
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
//  TightVNC homepage on the Web: http://www.tightvnc.com/

#include "stdio.h"

#include "echoPropView.h"

echoPropView::echoPropView(echoConCtrl *echoConCtrl, HWND hwnd)
{
	m_pEchoConCtrl = echoConCtrl;
	m_hwnd = hwnd;
	memset(&m_echoProps, 0, sizeof(ECHOPROP));
}

echoPropView::~echoPropView()
{
}


BOOL CALLBACK 
echoPropView::editParamsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	echoPropView *_this = (echoPropView *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetWindowLong(hwnd, GWL_USERDATA, (LONG) lParam);
		_this = (echoPropView *)lParam;
		if ((strcmp(_this->m_echoProps.port, _this->m_pEchoConCtrl->getDefaultPort()) != 0) &&
			(strlen(_this->m_echoProps.server) != 0) && (strlen(_this->m_echoProps.port) != 0)) {
			char buf[ID_STRING_SIZE];
			sprintf(buf, "%s:%s", _this->m_echoProps.server, _this->m_echoProps.port);
			SetDlgItemText(hwnd, IDC_ECHO_SERVER, buf);
		} else {
			SetDlgItemText(hwnd, IDC_ECHO_SERVER, _this->m_echoProps.server);
		}
		SetDlgItemText(hwnd, IDC_ECHO_USER, _this->m_echoProps.username);
		SetDlgItemText(hwnd, IDC_ECHO_PWD, _this->m_echoProps.pwd);
		return FALSE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					char buf[ID_STRING_SIZE];
					GetDlgItemText(hwnd, IDC_ECHO_SERVER, buf, ID_STRING_SIZE);
					if (!_this->m_pEchoConCtrl->parseConnectionString((char *)buf, 
																	 (char *)_this->m_echoProps.server, 
																	 (char *)_this->m_echoProps.port)) {
						_this->m_echoProps.server[0] = '\0';
						_this->m_echoProps.port[0] = '\0';
					}
					GetDlgItemText(hwnd, IDC_ECHO_USER, _this->m_echoProps.username, ID_STRING_SIZE);
					GetDlgItemText(hwnd, IDC_ECHO_PWD, _this->m_echoProps.pwd, ID_STRING_SIZE);
					EndDialog(hwnd, IDOK);
				}
				return TRUE;
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				return TRUE;
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, IDCANCEL);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK 
echoPropView::advEchoParamsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	echoPropView *_this = (echoPropView *) GetWindowLong(hwnd, GWL_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwnd, GWL_USERDATA, (LONG) lParam);
			_this = (echoPropView *)lParam;
			ZeroMemory(&_this->m_echoProps, sizeof(_this->m_echoProps));
			_this->m_pEchoConCtrl->getLocalProxyInfo(&_this->m_echoProps);
			SetDlgItemText(hwnd, IDC_ECHO_HTTP_ADDR, _this->m_echoProps.server);
			SetDlgItemText(hwnd, IDC_ECHO_HTTP_PORT, _this->m_echoProps.port);
			SetDlgItemText(hwnd, IDC_ECHO_HTTP_USERNAME, _this->m_echoProps.username);
			SetDlgItemText(hwnd, IDC_ECHO_HTTP_PWD, _this->m_echoProps.pwd);
			BOOL bProxyAuth = FALSE;
			if ((strlen(_this->m_echoProps.username) == 0) && 
				(strlen(_this->m_echoProps.pwd) == 0)) {
				CheckDlgButton(hwnd, IDC_ECHO_AUTH_CHECK, BST_UNCHECKED);
			} else {
				bProxyAuth = TRUE;
				CheckDlgButton(hwnd, IDC_ECHO_AUTH_CHECK, BST_CHECKED);
			}
			EnableWindow(GetDlgItem(hwnd, IDC_ECHO_HTTP_USERNAME), bProxyAuth);
			EnableWindow(GetDlgItem(hwnd, IDC_ECHO_HTTP_PWD), bProxyAuth);
		}
		return FALSE;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					GetDlgItemText(hwnd, IDC_ECHO_HTTP_ADDR, _this->m_echoProps.server, ID_STRING_SIZE);
					GetDlgItemText(hwnd, IDC_ECHO_HTTP_PORT, _this->m_echoProps.port, ID_STRING_SIZE);
					GetDlgItemText(hwnd, IDC_ECHO_HTTP_USERNAME, _this->m_echoProps.username, ID_STRING_SIZE);
					GetDlgItemText(hwnd, IDC_ECHO_HTTP_PWD, _this->m_echoProps.pwd, ID_STRING_SIZE);
					EndDialog(hwnd, IDOK);
				}
				return TRUE;
			case IDCANCEL:
				EndDialog(hwnd, IDCANCEL);
				return TRUE;
			case IDC_ECHO_AUTH_CHECK:
				{
					BOOL bProxyAuth = FALSE;
					if (IsDlgButtonChecked(hwnd, IDC_ECHO_AUTH_CHECK) == BST_CHECKED) {
						bProxyAuth = TRUE;
					} else {
						SetDlgItemText(hwnd, IDC_ECHO_HTTP_USERNAME, "");
						SetDlgItemText(hwnd, IDC_ECHO_HTTP_PWD, "");
					}
					EnableWindow(GetDlgItem(hwnd, IDC_ECHO_HTTP_USERNAME), bProxyAuth);
					EnableWindow(GetDlgItem(hwnd, IDC_ECHO_HTTP_PWD), bProxyAuth);
				}
				return TRUE;
			}
		}
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		EndDialog(hwnd, IDCANCEL);
		return TRUE;
	}
	return FALSE;
}

void 
echoPropView::Apply()
{
}

void 
echoPropView::Init()
{
	RECT Rect;
	GetClientRect(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), &Rect);

	int xwidth0 = (int) (0.22 * Rect.right);
	int xwidth1 = (int) (0.22 * Rect.right);
	int xwidth2 = (int) (0.37 * Rect.right);
	int xwidth3 = (int) (0.19 * Rect.right);

	addColumn("echoServer", 0, xwidth0, LVCFMT_LEFT);
	addColumn("IP Address", 1, xwidth1, LVCFMT_CENTER);
	addColumn("Status", 2, xwidth2, LVCFMT_CENTER);
	addColumn("Connected as", 3, xwidth3, LVCFMT_CENTER);
	
	setExtendedLVStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	addLVItem(m_pEchoConCtrl->getNumEntries());

	memset(&m_echoProps, 0, sizeof(ECHOPROP));

	ShowStatus();

	if (!m_pEchoConCtrl->getEnableEchoConnection()) {
		enableWindows(FALSE);
		CheckDlgButton(m_hwnd, IDC_ECHO_CON_DISABLE, BST_CHECKED);
	}

	if (!m_pEchoConCtrl->isEncryptionPossible()) {
		EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ENCRYPTION), FALSE);
	} else {
		EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ENCRYPTION), TRUE);
		if (m_pEchoConCtrl->isEncrypted() != 0) {
			CheckDlgButton(m_hwnd, IDC_ECHO_ENCRYPTION, BST_CHECKED);
		} else {
			CheckDlgButton(m_hwnd, IDC_ECHO_ENCRYPTION, BST_UNCHECKED);
		}
	}

	if (m_pEchoConCtrl->isLocalProxyEnable()) {
		EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ADV), TRUE);
	} else {
		EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ADV), FALSE);
	}
}

void 
echoPropView::Remove()
{
	HWND hListView = GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST);

	int selCount = ListView_GetSelectedCount(hListView);
	int selItem = ListView_GetSelectionMark(hListView);
	if ((selCount < 1) || (selItem < 0)) return;
	
	selItem = -1;
	selItem = ListView_GetNextItem(hListView, selItem, LVNI_SELECTED);
	do {
		deleteItem(selItem);
		selItem = ListView_GetNextItem(hListView, selItem, LVNI_SELECTED);
	} while (selItem >= 0);
}

bool
echoPropView::Add(ECHOPROP *echoProp)
{
	m_pEchoConCtrl->copyConnectionParams(&m_echoProps, echoProp);

	return Add();
}

bool 
echoPropView::Add()
{
	bool bContinue = true;
	bool bResult = false;
	do
	{
		INT_PTR result = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ECHO_PARAMS), 
										m_hwnd, (DLGPROC) editParamsDlgProc, (LONG) this);
		if (result == IDOK) {
			if (!checkEchoParams(&m_echoProps)) {
				if (MessageBox(m_hwnd, 
					"This echo connection parameters is incorrect.\nDo you want edit parameters?", 
					"Echo connection parameters", 
					MB_YESNO) == IDYES) { bContinue = true; } else { bContinue = false; }
			} else {
				bool bRes = m_pEchoConCtrl->add(&m_echoProps);
				if (bRes) {
					addLVItem(1);
					bContinue = false;
					bResult = true;
				} else {
					bResult = false;
					switch (m_pEchoConCtrl->getLastError())
					{
					case ID_ECHO_ERROR_ALREADY_EXIST:
					case ID_ECHO_ERROR_CHANNEL_EXIST:
						if (MessageBox(m_hwnd, 
							"This echo connection parameters already exists.\nDo you want edit parameters?", 
							"Echo connection parameters exists", 
							MB_YESNO) == IDYES) { bContinue = true; } else { bContinue = false; }
						break;
					case ID_ECHO_ERROR_WRONG_ADDRESS:
						m_pEchoConCtrl->del(&m_echoProps);
						if (MessageBox(m_hwnd, 
							"No proxy server could be connected to at the provided ip-address/ip-port.\nDo you want edit parameters?",
							"Echo connection parameters",
							MB_YESNO) == IDYES)  { bContinue = true; } else { bContinue = false; }
						break;
					case ID_ECHO_ERROR_WRONG_LOGIN:
						m_pEchoConCtrl->del(&m_echoProps);
						if (MessageBox(m_hwnd, 
							"Authentication failed with the proxy server.\nDo you want edit parameters?",
							"Echo connection parameters",
							MB_YESNO) == IDYES)  { bContinue = true; } else { bContinue = false; }
						break;
					case ID_ECHO_ERROR_LIB_MISSING:
					case ID_ECHO_ERROR_LIB_NOT_INITIALIZED:
						m_pEchoConCtrl->del(&m_echoProps);
						if (MessageBox(m_hwnd, 
							"Now the echo connection is impossible.\nDo you want edit parameters?", 
							"Echo connection parameters", 
							MB_YESNO) == IDYES)  { bContinue = true; } else { bContinue = false; }
						break;
					default:
						if (MessageBox(m_hwnd, 
							"Echo connection with this parameters can be not created\nDo you want edit parameters?", 
							"Echo connection parameters", 
							MB_YESNO) == IDYES) { bContinue = true; } else { bContinue = false; }
						m_pEchoConCtrl->del(&m_echoProps);
					}
				}
			}
		} else {
			bContinue = false;
		}
	} while (bContinue);

	if (m_hwnd != NULL) {
		HWND hListView = GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST);
		int selItem = ListView_GetSelectionMark(hListView);
		ListView_RedrawItems(hListView, 0, ListView_GetItemCount(hListView));
		UpdateWindow(hListView);
	}

	return bResult;
}

void 
echoPropView::Edit()
{
	HWND hListView = GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST);

	int selItem = ListView_GetSelectionMark(hListView);

	if (selItem < 0) return;

	m_pEchoConCtrl->getEntriesAt(selItem, &m_echoProps);

	if (Add()) {
		deleteItem(selItem);
	}
	
	memset(&m_echoProps, 0, sizeof(ECHOPROP));

	ListView_RedrawItems(hListView, 0, ListView_GetItemCount(hListView));
	UpdateWindow(hListView);
}

void
echoPropView::AdvancedProps()
{
	INT_PTR result = DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_ECHO_CONN_ADV), 
									m_hwnd, (DLGPROC) advEchoParamsDlgProc, (LONG) this);
	if (result == IDOK) {
		m_pEchoConCtrl->setLocalProxyInfo(&m_echoProps);
	}
	memset(&m_echoProps, 0, sizeof(ECHOPROP));
}

void 
echoPropView::addColumn(char *iText, int iOrder, int xWidth, int alignFmt)
{
	LVCOLUMN lvc; 
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_ORDER;
	lvc.fmt = alignFmt;
	lvc.iSubItem = iOrder;
	lvc.pszText = iText;	
	lvc.cchTextMax = 32;
	lvc.cx = xWidth;
	lvc.iOrder = iOrder;
	ListView_InsertColumn(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), iOrder, &lvc);
}

void
echoPropView::addLVItem(int num)
{
	for (int i = 0; i < num; i++) {
		LVITEM LVItem;
		LVItem.mask = LVIF_TEXT | LVIF_STATE; 
		LVItem.state = 0; 
		LVItem.stateMask = 0; 
		LVItem.iItem = 0;
		LVItem.iSubItem = 0;
		LVItem.pszText = LPSTR_TEXTCALLBACK;
		ListView_InsertItem(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), &LVItem);
	}
}

void 
echoPropView::setExtendedLVStyle(DWORD styles)
{
	ListView_SetExtendedListViewStyleEx(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), styles, styles);
}

void
echoPropView::deleteItem(int num)
{
	HWND hListView = GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST);

	ListView_DeleteItem(hListView, num);
	ECHOPROP prop;
	if (m_pEchoConCtrl->getEntriesAt(num, &prop)) m_pEchoConCtrl->del(&prop);
}

void 
echoPropView::onGetDispInfo(NMLVDISPINFO *pDI)
{
	ECHOPROP echoProps;
	if (!m_pEchoConCtrl->getEntriesAt(pDI->item.iItem, &echoProps)) return;
	
	switch (pDI->item.iSubItem)
	{
    case 0:
		{
			char buf[ID_STRING_SIZE];
			sprintf(buf, "%s:%s", echoProps.server, echoProps.port);
			pDI->item.pszText = buf;
		}
		break;
	case 1:
		pDI->item.pszText = echoProps.ipaddr;
		break;
	case 2:
		pDI->item.pszText = m_pEchoConCtrl->getConnectionStatus(&echoProps);
		break;
	case 3:
		pDI->item.pszText = echoProps.username;
		break;
	default:
		break;
	}
}

void
echoPropView::ShowStatus()
{
	SetDlgItemText(m_hwnd, IDC_ECHOWARE_STATUS, m_pEchoConCtrl->getEchoWareVersion());
}

bool
echoPropView::checkEchoParams(ECHOPROP *echoProps)
{
	if ((strlen(echoProps->server) == 0) || 
		(strlen(echoProps->username) == 0) ||
		(strlen(echoProps->pwd) == 0)) return false;

	return true;
}

void 
echoPropView::DisableCheck()
{
	if (IsDlgButtonChecked(m_hwnd, IDC_ECHO_CON_DISABLE) == BST_CHECKED) {
		if (MessageBox(m_hwnd, 
					   "Some echo connections may be established.\nDo you really disable echo connection now?", 
					   "Echo connections", 
					   MB_YESNO) != IDYES) {
			CheckDlgButton(m_hwnd, IDC_ECHO_CON_DISABLE, BST_UNCHECKED);
		} else {
			enableWindows(FALSE);
			m_pEchoConCtrl->allowEchoConnection(0);
			ShowStatus();
		}
	} else {
		enableWindows(TRUE);
		m_pEchoConCtrl->allowEchoConnection(1);
		ShowStatus();
	}			
}

void
echoPropView::EncryptionCheck()
{
	if (IsDlgButtonChecked(m_hwnd, IDC_ECHO_ENCRYPTION) == BST_CHECKED) {
		m_pEchoConCtrl->setEncryption(1);
	} else {
		m_pEchoConCtrl->setEncryption(0);
	}			
	ShowStatus();
}

void
echoPropView::enableWindows(BOOL enable)
{

	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHOSERVERS_LIST), enable); 
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ADD), enable); 
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_REMOVE), enable); 
	EnableWindow(GetDlgItem(m_hwnd, IDC_ECHO_ENCRYPTION), enable);
}

int
echoPropView::checkEchoError(DWORD lastError)
{
	switch (lastError) 
	{
		case ID_ECHO_ERROR_ALREADY_EXIST:
			if (MessageBox(NULL, 
				"This echo connection parameters already exists.\nDo you want edit parameters?", 
				"Echo connection parameters exists", 
				MB_YESNO) == IDYES) { return 1; } else { return 2; }
		case ID_ECHO_ERROR_LIB_NOT_INITIALIZED:
			if (MessageBox(NULL, 
				"EchoWare library not initialized.\nDo you want to save this parameters in the known echoservers list?",
				"Echo connection parameters",
				MB_YESNO) == IDYES) { return 3; } else { return 4; }
			break;
		case ID_ECHO_ERROR_WRONG_ADDRESS:
			if (MessageBox(NULL, 
				"No proxy server could be connected to at the provided ip-address/ip-port.\nDo you want to save this parameters in the known echoservers list?",
				"Echo connection parameters",
				MB_YESNO) == IDYES) { return 3; } else { return 4; }
			break;
		case ID_ECHO_ERROR_WRONG_LOGIN:
			if (MessageBox(NULL, 
				"Authentication failed with the proxy server.\nDo you want to save this parameters in the known echoservers list?",
				"Echo connection parameters",
				MB_YESNO) == IDYES) { return 3; } else { return 4; }
			break;
		case ID_ECHO_ERROR_CHANNEL_EXIST:
			if (MessageBox(NULL, 
				"The authentication channel with this parameters exists and already active.\nDo you want edit parameters?",
				"Echo connection parameters",
				MB_YESNO) == IDYES) { return 1; } else { return 2; }
			break;
		default:
			if (MessageBox(NULL, 
				"Echo connection with this parameters can be not created\nDo you want edit parameters?", 
				"Echo connection parameters", 
				MB_YESNO) == IDYES) { return 1; } else { return 2; }
	}
	return 0;
}