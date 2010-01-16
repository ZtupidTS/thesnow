/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskMark.h"
#include "DiskMarkDlg.h"

#include "GetFileVersion.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define THEME_DIR			_T("resource\\theme\\")
#define LANGUAGE_DIR		_T("resource\\language\\")
#define HTML_MAIN_DIALOG	_T("resource\\theme\\Main.dlg")
#define DEFAULT_THEME		THEME_DIR _T("default\\Main.css")
#define DEFAULT_LANGUAGE	LANGUAGE_DIR _T("English.lang")

BEGIN_MESSAGE_MAP(CDiskMarkApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CDiskMarkApp::CDiskMarkApp()
{
}

CDiskMarkApp theApp;

BOOL CDiskMarkApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	CString DefaultTheme;
	CString DefaultLang;
	TCHAR tmp[MAX_PATH];
	TCHAR *ptrEnd;

	GetModuleFileName(NULL, tmp, MAX_PATH);
	if((ptrEnd = _tcsrchr(tmp, '\\')) != NULL){*ptrEnd = '\0';}
	m_MainDialogPath.Format(_T("%s\\")HTML_MAIN_DIALOG, tmp);
	m_ThemeDir.Format(_T("%s\\%s"), tmp, THEME_DIR);
	m_LangDir.Format(_T("%s\\%s"), tmp, LANGUAGE_DIR);

	DefaultTheme.Format(_T("%s\\%s"), tmp, DEFAULT_THEME);
	DefaultLang.Format(_T("%s\\%s"), tmp, DEFAULT_LANGUAGE);
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile(m_MainDialogPath, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE){
		AfxMessageBox(_T("Not Found \"")HTML_MAIN_DIALOG _T("\"."));
		return FALSE;
	}else{
		FindClose(hFind);
	}

	hFind = FindFirstFile(DefaultTheme, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE){
		AfxMessageBox(_T("Not Found \"")DEFAULT_THEME _T("\"."));
		return FALSE;
	}else{
		FindClose(hFind);
	}

	hFind = FindFirstFile(DefaultLang, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE){
		AfxMessageBox(_T("Not Found \"")DEFAULT_LANGUAGE _T("\"."));
		return FALSE;
	}else{
		FindClose(hFind);
	}

	// Multimedia Timer Setting
	TIMECAPS tc;
	timeGetDevCaps(&tc,sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin);

	// IE Version Check.
	if(GetFileVersion(_T("Shdocvw.dll")) >= 471)
	{
		CDiskMarkDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
	}else{
		AfxMessageBox(_T("CrystalDiskMark is required IE6 or later."));
	}

	timeEndPeriod(tc.wPeriodMin);

	return FALSE;
}
