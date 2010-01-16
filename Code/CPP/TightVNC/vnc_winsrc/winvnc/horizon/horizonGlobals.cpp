
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonGlobals.cpp 2009 2005-05-05 19:01:23Z jeffo3 $

#include "horizonGlobals.h"

//
// application name
//

const char* szAppName = "Horizon Wimba AppShare" ;
const char* szProcessName = "AppShare" ;

//
// log and pid filenames
//

#ifdef _DEBUG
const int hzLogLevel = LL_ALL ;
const int hzLogMode = Log::ToConsole | Log::ToFile ;
// const char* hzLogFileName = "G:\\appshare_debug.log" ;
const char* hzLogFileName = "..\\logs\\appshare_debug.log" ;
const char* hzPIDFileName = "..\\logs\\appshare.pid" ;
#else
const int hzLogLevel = LL_INTWARN ;
const int hzLogMode = Log::ToFile ;
const char* hzLogFileName = "..\\logs\\appshare.log" ;
const char* hzPIDFileName = "..\\logs\\appshare.pid" ;
#endif

//
// custom signal for requesting quit
//

const int LS_QUIT = 0x800D ;

//
// handles to app data
//

HINSTANCE hAppInstance ;
DWORD mainthreadId ;

//
// needed by vncService
//

const char* winvncRunService = "-service" ;
const char* winvncRunServiceHelper = "-servicehelper" ;

//
// deprecated functions
//

int 
WinVNCAppMain( void )
{
	MessageBox( NULL, "Invalid call to WinVNCAppMain()", 
		szAppName, MB_OK | MB_ICONERROR ) ;
	return 0 ;
}
