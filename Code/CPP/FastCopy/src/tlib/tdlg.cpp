static char *tdlg_id = 
	"@(#)Copyright (C) H.Shirouzu 1996-2009   tdlg.cpp	Ver0.97";
/* ========================================================================
	Project  Name			: Win32 Lightweight  Class Library Test
	Module Name				: Dialog Class
	Create					: 1996-06-01(Sat)
	Update					: 2009-03-09(Mon)
	Copyright				: H.Shirouzu
	Reference				: 
	======================================================================== */

#include "tlib.h"

TDlg::TDlg(UINT _resId, TWin *_parent) : TWin(_parent)
{
	resId	= _resId;
	modalFlg = FALSE;
	maxItems = allocItems = 0;
	dlgItems = NULL;
}

TDlg::~TDlg()
{
	if (hWnd) EndDialog(FALSE);
	delete [] dlgItems;
}

BOOL TDlg::Create(HINSTANCE hInstance)
{
	TApp::GetApp()->AddWin(this);

	hWnd = ::CreateDialogV(hInstance ? hInstance : TApp::GetInstance(), (void *)resId,
				parent ? parent->hWnd : NULL, (DLGPROC)TApp::WinProc);

	if (hWnd)
		return	TRUE;
	else
		return	TApp::GetApp()->DelWin(this), FALSE;
}

int TDlg::Exec(void)
{
	TApp::GetApp()->AddWin(this);
	modalFlg = TRUE;
	int result = ::DialogBoxV(TApp::GetInstance(), (void *)resId, parent ? parent->hWnd : NULL,
					(DLGPROC)TApp::WinProc);
	modalFlg = FALSE;
	return	result;
}

void TDlg::Destroy(void)
{
	EndDialog(FALSE);
}

LRESULT TDlg::WinProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	result = 0;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		if (rect.left != CW_USEDEFAULT && !(::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)) {
			MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,FALSE);
		}
		GetWindowRect(&orgRect);
		return	EvCreate(lParam);

	case WM_CLOSE:
		EvClose();
		return	0;

	case WM_COMMAND:
		EvCommand(HIWORD(wParam), LOWORD(wParam), lParam);
		return	0;

	case WM_SYSCOMMAND:
		EvSysCommand(wParam, MAKEPOINTS(lParam));
		return	0;

	case WM_TIMER:
		EvTimer(wParam, (TIMERPROC)lParam);
		return	0;

	case WM_NCDESTROY:
		GetWindowRect(&rect);
		EvNcDestroy();
		TApp::GetApp()->DelWin(this);
		hWnd = 0;
		return	0;

	case WM_QUERYENDSESSION:
		result = EvQueryEndSession((BOOL)wParam, (BOOL)lParam);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	0;

	case WM_ENDSESSION:
		EvEndSession((BOOL)wParam, (BOOL)lParam);
		return	0;

	case WM_QUERYOPEN:
		result = EvQueryOpen();
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_PAINT:
		EvPaint();
		return	0;

	case WM_NCPAINT:
		EvNcPaint((HRGN)wParam);
		return	0;

	case WM_SIZE:
		EvSize((UINT)wParam, LOWORD(lParam), HIWORD(lParam));
		return	0;

	case WM_SHOWWINDOW:
		EvShowWindow(wParam, lParam);
		return	0;

	case WM_GETMINMAXINFO:
		EvGetMinMaxInfo((MINMAXINFO *)lParam);
		return	0;

	case WM_SETCURSOR:
		result = EvSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_MOUSEMOVE:
		return	EvMouseMove((UINT)wParam, MAKEPOINTS(lParam));

	case WM_NCHITTEST:
		EvNcHitTest(MAKEPOINTS(lParam), &result);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_MEASUREITEM:
		result = EvMeasureItem((UINT)wParam, (LPMEASUREITEMSTRUCT)lParam);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_DRAWITEM:
		result = EvDrawItem((UINT)wParam, (LPDRAWITEMSTRUCT)lParam);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_NOTIFY:
		result = EvNotify((UINT)wParam, (LPNMHDR)lParam);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_CONTEXTMENU:
		result = EvContextMenu((HWND)wParam, MAKEPOINTS(lParam));
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_HOTKEY:
		result = EvHotKey((int)wParam);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONDBLCLK:
		EventButton(uMsg, wParam, MAKEPOINTS(lParam));
		return	0;

	case WM_KEYUP:
	case WM_KEYDOWN:
		EventKey(uMsg, wParam, lParam);
		return	0;

	case WM_HSCROLL:
	case WM_VSCROLL:
		EventScroll(uMsg, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		return	0;

	case WM_INITMENU:
	case WM_INITMENUPOPUP:
		EventInitMenu(uMsg, (HMENU)wParam, LOWORD(lParam), (BOOL)HIWORD(lParam));
		return	0;

	case WM_MENUSELECT:
		EvMenuSelect(LOWORD(wParam), (BOOL)HIWORD(wParam), (HMENU)lParam);
		return	0;

	case WM_DROPFILES:
		EvDropFiles((HDROP)wParam);
		return	0;

	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
		EventCtlColor(uMsg, (HDC)wParam, (HWND)lParam, (HBRUSH *)&result);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;

	case WM_KILLFOCUS:
	case WM_SETFOCUS:
		EventFocus(uMsg, (HWND)wParam);
		return	0;

	default:
		if (uMsg >= WM_USER && uMsg <= 0x7FFF || uMsg >= 0xC000 && uMsg <= 0xFFFF)
			result = EventUser(uMsg, wParam, lParam);
		else
			result = EventSystem(uMsg, wParam, lParam);
#ifndef X64
		SetWindowLong(DWL_MSGRESULT, result);
#else
		SetWindowLong(0, result);
#endif
		return	result;
	}

	return	FALSE;
}

BOOL TDlg::PreProcMsg(MSG *msg)
{
	if (hAccel && ::TranslateAccelerator(hWnd, hAccel, msg))
		return	TRUE;

	if (!modalFlg)
		return	::IsDialogMessage(hWnd, msg);

	return	FALSE;
}

BOOL TDlg::EvSysCommand(WPARAM uCmdType, POINTS pos)
{
	return	FALSE;
}

BOOL TDlg::EvCommand(WORD wNotifyCode, WORD wID, LPARAM hwndCtl)
{
	switch (wID)
	{
	case IDOK: case IDCANCEL: case IDYES: case IDNO:
	case IDABORT: case IDIGNORE: case IDRETRY:
		EndDialog(wID);
		return	TRUE;
	}

	return	FALSE;
}

BOOL TDlg::EvQueryOpen(void)
{
	return	FALSE;
}

BOOL TDlg::EvCreate(LPARAM lParam)
{
	return	TRUE;
}

void TDlg::EndDialog(int result)
{
	if (::IsWindow(hWnd))
	{
		if (modalFlg)
			::EndDialog(hWnd, result);
		else
			::DestroyWindow(hWnd);
	}
}

BOOL TDlg::SetDlgItem(UINT ctl_id, int idx, DWORD flags)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);

	if (idx >= allocItems) return FALSE;
	DlgItem *item = dlgItems + idx;

	item->hWnd = GetDlgItem(ctl_id);
	::GetWindowPlacement(item->hWnd, &wp);
	item->wpos.x = wp.rcNormalPosition.left;
	item->wpos.y = wp.rcNormalPosition.top;
	item->wpos.cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
	item->wpos.cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
	item->flags = flags;
	return	TRUE;
}

BOOL TDlg::FitDlgItems()
{
	GetWindowRect(&rect);
	int	xdiff = (rect.right - rect.left) - (orgRect.right - orgRect.left);
	int ydiff = (rect.bottom - rect.top) - (orgRect.bottom - orgRect.top);

	HDWP	hdwp = ::BeginDeferWindowPos(maxItems);	// MAX item number
	UINT	dwFlg = SWP_SHOWWINDOW | SWP_NOZORDER;

	for (int i=0; i < maxItems; i++) {
		DlgItem *item = dlgItems + i;
		DWORD	f = item->flags;

		if (f & FIT_SKIP) continue;
		int x = (f & LEFT_FIT) == LEFT_FIT ? item->wpos.x : item->wpos.x + xdiff;
		int y = (f & TOP_FIT)  == TOP_FIT  ? item->wpos.y : item->wpos.y + ydiff;
		int w = (f & X_FIT)    == X_FIT    ? item->wpos.cx + xdiff : item->wpos.cx;
		int h = (f & Y_FIT)    == Y_FIT    ? item->wpos.cy + ydiff : item->wpos.cy;

		hdwp = ::DeferWindowPos(hdwp, item->hWnd, 0, x, y, w, h, dwFlg);
	}
	EndDeferWindowPos(hdwp);

	return	TRUE;
}
