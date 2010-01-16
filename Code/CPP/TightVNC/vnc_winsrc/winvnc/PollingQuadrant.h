
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: PollingQuadrant.h 1891 2004-10-06 14:14:11Z const_k $

class PollingQuadrant ;

#ifndef __POLLING_QUADRANT_H
#define __POLLING_QUADRANT_H

#include "vncDesktop.h"
#include "PollingBase.h"

class PollingQuadrant : public PollingBase
{
public:
	PollingQuadrant( vncDesktop* desktop ) ;
	virtual ~PollingQuadrant() {} ;

	void PerformPolling( void ) ;
	int GetCycleDivisor( void ) { return 1 ; } ;

private:
	void PollWindow( HWND hwnd, RECT &poll_rect ) ;

	int m_pollingStep ;

} ;

#endif // __POLLING_QUADRANT_H
