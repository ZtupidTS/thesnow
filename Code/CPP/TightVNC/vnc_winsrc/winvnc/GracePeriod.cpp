
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: GracePeriod.cpp 1943 2004-11-15 15:03:47Z jeffo3 $

#include "GracePeriod.h"

GracePeriod::GracePeriod()
	: m_period( 0 ), m_starttime( 0 ) 
{
}

GracePeriod::~GracePeriod()
{
}

void 
GracePeriod::restart( void )
{
	omni_mutex_lock l( m_mutex ) ;

	// set the start time
	time( &m_starttime ) ;

	return ;
}

bool 
GracePeriod::isPeriodExpired()
{
	// return true it there's no start time
	if ( m_starttime == 0 )
		return true ;

	omni_mutex_lock l( m_mutex ) ;
 
	// get the current time
	time( &m_nowtime ) ;	

	// check if grace period has expired
	if ( m_nowtime - m_period < m_starttime )
	{
		// grace period has not expired
		return false ;
	}
	
	// clear the start time
	m_starttime = 0 ;

	return true ;
}

void 
GracePeriod::setPeriod( unsigned int period )
{
	omni_mutex_lock l( m_mutex ) ;

	// save the new period
	m_period = period ;	

	return ;
}
