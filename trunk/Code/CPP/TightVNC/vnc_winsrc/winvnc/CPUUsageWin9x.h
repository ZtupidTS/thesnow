
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsageWin9x.h 1891 2004-10-06 14:14:11Z const_k $

class CPUUsageWin9x ;

#ifndef __CPUUSAGE_WIN9X_H
#define __CPUUSAGE_WIN9X_H

#include <windows.h>

//
// class definition
//

class CPUUsageWin9x
{
public:
	static CPUUsageWin9x* GetInstance( void ) ;

	bool GetCPUUsage( int& usage ) ;
	bool isValid( void ) { return m_is_valid ; } ;

private:
	CPUUsageWin9x() ;
	~CPUUsageWin9x() ;

	CPUUsageWin9x( const CPUUsageWin9x& rhs ) ;
	const CPUUsageWin9x& operator=( const CPUUsageWin9x& rhs ) ;
	
	bool StartStats( void ) ;
	bool StopStats( void ) ;
	
	static CPUUsageWin9x* m_instance ;
	
	bool m_is_valid ;
};

#endif // __CPUUSAGE_WIN9X_H
