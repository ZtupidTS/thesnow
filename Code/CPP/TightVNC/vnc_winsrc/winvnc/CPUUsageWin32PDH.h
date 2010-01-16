
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsageWin32PDH.h 1891 2004-10-06 14:14:11Z const_k $

class CPUUsageWin32PDH ;

#ifndef __CPUUSAGE_WINPDH_H
#define __CPUUSAGE_WINPDH_H

#include "stdhdrs.h"

#include <windows.h>

#include <pdh.h>
#include <pdhmsg.h>

#include <stdio.h>

#include "PDHFunctions.h"

//
// class definition
//

class CPUUsageWin32PDH
{
public:
	CPUUsageWin32PDH( const char* process_name = NULL ) ;
	~CPUUsageWin32PDH() ;

	bool GetCPUUsage( int& usage ) ;
	bool isValid( void ) ;

private:

	CPUUsageWin32PDH( const CPUUsageWin32PDH& rhs ) ;
	const CPUUsageWin32PDH& operator=( const CPUUsageWin32PDH& rhs ) ;
	
	bool StartStats( const char* instance_name = NULL ) ;
	bool StopStats( void ) ;

	bool CheckStatus( PDH_STATUS status ) ;

	// holds function pointers
	PDHFunctions* m_pdh_functions ;
	
	HQUERY m_hQuery ;
	HCOUNTER m_hCounter ;
	
	bool m_is_valid ;
} ;


#endif // __CPUUSAGE_WIN9X_H
