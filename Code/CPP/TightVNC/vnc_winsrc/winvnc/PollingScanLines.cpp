
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: PollingScanLines.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "PollingScanLines.h"

//
// static variable initialization
//

const int PollingScanLines::m_pollingOrder[32] = {
	 0, 16,  8, 24,  4, 20, 12, 28,
	10, 26, 18,  2, 22,  6, 30, 14,
	 1, 17,  9, 25,  7, 23, 15, 31,
	19,  3, 27, 11, 29, 13,  5, 21
};

//
// class implementation
//

PollingScanLines::PollingScanLines( vncDesktop* desktop ) 
	: PollingBase(), m_pollingStep( 0 ) 
{
	m_desktop = desktop ;
}


void
PollingScanLines::PerformPolling( void ) 
{
	if ( m_server->PollFullScreen() )
	{
		// poll full screen
		RECT full_rect = m_server->GetSharedRect() ;
		PollArea( full_rect ) ;
	}
	else 
	{
		// poll windows

		// get the window rectangle for the currently selected window
		if ( m_server->PollForeground() )
		{
			HWND hwnd = GetForegroundWindow() ;
			if ( hwnd != NULL ) { PollWindow( hwnd ) ; }
		}
	
		// get the window rectangle for the window under the mouse
		if ( m_server->PollUnderCursor() ) 
		{
			// find the mouse position
			POINT mousepos ;
			if ( GetCursorPos( &mousepos ) ) 
			{
				// Find the window under the mouse
				HWND hwnd = WindowFromPoint( mousepos ) ;
				if ( hwnd != NULL ) { PollWindow( hwnd ) ; }
			}
		}
	}

	return ;
}

void
PollingScanLines::PollWindow( HWND hwnd )
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
	// and server's shared rect
	//

	RECT full_rect = m_server->GetSharedRect() ;
	RECT rect ;

	if ( 
		GetWindowRect( hwnd, &rect ) 
		&& IntersectRect( &rect, &rect, &full_rect )
	) 
	{
		PollArea( rect ) ;
	}
	
	return ;
}

//
// Implementation of the polling algorithm.
//

void
PollingScanLines::PollArea( RECT &rect )
{
	//
	// !!! FIXME !!!
	//
	// there seems to be an occasional crash in here
	// due to an invalid or null pointer
 	//

	int scanLine = m_pollingOrder[m_pollingStep++ % 32];
	const UINT bytesPerPixel = m_desktop->m_scrinfo.format.bitsPerPixel / 8;

	// Align 32x32 tiles to the left top corner of the shared area
	RECT shared = m_server->GetSharedRect();
	int leftAligned = ((rect.left - shared.left) & 0xFFFFFFE0) + shared.left;
	int topAligned = (rect.top - shared.top) & 0xFFFFFFE0;
	if (topAligned + scanLine < rect.top - shared.top)
		topAligned += 32;
	topAligned += shared.top;

	RECT rowRect = rect;	// we'll need left and right borders
	RECT tileRect;

	for (int y = topAligned; y < rect.bottom; y += 32) {
		int tile_h = (rect.bottom - y >= 32) ? 32 : rect.bottom - y;
		if (scanLine >= tile_h)
			continue;
		int scan_y = y + scanLine;
		rowRect.top = scan_y;
		rowRect.bottom = scan_y + 1;
		m_desktop->CaptureScreen(rowRect, m_desktop->m_mainbuff);
		int offset = scan_y * m_desktop->m_bytesPerRow + leftAligned * bytesPerPixel;
		unsigned char *o_ptr = m_desktop->m_backbuff + offset;
		unsigned char *n_ptr = m_desktop->m_mainbuff + offset;
		for (int x = leftAligned; x < rect.right; x += 32) {
			int tile_w = (rect.right - x >= 32) ? 32 : rect.right - x;
			int nBytes = tile_w * bytesPerPixel;
			if (memcmp(o_ptr, n_ptr, nBytes) != 0) {
				SetRect(&tileRect, x, y, x + tile_w, y + tile_h);
				m_desktop->m_changed_rgn.AddRect(tileRect);
			}
			o_ptr += nBytes;
			n_ptr += nBytes;
		}
	}
}
