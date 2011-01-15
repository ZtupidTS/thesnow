
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonProperties.cpp 1971 2004-11-30 15:05:43Z jeffo3 $

#include "horizonProperties.h"

// static instance
horizonProperties* horizonProperties::m_instance = NULL ;

//
// class implementation
//

horizonProperties* 
horizonProperties::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new horizonProperties() ;
	return m_instance ;
}

horizonProperties::horizonProperties()
	: m_matchwindow( NULL )
{
}

horizonProperties::~horizonProperties()
{
	// clean up match window
	if ( m_matchwindow == NULL )
	{
		delete m_matchwindow ;
		m_matchwindow = NULL ;
	}
}

//
// display dialog boxes
//

bool
horizonProperties::ShowBasic( void )
{
	//
	// display the settings dialog box
	// ( delegated to helper classses )
	//

	horizonBasicSettings settings ;
	
	// run dialog
	if ( settings.Show() == false )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to open settings dialog\n" ) ) ;

		MessageBox(
			NULL,
			"Unable to open settings dialog.\nAppShare will quit.",
			szAppName,
			MB_OK | MB_ICONERROR
		) ;

		PostQuitMessage(0) ;
		return false ;
	}

	// if cancel was clicked
	if ( settings.wasOKClicked() == false )
	{
		//
		// if this is the first time through
		// quit the application
		//
		if ( settings.wasFirstTimeThrough() == true )
			PostQuitMessage(0) ;
		
		return false ;		
	}

	// make client connection
	horizonConnect::GetInstance()->Start() ;
	
	// start the cpu <-> polling updates
	PollCycleControl::GetInstance()->Start() ;

	return true ;
}

bool
horizonProperties::ShowNormal( void )
{
	//
	// display the settings dialog box
	// ( delegated to helper classses )
	//

	horizonNormalSettings settings ;
	
	// run dialog
	if ( settings.Show() == false )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to open settings dialog\n" ) ) ;

		MessageBox(
			NULL,
			"Unable to open settings dialog.\nAppShare will quit.",
			szAppName,
			MB_OK | MB_ICONERROR
		) ;

		PostQuitMessage(0) ;
		return false ;
	}

	// if cancel was clicked
	if ( settings.wasOKClicked() == false )
	{
		//
		// if this is the first time through
		// quit the application
		//
		if ( settings.wasFirstTimeThrough() == true )
			PostQuitMessage(0) ;
		
		return false ;		
	}

	// make client connection, if necessary
	horizonConnect::GetInstance()->Start() ;

	// start the cpu <-> polling updates
	PollCycleControl::GetInstance()->Start() ;

	return true ;
}

bool
horizonProperties::ShowAdvanced( void ) 
{
	//
	// display the settings dialog box
	// ( delegated to helper classses )
	//
	
	// create new instance
	horizonAdvancedSettings settings ;
	
	// run dialog
	if ( settings.Show() == false )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to open advanced settings dialog\n" ) ) ;

		MessageBox(
			NULL,
			"Unable to open advanced settings dialog.\nAppShare will quit.",
			szAppName,
			MB_OK | MB_ICONERROR
		) ;

		return false ;
	}
		
	return true ;
}

CMatchWindow* 
horizonProperties::GetMatchWindow( bool reset_window )
{
	if ( m_matchwindow == NULL )
	{
		m_matchwindow = new CMatchWindow( 
			vncServerSingleton::GetInstance(),
			0, 0, 0, 0 // don't worry about the defaults here
		) ;
		
		m_matchwindow->CanModify( TRUE ) ;
	
		// make sure to set the default bounds
		reset_window = true ;
	}
	
	if ( reset_window == true )
	{
		RECT rect ;
		GetWindowRect( GetDesktopWindow(), &rect ) ;

		m_matchwindow->ModifyPosition(
			rect.left + 5, 
			rect.top + 5,
			rect.right / 2, 
			rect.bottom / 2
		) ;
	}
	
	return m_matchwindow ;
}

