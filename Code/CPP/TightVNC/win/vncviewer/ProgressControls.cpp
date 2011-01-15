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

#include "stdhdrs.h"
#include "vncviewer.h"

#include "ProgressControls.h"

#define MAX_RANGE 0xFFFF

ProgressControls::ProgressControls(HWND hParent)
{
	m_hParent = hParent;
	m_hPercentText = GetDlgItem(m_hParent, IDC_CURRENTFILEPERCENT);
	m_hProgressBar = GetDlgItem(m_hParent, IDC_PROGRESS);
	m_hFTPercentText = GetDlgItem(m_hParent, IDC_FILETRANSFERPERCENT);
	m_hFTProgressBar = GetDlgItem(m_hParent, IDC_FTPROGRESS);
}

ProgressControls::~ProgressControls()
{
}

bool
ProgressControls::initLeftControl(DWORD64 maxFT, DWORD64 posFT)
{
	m_dw64FTBarValue = posFT;
	m_dw64FTBarValueMax = maxFT;

	SendMessage(m_hFTProgressBar, PBM_SETRANGE, (WPARAM) 0, MAKELPARAM(0, MAX_RANGE)); 
	SendMessage(m_hFTProgressBar, PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
	
	return true;
		//showControlsValue();
}

bool
ProgressControls::initRightControl(DWORD max, DWORD pos)
{
	m_dwBarValue = pos;
	m_dwBarValueMax = max;

	SendMessage(m_hProgressBar, PBM_SETRANGE, (WPARAM) 0, MAKELPARAM(0, MAX_RANGE)); 
	SendMessage(m_hProgressBar, PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);

	return true;
//	return showControlsValue();
}

bool
ProgressControls::clear()
{
	m_dwBarValue = 0;
	m_dw64FTBarValue = 0;

	m_dwBarValueMax = 0;
	m_dw64FTBarValueMax = 0;

	return showControlsValue();
}

bool
ProgressControls::increase(DWORD value)
{
	m_dwBarValue += value;
	m_dw64FTBarValue += value;

	if (m_dwBarValue > m_dwBarValueMax) m_dwBarValue = m_dwBarValueMax;
	if (m_dw64FTBarValue > m_dw64FTBarValueMax) m_dw64FTBarValue = m_dw64FTBarValueMax;

	return showControlsValue();
}

bool
ProgressControls::showControlsValue()
{
	DWORD curPos = (DWORD) ((((float) m_dwBarValue) / m_dwBarValueMax) * MAX_RANGE);
	SendMessage(m_hProgressBar, PBM_SETPOS, (WPARAM) curPos, (LPARAM) 0);

	DWORD percent = (DWORD) ((((float) m_dwBarValue) / m_dwBarValueMax) * 100);
	char buf[5];
	sprintf(buf, "%d%%", percent);
	SetWindowText(m_hPercentText, buf);
	
	curPos = (DWORD) ((m_dw64FTBarValue * MAX_RANGE) / m_dw64FTBarValueMax);
	SendMessage(m_hFTProgressBar, PBM_SETPOS, (WPARAM) curPos, (LPARAM) 0);
	
	percent = (DWORD) ((m_dw64FTBarValue * 100) / m_dw64FTBarValueMax);
	sprintf(buf, "%d%%", percent);
	SetWindowText(m_hFTPercentText, buf);

	return true;
}
