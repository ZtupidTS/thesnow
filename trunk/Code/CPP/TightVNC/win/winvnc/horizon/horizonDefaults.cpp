
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonDefaults.cpp 1958 2004-11-16 22:29:41Z jeffo3 $

#include "horizonDefaults.h"

extern const char* szAppName ;

//
// class implementation
//

bool
horizonDefaults::InitializeApplication( void )
{
	//
	// server defaults
	//

	vncServer* server = vncServerSingleton::GetInstance() ;

	// set server name
	server->SetName( szAppName ) ;

	// force loopback connections
	server->SetLoopbackOk( TRUE ) ;
	server->SetLoopbackOnly( TRUE ) ;
	
	// disable authentication
	server->SetAuthRequired( FALSE ) ;

	// input priority
	server->LocalInputPriority( TRUE ) ;
	
	// external authentication
	server->EnableExternalAuth( FALSE ) ;
	server->SockConnect( FALSE ) ;

	// polling
	server->PollFullScreen( TRUE ) ;
	server->PollForeground( TRUE ) ;
	server->PollUnderCursor( FALSE ) ;
	server->PollConsoleOnly( TRUE ) ;
	server->PollOnEventOnly( FALSE ) ;

	server->DontSetHooks( TRUE ) ;
	server->DontUseDriver( TRUE ) ;

	server->SetPollingCycle( 300 ) ;

	// shared region type
	server->FullScreen( FALSE ) ;
	server->WindowShared( TRUE ) ;
	server->ScreenAreaShared( FALSE ) ;
	server->SetApplication( FALSE ) ;

	//
	// connection defaults
	//

	horizonConnect* connect = horizonConnect::GetInstance() ;
	connect->SetConnectionInfo( "127.0.0.1", 5500 ) ;

	return true ;
}

