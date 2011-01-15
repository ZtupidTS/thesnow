#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "VirtuaNESres.h"

BOOL WINAPI DllMain( HINSTANCE hInstanceDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	return	TRUE;
}

INT	GetVlpVersion( void )
{
	return	VIRTUANES_PLUGIN_VERSION;
}

void	GetVlpLanguage( TCHAR* lpLanguege )
{
	_tcscpy( lpLanguege, _T("¼òÌåÖÐÎÄ"));
}

LCID	GetVlpLocaleID( void )
{
	return	0x0804;
}

