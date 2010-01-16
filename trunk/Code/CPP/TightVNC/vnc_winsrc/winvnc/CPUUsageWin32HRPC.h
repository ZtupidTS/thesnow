
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsageWin32HRPC.h 1891 2004-10-06 14:14:11Z const_k $

class CPUUsageWin32HRPC ;

#ifndef __POLLING_CPU_USAGE_H
#define __POLLING_CPU_USAGE_H

//
// include files
//

#include <windows.h>

//
// class definition
//

class CPUUsageWin32HRPC
{
public:
	CPUUsageWin32HRPC( int ms = 0 ) ;
	~CPUUsageWin32HRPC() ;

	// set expected interval in milliseconds
	bool SetPollCycle( unsigned int ms_per_cycle )  ;

	// is the usage valid?
	bool isValid( void ) { return m_is_valid ; } ; 

	// make the next interval
	bool MarkPollStart( void ) ;	
	bool MarkPollStop( void ) ;	

	// get the usage
	int GetCPUUsage( void ) { return m_usage ; } ;
	int GetCPUAverage( void ) { return m_usage_average ; } ;

private:
	CPUUsageWin32HRPC( const CPUUsageWin32HRPC& rhs ) ;
	const CPUUsageWin32HRPC& operator=( const CPUUsageWin32HRPC& rhs ) ;
	
	bool CalculateCPUUsage( void ) ;

	LARGE_INTEGER m_start_counter ;
	LARGE_INTEGER m_stop_counter ;
	
	// expected ticks-per-interval
	LONGLONG m_expected_delta ;
	
	int m_usage ;
	int m_usage_average ;
	
	bool m_is_valid ;
} ;

#endif // __POLLING_CPU_USAGE_H
