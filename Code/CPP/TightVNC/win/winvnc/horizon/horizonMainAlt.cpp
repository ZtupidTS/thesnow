
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonMainAlt.cpp 2010 2005-05-05 19:58:03Z jeffo3 $

#ifdef USE_ALT_SOCKETS

#include "horizonMain.h"

#include "horizonDoorReader.h"
#include "horizonDoorWriter.h"

#include "VSocketSystemAlt.h"
#include "VSocketAlt.h"

#include <cstring>
#include <iostream>
#include <string>


//
// local functions declarations
//

// main functions
void AppShareUsage( const string& args ) ;
int AppShareMain( const string& args ) ;
bool ConnectToServer( const string& args ) ; 

// utility functions
void getCommandLineArgs( string& args, int& argc, char**& argv ) ;
bool getHostAndPort( const string& args, string& host, string& port ) ;

//
// main functions
//

int WINAPI WinMain(
	HINSTANCE hInstance, 
	HINSTANCE hPrevInstance, 
	PSTR szCmdLine, 
	int iCmdShow
)
{
	// first, start the door reader and writer threads
	horizonDoorReader::Start() ;
	horizonDoorWriter::Start() ;

	// first, let the caller know we're initializing	
	horizonDoorWriter::Report( "agent_initializing" ) ;
	
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
	vnclog.SetMode( hzLogMode | Log::ToStderr ) ;
	vnclog.SetStyle( Log::TIME_INLINE ) ;

	vnclog.Print( LL_ALL, VNCLOG( "starting %s, file => %s, level => %d, mode => %d\n" ), 
		szAppName, hzLogFileName, hzLogLevel, hzLogMode ) ;

	//
	// init globals
	//
	
	hAppInstance = hInstance ;
	mainthreadId = GetCurrentThreadId() ;

	//
	// get the command-line arguments
	//
	
	string args ; 

	int argc = 0 ;
	char** argv = 0 ;

	getCommandLineArgs( args, argc, argv ) ;

	//
	// initialize the vsocket subsystem
	//
	
	// initialize the socket system
	VSocketSystem socksys( args, argc, argv ) ;

	// see if the socket system was initialized
	if ( ! socksys.Initialised() )
	{
		vnclog.Print( LL_INTERR, VNCLOG( "unable to initialize socket system\n" ) ) ;
		horizonDoorWriter::ReportFatalError( "Unable to initialize socket system." ) ;
		return 0 ;
	}

	//
	// start appshare-specific code
	//
	
	int rv = AppShareMain( args ) ;

	// last, stop the door reader and writer threads
	horizonDoorReader::Stop() ;
	horizonDoorWriter::Stop() ;

	return rv ;
}

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
		horizonDoorWriter::ReportFatalError( "An instance of AppShare is already running." ) ;
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
		horizonDoorWriter::ReportFatalError( "Unable to record process id." ) ;
		return -1 ;
	}

	pidf << GetCurrentProcessId() << std::endl ;
	pidf.close() ;

	vnclog.Print( LL_STATE, VNCLOG( "successfully wrote pid to file, file => %s, pid => %d\n" ),
		hzPIDFileName, GetCurrentProcessId() ) ;

	//
	// application setup
	//
	
	// set this process to be the last application to be shut down	
	SetProcessShutdownParameters( 0x100, SHUTDOWN_NORETRY ) ;

	horizonDefaults::InitializeApplication() ;

	vnclog.Print( LL_STATE, VNCLOG( "successfully initialized application defaults\n" ) ) ;

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
		horizonDoorWriter::ReportFatalError( "Unable to install AppShare menu." ) ;
		return -1 ;
	}

	vnclog.Print( LL_STATE, VNCLOG( "successfully installed systray menu\n" ) ) ;

	// make connection to server
	if ( ConnectToServer( args ) == false )
	{
		vnclog.Print( LL_STATE, VNCLOG( "unable to connect to server\n" ) ) ;
		horizonDoorWriter::ReportFatalError( "Unable to connect to server." ) ;
		return -1 ;
	}
	
	vnclog.Print( LL_STATE, VNCLOG( "successfully connected to server\n" ) ) ;


	// let the caller know we're initialzed	
	horizonDoorWriter::Report( "agent_initialized" ) ;

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

	//
	// clean-up
	//
	
	// now, let the caller know we're terminating
	horizonDoorWriter::Report( "agent_terminating" ) ;	

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
	string hostname ;
	string port ;

	// get hostname and port
	getHostAndPort( args, hostname, port ) ;
	
	// resolve address
	VCard32 host_x = VSocket::Resolve( hostname.c_str() ) ;
	
	if ( host_x == 0 )
	{
		vnclog.Print( 
			LL_INTERR, 
			VNCLOG( "unable to resolve hostname, hostname => %s\n" ),
			hostname.c_str()
		) ;
		
		horizonDoorWriter::ReportFatalError( "Unable to resolve hostname." ) ;
		
		return false ;
	}

	//
	// convert display into port
	//

	unsigned short port_x = static_cast<unsigned short>( atoi( port.c_str() ) ) ;

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
		horizonDoorWriter::ReportFatalError( "Unable to connect to AppShare instance." ) ;
		return false ;
	}

	vnclog.Print( LL_STATE, VNCLOG( "sent connect message to existing instance\n" ) ) ;

	return true ;
}

//
// usage
//

void 
AppShareUsage( const string& args )
{
	vnclog.Print( LL_NONE, VNCLOG( "printing usage, args => %s\n" ), args.c_str() ) ;
	horizonDoorWriter::ReportFatalError( "appshare [ -basic ] [ -connect host:port ]\n" ) ;
	return ;
}

//
// utility functions
//

void
getCommandLineArgs( string& args, int& argc, char**& argv )
{
	args = GetCommandLine() ;

	// get vector of args
	vector< string > arg_vector ;
	size_t hp1 = 0, hp2 = 0 ;
	size_t hpq = 0 ;
	
	while ( ( hp2 = args.find( ' ', hp1 ) ) != string::npos )
	{
		// account for quotes		
		if ( 
			( ( hpq = args.rfind( '"', hp2 ) ) != string::npos )
			&& ( hpq >= hp1 ) 
		)
		{
			hp2 = args.find( '"', hp2 ) ;
		}

		// only push non-zero-sized args
		if ( hp2 - hp1 > 0 )
			arg_vector.push_back( args.substr( hp1, hp2 - hp1 ) ) ;
		
		hp1 = hp2 + 1 ;
	}
	
	// push the last arg
	if ( hp1 < args.size() )
		arg_vector.push_back( args.substr( hp1 ) ) ;
	
	// allocate argv, argc
	argc = arg_vector.size() ;
	argv = new char*[ argc ] ;
	
	// copy the args
	for ( int i = 0 ; i < argc ; ++i )
	{
		argv[i] = new char[ ( arg_vector[i] ).size() + 1 ] ;
		strcpy( argv[i], ( arg_vector[i] ).c_str() ) ;
	}

	return ;
}

bool
getHostAndPort( const string& args, string& hostname, string& port )
{
	//
	// find connect flag
	//

	size_t p1 ;
	size_t p2 ;
	
	if ( ( p1 = args.find( "-connect" ) ) == string::npos )
		return false ;

	//
	// make client connection
	//
	
	// determine hostname
	p1 = args.find_first_of( " ", p1 ) ;
	p2 = args.find_first_of( ":", p1 ) ;
	
	if ( p1 == string::npos || p2 == string::npos )
		return false ;

	hostname = args.substr( p1 + 1, p2 - p1 - 1 ) ;

	// determine the port
	p1 = args.find_first_of( " ", p2 ) ;
	
	if ( p1 == string::npos )
		p1 = args.length() ;
	
	port = args.substr( p2 + 1 , p1 - p2 - 1 ) ;

	return true ;
}

#endif // USE_ALT_SOCKETS
