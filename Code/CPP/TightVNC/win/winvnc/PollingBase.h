
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: PollingBase.h 1891 2004-10-06 14:14:11Z const_k $

class PollingBase ;

#ifndef __POLLING_BASE_H
#define __POLLING_BASE_H

#include "vncServerSingleton.h"
#include "vncServer.h"

class PollingBase
{
public:
	PollingBase() : m_server( vncServerSingleton::GetInstance() ),
		m_desktop( NULL ) {} ;
	virtual ~PollingBase() {} ;

	virtual void PerformPolling( void ) = 0 ;
	virtual int GetCycleDivisor( void ) = 0 ;

protected:
	vncDesktop* m_desktop ;
	vncServer* m_server ;

private:
	PollingBase( const PollingBase& rhs ) ;
	const PollingBase& operator=( const PollingBase& rhs ) ;

} ;

#endif // __POLLING_BASE_H
