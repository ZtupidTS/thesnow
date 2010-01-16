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

class CDiskMarkApp : public CWinApp
{
public:
	CDiskMarkApp();

	CString m_MainDialogPath;
	CString m_ThemeDir;
	CString m_LangDir;

public:
	virtual BOOL InitInstance();


	DECLARE_MESSAGE_MAP()
};

extern CDiskMarkApp theApp;