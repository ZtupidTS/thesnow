// Copyright (C) 2003-2009 Dolphin Project.

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

#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/notebook.h>
#include <wx/dialog.h>
#include <wx/aboutdlg.h>

#include "ConfigManager.h"
#include "VideoBackend.h"
#include "SWVideoConfig.h"
#include "Win32.h"

#include "StringUtil.h"

// ----------------------
// The rendering window
// ----------------------
namespace EmuWindow
{

HWND m_hWnd = NULL; // The new window that is created here
HWND m_hParent = NULL;

WNDCLASSEX wndClass;
const TCHAR m_szClassName[] = _T("DolphinEmuWnd");
int g_winstyle;

// ------------------------------------------
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
	HDC			hdc;
	PAINTSTRUCT ps;
	switch( iMsg )
	{
	case WM_CREATE:
		PostMessage(m_hParent, WM_USER, WM_USER_CREATE, (int)m_hParent);
		break;
	
	case WM_PAINT:
		hdc = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;
	
	case WM_SYSKEYDOWN:
		switch( LOWORD( wParam ))
		{
		case VK_RETURN:
			// Pressing Alt+Enter switch FullScreen/Windowed
			if (m_hParent == NULL && !g_SWVideoConfig.renderToMainframe)
			{
				ToggleFullscreen(hWnd);
			}
			break;
		case VK_F5: case VK_F6: case VK_F7: case VK_F8:
			PostMessage(m_hParent, WM_SYSKEYDOWN, wParam, lParam);
			break;
		default:
			return DefWindowProc(hWnd, iMsg, wParam, lParam);
		}
		break;

	case WM_KEYDOWN:
		switch (LOWORD( wParam ))
		{
		case VK_ESCAPE:
			if (g_SWVideoConfig.bFullscreen)
			{
				// Pressing Esc switches to Windowed mode from Fullscreen mode
				ToggleFullscreen(hWnd);
			}
			// And pause the emulation when already in Windowed mode
			PostMessage(m_hParent, WM_USER, WM_USER_PAUSE, 0);
			break;
		}
		break;

	/* The reason we pick up the WM_MOUSEMOVE is to be able to change this option
	   during gameplay. The alternative is to load one of the cursors when the plugin
	   is loaded and go with that. This should only produce a minimal performance hit
	   because SetCursor is not supposed to actually change the cursor if it's the
	   same as the one before. */
	case WM_MOUSEMOVE:
		/* Check rendering mode; child or parent. Then post the mouse moves to the main window
		   it's nessesary for both the chil dwindow and separate rendering window because
		   moves over the rendering window do not reach the main program then. */
		if (GetParentWnd() == NULL) // Separate rendering window
			PostMessage(m_hParent, iMsg, wParam, -1);
		else
			PostMessage(GetParentWnd(), iMsg, wParam, lParam);
		break;

	/* To support the separate window rendering we get the message back here. So we basically
	   only let it pass through Dolphin > Frame.cpp to determine if it should be on or off
	   and coordinate it with the other settings if necessary */
	case WM_USER:
		if (wParam == WM_USER_STOP)
			SetCursor((lParam) ? hCursor : hCursorBlank);
		else if (wParam == WIIMOTE_DISCONNECT)
			PostMessage(m_hParent, WM_USER, wParam, lParam);
		break;

	/* Post these mouse events to the main window, it's nessesary becase in difference to the
	   keyboard inputs these events only appear here, not in the main WndProc() */
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		PostMessage(GetParentWnd(), iMsg, wParam, lParam);
	break;

	// This is called when we close the window when we render to a separate window
	case WM_CLOSE:	
		if (m_hParent == NULL)
		{
			// Take it out of fullscreen and stop the game
			if( g_SWVideoConfig.bFullscreen )
				ToggleFullscreen(m_hParent);
			PostMessage(m_hParent, WM_USER, WM_USER_STOP, 0);
		}
		break;

	// Called when a screensaver wants to show up while this window is active
	case WM_SYSCOMMAND:
		switch (wParam) 
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			break;
		default:
			return DefWindowProc(hWnd, iMsg, wParam, lParam);
		}
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}


// This is called from Create()
HWND OpenWindow(HWND parent, HINSTANCE hInstance, int width, int height, const TCHAR *title)
{
	wndClass.cbSize = sizeof( wndClass );
	wndClass.style  = CS_HREDRAW | CS_VREDRAW;
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

	//m_hInstance = hInstance;
	RegisterClassEx( &wndClass );

	CreateCursors(/*m_hInstance*/GetModuleHandle(0));

	// Create child window
	m_hParent = parent;

	m_hWnd = CreateWindow(m_szClassName, title,
			WS_CHILD,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			parent, NULL, hInstance, NULL);

	ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);

	return m_hWnd;
}

void ToggleFullscreen(HWND hParent)
{
	if (m_hParent == NULL)
	{ 
		int	w_fs = 640, h_fs = 480;
		if (g_SWVideoConfig.bFullscreen)
		{
			// Get out of fullscreen
			g_SWVideoConfig.bFullscreen = false;
			RECT rc = {0, 0, w_fs, h_fs};

			// FullScreen -> Desktop
			ChangeDisplaySettings(NULL, 0);

			RECT rcdesktop;	// Get desktop resolution
			GetWindowRect(GetDesktopWindow(), &rcdesktop);

			ShowCursor(TRUE);

			int X = (rcdesktop.right-rcdesktop.left)/2 - (rc.right-rc.left)/2;
			int Y = (rcdesktop.bottom-rcdesktop.top)/2 - (rc.bottom-rc.top)/2;
			// SetWindowPos to the center of the screen
			SetWindowPos(hParent, NULL, X, Y, w_fs, h_fs, SWP_NOREPOSITION | SWP_NOZORDER);

			// Set new window style FS -> Windowed
			SetWindowLong(hParent, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			// Eventually show the window!
			EmuWindow::Show();
		}
		else
		{
			// Get into fullscreen
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));

			// Desktop -> FullScreen
			dmScreenSettings.dmSize			= sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth	= w_fs;
			dmScreenSettings.dmPelsHeight	= h_fs;
			dmScreenSettings.dmBitsPerPel	= 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				return;
			
			// Set new window style -> PopUp
			SetWindowLong(hParent, GWL_STYLE, WS_POPUP);
			g_SWVideoConfig.bFullscreen = true;
			ShowCursor(FALSE);

			// SetWindowPos to the upper-left corner of the screen
			SetWindowPos(hParent, NULL, 0, 0, w_fs, h_fs, SWP_NOREPOSITION | SWP_NOZORDER);

			// Eventually show the window!
			EmuWindow::Show();
		}
	}
}

void Show()
{
	ShowWindow(m_hWnd, SW_SHOW);
	BringWindowToTop(m_hWnd);
	UpdateWindow(m_hWnd);
}

HWND Create(HWND hParent, HINSTANCE hInstance, const TCHAR *title)
{
	return OpenWindow(hParent, hInstance, 640, 480, title);
}

void Close()
{
	if (!m_hParent)
		DestroyWindow(m_hWnd);
	UnregisterClass(m_szClassName, /*m_hInstance*/GetModuleHandle(0));
}

// ------------------------------------------
// Set the size of the child or main window
// ------------------------------------------
void SetSize(int width, int height)
{
	RECT rc = {0, 0, width, height};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;

	// Move and resize the window
	rc.left = (1280 - w)/2;
	rc.right = rc.left + w;
	rc.top = (1024 - h)/2;
	rc.bottom = rc.top + h;
	MoveWindow(m_hWnd, rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top, TRUE);
}

} // EmuWindow
