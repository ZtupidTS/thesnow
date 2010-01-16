
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonMenu.h 1933 2004-11-12 15:14:50Z jeffo3 $

class horizonMenu ;

#ifndef __HORIZONMENU_H
#define __HORIZONMENU_H

#include "stdhdrs.h"

#include <lmcons.h>

#include <string>
using std::string ;

#include "WinVNC.h"

#include "vncAbout.h"
#include "vncServer.h"
#include "vncService.h"

#include "horizonConnect.h"
#include "horizonProperties.h"
#include "vncServerSingleton.h"

#include "WallpaperUtils.h"

//
// custom messages
//

extern const char* MENU_CLASS_NAME ;

// used by appshare
extern const UINT APPSHARE_ADD_CLIENT_MSG ;

// not used by appshare
extern const UINT MENU_ADD_CLIENT_MSG ;
extern const UINT MENU_SERVER_SHAREWINDOW ;
extern const UINT MENU_PROPERTIES_SHOW ;
extern const UINT MENU_CONTROL_PANEL_SHOW ;
extern const UINT MENU_DEFAULT_PROPERTIES_SHOW ;
extern const UINT MENU_ABOUTBOX_SHOW ;
extern const UINT MENU_SERVICEHELPER_MSG ;
extern const UINT MENU_RELOAD_MSG ;
extern const UINT MENU_ADD_CLIENT_MSG ;
extern const UINT MENU_KILL_ALL_CLIENTS_MSG ;

extern const UINT fileTransferDownloadMessage ;

//
// hard-coded values
//

static const int TIMER_INTERVAL = 5000 ;

//
// class definition
//

class horizonMenu
{
public:
	static horizonMenu* GetInstance( void ) ;

	bool Start( void ) ;
	bool Shutdown( void ) ;

	// show dialog functions
	bool ShowPropertiesDialog( void ) ;

	// Message handler for the tray window
	static LRESULT CALLBACK WndProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam ) ;

	//
	// public user type ( should be it's own class )
	//
	
	enum UserType { NORMAL = 0, BASIC } ;

	void SetUserType( UserType type ) { m_usertype = type ; }
	UserType getUserType( void ) { return m_usertype ; }

	bool isNormalUser( void ) { return m_usertype == NORMAL ; }
	bool isBasicUser( void ) { return m_usertype == BASIC ; }
	
	// user type questions
	bool AllowProperties() { return ( m_usertype == BASIC ) ? false : true ; } ;

private:
	horizonMenu() ;
	~horizonMenu() ;

	horizonMenu( const horizonMenu& rhs ) ;
	const horizonMenu& operator=( const horizonMenu& rhs ) ;

	static horizonMenu* m_instance ;
	
	//
	// systray menu methods
	//

	// send message to the systray menu
	void SendTrayMsg( DWORD msg, BOOL flash = FALSE ) ;

	// Tray icon handling
	void AddTrayIcon( void ) ;
	void DelTrayIcon( void ) ;
	void FlashTrayIcon( void ) ;

	// tray state flag
	bool m_is_started ;

	//
	// systray icon handles
	//
	
	HICON m_winvnc_icon ;
	HICON m_winvnc_normal_icon ;
	HICON m_winvnc_disabled_icon ;
	HICON m_flash_icon ;
	
	// window and menu handles
	HWND m_hwnd ;
	HMENU m_hmenu ;

	// carries message from object to window callback
	NOTIFYICONDATA m_nid ;

	//
	// message handlers
	//

	void handleDebugLoggingEvent( void ) ;
	void handleTrayNotify( LPARAM lParam ) ;
	bool handleCustomMessages( HWND hwnd, UINT iMsg, WPARAM wparam, LPARAM lParam ) ;

	//
	// local pointers
	//

	// singletons
	vncServer* m_server ;
	horizonProperties* m_properties ;

	// about dialog for this server
	vncAbout m_about ;	

	// user type
	UserType m_usertype ;

} ;


#endif // __HORIZONMENU_H
