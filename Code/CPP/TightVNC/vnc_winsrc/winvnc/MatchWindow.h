//  Copyright (C) 2001 HorizonLive.com, Inc. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
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
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/


// MatchWindow.h: interface for the CMatchWindow class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __MATCHWINDOW_H
#define __MATCHWINDOW_H

class CMatchWindow;

#include "stdhdrs.h"
#include "vncServer.h"

class CMatchWindow  
{
public:
	BOOL ModifyPosition(int left,int top, int right,int bottom);
	CMatchWindow(vncServer* pDlg,int left,int top, int right,int bottom);
	virtual ~CMatchWindow();
	void GetPosition(int &left,int &top, int &right,int &bottom);
	HWND m_hWnd;
	BOOL m_bSized;
	POINT m_TrackerHits[8];
	int HitTest(POINT&); // -1 = no Hits; 0-7 Numb. of Hit
    void ArrangeHits();
    void ChangeRegion();
	void Show();
	void Hide();
	void CanModify(BOOL);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    vncServer* m_pServer;

	bool StartUpdateServerTimer( void );
	bool StopUpdateServerTimer( void );

	bool SetServerScreenArea( void );
	
protected:
	bool m_bMouseCaptured;
	bool m_bWindowMoving;

	UINT m_timer_id;
};

#endif // __MATCHWINDOW_H
