
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsage.h 1891 2004-10-06 14:14:11Z const_k $

class CPUUsage ;

#ifndef __CPUUSAGE_H
#define __CPUUSAGE_H

#include "WinVNC.h"

#include "CPUUsageWin32PDH.h"
#include "CPUUsageWin9x.h"

class CPUUsage
{
public:
	// static instance accessor
	static CPUUsage* GetInstance( void ) ;

	// system
	int GetSystemCPU( void ) ;
	int GetSystemAverageCPU( void ) { return m_system_average ; } ;

	// process
	int GetProcessCPU( void ) ;
	int GetProcessAverageCPU( void ) { return m_process_average ; } ;

private:
	CPUUsage() ;
	~CPUUsage() ;
	
	CPUUsage( const CPUUsage& rhs ) ;
	const CPUUsage& operator=( const CPUUsage& rhs ) ;

	CPUUsageWin32PDH* m_system_cpu ;
	int m_system_usage ;
	int m_system_average ;

	CPUUsageWin9x* m_win9x_cpu ;

	CPUUsageWin32PDH* m_process_cpu ;
	int m_process_usage ; 
	int m_process_average ;

	static CPUUsage* m_instance ;
} ;

#endif // __CPUUSAGE_H
