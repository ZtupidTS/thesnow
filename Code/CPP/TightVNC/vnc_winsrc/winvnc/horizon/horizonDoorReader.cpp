
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonDoorReader.cpp 2009 2005-05-05 19:01:23Z jeffo3 $

#include "horizonDoorReader.h"

//
// static members
//

horizonDoorReader* horizonDoorReader::m_instance = 0 ;

//
// class methods
//

void
horizonDoorReader::Start( void )
{
	if ( m_instance == 0 )
	{
		m_instance = new horizonDoorReader() ;
		m_instance->start_undetached() ;
	}
	return ;
}

void
horizonDoorReader::Stop( void )
{
	if ( m_instance != 0 )
	{
		m_instance->exit() ;
		m_instance = 0 ;
	}
	return ;
}


horizonDoorReader::horizonDoorReader() 
	: omni_thread()
{ 
}

horizonDoorReader::~horizonDoorReader() 
{
}

void* 
horizonDoorReader::run_undetached( void* arg )
{
	char line[256] ;
	
	while ( std::cin.good() )
	{
		if ( std::cin.peek() == 0 )
		{
			Sleep( 100 ) ; // sleep a bit
			continue ;
		}
	
		std::cin.getline( line, 256 ) ;
		// std::cerr << line << std::endl << std::flush ; 
	}

	return 0 ;
}

