
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: vncServerSingleton.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "vncServerSingleton.h"
#include "vncServer.h"

vncServer* vncServerSingleton::m_server = 0 ;

vncServer*
vncServerSingleton::GetInstance( void ) 
{ 
	if ( m_server == NULL )
		m_server = new vncServer() ;
	return m_server ;
}
