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

#ifndef _ECHOPROPVIEW_H
#define _ECHOPROPVIEW_H

#include "resource.h"
#include "echoConCtrl.h"
#include "commctrl.h"
#include "echoTypes.h"

class echoPropView  
{
public:
	echoPropView(echoConCtrl *echoConCtrl, HWND hwnd);
	~echoPropView();

	static BOOL CALLBACK editParamsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK advEchoParamsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	void Apply();
	void Init();
	void Remove();
	bool Add();
	bool Add(ECHOPROP *echoProp);
	void Edit();
	void AdvancedProps();
	void onGetDispInfo(NMLVDISPINFO *pDI);
	void DisableCheck();
	void EncryptionCheck();
	void ShowStatus();

private:
	HWND m_hwnd;
    echoConCtrl *m_pEchoConCtrl;

	ECHOPROP m_echoProps;

	void addLVItem(int num);
	void addColumn(char *iText, int iOrder, int xWidth, int alignFmt);
	void addColumn(wchar_t *iText, int iOrder, int xWidth, int alignFmt);		//added
	void setExtendedLVStyle(DWORD styles);
	void enableWindows(BOOL enable);
	void deleteItem(int num);

	bool checkEchoParams(ECHOPROP *echoProps);
	int  checkEchoError(DWORD lastError);

	void setCursor(LPCTSTR cursor);


};

#endif //_ECHOPROPVIEW_H
