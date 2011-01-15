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

#ifndef FTLISTVIEW_H__
#define FTLISTVIEW_H__

#include "commctrl.h"

#include "FileInfo.h"
#include "FileTransferTypes.h"

class FTListView
{
public:
	FTListView(HWND hLV);
	~FTListView();

	void onGetDispInfo(NMLVDISPINFO *di);
	void onItemActivate(NMITEMACTIVATE *pIA, FILEINFOEX *pFIEx);
	void addItems(FileInfo *pFI);
	void deleteAllItems();
	void addColumn(char *iText, int iOrder, int xWidth, int alignFmt);
	void setExtendedLVStyle(DWORD styles);
	void initImageList(HINSTANCE hInst);

	char *getActivateItemName(LPNMITEMACTIVATE lpnmia);
	int getSelectedItems(FileInfo *pFI);

	HWND getWndHandle() { return m_hListView; };

private:
	HWND m_hListView;
	FileInfo m_fileInfo;
    HIMAGELIST m_hImageList;
	
	void Time70ToFiletime(unsigned int time70, FILETIME *pftime);
};

#endif // FTLISTVIEW_H__
