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

// QuitFullScreenWindow.h: interface for the QuitFullScreenWindow class.


#ifndef QUITFULLSCREENWINDOW_H__
#define QUITFULLSCREENWINDOW_H__

class QuitFullScreenWindow
{
public:
	QuitFullScreenWindow(VNCviewerApp *pApp, ClientConnection * CConn);
	void ShowButton(BOOL show);
	HWND GetButton() {return m_hwndButton;};
	virtual ~QuitFullScreenWindow();
protected:
	static LRESULT CALLBACK QuitProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	
	VNCviewerApp *m_pApp;
	ClientConnection * m_CConn;
	HWND m_hwndButton;
	POINT m_MousePoint;
	HDC m_hdcCompat;
	HBITMAP m_hbmp;
	BOOL m_DblClick;
};

#endif // QUITFULLSCREENWINDOW_H__
