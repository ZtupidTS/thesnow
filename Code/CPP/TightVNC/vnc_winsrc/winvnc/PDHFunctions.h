
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: PDHFunctions.h 1891 2004-10-06 14:14:11Z const_k $

class PDHFunctions ;

#ifndef __PDF_FUNCTIONS_H
#define __PDF_FUNCTIONS_H

#pragma warning( disable : 4786 )  

#include <windows.h>
#include <pdh.h>

//
// class definition
//

class PDHFunctions
{
public: 
	// singleton accessor
	static PDHFunctions* GetInstance( void ) ;

	// true if functions loaded, false otherwise
	bool isValid( void ) { return m_is_valid ; } ;

	// function pointer typedefs
	typedef PDH_STATUS ( WINAPI* PdhOpenQuery_t )( LPVOID, DWORD, HQUERY ) ;
	typedef PDH_STATUS ( WINAPI* PdhExpandCounterPath_t )( LPCTSTR, TCHAR*, LPDWORD ) ;
	typedef PDH_STATUS ( WINAPI* PdhAddCounter_t )( HQUERY, LPCTSTR, DWORD, HCOUNTER ) ;
	typedef PDH_STATUS ( WINAPI* PdhCollectQueryData_t )( HQUERY ) ;
	typedef PDH_STATUS ( WINAPI* PdhGetFormattedCounterValue_t )( HCOUNTER, 
		DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE ) ;
	typedef PDH_STATUS ( WINAPI* PdhCloseQuery_t )( HQUERY ) ;

	// function pointer accessors
	PdhOpenQuery_t PdhOpenQuery( void ) ;
	PdhExpandCounterPath_t PdhExpandCounterPath( void ) ;
	PdhAddCounter_t PdhAddCounter( void ) ;
	PdhCollectQueryData_t PdhCollectQueryData( void ) ;
	PdhGetFormattedCounterValue_t PdhGetFormattedCounterValue( void ) ;
	PdhCloseQuery_t PdhCloseQuery( void ) ;

private:
	PDHFunctions() ;
	~PDHFunctions() ;

	PDHFunctions( const PDHFunctions& rhs ) ;
	const PDHFunctions& operator=( const PDHFunctions& rhs ) ;

	// initializer
	bool LoadFunctions( void ) ;

	// static instance
	static PDHFunctions* m_instance ;

	// handle to dll
	static HINSTANCE m_hInstPDH ;
	
	// true if functions loaded
	bool m_is_valid ;
	
	// map of loaded functions
	// typedef map< const char*, void* > PDHFunctionMap ;
	// static PDHFunctionMap m_pfm ;
	
	// function pointers
	FARPROC m_pPdhOpenQuery ;
	FARPROC m_pPdhExpandCounterPath ;
	FARPROC m_pPdhAddCounter ;
	FARPROC m_pPdhCollectQueryData ;
	FARPROC m_pPdhGetFormattedCounterValue ;
	FARPROC m_pPdhCloseQuery ;
	
} ;

#endif // __PDF_FUNCTIONS_H
