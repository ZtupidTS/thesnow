
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsageWin32HRPC.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "CPUUsageWin32HRPC.h"

//
// class implementation
//

CPUUsageWin32HRPC::CPUUsageWin32HRPC( int ms )
	: m_is_valid( true ), m_usage( 0 ), m_usage_average( 0 )
{
	SetPollCycle( ms ) ;
}

CPUUsageWin32HRPC::~CPUUsageWin32HRPC()
{
}

bool 
CPUUsageWin32HRPC::SetPollCycle( unsigned int ms_per_cycle )
{
	LARGE_INTEGER ticks_per_second ;

	// get performance frequency, on error mark object as invalid
	if ( QueryPerformanceFrequency( &ticks_per_second ) == FALSE )
	{
		m_is_valid = false ;
		return false ;
	}

	// ( milliseconds-per-interval * ticks-per-second ) / milliseconds-per-seconds = ticks-per-interval
	m_expected_delta = ( ms_per_cycle * ticks_per_second.QuadPart ) / 1000 ;

	// zero counters
	m_start_counter.QuadPart = 0L ;
	m_stop_counter.QuadPart = 0L ;
	
	return true ;
}

bool
CPUUsageWin32HRPC::MarkPollStart( void )
{
	if ( m_is_valid == false )
		return false ;

	// get the current counter
	if ( QueryPerformanceCounter( &m_start_counter ) == FALSE )
	{
		m_is_valid = false ;
		return false ;
	}
	
	// reset stop counter
	m_stop_counter.QuadPart = 0L ;

	return true ;
}

bool
CPUUsageWin32HRPC::MarkPollStop( void )
{
	if ( m_is_valid == false )
		return false ;

	// get the current counter
	if ( QueryPerformanceCounter( &m_stop_counter ) == FALSE )
	{
		m_is_valid = false ;
		return false ;
	}
	
	return CalculateCPUUsage() ;
}

bool 
CPUUsageWin32HRPC::CalculateCPUUsage( void )
{
	// less than two calls to MarkInterval()
	if ( 
		m_start_counter.QuadPart == 0 
		|| m_stop_counter.QuadPart == 0 
	)
	{
		return false ;
	}

	LONGLONG counter_delta = m_stop_counter.QuadPart - m_start_counter.QuadPart ;

	if ( m_expected_delta > counter_delta  )
	{
		double counter_ratio = static_cast< double >( counter_delta ) / static_cast< double >( m_expected_delta ) ;
		m_usage = static_cast< int >( counter_ratio * 100.0 ) ;
	}
	else
	{
		m_usage = 100 ;
	}

	// calculate average usage
	if ( m_usage_average > 0 )
		m_usage_average = ( m_usage + ( m_usage_average * 9 ) ) / 10 ;
	else
		m_usage_average = m_usage ;

	return true ;

}

