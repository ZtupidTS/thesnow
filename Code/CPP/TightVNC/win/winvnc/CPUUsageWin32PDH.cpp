
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsageWin32PDH.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "CPUUsageWin32PDH.h"

//
// class implementation
//

CPUUsageWin32PDH::CPUUsageWin32PDH( const char* szProcessName )
	: m_pdh_functions( PDHFunctions::GetInstance() ), m_is_valid( true )
{
	StartStats( szProcessName ) ;
}

CPUUsageWin32PDH::~CPUUsageWin32PDH()
{
	StopStats() ;
}

bool 
CPUUsageWin32PDH::isValid( void ) 
{ 
	if ( m_is_valid == false )
		return false ;
		
	if ( m_pdh_functions->isValid() == false )
		return false ;
		
	return true ;
} ;

bool
CPUUsageWin32PDH::StartStats( const char* process_name )
{
	if ( isValid() == false )
		return false ;

	PDH_STATUS status = ( m_pdh_functions->PdhOpenQuery() )( NULL, NULL, &m_hQuery ) ;
	
	if ( CheckStatus( status ) == false )
		return false ;
	
	char szWildcardPath[2048] ;

	if ( process_name == NULL )
	{
		sprintf( szWildcardPath, "%s", "\\Processor(_TOTAL)\\% Processor Time" ) ;
	}
	else
	{
		sprintf( szWildcardPath, "\\Process(%s)\\%c Processor Time", process_name, '%' ) ;
	}

	char* szCounterPath = new char[2048] ;
	DWORD dwPathSize = 2048 ;

	status = ( m_pdh_functions->PdhExpandCounterPath() )(
		szWildcardPath,
		szCounterPath, 
		&dwPathSize
	) ;

	LPSTR ptr = szCounterPath ;
	
	while ( 
		status == ERROR_SUCCESS 
		&& *ptr != NULL
	)
	{
		// add counter path
		status = ( m_pdh_functions->PdhAddCounter() )( m_hQuery, ptr, NULL, &m_hCounter ) ;
		
		// move to next string in buffer
		ptr += strlen( ptr ) ;
	}
	
	delete [] szCounterPath ;

	if ( CheckStatus( status ) == false )
		return false ;
	
	return true ;
}

bool 
CPUUsageWin32PDH::GetCPUUsage( int& usage )
{
	if ( isValid() == false )
		return false ;

	PDH_STATUS status = ( m_pdh_functions->PdhCollectQueryData() )( m_hQuery ) ;

	if ( CheckStatus( status ) == false )
		return false ;

	PDH_FMT_COUNTERVALUE counter_value ;

	status = ( m_pdh_functions->PdhGetFormattedCounterValue() )( 
		m_hCounter, 
		PDH_FMT_LONG,
		NULL,
		&counter_value
	) ;
		
	if ( CheckStatus( status ) == false )
		return false ;

	// copy the return value
	usage = static_cast< int >( counter_value.longValue ) ;

	return true ;
}

bool
CPUUsageWin32PDH::StopStats( void )
{
	PDH_STATUS status = ( m_pdh_functions->PdhCloseQuery() )( m_hQuery ) ;
	return CheckStatus( status ) ;
}

bool 
CPUUsageWin32PDH::CheckStatus( PDH_STATUS status )
{
	switch ( status )
	{
		case ERROR_SUCCESS:
			return true ;
	
		// general
		
		case PDH_INVALID_ARGUMENT:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_INVALID_ARGUMENT\n" ) ) ;
			break ;

		case PDH_INVALID_HANDLE:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_INVALID_HANDLE\n" ) ) ;
			break ;

		// PdhAddCounter
	
		case PDH_CSTATUS_BAD_COUNTERNAME:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_CSTATUS_BAD_COUNTERNAME\n" ) ) ;
			break ;

		case PDH_CSTATUS_NO_COUNTER:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_CSTATUS_NO_COUNTER\n" ) ) ;
			break ;

		case PDH_CSTATUS_NO_COUNTERNAME:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_CSTATUS_NO_COUNTERNAME\n" ) ) ;
			break ;

		case PDH_CSTATUS_NO_MACHINE:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_CSTATUS_NO_MACHINE\n" ) ) ;
			break ;

		case PDH_CSTATUS_NO_OBJECT:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_CSTATUS_NO_OBJECT\n" ) ) ;
			break ;

		case PDH_FUNCTION_NOT_FOUND:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_FUNCTION_NOT_FOUND\n" ) ) ;
			break ;

		case PDH_MEMORY_ALLOCATION_FAILURE:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_MEMORY_ALLOCATION_FAILURE\n" ) ) ;
			break ;

		// PdhGetFormattedCounterValue

		case PDH_INVALID_DATA:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => PDH_MEMORY_ALLOCATION_FAILURE\n" ) ) ;
			break ;

		default:
			vnclog.Print( LL_INTERR, VNCLOG( "CPUUsageWin32PDH(), status => UNKNOWN\n" ) ) ;
			break ;
	}
	
	// mark object as invalid
	m_is_valid = false ;
	
	return false ;
}




