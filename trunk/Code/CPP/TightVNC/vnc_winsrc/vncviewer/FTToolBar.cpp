//  Copyright (C) 2003-2004 Dennis Syrovatsky. All Rights Reserved.
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
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

//#include "windows.h"
//#include "commctrl.h"

#include "stdhdrs.h"
#include "vncviewer.h"

#include "FTToolBar.h"

FTToolBar::FTToolBar(HINSTANCE hInst, HWND hParent)
{
	m_hInst = hInst;
	m_hParentWnd = hParent;
}

FTToolBar::~FTToolBar()
{
}

void
FTToolBar::createToolBar()
{
	m_hToolBar = CreateWindow(TOOLBARCLASSNAME, (LPSTR) NULL, 
							  WS_CHILD | CCS_NORESIZE | CCS_NODIVIDER | 
							  CCS_NOPARENTALIGN | TBSTYLE_FLAT | TBSTYLE_WRAPABLE, 
							  320, 75, 160, 300, m_hParentWnd, NULL, m_hInst, NULL); 

	SendMessage(m_hToolBar,(UINT) TB_SETBUTTONWIDTH,(WPARAM) 0, (LPARAM) MAKELONG(150, 13));
	SendMessage(m_hToolBar, (UINT) TB_SETBUTTONSIZE, (WPARAM) 0, (LPARAM) MAKELONG(150, 13));
}

void
FTToolBar::addButtons()
{
	TBBUTTON tbb[14];
	int i = 0;
	unsigned char style = (unsigned char) (TBSTYLE_BUTTON | TBSTYLE_WRAPABLE);

	tbb[i].iBitmap = 0; 
	tbb[i].idCommand = IDC_CREATEFLD; 
	tbb[i].fsState = TBSTATE_INDETERMINATE | TBSTATE_WRAP; 
	tbb[i].fsStyle = style;
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 3; 
	tbb[i].idCommand = IDC_FTCOPY; 
	tbb[i].fsState = TBSTATE_INDETERMINATE | TBSTATE_WRAP; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 6; 
	tbb[i].idCommand = IDC_FTRENAME; 
	tbb[i].fsState = TBSTATE_INDETERMINATE | TBSTATE_WRAP; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 7; 
	tbb[i].idCommand = IDC_FTDELETE; 
	tbb[i].fsState = TBSTATE_INDETERMINATE | TBSTATE_WRAP; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 8; 
	tbb[i].idCommand = IDC_FTCANCEL; 
	tbb[i].fsState = TBSTATE_INDETERMINATE | TBSTATE_WRAP; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 0xFF; 
	tbb[i].idCommand = 0;
	tbb[i].fsState = TBSTATE_WRAP | TBSTATE_INDETERMINATE; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i++].dwData = 0; 

	tbb[i].iBitmap = 9; 
	tbb[i].idCommand = IDC_EXIT; 
	tbb[i].fsState = TBSTATE_ENABLED | TBSTATE_WRAP; 
	tbb[i].fsStyle = style; 
	tbb[i].iString = NULL;
	tbb[i].dwData = 0; 

	SendMessage(m_hToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

	SendMessage(m_hToolBar, (UINT) TB_ADDBUTTONS, (WPARAM) 14, (LPARAM) (LPTBBUTTON) &tbb);
	
	ShowWindow(m_hToolBar, SW_SHOW);
}

void
FTToolBar::initImageList()
{
    m_hImageList = ImageList_Create(150, 13, ILC_MASK, 0, 13); 

    HICON hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_CREATEFOLDERGRAY)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_CREATELOCALFOLDER)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_CREATEREMOTEFOLDER)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COPYGRAY)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_UPLOAD)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_DOWNLOAD)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_RENAME)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_DELETE)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_CANCELFT)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

    hiconItem = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_CLOSEDIALOG)); 
    ImageList_AddIcon(m_hImageList, hiconItem);  
    DestroyIcon(hiconItem); 

	SendMessage(m_hToolBar, TB_SETIMAGELIST, 0, (LPARAM)m_hImageList);
}

void
FTToolBar::setButtonState(int nButton, bool status, int iImage)
{
	SendMessage(m_hToolBar, (UINT) TB_SETSTATE, (WPARAM) nButton, 
		(LPARAM) MAKELONG (TBSTATE_INDETERMINATE | TBSTATE_WRAP, 0));
	SendMessage(m_hToolBar, (UINT) TB_CHANGEBITMAP, 
		(WPARAM) nButton, (LPARAM) MAKELPARAM (-1, 0));
	SendMessage(m_hToolBar, (UINT) TB_CHANGEBITMAP, 
		(WPARAM) nButton, (LPARAM) MAKELPARAM (iImage, 0));
	if (status) {
		SendMessage(m_hToolBar, (UINT) TB_SETSTATE, (WPARAM) nButton, 
			(LPARAM) MAKELONG (TBSTATE_ENABLED | TBSTATE_WRAP, 0));
	}
}

void
FTToolBar::setAllButtonsState(int statusCrtFldBtn, int statusCpyBtn, int statusRenBtn, 
							  int statusDelBtn, int statusCancelBtn)
{
	switch(statusCrtFldBtn)
	{
		case -1: break;
		case 0: setButtonState(IDC_CREATEFLD, false, 0); break;
		case 1: setButtonState(IDC_CREATEFLD, true, 1); break;
		case 2: setButtonState(IDC_CREATEFLD, true, 2); break;
		default: setButtonState(IDC_CREATEFLD, false, 0);
	}

	switch(statusCpyBtn)
	{
		case -1: break;
		case 0: setButtonState(IDC_FTCOPY, false, 3); break;
		case 1:	setButtonState(IDC_FTCOPY, true, 4);	break;
		case 2:	setButtonState(IDC_FTCOPY, true, 5);	break;
		default: setButtonState(IDC_FTCOPY, false, 3);
	}

	switch(statusRenBtn)
	{
		case -1: break;
		case 0:	setButtonState(IDC_FTRENAME, false, 6); break;
		case 1:	setButtonState(IDC_FTRENAME, true, 6);	break;
		default: setButtonState(IDC_FTRENAME, false, 6);
	}

	switch(statusDelBtn)
	{
		case -1: break;
		case 0:	setButtonState(IDC_FTDELETE, false, 7); break;
		case 1:	setButtonState(IDC_FTDELETE, true, 7);	break;
		default: setButtonState(IDC_FTDELETE, false, 7);
	}

	switch(statusCancelBtn)
	{
		case -1: break;
		case 0:	setButtonState(IDC_FTCANCEL, false, 8); break;
		case 1:	setButtonState(IDC_FTCANCEL, true, 8);	break;
		default: setButtonState(IDC_FTCANCEL, false, 8);
	}
}

void
FTToolBar::destroyToolBar()
{
	DestroyWindow(m_hToolBar);
}
