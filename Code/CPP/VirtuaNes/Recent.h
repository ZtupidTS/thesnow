//
// Recent File ƒNƒ‰ƒX
//
#ifndef	__CRECENT_INCLUDED__
#define	__CRECENT_INCLUDED__

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <string>
using namespace std;

#include "Pathlib.h"
#include "Registry.h"

class	CRecent
{
public:
	static	LPCTSTR	GetName( INT nID );
	static	LPCTSTR	GetPath( INT nID );

	static	void	UpdateMenu( HMENU hMenu );
	static	void	Add( LPCTSTR lpszPath );

	static	void	Load();
	static	void	Save();

protected:
	enum { RECENT_MAX=10 };
	static	TCHAR	m_RecentName[RECENT_MAX][_MAX_PATH];
	static	TCHAR	m_RecentPath[RECENT_MAX][_MAX_PATH];
	static	TCHAR	m_TempPath[_MAX_PATH];

	// Helper
	static	void	MakeManuPath( LPTSTR lpszPath );
private:
};

#endif	// !__CRECENT_INCLUDED__

