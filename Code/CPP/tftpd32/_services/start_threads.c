//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File start_threads.c:  Thread management
//
// Started by the main thread
// The procedure is itself a thread
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <process.h>

#include "threading.h"



#define BOOTPD_PORT   67
#define BOOTPC_PORT   68
#define TFTP_PORT     69
#define SNTP_PORT    123
#define DNS_PORT      53
#define SYSLOG_PORT  514

const int BootPdPort = BOOTPD_PORT;
const int SntpdPort  = SNTP_PORT;
const int DnsPort    = DNS_PORT;
const int SyslogPort = SYSLOG_PORT;

static const struct S_MultiThreadingConfig
{
    char   *name;                       // name of the service
    int     serv_mask;                  // identify service into sSettings.uServices
    void ( *thread_proc )( void * );    // the service main thread
    BOOL    manual_event;               // automatic/manual reset of its event
    int     stack_size;                 // 
    int     type;                       // socket type
    char   *service;                    // the port to be bound ascii
    const int    *def_port;             // the port to be bound numerical
	int		rfc_port;					// default port taken from RFC
    char   *sz_interface;               // the interface to be opened
    int     wake_up_by_ev;              // would a SetEvent wake up the thread, FALSE if thread blocked on recvfrom
}
tThreadsConfig [] =
{
	// Order is the same than enum in threading.h
    "Console", TFTPD32_CONSOLE,       TftpdConsole,          FALSE, 16384,          0,    NULL,            NULL,  0,          NULL,                     TRUE,  
    "Registry",TFTPD32_REGISTRY,      AsyncSaveKeyBckgProc,  FALSE,  1024,          0,    NULL,            NULL,  0,          NULL,                     TRUE, 
	"Scheduler", TFTPD32_SCHEDULER,   Scheduler,             FALSE,  1024,          0,    NULL,            NULL,  0,          NULL,                     TRUE,
    "DHCP",    TFTPD32_DHCP_SERVER,   ListenDhcpMessage,     FALSE,  8192, SOCK_DGRAM,"bootps",     & BootPdPort, BOOTPD_PORT,sSettings.szDHCPLocalIP, FALSE,  
    "TFTP",    TFTPD32_TFTP_SERVER,   TftpdMain,             FALSE,  1024, SOCK_DGRAM,  "tftp", & sSettings.Port, TFTP_PORT,  sSettings.szLocalIP,      TRUE, 
    "SNTP",    TFTPD32_SNTP_SERVER,   SntpdProc,		     FALSE,  1024, SOCK_DGRAM,   "ntp",      & SntpdPort, SNTP_PORT,  "",                      FALSE, 
    "DNS",     TFTPD32_DNS_SERVER,    ListenDNSMessage,      FALSE,  1024, SOCK_DGRAM,"domain",        & DnsPort, DNS_PORT,   NULL,                    FALSE,  
    "Syslog",  TFTPD32_SYSLOG_SERVER, SyslogProc,            FALSE,  1024, SOCK_DGRAM,"syslog",     & SyslogPort, SYSLOG_PORT,  "",                    FALSE, 
};

/////////////////////////////////////////////////////////////////
// common socket operations :
//      - bind its socket
//     - send a fake message on its listen port
/////////////////////////////////////////////////////////////////

// bind the thread socket 
static SOCKET BindServiceSocket (const char *name, int type, const char *service, int def_port, int rfc_port, const char *sz_if)
{
struct sockaddr_in SockAddr;
SOCKET             sListenSocket = INVALID_SOCKET;
int                Rc;
struct servent *lpServEnt;

   sListenSocket = socket (AF_INET, type, 0);
   if (sListenSocket == INVALID_SOCKET)
   {
         SVC_ERROR ("Error : Can't create socket\nError %d (%s)", GetLastError(), LastErrorText() );
         return sListenSocket;
   }

   	// REUSEADDR option in order to allow thread to open 69 port
	if (sSettings.bPortOption && lstrcmp (service, "tftp")==0)
	{int True=1;
		Rc = setsockopt (sListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *) & True, sizeof True);
		LogToMonitor (Rc==0 ? "Port %d may be reused" : "setsockopt error", sSettings.Port);
	}

   memset (& SockAddr, 0, sizeof SockAddr);
   SockAddr.sin_family = AF_INET;
   // get the port number: read it from conf or from /etc/services files else take default port
   if (def_port != rfc_port) // config has modified port
		SockAddr.sin_port =  htons ( (short) def_port );
   else
   {	// use /etc/services
		lpServEnt = getservbyname (service, type==SOCK_DGRAM ? "udp" : "tcp") ;
		SockAddr.sin_port =  (lpServEnt != NULL) ?  lpServEnt->s_port : htons ((short) rfc_port);
   }
   // bind the socket to the active interface
   // if no interface has been specified szLocalIP is empty
   // all interfaces are activated.
   SockAddr.sin_addr.s_addr = (sz_if==NULL || sz_if[0]==0) ? INADDR_ANY : inet_addr (sz_if);
   Rc = bind (sListenSocket, (struct sockaddr *) & SockAddr, sizeof SockAddr);
   if (Rc == INVALID_SOCKET)
   {
	   // 3 causes : access violation, socket already bound, bind on an adress 
	   switch (GetLastError ())
	   {
			case WSAEADDRNOTAVAIL :   // 10049
	  		    SVC_ERROR ("Error %d\n%s\n\n"
					   "Tftpd32 tried to bind the %s port\n"
					   "to the interface %s\nwhich is not available for this host\n"
					   "Either remove the %s service or suppress %s interface assignation",
 					    GetLastError (), LastErrorText (),
						name, sz_if, name, sz_if); 
				break;
			case WSAEINVAL :
			case WSAEADDRINUSE :
	  		    SVC_ERROR ("Error %d\n%s\n\n"
					   "Tftpd32 can not bind the %s port\n"
					   "an application is already listening on this port",
 					    GetLastError (), LastErrorText (),
						name );
				break;
			default :
				SVC_ERROR ("Bind error %d\n%s",
 					    GetLastError (), LastErrorText () );
				break;
	   } // switch error type
       closesocket (sListenSocket);
	   LogToMonitor ("bind port to %s port %d failed\n", 
			  inet_ntoa (SockAddr.sin_addr),
			  htons (SockAddr.sin_port) );

        return INVALID_SOCKET;
   }
return   sListenSocket;
} // BindServiceSocket


static void FreeThreadResources (int Idx)
{
	if (tThreads[Idx].skt  != INVALID_SOCKET)       closesocket (tThreads[Idx].skt);
	if (tThreads[Idx].hEv  != INVALID_HANDLE_VALUE) CloseHandle (tThreads[Idx].hEv);
    tThreads[Idx].skt = INVALID_SOCKET;
    tThreads [Idx].hEv = INVALID_HANDLE_VALUE;
} //  FreeThreadResources (Ark);


/////////////////////////////////////////////////
// Wake up a thread :
// two methods : either use SetEvent or 
//               send a "fake" message (thread blocked on recvfrom)
/////////////////////////////////////////////////

static int FakeServiceMessage (const char *name, int type, const char *service, int def_port, const char *sz_if)
{
SOCKET  s;
struct sockaddr_in SockAddr;
struct servent *lpServEnt;
int Rc;
    s = socket (AF_INET, type, 0);
    SockAddr.sin_family = AF_INET;
    lpServEnt = getservbyname (service, type==SOCK_DGRAM ? "udp" : "tcp") ;
    SockAddr.sin_port =  (lpServEnt != NULL) ?  lpServEnt->s_port : htons ((short) def_port);
    // bind the socket to the active interface
    SockAddr.sin_addr.s_addr = inet_addr (sz_if==NULL || sz_if[0]==0 ? "127.0.0.1" : sz_if);
    Rc = sendto (s, "wake up", 8, 0, (struct sockaddr *) & SockAddr, sizeof SockAddr);
    closesocket (s);
return Rc>0;    
} // FakeServiceMessage
 
int WakeUpThread (int Idx)
{
int Rc;
   if (tThreadsConfig[Idx].wake_up_by_ev) 
   {
            Rc = SetEvent (tThreads[Idx].hEv);
			assert (Rc!=0);
   }
   else     FakeServiceMessage (tThreadsConfig[Idx].name,
                                tThreadsConfig[Idx].type,
                                tThreadsConfig[Idx].service,
                              * tThreadsConfig[Idx].def_port,
                                tThreadsConfig[Idx].sz_interface );
return TRUE;
} // WakeUpThread


// return a OR between the running threads
int GetRunningThreads (void)
{
int Ark;
int uServices = TFTPD32_NONE;
   for ( Ark=0 ;  Ark<TH_NUMBER ; Ark++ )
	   if (tThreads[Ark].gRunning)
		   uServices |= tThreadsConfig[Ark].serv_mask;
return uServices;
} // GetRunningThreads


/////////////////////////////////////////////////
// of threads life and death
/////////////////////////////////////////////////
static int StartWorkerThread (int Ark)
{
	   // first open socket
   if (tThreadsConfig[Ark].type >= SOCK_STREAM)
   {
       tThreads [Ark].gRunning  = FALSE;
       tThreads[Ark].skt = BindServiceSocket (  tThreadsConfig[Ark].name,
                                                tThreadsConfig[Ark].type,
                                                tThreadsConfig[Ark].service,
                                              * tThreadsConfig[Ark].def_port,
                                                tThreadsConfig[Ark].rfc_port,
                                                tThreadsConfig[Ark].sz_interface );
	   // on error try next thread
       if ( tThreads[Ark].skt  == INVALID_SOCKET ) return FALSE;
   }
   else tThreads[Ark].skt = INVALID_SOCKET ;

   // Create the wake up event
   if (tThreadsConfig [Ark].wake_up_by_ev )
   {
		tThreads [Ark].hEv  = CreateEvent ( NULL, tThreadsConfig [Ark].manual_event, FALSE, NULL );
		if ( tThreads [Ark].hEv == INVALID_HANDLE_VALUE )
		{
			FreeThreadResources (Ark);
			return FALSE;
		}
   }
   else tThreads [Ark].hEv = INVALID_HANDLE_VALUE ;

   // now start the thread
   tThreads [Ark].tTh  = (HANDLE) _beginthread ( tThreadsConfig [Ark].thread_proc,
                                                 tThreadsConfig [Ark].stack_size,
                                                 NULL );
   if (tThreads [Ark].tTh == INVALID_HANDLE_VALUE)
   {
		FreeThreadResources (Ark);
		return FALSE;
   }
   else
	   // all resources have been allocated --> status OK
	   tThreads [Ark].gRunning  = TRUE;
return TRUE;
} // StartWorkerThread



int StartWorkerThreads (BOOL bSoft)
{
int Ark;

  for ( Ark=0 ;  Ark<TH_NUMBER ; Ark++ )
  {
		// process mangement threads and 
		if (    ( !bSoft   &&   TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask )
			 ||   sSettings.uServices  & tThreadsConfig[Ark].serv_mask )
		{
			StartWorkerThread (Ark);
			// Pause to synchronise GUI and console
			if (Ark==0) Sleep (200); 
		} // process all threads
  }
  Sleep (200); // time to let services init
  // let the GUI run
  SendMsgRequest (C_SERVICES_STARTED, NULL, 0, FALSE, FALSE);
return TRUE;
} // StartWorkerThreads


void TerminateWorkerThreads (BOOL bSoft)
{
int Ark;
HANDLE tHdle[TH_NUMBER];
int nCount;
    for ( Ark=0, nCount=0 ;  Ark<TH_NUMBER ; Ark++ )
    {
	  // if bSoft do not kill management threads
	  if ( bSoft  &&  TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask)
		  continue;

      if (tThreads [Ark].gRunning)  
	  {
		  tThreads [Ark].gRunning = FALSE;
		  WakeUpThread (nCount);
          tHdle[nCount++] = tThreads [Ark].tTh;
	  } // if service is running
    }
    // wait for end of threads
    WaitForMultipleObjects (nCount, tHdle, TRUE, 5000);

    for ( Ark=0 ;  Ark<TH_NUMBER ; Ark++ )
    {
		if ( ! (bSoft  &&  TFTPD32_MNGT_THREADS & tThreadsConfig[Ark].serv_mask) )
				FreeThreadResources (Ark);
    }
    LogToMonitor ("all level 1 threads have returned\n");
} // TerminateWorkerThreads


