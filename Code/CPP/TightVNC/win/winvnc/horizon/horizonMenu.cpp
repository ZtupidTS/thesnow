
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonMenu.cpp 2009 2005-05-05 19:01:23Z jeffo3 $

#include "horizonMenu.h"

//
// constants
//

const char *MENU_CLASS_NAME = "AppShare Tray Icon";

// appshare uses this one
const UINT APPSHARE_ADD_CLIENT_MSG = RegisterWindowMessage( "AppShare.AddClient.Message" ) ;

// appshare does _not_ use these
const UINT MENU_ADD_CLIENT_MSG = 0 ;
const UINT MENU_ABOUTBOX_SHOW = 0 ;
const UINT MENU_CONTROL_PANEL_SHOW = 0 ;
const UINT MENU_DEFAULT_PROPERTIES_SHOW = 0 ;
const UINT MENU_KILL_ALL_CLIENTS_MSG = 0 ;
const UINT MENU_PROPERTIES_SHOW = 0 ;
const UINT MENU_RELOAD_MSG = 0 ; 
const UINT MENU_SERVER_SHAREWINDOW = 0 ;
const UINT MENU_SERVICEHELPER_MSG = 0 ;
const UINT fileTransferDownloadMessage = 0 ;

// static instance
horizonMenu* horizonMenu::m_instance = 0 ;

//
// class implementation
//

horizonMenu*
horizonMenu::GetInstance( void ) 
{ 
	if ( m_instance == NULL )
		m_instance = new horizonMenu() ;
	return m_instance ;
}

horizonMenu::horizonMenu()
	: m_server( vncServerSingleton::GetInstance() ), 
		m_properties( horizonProperties::GetInstance() ),
		m_is_started( false )
{
}

horizonMenu::~horizonMenu()
{
}

bool 
horizonMenu::Start( void )
{
	if ( m_is_started == true )
		return true ;

	// initialize com library
	// CoInitialize(0);

	//
	// create a dummy window to handle tray icon messages
	//
	
	WNDCLASSEX wndclass;

	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= 0;
	wndclass.lpfnWndProc	= horizonMenu::WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hAppInstance;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName	= (const char *) NULL;
	wndclass.lpszClassName	= MENU_CLASS_NAME;
	wndclass.hIconSm		= LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx( &wndclass ) ;

	m_hwnd = CreateWindow(
		MENU_CLASS_NAME, MENU_CLASS_NAME, 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		200, 200, NULL, NULL, 
		hAppInstance, NULL
	) ;

	if ( m_hwnd == NULL )
	{
		vnclog.Print( 
			LL_INTERR, 
			VNCLOG( "unable to CreateWindow, error => %d\n" ), 
			GetLastError() 
		) ;
		
		return false ;
	}

	// timer to trigger icon updating
	SetTimer( m_hwnd, 1, TIMER_INTERVAL, NULL ) ;

	// record which client created this window
	SetWindowLong( m_hwnd, GWL_USERDATA, (LONG)( this ) ) ;

	// Ask the server object to notify us of stuff
	m_server->AddNotify( m_hwnd ) ;

	//
	// load the tray icons
	// ( try to use higher res icons where os > winxp )
	//
	
	bool useHighRes = false ;

	OSVERSIONINFO osvi ;
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;

	if ( GetVersionEx( &osvi ) != 0 )
	{
		if ( 
			( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1 ) // winxp
			|| ( osvi.dwMajorVersion > 5 ) // greater than winxp
		)
		{
			useHighRes = true ;
		}
	}
	
	if ( useHighRes == true )
	{
		m_winvnc_normal_icon = LoadIcon( hAppInstance, MAKEINTRESOURCE(IDI_LIVESHARE_NOCON) ) ;
		m_flash_icon = LoadIcon( hAppInstance, MAKEINTRESOURCE(IDI_LIVESHARE_CON) ) ;
	}
	else
	{
		m_winvnc_normal_icon = LoadIcon( hAppInstance, MAKEINTRESOURCE(IDI_LIVESHARE_NOCON_LR) ) ;
		m_flash_icon = LoadIcon( hAppInstance, MAKEINTRESOURCE(IDI_LIVESHARE_CON_LR) ) ;
	}

	// current icon
	m_winvnc_icon = m_winvnc_normal_icon ;

	//
	// load the popup menu
	//
	
	m_hmenu = LoadMenu( hAppInstance, MAKEINTRESOURCE( IDR_LSHTRAYMENU ) ) ;

	// display the appshare icon in the systray
	AddTrayIcon();

	// set m_is_started flag
	m_is_started = true ;

	return true ;
}

bool 
horizonMenu::Shutdown( void )
{
	if ( m_is_started == false )
		return false ;
	
	// remove the icon from the systray
	DelTrayIcon() ;

	if ( m_server != NULL )
	{
		// tell the server to drop all client connections
		m_server->KillAuthClients() ;

		// wait until the clients have died gracefully
		m_server->WaitUntilAuthEmpty() ;

		// tell the server to stop notifying us	
		m_server->RemNotify( m_hwnd ) ;
	}

	// clean up the window handle
	if ( m_hwnd != NULL )
	{
		DestroyWindow( m_hwnd ) ;
		m_hwnd = NULL ;
	}

	// clean up the menu
	if ( m_hmenu != NULL )
	{
		DestroyMenu( m_hmenu ) ;
		m_hmenu = NULL ;
	}
	
	// reset m_is_started flag
	m_is_started = false ;
	
	return true ;
}

//
// tray icon control methods
//

void
horizonMenu::AddTrayIcon( void )
{
	if ( ! m_server->GetDisableTrayIcon() )
		SendTrayMsg( NIM_ADD ) ;
	return ;
}

void
horizonMenu::DelTrayIcon( void )
{
	// SendTrayMsg( NIM_DELETE ) ;
	Shell_NotifyIcon( NIM_DELETE, &m_nid ) ; // cut to the chase
	return ;
}

void
horizonMenu::FlashTrayIcon( void )
{
	SendTrayMsg( NIM_MODIFY ) ;
	return ;
}

//
// messaging methods
//

void
horizonMenu::SendTrayMsg( DWORD msg, BOOL flash )
{
	//
	// are we connected to the server?
	//
	
	bool connected = ( m_server->AuthClientCount() == 0 ) 
		? false 
		: true
	;

	//
	// create the tray icon message
	//

	m_nid.hWnd = m_hwnd ;
	m_nid.cbSize = sizeof( m_nid ) ;
	m_nid.uID = IDI_WINVNC ;			// never changes after construction
	m_nid.hIcon = connected ? m_flash_icon : m_winvnc_icon ;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE ;
	m_nid.uCallbackMessage = WM_TRAYNOTIFY ;

	//
	// set the tip string
	//

	string tip( szAppName ) ;
				
	tip += ( connected == true )
		? " : Connected"
		: " : Not Connected"
	;
		
	if ( tip.empty() == false )
	{
		// clear the tip buffer
		memset( m_nid.szTip, 0x0, sizeof( m_nid.szTip ) ) ;
	
		// copy the tip to the buffer
		tip.copy( m_nid.szTip, sizeof( m_nid.szTip ) - 1 ) ;

		// set the tip flag
		m_nid.uFlags |= NIF_TIP ;
	}
	
	//
	// Send the message
	//
	
	if ( Shell_NotifyIcon( msg, &m_nid ) )
	{
		// Set the enabled/disabled state of the menu items
		// vnclog.Print( LL_INTINFO, VNCLOG( "message sent to systray\n" ) ) ;

		// enable/disable settings menu option
		EnableMenuItem( 
			m_hmenu, ID_PROPERTIES,
			AllowProperties() ? MF_ENABLED : MF_GRAYED 
		) ;
		
#ifdef _DEBUG
		// enable advanced settings menu option
		EnableMenuItem( m_hmenu, ID_ADVANCED_PROPERTIES, MF_ENABLED) ;
		EnableMenuItem( m_hmenu, ID_DEBUG_LOGGING, MF_ENABLED) ;

		//
		// set the 'enable debug-level logging' checkbox
		//
		
		MENUITEMINFO mii ;
		mii.cbSize = sizeof( MENUITEMINFO ) ;
		mii.fMask = MIIM_STATE ;
	
		if ( GetMenuItemInfo( m_hmenu, ID_DEBUG_LOGGING, FALSE, &mii ) )
		{			
			if ( vnclog.GetLevel() > LL_NONE )
				mii.fState = MFS_CHECKED ;
			else
				mii.fState = MFS_UNCHECKED ;
			SetMenuItemInfo( m_hmenu, ID_DEBUG_LOGGING, FALSE, &mii ) ;
		}
		else
		{
			vnclog.Print( LL_INTERR, VNCLOG( "GetMenuItemInfo() => %d\n" ), GetLastError() ) ;
		}						
#else
		// disable advanced settings menu option
		EnableMenuItem( m_hmenu, ID_ADVANCED_PROPERTIES, MF_GRAYED) ;
		EnableMenuItem( m_hmenu, ID_DEBUG_LOGGING, MF_GRAYED) ;
#endif
		
	} 
	else 
	{
		// ( it'd be nice to report and error to the user here.... )
		vnclog.Print( LL_INTERR, VNCLOG( "unable to send message to systray\n" ) ) ;
	}
	
	return ;
}

//
// window callback method
//

LRESULT CALLBACK 
horizonMenu::WndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
	//
	// This is a static method, so we don't know which instantiation we're 
	// dealing with. We use Allen Hadden's (ahadden@taratec.com) suggestion 
	// from a newsgroup to get the pseudo-this.
	//
	
	horizonMenu* _this = reinterpret_cast<horizonMenu*>( GetWindowLong( hwnd, GWL_USERDATA ) ) ;

	switch ( iMsg )
	{

	// Every five seconds, a timer message causes the icon to update
	case WM_TIMER:
		{	
			_this->FlashTrayIcon() ;
			break ;
		}

	//
	// NOTIFICATIONS FROM THE SERVER
	//
	
	case WM_SRV_CLIENT_AUTHENTICATED:
	{
		// Adjust the icon accordingly
		_this->FlashTrayIcon() ;

		// kill the background
		if ( 
			_this->m_server->RemoveWallpaperEnabled() 
			&& _this->m_server->AuthClientCount() == 1
		)
		{
			vnclog.Print( LL_INTERR, VNCLOG( "killing wallpaper\n" ) ) ;
//			KillWallpaper() ;
		}
			
		return 0 ;
	}
	
	case WM_SRV_CLIENT_DISCONNECT:
	{
		// Adjust the icon accordingly
		_this->FlashTrayIcon() ;

		// restore the background
		if ( 
			_this->m_server->RemoveWallpaperEnabled()
			&& _this->m_server->AuthClientCount() == 0 
		)
		{
			vnclog.Print( LL_INTERR, VNCLOG( "restoring wallpaper\n" ) ) ;
//			RestoreWallpaper() ;
		}
		
		return 0 ;
	}

	//
	// STANDARD MESSAGE HANDLING
	//

	case WM_CREATE:
		return 0 ;

	// User has clicked an item on the tray menu
	case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{	
				// show the properties dialog
				case ID_PROPERTIES:
					if ( _this->isBasicUser() == true ) 
						_this->m_properties->ShowBasic() ;
					else
						_this->m_properties->ShowNormal() ;
					_this->FlashTrayIcon() ;
					break ;
					
				// show the advanced properties dialog
				case ID_ADVANCED_PROPERTIES:
					_this->m_properties->ShowAdvanced() ;
					_this->FlashTrayIcon() ;
					break ;
					
				// update log level
				case ID_DEBUG_LOGGING:
					_this->handleDebugLoggingEvent() ;
					break ;
										
				// about box
				case ID_ABOUT:
					_this->m_about.Show( TRUE ) ;
					break ;
		
				// quit
				case ID_CLOSE:
				{
					// handle calls to quit
					vnclog.Print( LL_INTINFO, VNCLOG( "!!! ID_CLOSE !!!\n" ) ) ;
				
					// tell the system to quit the app
					PostQuitMessage( 0 ) ;
			
					break ;
				}

				default:
					break ;
			}
			
			return 0 ;
		}

	// User has clicked on the tray icon or the menu
	case WM_TRAYNOTIFY:
		_this->handleTrayNotify( lParam ) ;
		return 0 ;

	case WM_QUERYENDSESSION:	// no-ops
	case WM_ENDSESSION:			// no-ops
		break;

	case WM_USERCHANGED:
		return 0 ;
	
	case WM_CLOSE:
	case WM_QUIT:
	case WM_DESTROY:
		return 0 ;

	// handle custom message
	default:
		if ( _this->handleCustomMessages( hwnd, iMsg, wParam, lParam ) == true )
			return 0 ;
	
	}
	
	// pass unrecognized messages up the chain
	return DefWindowProc( hwnd, iMsg, wParam, lParam ) ;
}

void 
horizonMenu::handleDebugLoggingEvent( void )
{
	MENUITEMINFO mii ;
	mii.cbSize = sizeof( MENUITEMINFO ) ;
	mii.fMask = MIIM_STATE ;
	
	if ( GetMenuItemInfo( m_hmenu, ID_DEBUG_LOGGING, FALSE, &mii ) )
	{
		if ( mii.fState & MFS_CHECKED )
		{
			mii.fState = MFS_UNCHECKED ;
			vnclog.SetLevel( LL_NONE ) ;
		}
		else
		{
			mii.fState = MFS_CHECKED ;
			vnclog.SetLevel( LL_ALL ) ;
		}

		// update checkmark
		SetMenuItemInfo( m_hmenu, ID_DEBUG_LOGGING, FALSE, &mii ) ;
	}
	else
	{
		vnclog.Print( LL_INTERR, VNCLOG( " GetMenuItemInfo() => %d\n" ), GetLastError() ) ;
	}
	
	return ;
}

void 
horizonMenu::handleTrayNotify( LPARAM lParam )
{
	// Get the submenu to use as a pop-up menu
	HMENU submenu = GetSubMenu( m_hmenu, 0 ) ;

	//
	// What event are we responding to?
	// an RMB click -or- an LMB double click?
	//
	
	if ( lParam == WM_RBUTTONUP )
	{
		if ( submenu == NULL )
		{ 
			vnclog.Print( LL_INTERR, VNCLOG( "unable to get systray submenu\n" ) ) ;
			return ;
		}

		// Make the first menu item the default (bold font)
		SetMenuDefaultItem( submenu, 0, TRUE ) ;
		
		// Get the current cursor position, to display the menu at
		POINT mouse ;
		GetCursorPos( &mouse ) ;

		// There's a "bug"
		// (Microsoft calls it a feature) in Windows 95 that requires calling
		// SetForegroundWindow. To find out more, search for Q135788 in MSDN.
		SetForegroundWindow( m_nid.hWnd ) ;

		// display the menu where the mouse action was registered
		TrackPopupMenu(
			submenu,
			0, mouse.x, mouse.y, 0,
			m_nid.hWnd, NULL
		) ;

		return ;
	}
	else if ( lParam == WM_LBUTTONDBLCLK )
	{
		// double-click displays the properties dialog
		if ( AllowProperties() )
		{
			SendMessage(
				m_nid.hWnd,
				WM_COMMAND, 
				ID_PROPERTIES,
				0
			) ;
		}
	}

	return ;
}

bool 
horizonMenu::handleCustomMessages( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
	//
	// handle add client mesage caused by '-connect' flag
	// and/or calls to vncService::PostAddNewClient() 
	//
	
	// this is the only message we handle right now
	if ( iMsg != APPSHARE_ADD_CLIENT_MSG )
		return false ;
	
	// if we have a port and adddress, set the liveshare connection info
	if ( ! lParam || ! wParam )
	{
	
		return false ;
	}

	// get the ip address
	struct in_addr host ;
	host.S_un.S_addr = lParam ;

	horizonConnect::GetInstance()->SetConnectionInfo(
		static_cast<const char*>( inet_ntoa( host ) ),
		static_cast<unsigned short>( wParam )
	) ;

	// display the properties dialog
	PostMessage( hwnd, WM_COMMAND, MAKELONG(ID_PROPERTIES, 0), 0 ) ;

	return true ;
}

bool
horizonMenu::ShowPropertiesDialog( void )
{
	bool rv = false ;

	if ( isBasicUser() == true ) 
		rv = m_properties->ShowBasic() ;
	else
		rv = m_properties->ShowNormal() ;

	FlashTrayIcon() ;
	
	return rv ;
}
