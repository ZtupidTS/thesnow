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

// QuitFullScreenWindow.cpp: implementation of the QuitFullScreenWindow class.


#include "stdhdrs.h"
#include "vncviewer.h"
#include "ClientConnection.h"
#include "QuitFullScreenWindow.h"

#define SIZE_BUTTON 20

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

QuitFullScreenWindow::QuitFullScreenWindow(VNCviewerApp *pApp, ClientConnection * CConn)
{
	m_pApp = pApp;
	m_CConn = CConn;
	m_DblClick = FALSE;

	//Creating button window.
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_DBLCLKS;
	wcex.lpfnWndProc	= (WNDPROC)QuitFullScreenWindow::QuitProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= NULL;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "QuitFullScreenWindowClass";
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	RECT workrect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workrect, 0);
	int xpos = workrect.right - 45;
	int ypos = 32;

	m_hwndButton = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,			//dwExStyle
		"QuitFullScreenWindowClass",// pointer to registered class name
  		"QuitButton",				// pointer to window name
  		WS_POPUP,					// window style
		xpos,						// horizontal position of window
		ypos,						// vertical position of window
		SIZE_BUTTON,				// window width
		SIZE_BUTTON,				// window height
		NULL,						// handle to parent or owner window
		NULL,						// handle to menu, or child-window identifier
		NULL,						// handle to application instance
		NULL						// pointer to window-creation data
	);
  	
	SetWindowLong(m_hwndButton, GWL_USERDATA, (LONG)this);
	ShowButton(FALSE);

	// Load the bitmap resource. 	
	m_hbmp = LoadBitmap(m_pApp->m_instance, MAKEINTRESOURCE(IDB_DISABLE_FS));
	
	// Create a device context (DC) to hold the bitmap. 
	// The bitmap is copied from this DC to the window's DC 
	// whenever it must be drawn. 	
	HDC hdc = GetDC(m_hwndButton); 
	m_hdcCompat = CreateCompatibleDC(hdc);
	SelectObject(m_hdcCompat, m_hbmp);
	ReleaseDC(m_hwndButton, hdc);
}

void QuitFullScreenWindow::ShowButton(BOOL show)
{
	if (show) {
		ShowWindow(m_hwndButton, SW_SHOW);
		SetFocus(m_CConn->GetViewerWindow());
	} else {
		ShowWindow(m_hwndButton, SW_HIDE);
	}
}

LRESULT CALLBACK QuitFullScreenWindow::QuitProc(HWND hwnd, UINT iMsg,
									WPARAM wParam, LPARAM lParam)
{
	QuitFullScreenWindow *_this = (QuitFullScreenWindow*)GetWindowLong(hwnd,GWL_USERDATA);

	switch (iMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			BitBlt(ps.hdc, 0, 0, SIZE_BUTTON, SIZE_BUTTON, _this->m_hdcCompat,
					0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);			
		}
		break; 
	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {	
			POINTS ptsMousePoint=MAKEPOINTS(lParam);
			POINT ptMousePoint;
			ptMousePoint.x = ptsMousePoint.x;
			ptMousePoint.y = ptsMousePoint.y;
			ClientToScreen(hwnd,&ptMousePoint);
			RECT wrect;
			GetWindowRect(hwnd, &wrect);
			POINT newpos;
			newpos.x = wrect.left - (_this->m_MousePoint.x - ptMousePoint.x);
			newpos.y = wrect.top - (_this->m_MousePoint.y - ptMousePoint.y);
			if (newpos.x != wrect.left || newpos.y != wrect.top)
				SetWindowPos(hwnd, HWND_TOPMOST, newpos.x, newpos.y,
								SIZE_BUTTON, SIZE_BUTTON,
								SWP_NOSIZE | SWP_SHOWWINDOW);
			_this->m_MousePoint.x = ptMousePoint.x;
			_this->m_MousePoint.y = ptMousePoint.y;
		}
		break;
	case WM_LBUTTONDBLCLK:
		_this->m_DblClick = TRUE;
		break;
	case WM_LBUTTONDOWN:
		{
			POINTS ptsMousePoint = MAKEPOINTS(lParam);
			_this->m_MousePoint.x = ptsMousePoint.x;
			_this->m_MousePoint.y = ptsMousePoint.y;
			ClientToScreen(hwnd,&_this->m_MousePoint);
			SetCapture(hwnd);
		}
		break;	
	case WM_LBUTTONUP:
		if (_this->m_DblClick) {
			_this->m_DblClick = FALSE;
			_this->m_CConn->SetFullScreenMode(false);
		}
		SetCapture(NULL);
		SetFocus(_this->m_CConn->GetViewerWindow());
		break;
	case WM_DESTROY: 
		// Destroy compatible bitmap, 
		// and the bitmap.  
		DeleteDC(_this->m_hdcCompat); 
		DeleteObject(_this->m_hbmp);  
		break; 
	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
	return 0;
}

QuitFullScreenWindow::~QuitFullScreenWindow()
{
	DestroyWindow(m_hwndButton);
}
