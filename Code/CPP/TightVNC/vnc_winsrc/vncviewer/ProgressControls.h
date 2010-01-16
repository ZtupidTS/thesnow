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

#ifndef PROGRESSCONTROLS_H__
#define PROGRESSCONTROLS_H__

class ProgressControls
{
public:
	ProgressControls(HWND hParent);
	~ProgressControls();

	bool initLeftControl(DWORD64 maxFT, DWORD64 posFT);
	bool initRightControl(DWORD max, DWORD pos);

	bool increase(DWORD value);
	bool clear();

private:
	HWND m_hParent;

	HWND m_hProgressBar;
	HWND m_hPercentText;

	HWND m_hFTProgressBar;
	HWND m_hFTPercentText;

	DWORD m_dwBarValue;
	DWORD m_dwBarValueMax;

	DWORD64 m_dw64FTBarValue;
	DWORD64 m_dw64FTBarValueMax;

	bool showControlsValue();

};

#endif // PROGRESSCONTROLS_H__
