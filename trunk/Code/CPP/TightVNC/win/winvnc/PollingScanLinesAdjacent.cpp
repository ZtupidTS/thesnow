
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: PollingScanLinesAdjacent.cpp 1891 2004-10-06 14:14:11Z const_k $

#include "PollingScanLinesAdjacent.h"

//
// static variable initialization
//

const int PollingScanLinesAdjacent::m_pollingOrder[32] = {
	 0, 16,  8, 24,  4, 20, 12, 28,
	10, 26, 18,  2, 22,  6, 30, 14,
	 1, 17,  9, 25,  7, 23, 15, 31,
	19,  3, 27, 11, 29, 13,  5, 21
};

//
// class implementation
//

PollingScanLinesAdjacent::PollingScanLinesAdjacent( vncDesktop* desktop ) 
	: PollingBase(), m_pollingStep( 0 ) 
{
	m_desktop = desktop ;
}


void
PollingScanLinesAdjacent::PerformPolling( void ) 
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
PollingScanLinesAdjacent::PollWindow( HWND hwnd )
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
PollingScanLinesAdjacent::PollArea( RECT &rect )
{
	// get bytes per pixel
	const UINT bytesPerPixel = m_desktop->m_scrinfo.format.bitsPerPixel / 8 ;
	
	// determine next scanline index
	int scanLine = m_pollingOrder[ m_pollingStep++ % 32 ] ;

	//
	// align 32x32 tiles to the left top corner of the shared area
	//
	
	RECT shared = m_server->GetSharedRect() ;

	//
	// determine starting coordinates
	//

	int starting_x = ( rect.left - shared.left ) & 0xFFFFFFE0 ;
	starting_x += shared.left ;
	
	int starting_y = ( rect.top - shared.top ) & 0xFFFFFFE0 ;
	
	if ( starting_y + scanLine < rect.top - shared.top )
		starting_y += 32 ;		

	starting_y += shared.top ;

	//
	// run the scan
	//

	// vector of rows of rectangles
	ChangedRectanglesMap change_map ;

	// reserve enough room for n + 1 elements
	change_map.reserve( ( 32 + rect.bottom - starting_y ) / 32 ) ;

	RECT scan_rect = rect ; // we'll need left and right borders of the shared area
	RECT tile_rect ; // used to add rects to the shared region

	int rects_changed = 0 ;

	for ( int y = starting_y ; y < rect.bottom ; y += 32 ) 
	{
		// determine tile height
		int tile_h = ( rect.bottom - y <= 32 ) 
			? ( rect.bottom - y ) // less than 32
			: 32 // default
		;

		// skip if scan line is beyond tile height
		if ( scanLine >= tile_h )
			continue ;

		// determine y coordinate for scan rect
		int scan_y = y + scanLine ;
		
		// 
		scan_rect.top = scan_y ;
		scan_rect.bottom = scan_y + 1 ;
		
		// capture row to main buffer
		m_desktop->CaptureScreen( scan_rect, m_desktop->m_mainbuff ) ;

		// determine byte offset of scan y into buffers
		int offset = ( scan_y * m_desktop->m_bytesPerRow ) + ( starting_x * bytesPerPixel ) ;
		
		unsigned char *o_ptr = m_desktop->m_backbuff + offset ;
		unsigned char *n_ptr = m_desktop->m_mainbuff + offset ;
		
		// row of changes
		ChangedRectanglesRow change_row ;

		// reserve enough room for n + 1 elements
		change_row.reserve( ( 32 + rect.right - starting_x ) / 32 ) ;
		
		for ( int x = starting_x ; x < rect.right ; x += 32 )
		{
			// determine tile width
			int tile_w = ( rect.right - x <= 32) 
				? ( rect.right - x ) // less than 32
				: 32 // default
			; 
			
			// bytes to compare ( based on tile width
			int nBytes = tile_w * bytesPerPixel ;
			
			// rectangle for this tile
			SetRect( &tile_rect, x, y, x + tile_w, y + tile_h ) ;

			if ( memcmp( o_ptr, n_ptr, nBytes) != 0 ) 
			{
				// mark rectangle as changed
				change_row.push_back( ChangedRectangle( ChangedRectanglesKey::CHANGED, tile_rect ) ) ;

				// update rects changed count
				++rects_changed ;
			}
			else
			{
				// mark rectangle as unchanged
				change_row.push_back( ChangedRectangle( ChangedRectanglesKey::UNCHANGED, tile_rect ) ) ;
			}
			
			// move buffer pointers
			o_ptr += nBytes ;
			n_ptr += nBytes ;
		}

		// push row onto change map
		change_map.push_back( change_row ) ;		
	}
	
	// short-circuit if no rects changed
	if ( rects_changed == 0 )
		return ;
		
	// we're using indexes instead of iterator
	// so we can freely move up, down and around the map
	
	int index_y = 0 ;
	int index_x = 0 ;
	
	int changed_y = 0 ;
	int changed_x = 0 ;
	
	// move through map top to bottom
	for ( index_y = 0 ; index_y < change_map.size() ; ++index_y )
	{
		// more through row right to left
		for ( index_x = 0 ; index_x < change_map[ index_y ].size() ; ++index_x )
		{
			// skip non-changed rects
			if ( change_map[ index_y ][ index_x ].first != ChangedRectanglesKey::CHANGED )
				continue ;

			// check surrounding rectangles
			
			for ( int yy = -1 ; yy <= 1 ; ++yy )
			{
				changed_y = index_y + yy ;

				// check for out-of-bounds
				if ( ( changed_y < 0 ) || ( changed_y >= change_map.size() ) )
					continue ;
				
				for ( int xx = -1 ; xx <= 1 ; ++xx )
				{
					changed_x = index_x + xx ;

					// check for out-of-bounds
					if ( ( changed_x < 0 ) || ( changed_x >= change_map[ index_y ].size() ) )
						continue ;
					
					// update surrounding rectangle state

					if ( rects_changed < 9 )
					{
						// grap the bounding rectangles
						change_map[ changed_y ][ changed_x ].first = ChangedRectanglesKey::PROMOTE ;
					}
					else
					{
						if ( change_map[ changed_y ][ changed_x ].first == ChangedRectanglesKey::UNCHANGED )
						{
							// unchanged, mark as suspected
							change_map[ changed_y ][ changed_x ].first = ChangedRectanglesKey::NEIGHBOR ;
						}
						else if ( change_map[ changed_y ][ changed_x ].first == ChangedRectanglesKey::NEIGHBOR )
						{
							// already suspected, mark as changed
							change_map[ changed_y ][ changed_x ].first = ChangedRectanglesKey::PROMOTE ;
						}
						else
						{
							// already changed, do nothing
						}
					}
				}
			}
		}
	}

	//
	// add the changed rect to the changed region
	//

	ChangedRectanglesMap::iterator iter_y ;
	ChangedRectanglesRow::iterator iter_x ;
	
	for ( iter_y = change_map.begin() ; iter_y != change_map.end() ; ++iter_y )
	{
		for ( iter_x = (*iter_y).begin() ; iter_x != (*iter_y).end() ; ++iter_x )
		{
			// add rect with changed area
			if ( 
				( *iter_x ).first == ChangedRectanglesKey::CHANGED 
				|| ( *iter_x ).first == ChangedRectanglesKey::PROMOTE
			) 
			{
				m_desktop->m_changed_rgn.AddRect( ( *iter_x ).second ) ;
			}
		}
	}

/*
#ifdef _DEBUG
	//
	// log an ascii represenation of the changed rectangles
	//
	
	vnclog.Print( LL_INTINFO, VNCLOG( "<CHANGED_RECTANGLE>\n" ) ) ;
	for ( iter_y = change_map.begin() ; iter_y != change_map.end() ; ++iter_y )
	{
		std::string change_string ;
		for ( iter_x = (*iter_y).begin() ; iter_x != (*iter_y).end() ; ++iter_x )
		{
			switch ( (*iter_x).first )
			{
				case ChangedRectanglesKey::CHANGED :
					change_string += 'X' ;
					break ;

				case ChangedRectanglesKey::PROMOTE :
					change_string += 'x' ;
					break ;

				case ChangedRectanglesKey::NEIGHBOR :
					change_string += ',' ;
					break ;

				default:
					change_string += '.' ;
			}
		}
		vnclog.Print( LL_INTINFO, VNCLOG( "%s\n" ), change_string.c_str() ) ;
	}
	vnclog.Print( LL_INTINFO, VNCLOG( "</CHANGED_RECTANGLE>\n" ) ) ;
#endif
*/

	return ;
}




