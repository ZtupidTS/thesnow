
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonDoorWriter.h 2009 2005-05-05 19:01:23Z jeffo3 $

#ifndef __HORIZON_DOOR_WRITER_H
#define __HORIZON_DOOR_WRITER_H

//
// includes
//

#include "omnithread.h"
#include "vncServerSingleton.h"

#include <deque>
#include <iostream>
#include <string>

//
// class declaration
//

class horizonDoorWriter : protected omni_thread
{
public:
	static void Start( void ) ;
	static void Stop( void ) ;

	static void ReportFatalError( const std::string& msg ) ;
	static void Report( const std::string& msg ) ;
	
private:
	// private c'tor, d'tor
	horizonDoorWriter() ;
	~horizonDoorWriter() ;

	// copy c'tor, assignment operator
	horizonDoorWriter( const horizonDoorWriter& rhs ) ;
	const horizonDoorWriter& operator=( const horizonDoorWriter& rhs ) ;

	// private methods
	void* run_undetached( void* arg ) ;

	// singleton instance
	static horizonDoorWriter* m_instance ;

	// messages deque, mutex
	static std::deque< std::string > m_messages ;
	static omni_mutex m_messages_lock ;

} ;

#endif // __HORIZON_DOOR_WRITER_H

