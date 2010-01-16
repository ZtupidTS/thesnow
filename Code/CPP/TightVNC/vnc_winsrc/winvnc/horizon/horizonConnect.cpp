
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonConnect.cpp 1892 2004-10-06 14:20:15Z const_k $

#include "horizonConnect.h"

// static instance
horizonConnect* horizonConnect::m_instance = NULL ;

//
// class implementation
//

horizonConnect* 
horizonConnect::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new horizonConnect() ;
	return m_instance ;
}

horizonConnect::horizonConnect()
{
}

horizonConnect::~horizonConnect()
{
}

bool
horizonConnect::Start( void )
{
	vncServer* server = vncServerSingleton::GetInstance() ;

	// return if we're already connected
	if ( server->AuthClientCount() != 0 )
		return true ;

	//
	// Connect out to the specified host on the VNCviewer listen port
	// To be really good, we should allow a display number here but
	// for now we'll just assume we're connecting to display zero
	//
	
	VSocket* client_socket = new VSocket() ;

	if ( 
		client_socket == NULL 
		|| client_socket->Create() == VFalse 
		|| client_socket->Connect( m_pref_ConnectionHostname.c_str(), m_pref_ConnectionPort ) == VFalse 
	)
	{
		// log the error
		vnclog.Print( LL_INTERR, VNCLOG( "unable to connect to server, hostname => %s, port %d\n" ), 
			m_pref_ConnectionHostname.c_str(), m_pref_ConnectionPort ) ;

		// report the error to the user
		MessageBox(NULL, 
			"AppShare was unable to connect to the server.\n"
			"Please contact Horizon Wimba for support.",
			szAppName,
			MB_OK | MB_ICONERROR
		) ;

		// clean up client_socket
		if ( client_socket != NULL )
		{
			delete client_socket ;
			client_socket = NULL ;
		}
		
		return false ;
	}						
	
	//
	// add the new client to this server
	//
	
	vncClientId client_id = server->AddClient( client_socket, TRUE, TRUE ) ;
	
	if ( client_id == -1 )
	{
		// log the error
		vnclog.Print( LL_INTERR, VNCLOG( "unable to connect to add client, hostname => %s, port %d\n" ), 
			m_pref_ConnectionHostname.c_str(), m_pref_ConnectionPort ) ;

		// report the error to the user
		MessageBox(NULL, 
			"AppShare was unable to begin sharing your computer.\n"
			"Please contact Horizon Wimba for support.",
			szAppName,
			MB_OK | MB_ICONERROR
		) ;

		// if there's an error, the socket is delete by AddClient()
		
		return false ;
	}
	
	return true ;
}

//
// accessors/mutators
//

void 
horizonConnect::SetConnectionInfo( const char* hostname, int port )
{
	m_pref_ConnectionHostname = hostname ;
	m_pref_ConnectionPort = port ;
	return ;
}

void 
horizonConnect::GetConnectionInfo( string& hostname, int& port )
{
	hostname = m_pref_ConnectionHostname ;
	port = m_pref_ConnectionPort ;
	return ;
}

