// HttpCode.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "HttpCode.h"

DWORD fnHttpCode(LPCTSTR SITE,LPCTSTR URL=L""){
	DWORD dwSize = sizeof(DWORD);
	DWORD dwStatusCode = 0;
	BOOL  bResults = FALSE;
	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen( L"A WinHTTP Example Program/1.0", 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS,
		0 );

	// Specify an HTTP server.
	if( hSession )
		hConnect = WinHttpConnect( hSession,
		SITE,
		INTERNET_DEFAULT_HTTP_PORT,
		0 );
	// Create an HTTP Request handle.
	if( hConnect )
		hRequest = WinHttpOpenRequest( hConnect,
		L"GET",
		URL, 
		NULL,
		WINHTTP_NO_REFERER, 
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0 );

	// Add a request header.
	if( hRequest )
		bResults = WinHttpAddRequestHeaders( hRequest, 
		L"If-Modified-Since: Mon, 20 Nov 2000 20:00:00 GMT",
		-1,
		WINHTTP_ADDREQ_FLAG_ADD );

	// Send a Request.
	if( bResults ) 
		bResults = WinHttpSendRequest( hRequest, 
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0, 
		0,
		0 );

	// End the request.
	if( bResults )
		bResults = WinHttpReceiveResponse( hRequest, NULL);

	// Use WinHttpQueryHeaders to obtain the header buffer.
	if( bResults )
		bResults = WinHttpQueryHeaders( hRequest, 
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		NULL, 
		&dwStatusCode,
		&dwSize,
		WINHTTP_NO_HEADER_INDEX );

	// Based on the status code, determine whether 
	// the document was recently updated.
	if( bResults )
	{
/*
		if( dwStatusCode == 304 ) 
			printf( "Document has not been updated.\n" );
		else if( dwStatusCode == 200 ) 
			printf( "Document has been updated.\n" );
		else 
			printf( "Status code = %u.\n",dwStatusCode );
*/
	return dwStatusCode;
	}

	// Report any errors.
	if( !bResults )
//		printf( "Error %d has occurred.\n", GetLastError( ) );
		return 0;
	// Close open handles.
	if( hRequest ) WinHttpCloseHandle( hRequest );
	if( hConnect ) WinHttpCloseHandle( hConnect );
	if( hSession ) WinHttpCloseHandle( hSession );
	return dwStatusCode;
}