//
// Recent File ƒNƒ‰ƒX
//
#ifndef	__CRECENT_INCLUDED__
#define	__CRECENT_INCLUDED__

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
using namespace std;

#include "Pathlib.h"
#include "Registry.h"

class	CRecent
{
public:
	static	LPCWSTR	GetName( INT nID );
	static	LPCWSTR	GetPath( INT nID );

	static	void	UpdateMenu( HMENU hMenu );
	static	void	Add( LPCWSTR lpszPath );

	static	void	Load();
	static	void	Save();

protected:
	enum { RECENT_MAX=10 };
	static	WCHAR	m_RecentName[RECENT_MAX][_MAX_PATH];
	static	WCHAR	m_RecentPath[RECENT_MAX][_MAX_PATH];
	static	WCHAR	m_TempPath[_MAX_PATH];

	// Helper
	static	void	MakeManuPath( LPSTR lpszPath );
private:
};

#endif	// !__CRECENT_INCLUDED__

