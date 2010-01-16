
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonPollingType.h 1892 2004-10-06 14:20:15Z const_k $

// class horizonPolling ;

#ifndef __HORIZON_POLLINGTYPE_H
#define __HORIZON_POLLINGTYPE_H

class horizonPollingType
{
public:
	enum PollingType {
		SCANLINES = 0,
		SCANLINES_ADJACENT,
		QUADARANT
	} ;

	// polling type accessor/mutator
	static void SetPollingType( PollingType type ) { m_type = type ; } ;
	static PollingType GetPollingType( void ) { return m_type ; } ;

	static bool isTypeScanLinesAdjacent( void ) { return ( m_type == SCANLINES_ADJACENT ) ; } ;
	static bool isTypeScanLines( void ) { return ( m_type == SCANLINES ) ; } ;
	static bool isTypeQuadrant( void ) { return ( m_type == QUADARANT ) ; } ;

private:
	horizonPollingType() {} ;
	~horizonPollingType() {} ; 
	horizonPollingType( const horizonPollingType& rhs ) ;
	const horizonPollingType& operator=( const horizonPollingType& rhs ) ;

	static PollingType m_type ;
} ;

#endif // __HORIZON_POLLINGTYPE_H
