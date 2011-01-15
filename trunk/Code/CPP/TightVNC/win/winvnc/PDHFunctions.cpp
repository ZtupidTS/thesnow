
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: PDHFunctions.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "PDHFunctions.h"

// static instance
PDHFunctions* PDHFunctions::m_instance = NULL ;

// static members
HINSTANCE PDHFunctions::m_hInstPDH = NULL ;

//
// class implementation
//

PDHFunctions* 
PDHFunctions::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new PDHFunctions() ;
	return m_instance ;
}

PDHFunctions::PDHFunctions() 
{
	// inititialize function pointers
	m_pPdhOpenQuery = NULL ;
	m_pPdhExpandCounterPath = NULL ;
	m_pPdhAddCounter = NULL ;
	m_pPdhCollectQueryData = NULL ;
	m_pPdhGetFormattedCounterValue = NULL ;
	m_pPdhCloseQuery = NULL ;

	m_is_valid = ( LoadFunctions() == true ) ;
}

PDHFunctions::~PDHFunctions() 
{ 
	FreeLibrary( m_hInstPDH ) ;
}

bool 
PDHFunctions::LoadFunctions( void )
{	
	m_hInstPDH = LoadLibrary( "pdh.dll" ) ;
	
	if ( m_hInstPDH == NULL )
		return false ;
	
	m_pPdhOpenQuery = GetProcAddress( m_hInstPDH, "PdhOpenQuery" ) ;
	if ( m_pPdhOpenQuery == NULL ) return false ;
	
	m_pPdhExpandCounterPath = GetProcAddress( m_hInstPDH, "PdhExpandCounterPathA" ) ;
	if ( m_pPdhExpandCounterPath == NULL ) return false ;
	
	m_pPdhAddCounter = GetProcAddress( m_hInstPDH, "PdhAddCounterA" ) ;
	if ( m_pPdhAddCounter == NULL ) return false ;
	
	m_pPdhCollectQueryData = GetProcAddress( m_hInstPDH, "PdhCollectQueryData" ) ;
	if ( m_pPdhCollectQueryData == NULL ) return false ;
	
	m_pPdhGetFormattedCounterValue = GetProcAddress( m_hInstPDH, "PdhGetFormattedCounterValue" ) ;
	if ( m_pPdhGetFormattedCounterValue == NULL ) return false ;
	
	m_pPdhCloseQuery = GetProcAddress( m_hInstPDH, "PdhCloseQuery" ) ;
	if ( m_pPdhCloseQuery == NULL ) return false ;

	// mark instance as valid
	m_is_valid = true ;

	return true ;
}

//
// accessors
//

PDHFunctions::PdhOpenQuery_t
PDHFunctions::PdhOpenQuery( void ) 
{
	return reinterpret_cast< PdhOpenQuery_t >( m_pPdhOpenQuery ) ;
}

PDHFunctions::PdhExpandCounterPath_t
PDHFunctions::PdhExpandCounterPath( void )
{
	return reinterpret_cast< PdhExpandCounterPath_t >( m_pPdhExpandCounterPath ) ;
}

PDHFunctions::PdhAddCounter_t
PDHFunctions::PdhAddCounter( void )
{
	return reinterpret_cast< PdhAddCounter_t >( m_pPdhAddCounter ) ;
}

PDHFunctions::PdhCollectQueryData_t
PDHFunctions::PdhCollectQueryData( void )
{
	return reinterpret_cast< PdhCollectQueryData_t >( m_pPdhCollectQueryData ) ;
}

PDHFunctions::PdhGetFormattedCounterValue_t
PDHFunctions::PdhGetFormattedCounterValue( void )
{
	return reinterpret_cast< PdhGetFormattedCounterValue_t >( m_pPdhGetFormattedCounterValue ) ;
}

PDHFunctions::PdhCloseQuery_t
PDHFunctions::PdhCloseQuery( void )
{
	return reinterpret_cast< PdhCloseQuery_t >( m_pPdhCloseQuery ) ;
}
