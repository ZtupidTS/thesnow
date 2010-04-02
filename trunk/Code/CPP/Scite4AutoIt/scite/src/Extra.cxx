#include "Extra.h"
#include <windows.h>
#include <CommCtrl.h>

HWND ToolTip::Create(HWND ControlHWND, LPTSTR lpszText){

	// struct specifying control classes to register
	INITCOMMONCONTROLSEX iccex; 
	// struct specifying info about tool in ToolTip control
	TOOLINFO ti;
	unsigned int uid = 0;       // for ti initialization
	RECT rect;                  // for client area coordinates

	/* INITIALIZE COMMON CONTROLS */
	iccex.dwICC = ICC_WIN95_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccex);

	/* CREATE A TOOLTIP WINDOW */
	hwndTip = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS,
		NULL,
		WS_POPUP| TTS_NOPREFIX | TTS_ALWAYSTIP,		
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		ControlHWND,
		NULL,
		NULL,
		NULL
		);
	SetWindowPos(hwndTip,
		HWND_TOPMOST,
		0,
		0,
		0,
		0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	/* GET COORDINATES OF THE MAIN CLIENT AREA */
	GetClientRect (ControlHWND, &rect);

	/* INITIALIZE MEMBERS OF THE TOOLINFO STRUCTURE */
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_TRANSPARENT |TTF_SUBCLASS|TTS_USEVISUALSTYLE ;
	ti.hwnd = ControlHWND;
	ti.hinst = NULL;
	ti.uId = uid;
	ti.lpszText = lpszText;
	// ToolTip control will cover the whole window
	ti.rect.left = rect.left;    
	ti.rect.top = rect.top;
	ti.rect.right = rect.right;
	ti.rect.bottom = rect.bottom;

	/* SEND AN ADDTOOL MESSAGE TO THE TOOLTIP CONTROL WINDOW */
	SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	return(hwndTip);    
};
void ToolTip::Delete(HWND ControlHWND){
	CloseWindow(ControlHWND);
};

LPWSTR LanguageIni::GetValue(LPWSTR lpSection,LPWSTR lpKey,LPWSTR lpDefault){
	wcscpy(lang,L"\0");
	GetPrivateProfileStringW(lpSection,lpKey,lpDefault,lang,sizeof(lang),L".\\language.ini");
	if (wcslen(lang) == 0)
	{
	 return lpDefault;
	}else
	{
	 return lang;
	};
};

LPCSTR LanguageIni::GetValueA(LPCSTR lpSection,LPCSTR lpKey,LPCSTR lpDefault){
	strcpy(langA,"\0");
	GetPrivateProfileStringA(lpSection,lpKey,lpDefault,langA,sizeof(langA),".\\language.ini");
	if (strlen(langA) == 0)
	{
		return lpDefault;
	}else
	{
		return langA;
	};
};