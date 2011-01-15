
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: CPUUsageWin9x.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "CPUUsageWin9x.h"

// static instance
CPUUsageWin9x* CPUUsageWin9x::m_instance = NULL ;

//
// class implementation
//

CPUUsageWin9x* 
CPUUsageWin9x::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new CPUUsageWin9x() ;
	return m_instance ;
}

CPUUsageWin9x::CPUUsageWin9x()
	: m_is_valid( false )
{
	m_is_valid = ( StartStats() == true )
		? true 
		: false
	;
}

CPUUsageWin9x::~CPUUsageWin9x()
{
	m_is_valid = false ;
	StopStats() ;
}

bool
CPUUsageWin9x::StartStats( void )
{
	DWORD rc ;
	HKEY hOpen ;

	rc = RegOpenKeyEx(
		HKEY_DYN_DATA,
		"PerfStats\\StartStat", 
		0,
		KEY_READ, 
		&hOpen
	) ;
	
	if ( rc != ERROR_SUCCESS )
		return false ;
	
	unsigned char* startValue = NULL ;	
	DWORD startSize ;

	// query to get data size
	rc = RegQueryValueEx(
		hOpen,
		"KERNEL\\CPUUsage",
		NULL,
		NULL,
		NULL, 
		&startSize 
	) ;
	
	if ( rc == ERROR_SUCCESS )
	{	
		startValue = new unsigned char[ startSize ] ;	
	
		rc = RegQueryValueEx(
			hOpen,
			"KERNEL\\CPUUsage",
			NULL,
			NULL, 
			startValue,
			&startSize 
		) ;
	
		delete [] startValue ;
	}
	
	RegCloseKey( hOpen ) ;

	m_is_valid = ( rc == ERROR_SUCCESS ) ;
	return m_is_valid ;
}

bool 
CPUUsageWin9x::GetCPUUsage( int& usage )
{
	if ( isValid() == false )
		return false ;

	DWORD rc ;
	HKEY hData ;

	DWORD dataValue = 0 ;
	DWORD dataSize = sizeof( DWORD ) ;
	
	rc = RegOpenKeyEx(
		HKEY_DYN_DATA,
		"PerfStats\\StatData", 
		0,
		KEY_READ, 
		&hData
	) ;
	
	if ( rc == ERROR_SUCCESS )
	{
		rc = RegQueryValueEx(
			hData,
			"KERNEL\\CPUUsage",
			NULL,
			NULL, 
			reinterpret_cast< unsigned char* >( &dataValue ),
			&dataSize 
		) ;
		
		usage = static_cast< int >( dataValue ) ;
	}
	
	RegCloseKey( hData ) ;

	m_is_valid = ( rc == ERROR_SUCCESS ) ;
	return m_is_valid ;
}

bool
CPUUsageWin9x::StopStats( void )
{
	DWORD rc ;
	HKEY hClose ;

	unsigned char* stopValue = NULL ;
	DWORD stopSize ;
	
	rc = RegOpenKeyEx(
		HKEY_DYN_DATA,
		"PerfStats\\StopStat", 
		0,
		KEY_READ, 
		&hClose
	) ;
	
	if ( rc != ERROR_SUCCESS )
		return false ;
	
	// query to get data size
	rc = RegQueryValueEx(
		hClose,
		"KERNEL\\CPUUsage",
		NULL,
		NULL,
		NULL, 
		&stopSize 
	) ;
	
	if ( rc == ERROR_SUCCESS )
	{	
		stopValue = new unsigned char[ stopSize ] ;
		
		rc = RegQueryValueEx(
			hClose,
			"KERNEL\\CPUUsage",
			NULL,
			NULL, 
			stopValue,
			&stopSize 
		) ;
					
		delete [] stopValue ;
	}
	
	RegCloseKey( hClose ) ;

	m_is_valid = false ;
	return true ;
}
