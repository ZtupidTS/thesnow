
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: PollingQuadrant.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "PollingQuadrant.h"

//
// class implementation
//

PollingQuadrant::PollingQuadrant( vncDesktop* desktop ) 
	: PollingBase(), m_pollingStep( 0 ) 
{
	m_desktop = desktop ;
}


void
PollingQuadrant::PerformPolling( void ) 
{
	// calculate a quarter of the shared rect
	RECT quadrant = m_server->GetSharedRect() ;
	quadrant.right = quadrant.right / 2 ;
	quadrant.bottom = quadrant.bottom / 2 ;

	//
	// polling step			=> [ 0, 1, 2, 3 ]
	// polling step % 2		=> [ 0, 1, 0, 1 ]
	// polling step / 2		=> [ 0, 0, 1, 1 ]
	//

	RECT poll_rect ;
	
	// left is ( 0 or 0.5 times q.right )
	poll_rect.left = ( m_pollingStep % 2 ) * quadrant.right ;
	
	// right is ( ( 0 or 0.5 times q.right ) + q.right )
	poll_rect.right = poll_rect.left + quadrant.right ;

	// top is ( 0 or 1 times q.bottom )
	poll_rect.top = ( m_pollingStep / 2) * quadrant.bottom ;
	
	// bottom is ( ( 0 or 1 times q.bottom ) + q.bottom )
	poll_rect.bottom = poll_rect.top + quadrant.bottom ;
	
	if ( m_server->PollFullScreen() )
	{
		m_desktop->m_changed_rgn.AddRect( poll_rect ) ;
	} 
	else 
	{
		//Other polling 
		if ( m_server->PollForeground() )
		{
			// Get the window rectangle for the currently selected window
			HWND hwnd = GetForegroundWindow() ;
			if ( hwnd != NULL ) { PollWindow( hwnd, poll_rect ) ; }
		}
	
		if ( m_server->PollUnderCursor() ) 
		{
			// Find the mouse position
			POINT mousepos ;
			if ( GetCursorPos( &mousepos ) ) 
			{
				// Find the window under the mouse
				HWND hwnd = WindowFromPoint(mousepos);
				if ( hwnd != NULL ) { PollWindow( hwnd, poll_rect ) ; }
			}
		}
	}
	
	// increment the polling step
	m_pollingStep = ( m_pollingStep + 1 ) % 4 ;

	return ;
}

void
PollingQuadrant::PollWindow( HWND hwnd, RECT &poll_rect )
{
	//
	// return if the user hasn't done anything
	// and if we are in low-load polling
	//
	
	if ( 
		m_server->PollOnEventOnly() 
		&& ! m_server->RemoteEventReceived()
	)
	{
		return ;
	}

	//
	// does the client want us to poll only console windows?
	//
	
	if ( m_server->PollConsoleOnly() )
	{
		char classname[20];

		// Yes, so check that this is a console window...
		if (GetClassName(hwnd, classname, sizeof(classname))) {
			if ((strcmp(classname, "tty") != 0) &&
				(strcmp(classname, "ConsoleWindowClass") != 0)) {
				return;
			}
		}
	}

	//
	// only poll the intersection of the window 
	// and passed target rect
	//

	RECT window_rect ;

	if ( 
		GetWindowRect( hwnd, &window_rect ) 
		&& IntersectRect( &poll_rect, &poll_rect, &window_rect )
	) 
	{
		m_desktop->m_changed_rgn.AddRect( poll_rect ) ;
	}
	
	return ;
}
