
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsage.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "CPUUsage.h"

// static instance
CPUUsage* CPUUsage::m_instance = NULL ;

//
// class implementation
//

CPUUsage* 
CPUUsage::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new CPUUsage() ;
	return m_instance ;
}

CPUUsage::CPUUsage()
	: m_win9x_cpu( CPUUsageWin9x::GetInstance() ),
		m_system_cpu( new CPUUsageWin32PDH() ),
			m_system_usage( -1 ), 
			m_system_average( -1 ),
		m_process_cpu( new CPUUsageWin32PDH( szProcessName ) ),
			m_process_usage( -1 ), 
			m_process_average( -1 )
{
}

CPUUsage::~CPUUsage()
{
}

int 
CPUUsage::GetSystemCPU( void )
{
	if ( m_system_cpu->isValid() )
	{
		m_system_cpu->GetCPUUsage( m_system_usage ) ;
	}
	else if ( m_win9x_cpu->isValid() )
	{
		m_win9x_cpu->GetCPUUsage( m_system_usage ) ;
	}
	else
	{
		m_system_usage = -1 ;
	}
	
	// update moving average
	if ( 
		m_system_usage > 0 
		&& m_system_average > 0
	)
	{
		// compute the moving average
		m_system_average = ( m_system_usage + ( m_system_average * 9 ) ) / 10 ;
	}
	else
	{
		m_system_average = m_system_usage ;
	}
	
	return m_system_usage ;
}

int 
CPUUsage::GetProcessCPU( void )
{
	if ( m_process_cpu->isValid() )
	{
		m_process_cpu->GetCPUUsage( m_process_usage ) ;
	}
	else
	{
		m_process_usage = ( m_win9x_cpu->isValid() ) ? -95 : -1 ;
	}
	
	// update moving average
	if ( 
		m_process_usage > 0
		&& m_process_average > 0
	)
	{
		// compute the moving average
		m_process_average = ( m_process_usage + ( m_process_average * 9 ) ) / 10 ;
	}
	else
	{
		// use the usage as the average
		m_process_average = m_process_usage ;
	}

	return m_process_usage ;
}
