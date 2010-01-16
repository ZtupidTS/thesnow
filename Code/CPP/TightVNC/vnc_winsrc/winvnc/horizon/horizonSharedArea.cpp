
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonSharedArea.cpp 1984 2005-01-20 16:53:19Z jeffo3 $

#include "horizonSharedArea.h"

//
// class implementation
//

horizonSharedArea::horizonSharedArea( HWND hwnd, CMatchWindow *matchwindow ) 
	: m_server( vncServerSingleton::GetInstance() )
{
	m_hwnd = hwnd;
	
	m_bCaptured = FALSE;
	m_KeepHandle = NULL;

	// if a matchwindow wasn't passed in, 
	// we're responsible for it
	if ( matchwindow == NULL )
	{
		m_pMatchWindow = NULL ;
		m_deleteMatchWindow = true ;
	}
	else
	{
		m_pMatchWindow = matchwindow ;
		m_deleteMatchWindow = false ;
	}

	// set to false by default
	m_pref_WindowShared = FALSE ;
	m_pref_BlackRgn = FALSE ;
	m_pref_FullScreen = FALSE ;
	m_pref_ScreenAreaShared = FALSE ;

	Init() ;
}

horizonSharedArea::~horizonSharedArea()
{
	if ( 
		m_pMatchWindow != NULL 
		&& m_deleteMatchWindow == true 
	)
	{
		delete m_pMatchWindow ;
		m_pMatchWindow = NULL ;
	}
}

void 
horizonSharedArea::Init()
{
	//
	// get settings from server
	//

	SetPrefFullScreen( m_server->FullScreen() ) ;
	SetPrefWindowShared( m_server->WindowShared() ) ;
	SetPrefScreenAreaShared( m_server->ScreenAreaShared() ) ;
	SetPrefBlackRgn( m_server->GetApplication() ) ;
	
	//
	//  configure select window picture
	//
	
	HWND bmp_hWnd = GetDlgItem( m_hwnd, IDC_BMPCURSOR ) ;
	m_OldBmpWndProc = GetWindowLong( bmp_hWnd, GWL_WNDPROC ) ;
	SetWindowLong( bmp_hWnd, GWL_WNDPROC, (LONG)(BmpWndProc) ) ;
	SetWindowLong( bmp_hWnd, GWL_USERDATA, (LONG)(this) ) ;

	//
	// setup match window
	//
	
	if ( m_pMatchWindow == NULL ) 
	{
		// get the desktop's bounds
		RECT desktopRect ;
		GetWindowRect( GetDesktopWindow(), &desktopRect ) ;

		//
		// instantiate object with default bounds
		// ( i.e. top-left quadrant of the desktop )
		//
		
		m_pMatchWindow = new CMatchWindow( 
			m_server, 
			desktopRect.left+5, desktopRect.top+5, 
			desktopRect.right/2, desktopRect.bottom/2
		) ;

		//
		// if region sharing is the current setting,
		// the server rect is not whole screen, 
		// and the server rect is not empty, 
		// then use the server's bounds
		//
		
		if ( GetPrefScreenAreaShared() )
		{
			// get the server's current rect
			RECT serverRect ;
			serverRect = m_server->GetSharedRect() ;

			if ( 
				( EqualRect( &desktopRect, &serverRect ) == FALSE )
				&& ( IsRectEmpty( &serverRect ) == FALSE )
			)
			{
				// use it for match window
				m_pMatchWindow->ModifyPosition( 
					serverRect.left, serverRect.top, 
					serverRect.right, serverRect.bottom 
				) ;
			}
		}
	}

	m_pMatchWindow->CanModify( TRUE ) ;
	m_pMatchWindow->Hide() ;

	//
	// clear values
	//

	// full desktop
	HWND hFullScreen = GetDlgItem(m_hwnd, IDC_FULLSCREEN);
    SendMessage( hFullScreen, BM_SETCHECK, GetPrefFullScreen(), 0 ) ;

	// window
	HWND hWindowCaption = GetDlgItem(m_hwnd, IDC_WINDOW);
	SendMessage( hWindowCaption, BM_SETCHECK, GetPrefWindowShared(), 0 ) ;

	// window name
	hNameAppli = GetDlgItem( m_hwnd, IDC_NAME_APPLI ) ;
	EnableWindow( hNameAppli, GetPrefWindowShared() ) ;

	// screen area
	HWND hScreenCaption = GetDlgItem(m_hwnd, IDC_SCREEN);
	SendMessage( hScreenCaption, BM_SETCHECK, GetPrefScreenAreaShared(), 0 ) ;

	// black-out other windows
	HWND hBlackRgn = GetDlgItem(m_hwnd, IDC_CHECK_BLACK_RGN);
	SendMessage( hBlackRgn, BM_SETCHECK, GetPrefBlackRgn(), 0 ) ;

	//
	// toggle selected option
	//

	if ( GetPrefFullScreen() )
	{
		FullScreen() ;
	}
	else if ( GetPrefScreenAreaShared() ) 
	{
		SharedScreen() ;
	}
	else // if ( GetPrefWindowShared() ) 
	{
		// default
		
		// select radio button
		SendMessage( hWindowCaption, BM_SETCHECK, TRUE, 0 ) ;
		
		// run shared window helper
		SharedWindow() ;
	}

	// bring dialog to the front
	SetForegroundWindow( m_hwnd ) ;
	
	return ;
}

void
horizonSharedArea::CancelChanges()
{
	// return match window to proper state
	if ( m_server->ScreenAreaShared() )	
		m_pMatchWindow->Show() ;
	else
		m_pMatchWindow->Hide() ;

	return ;
}

bool 
horizonSharedArea::ApplySharedControls()
{
	if (
		GetPrefWindowShared() 
		&& m_server->GetWindowShared() == NULL
	) 
	{	
		MessageBox(NULL,
				"You have not yet selected a window to share.\n"
				"Please first select a window with the 'Window Target'\n"
				"icon, and try again.", "No Window Selected",
				 MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	//
	// set server's share type
	//

	// full screen
	HWND hFullScreen = GetDlgItem( m_hwnd, IDC_FULLSCREEN ) ;
	m_server->FullScreen( SendMessage( hFullScreen, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;

	// window 
	HWND hWindowCapture = GetDlgItem( m_hwnd, IDC_WINDOW ) ;
	m_server->WindowShared( SendMessage( hWindowCapture, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;

	// screen area
	HWND hScreenArea = GetDlgItem( m_hwnd, IDC_SCREEN ) ;
	m_server->ScreenAreaShared( SendMessage( hScreenArea, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;
	
	// unsupported
	HWND hBlackRgn = GetDlgItem( m_hwnd, IDC_CHECK_BLACK_RGN ) ;
	if ( SendMessage( hBlackRgn, BM_GETCHECK, 0, 0) == BST_CHECKED ) 
	{
		m_server->SetApplication( TRUE ) ;
		m_server->SetNewFBSize( TRUE ) ;
	} 
	else 
	{
		// SetNewFBSize() should probably be called here under some circumstances
		m_server->SetApplication( FALSE ) ;
	}

	//
	// set server's match size
	//

	if ( GetPrefScreenAreaShared() ) 
	{
		// set the server's shared area rect
		m_pMatchWindow->SetServerScreenArea() ;

		// start the update timer
		m_pMatchWindow->StartUpdateServerTimer() ;
	}
	else
	{
		// stop the update timer
		m_pMatchWindow->StopUpdateServerTimer() ;
	}

	if ( GetPrefFullScreen() ) 
	{
		RECT temp ;
		GetWindowRect( GetDesktopWindow(), &temp ) ;
		m_server->SetMatchSizeFields( temp.left, temp.top, temp.right, temp.bottom ) ;
	}

	return true ;
}

void horizonSharedArea::FullScreen()
{
	// disable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, FALSE);
	
	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP3));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);
				
	// set window text name
	EnableWindow(hNameAppli, FALSE);
	::SetWindowText(hNameAppli, "Full Desktop Selected");
			
	// hide match window
	m_pMatchWindow->Hide();

	// update properties
	SetPrefFullScreen( TRUE ) ;
	SetPrefWindowShared( FALSE ) ;
	SetPrefScreenAreaShared( FALSE ) ;

	return ;
}

void horizonSharedArea::SharedWindow()
{
	// enable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, TRUE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap( hAppInstance, MAKEINTRESOURCE( IDB_BITMAP1 ) ) ;
	HBITMAP hOldImage = ( HBITMAP )( ::SendMessage( bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, ( LPARAM )( hNewImage ) ) ) ;
	DeleteObject( hOldImage ) ;

	// set window text name
	EnableWindow(hNameAppli, FALSE);
	SetWindowCaption(m_server->GetWindowShared());
	EnableWindow(hNameAppli, TRUE);

	// hide match window
	m_pMatchWindow->Hide();

	// update properties
	SetPrefFullScreen(FALSE);
	SetPrefWindowShared(TRUE);
	SetPrefScreenAreaShared(FALSE);
	
	return ;
}

void horizonSharedArea::SharedScreen()
{
	// disable window cursor
	HWND bmp_hWnd = GetDlgItem(m_hwnd, IDC_BMPCURSOR);
	EnableWindow(bmp_hWnd, FALSE);

	// change cursor image
	HBITMAP hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP3));
	HBITMAP hOldImage = (HBITMAP)::SendMessage(bmp_hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
	DeleteObject(hOldImage);

	// set window text name
	EnableWindow(hNameAppli, FALSE);
	::SetWindowText(hNameAppli, "Screen Area Selected");
	
	// show match window
	m_pMatchWindow->Show();
	
	// update properties
	SetPrefFullScreen(FALSE);
	SetPrefWindowShared(FALSE);
	SetPrefScreenAreaShared(TRUE);

	return ;
}

LRESULT CALLBACK horizonSharedArea::BmpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP hNewImage, hOldImage;
	HCURSOR hNewCursor, hOldCursor;
	horizonSharedArea* pDialog = (horizonSharedArea*) GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) {

	case WM_SETCURSOR :
		if (HIWORD(lParam) == WM_LBUTTONDOWN) {
			SetCapture(hWnd);
			hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP2));
			hOldImage = (HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
			DeleteObject(hOldImage);
			hNewCursor = (HCURSOR)LoadImage(hAppInstance, MAKEINTRESOURCE(IDC_CURSOR1),
											IMAGE_CURSOR, 32, 32, LR_DEFAULTCOLOR);
			hOldCursor = SetCursor(hNewCursor);
			DestroyCursor(hOldCursor);
			pDialog->m_bCaptured = TRUE;
		}
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		
		if (pDialog->m_KeepHandle != NULL) {
			// We need to remove frame
			DrawFrameAroundWindow(pDialog->m_KeepHandle, false);
			pDialog->m_server->SetWindowShared(pDialog->m_KeepHandle);
			// No more need
			pDialog->m_KeepHandle = NULL;
		} else {
			pDialog->m_server->SetWindowShared(NULL);
		}

		hNewImage = LoadBitmap(hAppInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		hOldImage = (HBITMAP)::SendMessage(hWnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hNewImage);
		DeleteObject(hOldImage);
		hNewCursor = LoadCursor(hAppInstance, MAKEINTRESOURCE(IDC_ARROW));
		hOldCursor = SetCursor(hNewCursor);
		DestroyCursor(hOldCursor);
		pDialog->m_bCaptured = FALSE;
		break;
	
	case WM_MOUSEMOVE:
		if (pDialog->m_bCaptured) {
			HWND hParent = ::GetParent(hWnd);
			POINTS pnt;
			POINT pnt1;
			pnt = MAKEPOINTS(lParam);
			pnt1.x = pnt.x;
			pnt1.y = pnt.y;
			ClientToScreen(hWnd, &pnt1);
			HWND hMouseWnd=::WindowFromPoint(pnt1);
			HWND hAncestor = ::GetAncestor(hMouseWnd, GA_ROOT);
			if (hAncestor != NULL)
			    hMouseWnd = hAncestor;
			if (pDialog->m_KeepHandle != hMouseWnd) {
				// New Windows Handle
				// Was KeepHndle A Real Window ?
				if (pDialog->m_KeepHandle != NULL) {
					// We need to remove frame
					horizonSharedArea::DrawFrameAroundWindow(pDialog->m_KeepHandle, false);
				}
				pDialog->m_KeepHandle = hMouseWnd;
				if (!IsChild(hParent, hMouseWnd) && hMouseWnd != hParent) {
					pDialog->SetWindowCaption(hMouseWnd);
					horizonSharedArea::DrawFrameAroundWindow(hMouseWnd, true);
				} else {	// My Window
					pDialog->m_KeepHandle = NULL;
					pDialog->SetWindowCaption(NULL);
				}
			}
		}
		break;

	case WM_PAINT:
	case STM_SETIMAGE:
		return CallWindowProc( ( WNDPROC )( pDialog->m_OldBmpWndProc ),
			hWnd, message, wParam, lParam ) ;
		break ;
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void horizonSharedArea::DrawFrameAroundWindow(HWND hWnd, bool draw)
{
	static HBITMAP s_topBmp = NULL;
	static HBITMAP s_bottomBmp = NULL;
	static HBITMAP s_leftBmp = NULL;
	static HBITMAP s_rightBmp = NULL;
	static int thickness = 3;

	// Get window rectangle and region
	RECT rect;
	HDC hWindowDc=::GetWindowDC(hWnd);
	HRGN Rgn=CreateRectRgn(0,0,1,1);
	GetWindowRect(hWnd,&rect);
	// If window is maximized, adjust rect
	if (IsZoomed(hWnd)) {
	    rect.left += 4;
	    rect.right -= 4;
	    rect.top += 4;
	    rect.bottom -= 4;
	    OffsetRect(&rect,-rect.left + 4,-rect.top + 4);
	} else {
	    OffsetRect(&rect,-rect.left,-rect.top);
	}
	Rgn=CreateRectRgn(rect.left,rect.top,rect.right,rect.bottom);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	
	// create context
	HDC hdcCompatible = CreateCompatibleDC(hWindowDc);
	if (draw) {

	    // create bmps
	    if (s_topBmp != NULL) {
		::DeleteObject(s_topBmp);
		s_topBmp = NULL;
	    }
	    s_topBmp = CreateCompatibleBitmap(hWindowDc, width, thickness);
	    if (s_bottomBmp != NULL) {
		::DeleteObject(s_bottomBmp);
		s_bottomBmp = NULL;
	    }
	    s_bottomBmp = CreateCompatibleBitmap(hWindowDc, width, thickness);
	    if (s_leftBmp != NULL) {
		::DeleteObject(s_leftBmp);
		s_leftBmp = NULL;
	    }
	    s_leftBmp = CreateCompatibleBitmap(hWindowDc, thickness, height);
	    if (s_rightBmp != NULL) {
		::DeleteObject(s_rightBmp);
		s_rightBmp = NULL;
	    }
	    s_rightBmp = CreateCompatibleBitmap(hWindowDc, thickness, height);

	    // copy bmps 
	    SelectObject(hdcCompatible, s_topBmp);
	    BitBlt(hdcCompatible, 0, 0, width, thickness, hWindowDc, rect.left, rect.top, SRCCOPY);
	    
	    SelectObject(hdcCompatible, s_bottomBmp);
	    BitBlt(hdcCompatible, 0, 0, width, thickness, hWindowDc, rect.left, rect.bottom - thickness, SRCCOPY);

	    SelectObject(hdcCompatible, s_leftBmp);
	    BitBlt(hdcCompatible, 0, 0, thickness, height, hWindowDc, rect.left, rect.top, SRCCOPY);

	    SelectObject(hdcCompatible, s_rightBmp);
	    BitBlt(hdcCompatible, 0, 0, thickness, height, hWindowDc, rect.right - thickness, rect.top, SRCCOPY);

	    // draw frame
	    HBRUSH hBrush=CreateSolidBrush(RGB(193,30,22));
	    SetROP2(hWindowDc, R2_COPYPEN);
	    FrameRgn(hWindowDc,Rgn,hBrush,thickness,thickness);
	    ::DeleteObject(hBrush);
	} else {
	    // draw bmps
	    SelectObject(hdcCompatible, s_topBmp);
	    BitBlt(hWindowDc, rect.left, rect.top, width, thickness, hdcCompatible, 0, 0, SRCCOPY);

	    SelectObject(hdcCompatible, s_bottomBmp);
	    BitBlt(hWindowDc, rect.left, rect.bottom - thickness, width, thickness, hdcCompatible, 0, 0, SRCCOPY);

	    SelectObject(hdcCompatible, s_leftBmp);
	    BitBlt(hWindowDc, rect.left, rect.top, thickness, height, hdcCompatible, 0, 0, SRCCOPY);

	    SelectObject(hdcCompatible, s_rightBmp);
	    BitBlt(hWindowDc, rect.right - thickness, rect.top, thickness, height, hdcCompatible, 0, 0, SRCCOPY);

	    // delete bmps
	    ::DeleteObject(s_topBmp);
	    ::DeleteObject(s_bottomBmp);
	    ::DeleteObject(s_leftBmp);
	    ::DeleteObject(s_rightBmp);
	    s_topBmp = s_bottomBmp = s_leftBmp = s_rightBmp = NULL;

	}
	::DeleteDC(hdcCompatible);

	::DeleteObject(Rgn);
	::ReleaseDC(hWnd,hWindowDc);
    
	return ;
}

void horizonSharedArea::SetWindowCaption(HWND hWnd)
{
	char strWindowText[256];
	if (hWnd == NULL) 
	{
		strcpy(strWindowText, "<< Drag icon to select window");
	} else {
		GetWindowText(hWnd, strWindowText, sizeof(strWindowText));
		if (!strWindowText[0]) 
		{
			int bytes = sprintf(strWindowText, "0x%x ", hWnd);
			GetClassName(hWnd, strWindowText + bytes,
				sizeof(strWindowText) - bytes);
		}
	}

	if ( hNameAppli )
		::SetWindowText(hNameAppli, strWindowText) ;
		
	return ;
}

