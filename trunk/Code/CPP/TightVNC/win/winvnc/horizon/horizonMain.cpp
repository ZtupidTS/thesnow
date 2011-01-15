
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonMain.cpp 2011 2005-05-05 19:59:49Z jeffo3 $

#ifdef USE_ALT_SOCKETS
	// see horizonMainAlt.cpp
#else

#include "horizonMain.h"
#include "vncServerSingleton.h"

//
// main
//

int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	PSTR szCmdLine, 
	int iCmdShow
)
{
#ifdef _DEBUG
	{
		// Get current flag
		int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );

		// Turn on leak-checking bit
		tmpFlag |= _CRTDBG_LEAK_CHECK_DF;

		// Set flag to the new value
		_CrtSetDbgFlag( tmpFlag );
	}
#endif

	//
	// setup logging
	//

	vnclog.SetFile( hzLogFileName, true ) ;
	vnclog.SetLevel( hzLogLevel ) ;
	vnclog.SetMode( hzLogMode ) ;
	vnclog.SetStyle( Log::TIME_INLINE ) ;

	vnclog.Print( LL_ALL, VNCLOG( "starting %s, file => %s, level => %d, mode => %d\n" ), 
		szAppName, hzLogFileName, hzLogLevel, hzLogMode ) ;

	//
	// init globals
	//
	
	hAppInstance = hInstance ;
	mainthreadId = GetCurrentThreadId() ;

	//
	// parse the command-line args
	//
	
	// use this instead for lpCmdLine since lpCmdLine may be truncated
	string args = GetCommandLine() ;

	// check for -connect arg
	if ( args.find( "-connect" ) == string::npos )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to find -connect argument\n" ) ) ;

		// print the usage
		AppShareUsage( args ) ;
		
		return 0 ;
	}

	// initialize the socket system
	VSocketSystem socksys ;

	// see if the socket system was initialized
	if ( ! socksys.Initialised() )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to initialize socket system\n" ) ) ;
		
		MessageBox( NULL, "Unable to initialize socket system.", 
			szAppName, MB_OK | MB_ICONERROR ) ;
			
		return 0 ;
	}

	//
	// start appshare
	//
	
	int rv = AppShareMain( args ) ;
	
	return rv ;
}

//
// main appshare functions
//

// This is the main routine for WinVNC when running as an application
// (under Windows 95 or Windows NT)
// Under NT, WinVNC can also run as a service.  The WinVNCServerMain routine,
// defined in the vncService header, is used instead when running as a service.

int AppShareMain( const string& args )
{
	vnclog.Print( LL_STATE, VNCLOG( "entering AppShareMain()\n" ) ) ;

	// Check for previous instances of WinVNC
	vncInstHandler instance ;
	
	if ( ! instance.Init() )
	{
		vnclog.Print( LL_NONE, VNCLOG( "an instance of AppShare is already running\n" ) ) ;

		MessageBox( NULL, "An instance of AppShare is already running.", 
			szAppName, MB_OK | MB_ICONERROR ) ;

		return -1 ;
	}

	//
	// write out the pid
	//
	
	ofstream pidf ;
	
	pidf.open( hzPIDFileName, ofstream::out | ofstream::trunc ) ;

	if ( pidf.is_open() == false ) 
	{
		vnclog.Print(
			LL_INTERR, 
			VNCLOG( "unable to open pid file, filename => %s\n" ),
			hzPIDFileName
		) ;
		
		MessageBox( NULL, "Unable to record process id.", 
			szAppName, MB_OK| MB_ICONERROR ) ;

		return -1 ;
	}

	pidf << GetCurrentProcessId() << std::endl ;
	pidf.close() ;

	vnclog.Print( LL_STATE, VNCLOG( "wrote pid to file, file => %s, pid => %d\n" ),
		hzPIDFileName, GetCurrentProcessId() ) ;

	//
	// set this process to be the last application to be shut down
	//
	
	SetProcessShutdownParameters( 0x100, SHUTDOWN_NORETRY ) ;

	//
	// entering main application code
	//
	
	horizonDefaults::InitializeApplication() ;

	vnclog.Print( LL_STATE, VNCLOG( "initialized application\n" ) ) ;

	// setup the systray menu
	horizonMenu* menu = horizonMenu::GetInstance() ;

	// determine user type
	horizonMenu::UserType user_type = ( args.find( "-basic" ) != string::npos ) 
		? horizonMenu::BASIC 
		: horizonMenu::NORMAL
	;

	// set the user type
	menu->SetUserType( user_type ) ;

	if ( menu->Start() == false )
	{
		vnclog.Print( 
			LL_INTERR, 
			VNCLOG( "unable to open install appshare menu\n" ) 
		) ;
		
		MessageBox( NULL, "Unable to install AppShare menu.", 
			szAppName, MB_OK| MB_ICONERROR ) ;

		return -1 ;
	}

	vnclog.Print( LL_STATE, VNCLOG( "install systray menu\n" ) ) ;

	//
	// make connection to server
	//

	if ( ConnectToServer( args ) == false )
	{
		vnclog.Print( LL_STATE, VNCLOG( "unable to connect to server\n" ) ) ;
		return -1 ;
	}

	//
	// main event loop
	//

	vnclog.Print( LL_STATE, VNCLOG( "entering main message loop\n" ) ) ;

	MSG msg ;
		
	while ( GetMessage( &msg, NULL, 0,0 ) )
	{
		TranslateMessage( &msg ) ;
		DispatchMessage( &msg ) ;
	}

	vnclog.Print( LL_STATE, VNCLOG( "exited main message loop\n" ) ) ;

	// remove systray menu
	if ( menu->Shutdown() == true )
		vnclog.Print( LL_STATE, VNCLOG( "successfully shutdown processing thread\n" ) ) ;			
	else
		vnclog.Print( LL_STATE, VNCLOG( "unable shutdown processing thread\n" ) ) ;

	// release the instance mutex
	instance.Release() ;

	vnclog.Print( LL_STATE, VNCLOG( "exiting AppShareMain()\n" ) ) ;

	return 0 ;
}

bool
ConnectToServer( const string& args )
{
	//
	// find connect flag
	//

	size_t p1 ;
	size_t p2 ;
	
	if ( ( p1 = args.find( "-connect" ) ) == string::npos )
	{
		vnclog.Print( LL_STATE, VNCLOG( "unable to find '-connect' flag\n" ) ) ;
		AppShareUsage( args ) ;
		return false ;
	}

	//
	// make client connection
	//
	
	// determine hostname
	p1 = args.find_first_of( " ", p1 ) ;
	p2 = args.find_first_of( ":", p1 ) ;
	
	if ( p1 == string::npos || p2 == string::npos )
	{
		AppShareUsage( args ) ;
		return false ;
	}

	string hostname = args.substr( p1 + 1, p2 - p1 - 1 ) ;

	// determine the port
	p1 = args.find_first_of( " ", p2 ) ;
	
	if ( p1 == string::npos )
		p1 = args.length() ;
	
	string port = args.substr( p2 + 1 , p1 - p2 - 1 ) ;
	
	//
	// resolve the hostname
	//
	
	// resolve address
	VCard32 host_x = VSocket::Resolve( hostname.c_str() ) ;
	
	if ( host_x == 0 )
	{
		vnclog.Print( 
			LL_INTERR, 
			VNCLOG( "unable to resolve hostname, hostname => %s\n" ),
			hostname.c_str()
		) ;
		
		MessageBox( NULL, "Unable to resolve hostname.", 
			szAppName, MB_OK | MB_ICONERROR ) ;
		
		return false ;
	}

	//
	// convert display into port
	//

	unsigned short port_x = static_cast<unsigned short>( 
		atoi( port.c_str() ) + INCOMING_PORT_OFFSET ) ;

	//
	// tell the running instance to connect to the host and port
	//
			
	vnclog.Print( LL_STATE, VNCLOG( "sending connect message, hostname => %s, port => %s\n" ),
		hostname.c_str(), port.c_str() ) ;

	// ( PostToWinVNC is defined in vncService.h )
	BOOL posted = PostToWinVNC( 
		APPSHARE_ADD_CLIENT_MSG, 
		( WPARAM )( port_x ), 
		( LPARAM )( host_x ) 
	) ;
	
	// Post to the WinVNC menu window
	if ( posted == FALSE )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to connect to AppShare instance\n" ) ) ;
		
		MessageBox( NULL, "Unable to connect to AppShare instance.", 
			szAppName, MB_OK| MB_ICONERROR ) ;

		return false ;
	}

	vnclog.Print( LL_STATE, VNCLOG( "sent connect message to existing instance\n" ) ) ;

	return true ;
}

void 
AppShareUsage( const string& args )
{
	vnclog.Print( LL_NONE, VNCLOG( "printing usage, args => %s\n" ), args.c_str() ) ;

	MessageBox( NULL, "appshare [ -basic ] [ -connect host:port ]", 
		szAppName, MB_OK | MB_ICONINFORMATION ) ;

	return ;
}

#endif // USE_ALT_SOCKETS
