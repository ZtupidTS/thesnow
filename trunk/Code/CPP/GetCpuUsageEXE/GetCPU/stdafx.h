// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
#pragma once

#define     _WIN32_WINNT    (0x0500)    // WE ARE WinNT 5.0 SPECIFIC !!!
#define     UNICODE
#define     _UNICODE
#pragma  warning( disable:4201) // nonstandard extension used : nameless struct/union
#pragma  warning( disable:4710) // warning C4710: function 'toto' not expanded

#include <crtdbg.h>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include <iostream>
