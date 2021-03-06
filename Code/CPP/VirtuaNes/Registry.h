//
// レジストリサポートクラス
//
#ifndef	__CREGISTRY_INCLUDED__
#define	__CREGISTRY_INCLUDED__

#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <TCHAR.h>

#include <string>
using namespace std;

class	CRegistry
{
public:
	static	void	SetRegistryKey( LPCTSTR	lpszKey );

	static	UINT	GetProfileInt   ( LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault );
	static	BOOL	GetProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPVOID lpData, UINT nBytes );
	static	BOOL	GetProfileBinary( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPVOID lpData, UINT nBytes );

	static	BOOL	WriteProfileInt( LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nValue );
	static	BOOL	WriteProfileString( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue );
	static	BOOL	WriteProfileBinary( LPCTSTR lpszSection, LPCTSTR lpszEntry, LPVOID pData, UINT nBytes );

protected:
	static	TCHAR	m_szRegistryKey[MAX_PATH];

	static	HKEY	GetRegistryKey();
	static	HKEY	GetSectionKey( LPCTSTR lpszSection );

private:
};

#endif	// !__CREGISTRY_INCLUDED__
