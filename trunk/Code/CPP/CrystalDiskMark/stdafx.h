/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2007-2008 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT              
#define _WIN32_WINNT 0x0501
#endif						

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard component
#include <afxext.h>         // Extended MFC

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC IE4 Common Control support
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC Windows Common Control support
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxdhtml.h>        // CDHtmlDialog
#include "DHtmlDialogEx.h"		// CDHtmlDialogEx by hiyohiyo
#include "DHtmlMainDialog.h"	// CDHtmlMainDialog by hiyohiyo

#ifdef _UNICODE
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Version Information
#define PRODUCT_NAME			_T("CrystalDiskMark")
#ifdef _M_X64
#define PRODUCT_VERSION			_T("3.0 x64")
#else
#define PRODUCT_VERSION			_T("3.0")
#endif
#define PRODUCT_SHORT_NAME		_T("CDM 3")
#define PRODUCT_RELEASE			_T("2010/3/21")
#define PRODUCT_COPY_YEAR		_T("2007-2010")
#define PRODUCT_COPYRIGHT		_T("Copyright (C) 2007-2010 hiyohiyo.")

#define URL_CRYSTAL_DEW_WORLD_JA	_T("http://crystalmark.info/")
#define URL_CRYSTAL_DEW_WORLD_EN 	_T("http://crystalmark.info/?lang=en")

#define HTML_HELP_JA		_T("manual-ja.chm")
#define HTML_HELP_EN 		_T("manual-en.chm")

static const int RE_EXEC = 5963;

#ifdef WIN64
#ifdef _M_X64
#define PRODUCT_EDITION	_T("NT - x64 - Unicode")
#else // _M_IA64
#define PRODUCT_EDITION	_T("NT - IA64 - Unicode")
#endif
#else
#ifdef _UNICODE
#define PRODUCT_EDITION	_T("NT - x86 - Unicode")
#else
#define PRODUCT_EDITION	_T("9x - x86 - MBCS")
#endif // _UNICODE
#endif // WIN64

