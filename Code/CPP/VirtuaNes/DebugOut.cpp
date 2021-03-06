//
// Debug output
//
#include "DebugOut.h"

CDebugOut	Dbg;

static const WCHAR szClassName[] = L"DebugWindow_wndclass";

CDebugOut::CDebugOut()
{
#if	defined(_DEBUG) || defined(_DEBUGOUT)
	hWndDebugOutput = ::FindWindow( szClassName, NULL );
	if( !hWndDebugOutput ) {
		::OutputDebugString( L"DebugWindow がありません\n" );
	}
#endif
}


void CDebugOut::Clear()
{
#if	defined(_DEBUG) || defined(_DEBUGOUT)
	if( hWndDebugOutput ) {
		if( ::IsWindow( hWndDebugOutput ) ) {
			::SendMessage( hWndDebugOutput, WM_APP+1, (WPARAM)NULL, (LPARAM)NULL );
		}
	}
#endif
}

void __cdecl CDebugOut::Out( LPWSTR fmt, ... )
{
#if	defined(_DEBUG) || defined(_DEBUGOUT)
	WCHAR	buf[1000];
	va_list	va;
	va_start( va, fmt );
	::vswprintf( buf, fmt, va );

	if( hWndDebugOutput ) {
		if( ::IsWindow( hWndDebugOutput ) ) {
			COPYDATASTRUCT	cds;
			cds.dwData = 0;
			cds.lpData = (void*)buf;
			cds.cbData = ::wcslen(buf)+1; //  終端のNULLも送る
			//  文字列送信
			::SendMessage( hWndDebugOutput, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds );
		} else {
			::OutputDebugString( buf );
		}
	} else {
		::OutputDebugString( buf );
	}
#endif
}

void __cdecl CDebugOut::Out( LPSTR fmt, ... )
{
#if	defined(_DEBUG) || defined(_DEBUGOUT)
	CHAR	buf[1000];
	va_list	va;
	va_start( va, fmt );
	::vsprintf( buf, fmt, va );

	if( hWndDebugOutput ) {
		if( ::IsWindow( hWndDebugOutput ) ) {
			COPYDATASTRUCT	cds;
			cds.dwData = 0;
			cds.lpData = (void*)buf;
			cds.cbData = ::strlen(buf)+1; //  終端のNULLも送る
			//  文字列送信
			::SendMessage( hWndDebugOutput, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds );
		} else {
			::OutputDebugStringA( buf );
		}
	} else {
		::OutputDebugStringA( buf );
	}
#endif
}

void CDebugOut::Out( const wstring& str )
{
#if	defined(_DEBUG) || defined(_DEBUGOUT)
	Out( (LPWSTR)str.c_str() );
#endif
}

void CDebugOut::Out( const string& str )
{
#if	defined(_DEBUG) || defined(_DEBUGOUT)
	Out( (LPSTR)str.c_str() );
#endif
}