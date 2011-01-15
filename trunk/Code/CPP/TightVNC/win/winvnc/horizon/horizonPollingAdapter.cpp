
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonPollingAdapter.cpp 1892 2004-10-06 14:20:15Z const_k $

#include "horizonPollingAdapter.h"

horizonPollingAdapter::horizonPollingAdapter( vncDesktop* desktop )
	: m_quadrant( new PollingQuadrant( desktop ) ),
		m_scanlines( new PollingScanLines( desktop ) ),
		m_scanlines_adjacent( new PollingScanLinesAdjacent( desktop ) )		
{
}

horizonPollingAdapter::~horizonPollingAdapter()
{
	if ( m_quadrant != NULL )
	{
		delete m_quadrant ;
		m_quadrant = NULL ;
	}

	if ( m_scanlines != NULL )
	{
		delete m_scanlines ;
		m_scanlines = NULL ;
	}

	if ( m_scanlines_adjacent != NULL )
	{
		delete m_scanlines ;
		m_scanlines = NULL ;
	}
}

void
horizonPollingAdapter::PerformPolling( void )
{
	GetPollingObject()->PerformPolling() ;
}

int 
horizonPollingAdapter::GetCycleDivisor( void )
{
	return GetPollingObject()->GetCycleDivisor() ;
}

PollingBase*
horizonPollingAdapter::GetPollingObject( void )
{
	PollingBase* pp = NULL ;	

	// pick polling algorithm 
	if ( horizonPollingType::isTypeScanLinesAdjacent() ) 
	{
		pp = dynamic_cast< PollingBase* >( m_scanlines_adjacent ) ;
	}
	else if ( horizonPollingType::isTypeScanLines() ) 
	{
		pp = dynamic_cast< PollingBase* >( m_scanlines ) ;
	}
	else 
	{
		pp = dynamic_cast< PollingBase* >( m_quadrant ) ;
	}
			
	return pp ;
}

