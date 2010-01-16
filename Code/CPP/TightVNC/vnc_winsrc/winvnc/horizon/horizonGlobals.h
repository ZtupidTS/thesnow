
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonGlobals.h 2009 2005-05-05 19:01:23Z jeffo3 $

#ifndef __HORIZON_GLOBALS_H
#define __HORIZON_GLOBALS_H

// for DWORD and HINSTANCE via windows.h
#include "stdhdrs.h"

// horizon gui resource file
#include "horizon/resource.h"

//
// defines
//

// Application specific messages

// Message used for system tray notifications
#define WM_TRAYNOTIFY				WM_USER + 1

// Messages used for the server object to notify windows of things
#define WM_SRV_CLIENT_CONNECT		WM_USER + 2
#define WM_SRV_CLIENT_AUTHENTICATED	WM_USER + 3
#define WM_SRV_CLIENT_DISCONNECT	WM_USER + 4

#define WINVNC_REGISTRY_KEY "Software\\HorizonWimba\\AppShareHost"

//
// application name
//

extern const char* szAppName ;
extern const char* szProcessName ;

//
// log and pid filenames
//

#ifdef _DEBUG
extern const int hzLogLevel ;
extern const int hzLogMode ;
// extern const char hzLogFileName* ;
extern const char* hzLogFileName ;
extern const char* hzPIDFileName ;
#else
extern const int hzLogLevel ;
extern const int hzLogMode ;
extern const char* hzLogFileName ;
extern const char* hzPIDFileName ;
#endif

//
// custom signal for requesting quit
//

extern const int LS_QUIT ;

//
// handles to app data
//

extern HINSTANCE hAppInstance ;
extern DWORD mainthreadId ;

//
// strings needed by vncService
//

extern const char* winvncRunService ;
extern const char* winvncRunServiceHelper ;

//
// deprecated functions
//

int WinVNCAppMain( void ) ;


#endif // __HORIZON_GLOBALS_H ;
