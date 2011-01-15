
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonPollingAdapter.h 1892 2004-10-06 14:20:15Z const_k $

class horizonPollingAdapter ;

#ifndef __HORIZON_POLLING_ADAPTER_H
#define __HORIZON_POLLING_ADAPTER_H

#include "vncDesktop.h"

#include "PollingScanLinesAdjacent.h"
#include "PollingScanLines.h"
#include "PollingQuadrant.h"
#include "PollingBase.h"

class horizonPollingAdapter
{
public:
	horizonPollingAdapter( vncDesktop* desktop ) ;
	~horizonPollingAdapter() ; 

	// adapter methods
	void PerformPolling( void ) ;
	int GetCycleDivisor( void ) ;

	// polling object accessor ( could be private )
	PollingBase* GetPollingObject( void ) ;
	
private:
	horizonPollingAdapter( const horizonPollingAdapter& rhs ) ;
	const horizonPollingAdapter& operator=( const horizonPollingAdapter& rhs ) ;

	// available polling classes
	PollingScanLinesAdjacent* m_scanlines_adjacent ;
	PollingScanLines* m_scanlines ;
	PollingQuadrant* m_quadrant ;
} ;

#endif // __HORIZON_POLLINGTYPE_H
