
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: vncServerSingleton.h 1891 2004-10-06 14:14:11Z const_k $

class vncServerSingleton ;

#ifndef __VNCSERVER_SINGLETON_H
#define __VNCSERVER_SINGLETON_H

class vncServer ;

class vncServerSingleton
{
public:
	static vncServer* GetInstance( void ) ;

private:
	vncServerSingleton() {} ;
	~vncServerSingleton() {} ;

	vncServerSingleton( const vncServerSingleton& rhs ) ;
	const vncServerSingleton& operator=( const vncServerSingleton& rhs ) ;

	static vncServer* m_server ;
} ;

#endif // __VNCSERVER_SINGLETON_H
