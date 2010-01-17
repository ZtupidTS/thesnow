//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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


// WinVNC header file

#ifdef HORIZONLIVE
#include "horizon/horizonMain.h"
#else

#ifndef __WINVNC_H
#define __WINVNC_H

#include "stdhdrs.h"
#include "resource.h"
#include "VNCHelp.h"

// Application specific messages

// Message used for system tray notifications
#define WM_TRAYNOTIFY				WM_USER+1

// Messages used for the server object to notify windows of things
#define WM_SRV_CLIENT_CONNECT		WM_USER+2
#define WM_SRV_CLIENT_AUTHENTICATED	WM_USER+3
#define WM_SRV_CLIENT_DISCONNECT	WM_USER+4
#define WM_SRV_CLIENT_HIDEWALLPAPER	WM_USER+5

#define WINVNC_REGISTRY_KEY L"Software\\ORL\\WinVNC3"

// Export the application details
extern HINSTANCE	hAppInstance;
extern const wchar_t	*szAppName;
extern DWORD		mainthreadId;
extern VNCHelp		help;
// Main VNC server routine
extern int WinVNCAppMain();

// Standard command-line flag definitions
const wchar_t winvncRunService[]		= L"-service";
const wchar_t winvncRunServiceHelper[]	= L"-servicehelper";
const wchar_t winvncRunAsUserApp[]		= L"-run";

const wchar_t winvncInstallService[]	= L"-install";
const wchar_t winvncRemoveService[]	= L"-remove";
const wchar_t winvncReinstallService[]	= L"-reinstall";

const wchar_t winvncReload[]			= L"-reload";
const wchar_t winvncShowProperties[]	= L"-settings";
const wchar_t winvncShowDefaultProperties[]	= L"-defaultsettings";
const wchar_t winvncShowAbout[]		= L"-about";
const wchar_t winvncKillRunningCopy[]	= L"-kill";

const wchar_t winvncShareAll[]			= L"-shareall";
const wchar_t winvncSharePrimary[]		= L"-shareprimary";
const wchar_t winvncShareArea[]		= L"-sharearea";
const wchar_t winvncShareWindow[]		= L"-sharewindow";

const wchar_t winvncVideoClass[]		= L"-videoclass";

const wchar_t winvncAddNewClient[]		= L"-connect";
const wchar_t winvncKillAllClients[]	= L"-killallclients";

const wchar_t winvncShowHelp[]			= L"-help";

// Usage string
const wchar_t winvncUsageText[] =
	L"winvnc [-run] [-kill] [-service] [-servicehelper]\n"
	L" [-connect [host[:display]]] [-connect [host[::port]]]\n"
	L" [-install] [-remove] [-reinstall] [-reload]\n"
	L" [-settings] [-defaultsettings] [-killallclients]\n"
	L" [-shareall] [-shareprimary] [-sharearea WxH+X+Y]\n"
	L" [-sharewindow \"title\"] [-videoclass \"windowclass\"]\n"
    L" [-about] [-help]\n";

#endif // __WINVNC_H

#endif // HORIZONLIVE
