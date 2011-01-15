
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonDoorWriter.cpp 2009 2005-05-05 19:01:23Z jeffo3 $

#pragma warning( disable : 4786 )

#include "horizonDoorWriter.h"

//
// static members
//

// messages deque, mutex
std::deque< std::string > horizonDoorWriter::m_messages ;
omni_mutex horizonDoorWriter::m_messages_lock ;

horizonDoorWriter* horizonDoorWriter::m_instance = 0 ;

//
// class methods
//

// setup / shutdown methods

void
horizonDoorWriter::Start( void )
{
	if ( m_instance == 0 )
	{
		m_instance = new horizonDoorWriter() ;
		m_instance->start_undetached() ;
	}
	return ;
}

void
horizonDoorWriter::Stop( void )
{
	if ( m_instance != 0 )
	{
		m_instance->exit() ;
		m_instance = 0 ;
	}
	return ;
}

// reporting methods

void 
horizonDoorWriter::ReportFatalError( const std::string& msg )
{
	std::string s ;
	
	// construct the message
	s.assign( "appshare_fatal_error\t" ).append( msg ).append( "\n" ) ;
	
	// report the message
	Report( s ) ;	
	
	return ;
}

void 
horizonDoorWriter::Report( const std::string& msg )
{
	if ( m_instance == 0 )
		Start() ;

	// acquire messages lock
	omni_mutex_lock l( m_messages_lock ) ;

	// add message to queue
	m_messages.push_back( msg ) ;

	return ;
}

// c'tor, d'tor

horizonDoorWriter::horizonDoorWriter() 
	: omni_thread()
{ 
}

horizonDoorWriter::~horizonDoorWriter() 
{
}

// thread methods

void* 
horizonDoorWriter::run_undetached( void* arg )
{	
	std::deque< std::string >::const_iterator iter ;

	while ( 42 )
	{
		if ( m_messages.size() == 0 )
		{
			Sleep( 100 ) ; // sleep a bit
			continue ;
		}
		else	
		{
			// acquire messages lock
			omni_mutex_lock l( m_messages_lock ) ;
	
			// add message to queue
			iter = m_messages.begin() ;
		
			// report message
			std::cout << (*iter).c_str() << std::endl ;
		
			// remove message from the queue
			m_messages.pop_front() ;
		}
	}

	return 0 ;
}

