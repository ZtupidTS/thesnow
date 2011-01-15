
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: GracePeriod.h 1943 2004-11-15 15:03:47Z jeffo3 $

class GracePeriod ;

#ifndef __GRACE_PERIOD_H
#define __GRACE_PERIOD_H

#include "stdhdrs.h"
#include "omnithread.h"

#include <time.h>

//
// class definition
//

class GracePeriod
{
public:
	GracePeriod() ;
	~GracePeriod() ;

	void start( void ) { restart() ; return ; } ;
	void restart( void ) ;

	bool isPeriodExpired( void ) ;

	void setPeriod( unsigned int period ) ;

	int getPeriod( void ) { return m_period ; } ;
	time_t getStartTime( void ) { return m_starttime ; } ;

private:
	GracePeriod( const GracePeriod& rhs ) ;
	const GracePeriod& operator=( const GracePeriod& rhs ) ;
	
	int m_period ;

	time_t m_starttime ;
	time_t m_nowtime ;
	
	omni_mutex m_mutex ;
} ;

#endif // __GRACE_PERIOD_H
