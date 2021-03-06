//
// パスライブラリクラス
//
#include "Pathlib.h"

string	CPathlib::SplitPath( LPCSTR lpszPath )
{
	CHAR	szDrive[ _MAX_DRIVE ];
	CHAR	szDir  [ _MAX_DIR ];
	CHAR	szTemp [ _MAX_PATH+1 ];
	string	path;
	::_splitpath( lpszPath, szDrive, szDir, NULL, NULL );
	::_makepath( szTemp, szDrive, szDir, NULL, NULL );
	path = szTemp;
	return	path;
}
wstring	CPathlib::SplitPath( LPCWSTR lpszPath )
{
	WCHAR	szDrive[ _MAX_DRIVE ];
	WCHAR	szDir  [ _MAX_DIR ];
	WCHAR	szTemp [ _MAX_PATH+1 ];
	wstring	path;
	::_wsplitpath( lpszPath, szDrive, szDir, NULL, NULL );
	::_wmakepath( szTemp, szDrive, szDir, NULL, NULL );
	path = szTemp;
	return	path;
}


string	CPathlib::SplitFname( LPCSTR lpszPath )
{
	CHAR	szTemp [ _MAX_PATH+1 ];
	string	fname;
	::_splitpath( lpszPath, NULL, NULL, szTemp, NULL );
	fname = szTemp;
	return	fname;
}

wstring	CPathlib::SplitFname( LPCWSTR lpszPath )
{
	WCHAR	szTemp [ _MAX_PATH+1 ];
	wstring	fname;
	::_wsplitpath( lpszPath, NULL, NULL, szTemp, NULL );
	fname = szTemp;
	return	fname;
}


string	CPathlib::SplitFnameExt( LPCSTR lpszPath )
{
	CHAR	szFname[ _MAX_FNAME ];
	CHAR	szExt  [ _MAX_EXT ];
	CHAR	szTemp [ _MAX_PATH+1 ];
	string	fname;
	::_splitpath( lpszPath, NULL, NULL, szFname, szExt );
	::_makepath( szTemp, NULL, NULL, szFname, szExt );
	fname = szTemp;
	return	fname;
}

wstring	CPathlib::SplitFnameExt( LPCWSTR lpszPath )
{
	WCHAR	szFname[ _MAX_FNAME ];
	WCHAR	szExt  [ _MAX_EXT ];
	WCHAR	szTemp [ _MAX_PATH+1 ];
	wstring	fname;
	::_wsplitpath( lpszPath, NULL, NULL, szFname, szExt );
	::_wmakepath( szTemp, NULL, NULL, szFname, szExt );
	fname = szTemp;
	return	fname;
}

string	CPathlib::SplitExt( LPCSTR lpszPath )
{
	CHAR	szExt[ _MAX_EXT ];
	string	fname;
	::_splitpath( lpszPath, NULL, NULL, NULL, szExt );
	fname = szExt;
	return	fname;
}

wstring	CPathlib::SplitExt( LPCWSTR lpszPath )
{
	WCHAR	szExt[ _MAX_EXT ];
	wstring	fname;
	::_wsplitpath( lpszPath, NULL, NULL, NULL, szExt );
	fname = szExt;
	return	fname;
}

string	CPathlib::MakePath( LPCSTR lpszPath, LPCSTR lpszFname )
{
	CHAR	szDrive[ _MAX_DRIVE ];
	CHAR	szDir  [ _MAX_DIR ];
	CHAR	szTemp [ _MAX_PATH+1 ];
	string	path;
	::_splitpath( lpszPath, szDrive, szDir, NULL, NULL );
	::_makepath( szTemp, szDrive, szDir, lpszFname, NULL );
	path = szTemp;
	return	path;
}

wstring	CPathlib::MakePath( LPCTSTR lpszPath, LPCTSTR lpszFname )
{
	TCHAR	szDrive[ _MAX_DRIVE ];
	TCHAR	szDir  [ _MAX_DIR ];
	TCHAR	szTemp [ _MAX_PATH+1 ];
	wstring	path;
	::_wsplitpath( lpszPath, szDrive, szDir, NULL, NULL );
	::_wmakepath( szTemp, szDrive, szDir, lpszFname, NULL );
	path = szTemp;
	return	path;
}


string	CPathlib::MakePathExt( LPCSTR lpszPath, LPCSTR lpszFname, LPCSTR lpszExt )
{
	CHAR	szDrive[ _MAX_DRIVE ];
	CHAR	szDir  [ _MAX_DIR ];
	CHAR	szTemp [ _MAX_PATH+1 ];
	string	path;
	::_splitpath( lpszPath, szDrive, szDir, NULL, NULL );
	::_makepath( szTemp, szDrive, szDir, lpszFname, lpszExt );
	path = szTemp;
	return	path;
}

wstring	CPathlib::MakePathExt( LPCWSTR lpszPath, LPCWSTR lpszFname, LPCWSTR lpszExt )
{
	WCHAR	szDrive[ _MAX_DRIVE ];
	WCHAR	szDir  [ _MAX_DIR ];
	WCHAR	szTemp [ _MAX_PATH+1 ];
	wstring	path;
	::_wsplitpath( lpszPath, szDrive, szDir, NULL, NULL );
	::_wmakepath( szTemp, szDrive, szDir, lpszFname, lpszExt );
	path = szTemp;
	return	path;
}

string	CPathlib::CreatePath( LPCSTR lpszBasePath, LPCSTR lpszPath )
{
	CHAR	szPath[ _MAX_PATH ];
	string	path;
	if( ::PathIsRelativeA( lpszPath ) ) {
		CHAR	szTemp[ _MAX_PATH ];
		::strcpy( szTemp, lpszBasePath );
		::PathAppendA( szTemp, lpszPath );
		::PathCanonicalizeA( szPath, szTemp );
	} else {
		::strcpy( szPath, lpszPath );
	}
	path = szPath;
	return	path;
}
wstring	CPathlib::CreatePath( LPCWSTR lpszBasePath, LPCWSTR lpszPath )
{
	WCHAR	szPath[ _MAX_PATH ];
	wstring	path;
	if( ::PathIsRelative( lpszPath ) ) {
		WCHAR	szTemp[ _MAX_PATH ];
		::wcscpy( szTemp, lpszBasePath );
		::PathAppend( szTemp, lpszPath );
		::PathCanonicalize( szPath, szTemp );
	} else {
		::wcscpy( szPath, lpszPath );
	}
	path = szPath;
	return	path;
}
INT CALLBACK	CPathlib::BffCallback( HWND hWnd, UINT uMsg, LPARAM lParam, WPARAM wParam )
{
	if( uMsg == BFFM_INITIALIZED && wParam ) {
		::SendMessage( hWnd, BFFM_SETSELECTION, TRUE, wParam );
	}
	return	TRUE;
}

BOOL	CPathlib::SelectFolder( HWND hWnd, LPCSTR lpszTitle, LPSTR lpszFolder )
{
	BROWSEINFOA	bi;
	LPITEMIDLIST	pidl;

	ZeroMemory( &bi, sizeof(bi) );
	bi.hwndOwner = hWnd;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	// For Folder setup
	bi.lpfn = (BFFCALLBACK)BffCallback;

	// 最後に'\'が付いているとデフォルト選択してくれない(Win98)ので...
	if( lpszFolder ) {
		if( ::strlen(lpszFolder) > 3 ) {
			if( lpszFolder[::strlen(lpszFolder)-1] == '\\' )
				lpszFolder[::strlen(lpszFolder)-1] = NULL;
		}
		bi.lParam = (LPARAM)lpszFolder;
	} else {
		bi.lParam = NULL;
	}

	string	path;
	if( (pidl = ::SHBrowseForFolderA( &bi )) ) {
		path.resize( _MAX_PATH+1 );
		::SHGetPathFromIDListA( pidl, lpszFolder );
		if( ::strlen(lpszFolder) > 3 ) {	// ドライブ名の場合を除く
			::strcat( lpszFolder, "\\" );
		}
		IMalloc* pMalloc;
		::SHGetMalloc( &pMalloc );
		if( pMalloc ) {
			pMalloc->Free( pidl );
			pMalloc->Release();
		}
		return	TRUE;
	}

	return	FALSE;
}

BOOL	CPathlib::SelectFolder( HWND hWnd, LPCWSTR lpszTitle, LPWSTR lpszFolder )
{
	BROWSEINFO	bi;
	LPITEMIDLIST	pidl;

	ZeroMemory( &bi, sizeof(bi) );
	bi.hwndOwner = hWnd;
	bi.lpszTitle = lpszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	// For Folder setup
	bi.lpfn = (BFFCALLBACK)BffCallback;

	// 最後に'\'が付いているとデフォルト選択してくれない(Win98)ので...
	if( lpszFolder ) {
		if( ::wcslen(lpszFolder) > 3 ) {
			if( lpszFolder[::wcslen(lpszFolder)-1] == L'\\' )
				lpszFolder[::wcslen(lpszFolder)-1] = NULL;
		}
		bi.lParam = (LPARAM)lpszFolder;
	} else {
		bi.lParam = NULL;
	}

	wstring	path;
	if( (pidl = ::SHBrowseForFolder( &bi )) ) {
		path.resize( _MAX_PATH+1 );
		::SHGetPathFromIDList( pidl, lpszFolder );
		if( ::wcslen(lpszFolder) > 3 ) {	// ドライブ名の場合を除く
			::wcscat( lpszFolder, L"\\" );
		}
		IMalloc* pMalloc;
		::SHGetMalloc( &pMalloc );
		if( pMalloc ) {
			pMalloc->Free( pidl );
			pMalloc->Release();
		}
		return	TRUE;
	}

	return	FALSE;
}