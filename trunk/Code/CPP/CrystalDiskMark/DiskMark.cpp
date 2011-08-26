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

#include <afxole.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CDiskMarkApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CDiskMarkApp::CDiskMarkApp()
{
}

CDiskMarkApp theApp;

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
static BOOL IsFileExistEx(const TCHAR* path, const TCHAR* fileName);

BOOL CDiskMarkApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Init m_Ini
	TCHAR *ptrEnd;
	TCHAR ini[MAX_PATH];
	::GetModuleFileName(NULL, ini, MAX_PATH);
	if((ptrEnd = _tcsrchr(ini, '.')) != NULL)
	{
		*ptrEnd = '\0';
		_tcscat_s(ini, MAX_PATH, _T(".ini"));
	}
	m_Ini = ini;

	CString DefaultTheme;
	CString DefaultLanguage;
	TCHAR tmp[MAX_PATH];

	GetModuleFileName(NULL, tmp, MAX_PATH);
	if((ptrEnd = _tcsrchr(tmp, '\\')) != NULL){*ptrEnd = '\0';}
	m_ExeDir.Format(_T("%s\\"), tmp);
	m_ThemeDir.Format(_T("%s\\%s"), tmp, THEME_DIR);
	m_LangDir.Format(_T("%s\\%s"), tmp, LANGUAGE_DIR);

	m_ThemeIndex = MENU_THEME_INDEX;
	m_LangIndex = MENU_LANG_INDEX;

	m_MainDlgPath.Format(_T("%s\\") DIALOG_DIR MAIN_DIALOG, tmp);
	m_AboutDlgPath.Format(_T("%s\\") DIALOG_DIR ABOUT_DIALOG, tmp);

	DefaultTheme.Format(_T("%s\\%s"), tmp, DEFAULT_THEME);
	DefaultLanguage.Format(_T("%s\\%s"), tmp, DEFAULT_LANGUAGE);
	
	if(! IsFileExist(m_MainDlgPath))
	{
		m_MainDlgPath.Format(_T("%s\\") DIALOG_DIR MAIN_DIALOG, tmp);
	}

	if(! IsFileExistEx(m_MainDlgPath, MAIN_DIALOG))					{	return FALSE;	}
	if(! IsFileExistEx(DefaultTheme, DEFAULT_THEME))				{	return FALSE;	}
	if(! IsFileExistEx(DefaultLanguage, DEFAULT_LANGUAGE))			{	return FALSE;	}


	// No Server Busy Dialog!!
	if(!AfxOleInit())
	{
		AfxMessageBox(_T("Unsupported OLE version"));
		return FALSE;
	}

	// No Server Busy Dialog!!
	AfxOleGetMessageFilter()->SetMessagePendingDelay(60 * 1000);
	AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);
	AfxOleGetMessageFilter()->EnableBusyDialog(FALSE);

	// Multimedia Timer Setting
	TIMECAPS tc;
	timeGetDevCaps(&tc,sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin);

	BOOL flagReExec = FALSE;
	// IE Version Check.
	if(GetFileVersion(_T("Shdocvw.dll")) >= 471)
	{
		CDiskMarkDlg dlg;
		m_pMainWnd = &dlg;

		if(dlg.DoModal() == RE_EXEC)
		{
			flagReExec = TRUE;
		}
	}
	else
	{
		AfxMessageBox(_T("CrystalDiskMark is required IE6 or later."));
	}

	timeEndPeriod(tc.wPeriodMin);

	if(flagReExec)
	{
		TCHAR str[MAX_PATH];
		::GetModuleFileName(NULL, str, MAX_PATH);
		ShellExecute(NULL, NULL, str, NULL, NULL, SW_SHOWNORMAL);
	}

	return FALSE;
}

BOOL IsFileExistEx(const TCHAR* path, const TCHAR* fileName)
{
	if(! IsFileExist(path))
	{
		CString cstr;
		cstr.Format(_T("Not Found \"%s\"."), fileName); 
		AfxMessageBox(cstr);
		return FALSE;
	}
	return TRUE;
}