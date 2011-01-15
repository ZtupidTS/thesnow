
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonDoorReader.h 2009 2005-05-05 19:01:23Z jeffo3 $

#ifndef __HORIZON_DOOR_READER_H
#define __HORIZON_DOOR_READER_H

//
// includes
//

#include "omnithread.h"
#include "vncServerSingleton.h"

#include <iostream>

//
// class declaration
//

class horizonDoorReader : protected omni_thread
{
public:
	static void Start( void ) ;
	static void Stop( void ) ;
	
private:
	// private c'tor, d'tor
	horizonDoorReader() ;
	~horizonDoorReader() ;

	// copy c'tor, assignment operator
	horizonDoorReader( const horizonDoorReader& rhs ) ;
	const horizonDoorReader& operator=( const horizonDoorReader& rhs ) ;

	// private methods
	void* run_undetached( void* arg ) ;

	static horizonDoorReader* m_instance ;
} ;


#endif // __HORIZON_DOOR_READER_H
