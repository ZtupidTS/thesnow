/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#include "resource.h"


#define THEME_DIR					_T("CdmResource\\theme\\")
#define LANGUAGE_DIR				_T("CdmResource\\language\\")
#define DIALOG_DIR					_T("CdmResource\\dialog\\")

#define MENU_THEME_INDEX			2
#define MENU_LANG_INDEX				4

#define MAIN_DIALOG					_T("Main.html")
#define ABOUT_DIALOG				_T("About.html")

#define DEFAULT_THEME				THEME_DIR _T("default\\Main.css")
#define DEFAULT_LANGUAGE			LANGUAGE_DIR _T("English.lang")

class CDiskMarkApp : public CWinApp
{
public:
	CDiskMarkApp();

	CString m_MainDlgPath;
	CString m_AboutDlgPath;
	CString m_ThemeDir;
	CString m_LangDir;
	DWORD m_ThemeIndex;
	DWORD m_LangIndex;

	CString m_ExeDir;
	CString m_Ini;

public:
	virtual BOOL InitInstance();


	DECLARE_MESSAGE_MAP()
};

extern CDiskMarkApp theApp;