
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonBasicSettings.cpp 1971 2004-11-30 15:05:43Z jeffo3 $

#include "horizonBasicSettings.h"

//
// class implemtentation
//

bool
horizonBasicSettings::Show()
{
	// debugging checks
	assert( m_properties != NULL ) ;
	assert( m_server != NULL ) ;
	
	if ( m_dlgvisible == true )
		return true ;
	else
		m_dlgvisible = true ;

	CMatchWindow* matchwindow = m_properties->GetMatchWindow() ;
	
	if ( matchwindow == NULL )
		return false ;
		
	// show the match window
	matchwindow->Show() ;

	// set initial rect
	matchwindow->SetServerScreenArea() ;

	// start the update timer
	matchwindow->StartUpdateServerTimer() ;

	// update properties
	m_server->FullScreen( FALSE ) ;
	m_server->WindowShared( FALSE ) ;
	m_server->ScreenAreaShared( TRUE ) ;

	// pretend ok was clicked
	m_ok_clicked = true ;
	
	// increment times through
	++m_times_through ;
	
	return true ;
}
