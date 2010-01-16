
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: PollingScanLinesAdjacent.h 1891 2004-10-06 14:14:11Z const_k $

class PollingScanLinesAdjacent ;

#ifndef __POLLING_SCANLINES_PLUS_H
#define __POLLING_SCANLINES_PLUS_H

#pragma warning( disable : 4786 )

#include <vector>
#include <string>
#include <utility>

#include "vncDesktop.h"
#include "PollingBase.h"

//
// class definition
//

class PollingScanLinesAdjacent : public PollingBase
{
public:
	PollingScanLinesAdjacent( vncDesktop* desktop ) ;
	virtual ~PollingScanLinesAdjacent() {} ;

	void PerformPolling( void ) ;
	int GetCycleDivisor( void ) { return 16 ; } ;

private:
	void PollWindow( HWND hwnd ) ;
	void PollArea( RECT &rect ) ;

	static const int m_pollingOrder[32] ;
	int m_pollingStep ;

	//
	// changed rectangles map stuff
	//
	
	typedef std::pair< char, RECT > ChangedRectangle ;
	typedef std::vector< ChangedRectangle > ChangedRectanglesRow ;
	typedef std::vector< ChangedRectanglesRow > ChangedRectanglesMap ;

	struct ChangedRectanglesKey {
		static enum {
			UNCHANGED = 0,
			NEIGHBOR,
			PROMOTE,
			CHANGED,			
		} ;
	} ; 
	
} ;

#endif // __POLLING_SCANLINES_H
