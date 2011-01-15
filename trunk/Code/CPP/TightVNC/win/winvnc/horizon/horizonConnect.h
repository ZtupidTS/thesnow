
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonConnect.h 1892 2004-10-06 14:20:15Z const_k $

class horizonConnect ;

#ifndef __HORIZON_CONNECT_H
#define __HORIZON_CONNECT_H

#include "stdhdrs.h"

#include <string>
using std::string ;

#include "VSocket.h" 

#include "vncServer.h"
#include "vncServerSingleton.h"

#include "horizonMain.h"

//
// class definition
//

class horizonConnect
{
public:
	static horizonConnect* GetInstance( void ) ;

	// add self as client to server
	bool Start( void ) ;

	// connection info accessors/mutators	
	void SetConnectionInfo( const char* hostname, int port ) ;
	void GetConnectionInfo( string& hostname, int& port ) ;

private:
	horizonConnect() ;
	~horizonConnect() ;

	horizonConnect( const horizonConnect& rhs ) ;
	const horizonConnect& operator=( const horizonConnect& rhs ) ;
	
	// connection info
	string m_pref_ConnectionHostname ;
	int m_pref_ConnectionPort ;

	static horizonConnect* m_instance ;
};

#endif // __HORIZON_CONNECT_H
