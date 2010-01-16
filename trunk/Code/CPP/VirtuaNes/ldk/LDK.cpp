#define	WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "VirtuaNESres.h"

BOOL WINAPI DllMain( HINSTANCE hInstanceDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	return	TRUE;
}

INT	GetVlpVersion( void )
{
	return	VIRTUANES_PLUGIN_VERSION;
}

void	GetVlpLanguage( LPWSTR lpLanguege )
{
	wcscpy( lpLanguege, L"简体中文" );
}
void	GetVlpLanguage( LPSTR lpLanguege )
{
	strcpy( lpLanguege, "简体中文" );
}

LCID	GetVlpLocaleID( void )
{
	return	0x0804;
}

