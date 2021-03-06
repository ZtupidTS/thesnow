//
// Recent File クラス
//
#include <TCHAR.h>

#include "DebugOut.h"
#include "PathLib.h"

#include "VirtuaNESres.h"
#include "Recent.h"

TCHAR	CRecent::m_RecentName[RECENT_MAX][_MAX_PATH];
TCHAR	CRecent::m_RecentPath[RECENT_MAX][_MAX_PATH];
TCHAR	CRecent::m_TempPath[_MAX_PATH];

LPCTSTR	CRecent::GetName( INT nID )
{
	// CRecent::Add で同じポインタを指して戻ってくる事があるので一旦テンポラリにコピー
	::_tcscpy( m_TempPath, m_RecentName[nID] );
	return	m_TempPath;
}

LPCTSTR	CRecent::GetPath( INT nID )
{
	return	m_RecentPath[nID];
}

void	CRecent::MakeManuPath( LPTSTR lpszPath )
{
	wstring	FullPath = lpszPath;
	wstring	FileName = CPathlib::SplitFnameExt( lpszPath );

	// 30文字以下はそのまま
	if( FullPath.size() <= 30 )
		return;

	// ファイル名が30文字以上の場合
	if( ::lstrlen( FileName.c_str() ) >= 30 ) {
		::lstrcpy( lpszPath, FileName.c_str() );
		return;
	}

	LPCTSTR lpszCur = lpszPath + 2;
	if( lpszPath[0] == _T('\\') && lpszPath[1] == _T('\\') ) {
		while (*lpszCur != _T('\\')) {
//			lpszCur = _tcsinc(lpszCur);
		}
	}

	if( ::lstrlen(FullPath.c_str()) - ::lstrlen(FileName.c_str()) > 3 ) {
//		lpszCur = _tcsinc(lpszCur);
		while( *lpszCur != _T('\\') ) {
//			lpszCur = _tcsinc(lpszCur);
		}
	}

	INT	nVolume = lpszCur - lpszPath;
	if( 30 < nVolume+5+::lstrlen(FileName.c_str()) ) {
		::lstrcpy( lpszPath, FileName.c_str() );
		return;
	}

	while ( nVolume+4+::lstrlen(lpszCur) > 30 ) {
		do {
			lpszCur = _tcsinc(lpszCur);
		}
		while( *lpszCur != _T('\\') );
	}

	lpszPath[nVolume] = _T('\0');
	::lstrcat( lpszPath, _T("\\...") );
	::lstrcat( lpszPath, lpszCur );
}

void	CRecent::UpdateMenu( HMENU hMenu )
{
	// メニューなし？
	if( !hMenu )
		return;

	// ﾌｧｲﾙ(&F)メニューの取得
	HMENU hSubMenu = ::GetSubMenu( hMenu, 0 );

	// 最近使ったﾌｫﾙﾀﾞ(&P)ポップアップメニューの取得
	HMENU hPathMenu = ::GetSubMenu( hSubMenu, 12 );
	// 最近使ったﾌｧｲﾙ(&F)ポップアップメニューの取得
	HMENU hFileMenu = ::GetSubMenu( hSubMenu, 13 );

	// 項目が無い場合
	if( ::_tcslen(m_RecentPath[0]) <= 0 ) {
		// ディセーブルにする
		::EnableMenuItem( hPathMenu, ID_MRU_PATH0, MF_BYCOMMAND|MF_GRAYED );
	} else {
		INT	i;
		// メニューアイテムの削除
		for( i = 0; i < RECENT_MAX; i++ ) {
			::DeleteMenu( hPathMenu, ID_MRU_PATH0+i, MF_BYCOMMAND );
		}

		TCHAR	szRecent[_MAX_PATH];
		TCHAR	szTemp[_MAX_PATH];
		for( i = 0; i < RECENT_MAX; i++ ) {
			if( ::_tcslen(m_RecentPath[i]) > 0 ) {
				// パスをメニュー用に短くしたりする
				::_tcscpy( szRecent, m_RecentPath[i] );

				// '&'付きのファイルの'&'を'&&'に変換する
				LPCTSTR	pSrc = szRecent;
				LPTSTR	pDst = szTemp;
				while( *pSrc != 0 ) {
					if( *pSrc == _T('&') )
						*pDst++ = _T('&');
					if( _istlead(*pSrc) )
						*pDst++ = *pSrc++;
					*pDst++ = *pSrc++;
				}
				*pDst = 0;
				::wsprintf( szRecent, _T("&%d "), (i+1)%10 );
				::_tcscat( szRecent, szTemp );

				// メニューに追加
				::InsertMenu( hPathMenu, i, MF_BYPOSITION, ID_MRU_PATH0+i, szRecent );
			} else {
				break;
			}
		}
	}

	// 項目が無い場合
	if( ::_tcslen(m_RecentName[0]) <= 0 ) {
		// ディセーブルにする
		::EnableMenuItem( hFileMenu, ID_MRU_FILE0, MF_BYCOMMAND|MF_GRAYED );
	} else {
		INT	i;
		// メニューアイテムの削除
		for( i = 0; i < RECENT_MAX; i++ ) {
			::DeleteMenu( hFileMenu, ID_MRU_FILE0+i, MF_BYCOMMAND );
		}

		TCHAR	szRecent[_MAX_PATH];
		TCHAR	szTemp[_MAX_PATH];
		for( i = 0; i < RECENT_MAX; i++ ) {
			if( ::_tcslen(m_RecentName[i]) > 0 ) {
				// パスをメニュー用に短くしたりする
				::_tcscpy( szRecent, m_RecentName[i] );
				MakeManuPath( szRecent );

				// '&'付きのファイルの'&'を'&&'に変換する
				LPCTSTR	pSrc = szRecent;
				LPTSTR	pDst = szTemp;
				while( *pSrc != 0 ) {
					if( *pSrc == _T('&') )
						*pDst++ = _T('&');
					if( _istlead(*pSrc) )
						*pDst++ = *pSrc++;
					*pDst++ = *pSrc++;
				}
				*pDst = 0;
				::wsprintf( szRecent, _T("&%d "), (i+1)%10 );
				::_tcscat( szRecent, szTemp );

				// メニューに追加
				::InsertMenu( hFileMenu, i, MF_BYPOSITION, ID_MRU_FILE0+i, szRecent );
			} else {
				break;
			}
		}
	}
}

void	CRecent::Add( LPCTSTR lpszPath )
{
	INT	i, j;

	if( ::_tcslen(m_RecentName[0]) > 0 ) {
		for( i = 0; i < RECENT_MAX; i++ ) {
			if( ::_tcslen(m_RecentName[i]) <= 0 )
				break;
		}
		for( j = 0; j < i; j++ ) {
			if( ::_tcscmp( lpszPath, m_RecentName[j] ) == 0 )
				break;
		}
		if( j == RECENT_MAX )
			j--;
		for( ; j > 0; j-- ) {
			::_tcscpy( &m_RecentName[j][0], &m_RecentName[j-1][0] );
		}
	}
	::_tcscpy( m_RecentName[0], lpszPath );

	wstring	temp = CPathlib::SplitPath(lpszPath );
	if( ::_tcslen(m_RecentPath[0]) > 0 ) {
		for( i = 0; i < RECENT_MAX; i++ ) {
			if( ::_tcslen(m_RecentPath[i]) <= 0 )
				break;
		}
		for( j = 0; j < i; j++ ) {
			if( ::_tcscmp( m_RecentPath[j], temp.c_str() ) == 0 )
				break;
		}
		if( j == RECENT_MAX )
			j--;
		for( ; j > 0; j-- ) {
			::_tcscpy( m_RecentPath[j], m_RecentPath[j-1] );
		}
	}
	::_tcscpy( m_RecentPath[0], temp.c_str() );
}

void	CRecent::Load()
{
	INT	i;
	TCHAR	szTemp[MAX_PATH];
	TCHAR	szEntry[32];
	for( i = 0; i < RECENT_MAX; i++ ) {
		::wsprintf( szEntry, _T("Path%d"), i+1 );
		if( CRegistry::GetProfileString( _T("Recent Path List"), szEntry, szTemp, sizeof(szTemp) ) )
			::_tcscpy( m_RecentPath[i], szTemp );
	}
	for( i = 0; i < RECENT_MAX; i++ ) {
		::wsprintf( szEntry, _T("File%d"), i+1 );
		if( CRegistry::GetProfileString( _T("Recent File List"), szEntry, szTemp, sizeof(szTemp) ) )
			::_tcscpy( m_RecentName[i], szTemp );
	}
}

void	CRecent::Save()
{
	INT	i;
	TCHAR	szEntry[32];
	for( i = 0; i < RECENT_MAX; i++ ) {
		if( ::_tcslen(m_RecentPath[i]) > 0 ) {
			::wsprintf( szEntry, _T("Path%d"), i+1 );
			CRegistry::WriteProfileString( _T("Recent Path List"), szEntry, m_RecentPath[i] );
		}
	}
	for( i = 0; i < RECENT_MAX; i++ ) {
		if( ::_tcslen(m_RecentName[i]) > 0 ) {
			::wsprintf( szEntry, _T("File%d"), i+1 );
			CRegistry::WriteProfileString( _T("Recent File List"), szEntry, m_RecentName[i] );
		}
	}
}

