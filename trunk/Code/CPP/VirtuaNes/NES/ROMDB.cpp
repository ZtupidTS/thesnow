//
// NES ROMDB class
//
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <mbstring.h>

#include "typedef.h"
#include "macro.h"

#include "DebugOut.h"
#include "App.h"
#include "Plugin.h"
#include "Pathlib.h"
#include "Crclib.h"

#include "Archive.h"

#include "romdb.h"

ROMDATABASE	romdatabase;

//
// ROM DATABASE (NESToy&NNNesterJ database)
//
INT	ROMDATABASE::HeaderCheck( NESHEADER& hdr, DWORD crcall, DWORD crc, ROMDB& data )
{
	if( m_DataBaseList.empty() ) {
		LoadDatabase();
	}

	if( m_DataBaseList.empty() )
		return	-2;	// データベースが無い

	map<DWORD, ROMDB, ROMDBCMP>::iterator it = m_DataBaseList.find( crcall );

	if( it == m_DataBaseList.end() )
		return	-1;	// データベースに無い

	data = (*it).second;

	// 一応チェック
	if( data.crcall == crcall || (data.crc == crc && data.crc) ) {
		if( hdr.control1 == data.control1 && hdr.control2 == data.control2 ) {
			return	0;	// 完全適合
		}
	}
	return	1;	// CRCは一致したがヘッダが違う
}

BOOL	ROMDATABASE::HeaderCorrect( NESHEADER& hdr, DWORD crcall, DWORD crc )
{
	if( m_DataBaseList.empty() ) {
		LoadDatabase();
	}

	if( m_DataBaseList.empty() )
		return	FALSE;	// データベースが無い

	map<DWORD, ROMDB, ROMDBCMP>::iterator it = m_DataBaseList.find( crcall );

	if( it == m_DataBaseList.end() )
		return	FALSE;	// データベースに無い

	ROMDB	data = (*it).second;

	// 一応チェック
	if( data.crcall == crcall || (data.crc == crc && data.crc) ) {
		hdr.control1 = data.control1;
		hdr.control2 = data.control2;
		for( INT i = 0; i < 8; i++ ) {
			hdr.reserved[i] = 0;
		}
		return	TRUE;
	}
	return	FALSE;
}

void	ROMDATABASE::LoadDatabase()
{
FILE*	fp = NULL;
TCHAR	buf[512];
const TCHAR seps[] = _T(";\n\0");	// セパレータ
ROMDB	db;

DEBUGOUT( "Database loading...\n" );

	wstring	Path = CPathlib::MakePathExt( CApp::GetModulePath(), L"nesromdb", L"dat" );

DEBUGOUT( "File:%s\n", Path.c_str() );

	m_DataBaseList.clear();

	if( (fp = _tfopen( Path.c_str(), _T("r") )) ) {
		while( fgetws( buf, 512, fp ) ) {
			if( buf[0] == _T(';') ) {	// コメントフィールドとみなす
				continue;
			}

			TCHAR*	pToken;

			// ALL CRC
			if( !(pToken = _tcstok( buf, seps )) )
				continue;
			db.crcall = _tcstoul( pToken, NULL, 16 );
			// PRG CRC
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.crc = _tcstoul( pToken, NULL, 16 );

			// Title
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.title = pToken;

			// Control 1
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.control1 = _ttoi( pToken );
			// Control 2
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.control2 = _ttoi( pToken );

			// PRG SIZE
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.prg_size = _ttoi( pToken );
			// CHR SIZE
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.chr_size = _ttoi( pToken );

			// Country
			if( !(pToken = _tcstok( NULL, seps )) )
				continue;
			db.country = pToken;

			db.bNTSC = TRUE;
			// Europe (PAL???)
			if( lstrcmp( pToken, L"E"   ) == 0
			 || lstrcmp( pToken, L"Fra" ) == 0
			 || lstrcmp( pToken, L"Ger" ) == 0
			 || lstrcmp( pToken, L"Spa" ) == 0
			 || lstrcmp( pToken, L"Swe" ) == 0
			 || lstrcmp( pToken, L"Ita" ) == 0
			 || lstrcmp( pToken, L"Aus" ) == 0 ) {
				db.bNTSC = FALSE;
			}

			// Manufacturer
			if( pToken = _tcstok( NULL, seps ) ) {
				db.manufacturer = pToken;
			} else {
				db.manufacturer.erase( db.manufacturer.begin(), db.manufacturer.end() );
			}

			// Sale date
			if( pToken = _tcstok( NULL, seps ) ) {
				db.saledate = pToken;
			} else {
				db.saledate.erase( db.saledate.begin(), db.saledate.end() );
			}

			// Price
			if( pToken = _tcstok( NULL, seps ) ) {
				db.price = pToken;
			} else {
				db.price.erase( db.price.begin(), db.price.end() );
			}

			// Genre
			if( pToken = _tcstok( NULL, seps ) ) {
				db.genre = pToken;
			} else {
				db.genre.erase( db.genre.begin(), db.genre.end() );
			}

			m_DataBaseList[db.crcall] = db;
		}
		FCLOSE( fp );
	} else {
DEBUGOUT( "Database file not found.\n" );
	}
}

