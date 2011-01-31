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
#include "EmuWindow.h"
#include "Fifo.h"
#include "VertexShaderManager.h"
#include "RenderBase.h"
#include "VideoBackendBase.h"
#include "Core.h"

namespace EmuWindow
{
HWND m_hWnd = NULL;
HWND m_hParent = NULL;
HINSTANCE m_hInstance = NULL;
WNDCLASSEX wndClass;
const TCHAR m_szClassName[] = _T("DolphinEmuWnd");
int g_winstyle;
static volatile bool s_sizing;

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

// ---------------------------------------------------------------------
// KeyDown events
// -------------
void OnKeyDown(WPARAM wParam)
{
	switch (LOWORD( wParam ))
	{
	case '3': // OSD keys
	case '4':
	case '5':
	case '6':
	case '7':
		if (g_Config.bOSDHotKey)
			OSDMenu(wParam);
		break;
	}
}
// ---------------------------------------------------------------------

void FreeLookInput( UINT iMsg, WPARAM wParam )
{
	static float debugSpeed = 1.0f;
	static bool mouseLookEnabled = false;
	static bool mouseMoveEnabled = false;
	static float lastMouse[2];
	POINT point;	
	switch(iMsg)
	{
	case WM_USER_KEYDOWN:
	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case '9':
			debugSpeed /= 2.0f;
			break;
		case '0':
			debugSpeed *= 2.0f;
			break;
		case 'W':
			VertexShaderManager::TranslateView(0.0f, debugSpeed);
			break;
		case 'S':
			VertexShaderManager::TranslateView(0.0f, -debugSpeed);
			break;
		case 'A':
			VertexShaderManager::TranslateView(debugSpeed, 0.0f);
			break;
		case 'D':
			VertexShaderManager::TranslateView(-debugSpeed, 0.0f);
			break;
		case 'R':
			VertexShaderManager::ResetView();
			break;
		}
		break;

	case WM_MOUSEMOVE:
		if (mouseLookEnabled) {
			GetCursorPos(&point);
			VertexShaderManager::RotateView((point.x - lastMouse[0]) / 200.0f, (point.y - lastMouse[1]) / 200.0f);
			lastMouse[0] = point.x;
			lastMouse[1] = point.y;
		}

		if (mouseMoveEnabled) {
			GetCursorPos(&point);
			VertexShaderManager::TranslateView((point.x - lastMouse[0]) / 50.0f, (point.y - lastMouse[1]) / 50.0f);
			lastMouse[0] = point.x;
			lastMouse[1] = point.y;
		}
		break;

	case WM_RBUTTONDOWN:
		GetCursorPos(&point);
		lastMouse[0] = point.x;
		lastMouse[1] = point.y;
		mouseLookEnabled= true;
		break;
	case WM_MBUTTONDOWN:		
		GetCursorPos(&point);
		lastMouse[0] = point.x;
		lastMouse[1] = point.y;
		mouseMoveEnabled= true;
		break;
	case WM_RBUTTONUP:
		mouseLookEnabled = false;
		break;
	case WM_MBUTTONUP:
		mouseMoveEnabled = false;
		break;
	}
}


LRESULT CALLBACK WndProc( HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
	if (g_ActiveConfig.bFreeLook)
		FreeLookInput( iMsg, wParam );

	switch( iMsg )
	{
	case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_ENTERSIZEMOVE:
		s_sizing = true;
		break;

	case WM_EXITSIZEMOVE:
		s_sizing = false;
		break;

	/* Post the mouse events to the main window, it's nessesary because in difference to the
	   keyboard inputs these events only appear here, not in the parent window or any other WndProc()*/
	case WM_LBUTTONDOWN:
		if(g_ActiveConfig.backend_info.bSupports3DVision && g_ActiveConfig.b3DVision)
		{
			// This basically throws away the left button down input when b3DVision is activated so WX 
			// can't get access to it, stopping focus pulling on mouse click. 
			// (Input plugins use a different system so it doesn't cause any weirdness)
			break;
		}
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
		PostMessage(GetParentWnd(), iMsg, wParam, lParam);
		break;

	// This is called when we close the window when we render to a separate window
	case WM_CLOSE:
		// When the user closes the window, we post an event to the main window to call Stop()
		// Which then handles all the necessary steps to Shutdown the core + the plugins
		if (m_hParent == NULL)
		{
			// Stop the game
			PostMessage(m_hParent, WM_USER, WM_USER_STOP, 0);
		}
		break;

	case WM_USER:
		if (wParam == WM_USER_KEYDOWN)
		{
			OnKeyDown(lParam);
			FreeLookInput(wParam, lParam);
		}
		else if (wParam == WIIMOTE_DISCONNECT)
		{
			PostMessage(m_hParent, WM_USER, wParam, lParam);
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
	case WM_SETCURSOR:
		PostMessage(m_hParent, WM_USER, WM_USER_SETCURSOR, 0);
		return true;

	case WM_DESTROY:
		g_video_backend->Shutdown();
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
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
		g_Config.iEFBScale = g_Config.iEFBScale + 1;
		if (g_Config.iEFBScale > 4) g_Config.iEFBScale = 0;
		break;
	case '4':
		OSDChoice = 2;
		// Toggle aspect ratio
		g_Config.iAspectRatio = (g_Config.iAspectRatio + 1) & 3;
		break;
	case '5':
		OSDChoice = 3;
		// Toggle EFB copy
		if (!g_Config.bEFBCopyEnable)
		{
			g_Config.bEFBCopyEnable = true;
			g_Config.bCopyEFBToTexture = false;
		}
		else if (!g_Config.bCopyEFBToTexture)
		{
			g_Config.bCopyEFBToTexture = true;
		}
		else
		{
			g_Config.bEFBCopyEnable = false;
			g_Config.bCopyEFBToTexture = false;
		}
		break;
	case '6':
		OSDChoice = 4;
		g_Config.bDisableFog = !g_Config.bDisableFog;
		break;
	case '7':
		// TODO: Not implemented in the D3D backends, yet
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
	wndClass.hCursor = NULL;
	wndClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = m_szClassName;
	wndClass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );

	m_hInstance = hInstance;
	RegisterClassEx( &wndClass );

	m_hParent = parent;

	m_hWnd = CreateWindow(m_szClassName, title, (g_ActiveConfig.backend_info.bSupports3DVision && g_ActiveConfig.b3DVision) ? WS_EX_TOPMOST | WS_POPUP : WS_CHILD,
		0, 0, width, height, m_hParent, NULL, hInstance, NULL);

	return m_hWnd;
}

void Show()
{
	ShowWindow(m_hWnd, SW_SHOW);
	BringWindowToTop(m_hWnd);
	UpdateWindow(m_hWnd);
	SetFocus(m_hParent);
}

HWND Create(HWND hParent, HINSTANCE hInstance, const TCHAR *title)
{
	// TODO:
	// 1. Remove redundant window manipulation,
	// 2. Make DX9 in fullscreen can be overlapped by other dialogs
	// 3. Request window sizes which actually make the client area map to a common resolution
	HWND Ret;
	int x=0, y=0, width=640, height=480;
	Core::Callback_VideoGetWindowSize(x, y, width, height);

	 // TODO: Don't show if fullscreen
	Ret = OpenWindow(hParent, hInstance, width, height, title);

	if (Ret)
	{
		Show();
	}
	return Ret;
}

void Close()
{
	if (m_hParent == NULL)
		DestroyWindow(m_hWnd);
	UnregisterClass(m_szClassName, m_hInstance);
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

}
