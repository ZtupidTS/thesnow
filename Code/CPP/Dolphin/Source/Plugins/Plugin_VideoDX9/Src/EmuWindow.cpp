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

#include <windows.h>

#include "VideoConfig.h"
#include "main.h"
#include "EmuWindow.h"
#include "D3DBase.h"
#include "Fifo.h"


int OSDChoice = 0 , OSDTime = 0, OSDInternalW = 0, OSDInternalH = 0;

namespace EmuWindow
{
HWND m_hWnd = NULL;
HWND m_hMain = NULL;
HWND m_hParent = NULL;
HINSTANCE m_hInstance = NULL;
WNDCLASSEX wndClass;
const TCHAR m_szClassName[] = _T("DolphinEmuWnd");
int g_winstyle;
static volatile bool s_sizing;

// ---------------------------------------------------------------------
/* Invisible cursor option. In the lack of a predefined IDC_BLANK we make
   an empty transparent cursor */
// ------------------
HCURSOR hCursor = NULL, hCursorBlank = NULL;
void CreateCursors(HINSTANCE hInstance)
{
	BYTE ANDmaskCursor[] = { 0xff };
	BYTE XORmaskCursor[] = { 0x00 };
	hCursorBlank = CreateCursor(hInstance, 0,0, 1,1, ANDmaskCursor,XORmaskCursor);

	hCursor = LoadCursor(NULL, IDC_ARROW);
}

bool IsSizing()
{
	return s_sizing;
}

HWND GetWnd()
{
	return m_hWnd;
}

HWND GetParentWnd()
{
	return m_hParent;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
	switch( iMsg )
	{
	case WM_CREATE:
		PostMessage( m_hMain, WM_USER, WM_USER_CREATE, g_Config.RenderToMainframe );
		break;
	case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		s_sizing = true;
		break;

	case WM_EXITSIZEMOVE:
		s_sizing = false;
		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
			case VK_ESCAPE:
				if (g_Config.bFullscreen)
				{
					// Pressing Esc switches to Windowed in Fullscreen mode
					ToggleFullscreen(hWnd);
					return 0;
				}
				else
				{
					// And stops the emulation when already in Windowed mode
					PostMessage(m_hMain, WM_USER, WM_USER_STOP, 0);
					return 0;
				}
				break;
			case '3': // OSD keys
			case '4':
			case '5':
			case '6':
			case '7':
				if (g_Config.bOSDHotKey)
					OSDMenu(wParam);
				break;
		}
		// Tell the hotkey function that this key was pressed
		g_VideoInitialize.pKeyPress(LOWORD(wParam), GetAsyncKeyState(VK_SHIFT) != 0, GetAsyncKeyState(VK_CONTROL) != 0);
		break;

	case WM_SYSKEYDOWN:
		switch( LOWORD( wParam ))
		{
			case VK_RETURN:   // Pressing Alt+Enter switch FullScreen/Windowed
				if (m_hParent == NULL && !g_Config.RenderToMainframe)
				{
					ToggleFullscreen(hWnd);
					return 0;
				}
				break;
			case VK_F5: case VK_F6: case VK_F7: case VK_F8:
				PostMessage(m_hMain, WM_SYSKEYDOWN, wParam, lParam);
				return 0;
		}
		break;

	/* Post thes mouse events to the main window, it's nessesary because in difference to the
	   keyboard inputs these events only appear here, not in the parent window or any other WndProc()*/
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		PostMessage(GetParentWnd(), iMsg, wParam, lParam);
	break;

	case WM_CLOSE:
		// When the user closes the window, we post an event to the main window to call Stop()
		// Which then handles all the necessary steps to Shutdown the core + the plugins
		if (m_hParent == NULL)
		{
			PostMessage(m_hMain, WM_USER, WM_USER_STOP, 0);
			return 0;
		}
		break;

	case WM_USER:
		if (wParam == WM_USER_STOP)
		{
			SetCursor((lParam) ? hCursor : hCursorBlank);
		}
		else if (wParam == TOGGLE_FULLSCREEN)
		{
			ToggleFullscreen(hWnd);
		}
		else if (wParam == WIIMOTE_DISCONNECT)
		{
			PostMessage(m_hMain, WM_USER, wParam, lParam);
		}
		break;

	case WM_SYSCOMMAND:
		switch (wParam) 
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0;
		}
		break;
	}

	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

// ---------------------------------------------------------------------
// OSD Menu
// -------------
// Let's begin with 3 since 1 and 2 are default Wii keys
// -------------
void OSDMenu(WPARAM wParam)
{
	switch( LOWORD( wParam ))
	{
	case '3':
		OSDChoice = 1;
		// Toggle native resolution
/*
		if (!(g_Config.bNativeResolution || g_Config.b2xResolution))
			g_Config.bNativeResolution = true;
		else if (g_Config.bNativeResolution && Renderer::AllowCustom())
			{ g_Config.bNativeResolution = false; if (Renderer::Allow2x()) {g_Config.b2xResolution = true;} }
		else if (Renderer::AllowCustom())
			g_Config.b2xResolution = false;
*/
		OSDInternalW = D3D::GetBackBufferWidth();
		OSDInternalH = D3D::GetBackBufferHeight();
		break;
	case '4':
		OSDChoice = 2;
		// Toggle aspect ratio
		g_Config.iAspectRatio = (g_Config.iAspectRatio + 1) & 3;
		break;
	case '5':
		OSDChoice = 3;
		// Toggle EFB copy
		if (g_Config.bEFBCopyDisable || g_Config.bCopyEFBToTexture)
		{
			g_Config.bEFBCopyDisable = !g_Config.bEFBCopyDisable;
			g_Config.bCopyEFBToTexture = false;
		}
		else
		{
			g_Config.bCopyEFBToTexture = !g_Config.bCopyEFBToTexture;
		}
		break;
	case '6':
		OSDChoice = 4;
		g_Config.bDisableFog = !g_Config.bDisableFog;
		break;
	case '7':
		OSDChoice = 5;
		g_Config.bDisableLighting = !g_Config.bDisableLighting;
		break;		
	}
}

HWND OpenWindow(HWND parent, HINSTANCE hInstance, int width, int height, const TCHAR *title)
{
	wndClass.cbSize = sizeof( wndClass );
	wndClass.style  = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	// To interfer less with SetCursor() later we set this to NULL
	//wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	wndClass.hCursor = NULL;
	wndClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = m_szClassName;
	wndClass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );

	m_hInstance = hInstance;
	RegisterClassEx( &wndClass );

	CreateCursors(m_hInstance);

	if (g_Config.RenderToMainframe)
	{
		m_hParent = m_hMain = parent;

		m_hWnd = CreateWindow(m_szClassName, title, WS_CHILD,
			0, 0, width, height, m_hParent, NULL, hInstance, NULL);
	}
	else
	{
		m_hMain = parent;
		m_hParent = NULL;

		DWORD style = WS_OVERLAPPEDWINDOW;

		RECT rc = {0, 0, width, height};
		AdjustWindowRect(&rc, style, false);

		RECT rcdesktop;
		GetWindowRect(GetDesktopWindow(), &rcdesktop);

		int X = (rcdesktop.right-rcdesktop.left)/2 - (rc.right-rc.left)/2;
		int Y = (rcdesktop.bottom-rcdesktop.top)/2 - (rc.bottom-rc.top)/2;

		m_hWnd = CreateWindow(m_szClassName, title, style,
			X, Y, rc.right-rc.left, rc.bottom-rc.top,
			NULL, NULL, hInstance, NULL);
	}

	return m_hWnd;
}

void Show()
{
	ShowWindow(m_hWnd, SW_SHOW);
	BringWindowToTop(m_hWnd);
	UpdateWindow(m_hWnd);
}

HWND Create(HWND hParent, HINSTANCE hInstance, const TCHAR *title)
{
	// TODO:
	// 1. Remove redundant window manipulation,
	// 2. Make DX9 in fullscreen can be overlapped by other dialogs
	HWND Ret;
	int width=640, height=480;
	sscanf(g_Config.cInternalRes, "%dx%d", &width, &height );

	Ret = OpenWindow(hParent, hInstance, width, height, title);

	if (Ret)
	{
		if (g_Config.bFullscreen)
			ToggleFullscreen(Ret, true);
		else
			Show();
	}
	return Ret;
}

void Close()
{
	if (m_hWnd && !g_Config.RenderToMainframe)
	{
		DestroyWindow(m_hWnd);
		UnregisterClass(m_szClassName, m_hInstance);
	}
}

void SetSize(int width, int height)
{
	RECT rc = {0, 0, width, height};
	DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
	AdjustWindowRect(&rc, style, false);

	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	rc.left = (1280 - w)/2;
	rc.right = rc.left + w;
	rc.top = (1024 - h)/2;
	rc.bottom = rc.top + h;
	MoveWindow(m_hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
}

void ToggleFullscreen(HWND hParent, bool bForceFull)
{
	if (m_hParent == NULL)
	{
		if (D3D::IsFullscreen())
		{
			PostMessage( m_hMain, WM_USER, WM_USER_STOP, 0 );
			return;
		}

		int	w_fs = 640, h_fs = 480;
		if (!g_Config.bFullscreen || bForceFull)
		{
			if (strlen(g_Config.cFSResolution) > 1)
				sscanf(g_Config.cFSResolution, "%dx%d", &w_fs, &h_fs);

			// Get into fullscreen
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));

			// Desktop -> FullScreen
			dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth	= w_fs;
			dmScreenSettings.dmPelsHeight	= h_fs;
			dmScreenSettings.dmFields = DM_PELSWIDTH|DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				return;

			// Set new window style -> PopUp
			SetWindowLong(hParent, GWL_STYLE, WS_POPUP);

			// SetWindowPos to the upper-left corner of the screen
			SetWindowPos(hParent, HWND_TOP, 0, 0, w_fs, h_fs, SWP_NOREPOSITION);

			// Disable the cursor
			ShowCursor(FALSE);
			g_Config.bFullscreen = true;

			// Eventually show the window!
			EmuWindow::Show();
		}
		else
		{
			if (strlen(g_Config.cInternalRes) > 1)
				sscanf(g_Config.cInternalRes, "%dx%d", &w_fs, &h_fs);

			// FullScreen - > Desktop
			ChangeDisplaySettings(NULL, 0);

			DWORD style = WS_OVERLAPPEDWINDOW;
			RECT rc = {0, 0, w_fs, h_fs};
			AdjustWindowRect(&rc, style, false);
			RECT rcdesktop;
			GetWindowRect(GetDesktopWindow(), &rcdesktop);		

			// Set new window style FS -> Windowed
			SetWindowLong(hParent, GWL_STYLE, style);

			// SetWindowPos to the center of the screen
			int X = (rcdesktop.right-rcdesktop.left)/2 - (rc.right-rc.left)/2;
			int Y = (rcdesktop.bottom-rcdesktop.top)/2 - (rc.bottom-rc.top)/2;
			SetWindowPos(hParent, NULL, X, Y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOREPOSITION | SWP_NOZORDER);

			// Re-Enable the cursor
			ShowCursor(TRUE);
			g_Config.bFullscreen = false;

			// Eventually show the window!
			EmuWindow::Show();
		}
	}
}

}
