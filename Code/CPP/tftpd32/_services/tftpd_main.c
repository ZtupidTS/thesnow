//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - June 2006
// File tftp.c:   worker threads management
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


// number of permanent worker threads
#define TFTP_PERMANENTTHREADS 2

#define TFTP_MAXTHREADS     100

#undef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))


// #define DEB_TEST

#include "../_common/headers.h"
#include <process.h>
#include <stdio.h>

#include "threading.h"
#include "tftpd_functions.h"

// First item -> structure belongs to the module and is shared by all threads
struct LL_TftpInfo *pTftpFirst;

// void StartTftpTransfer (void *pThreadArgs);  if called by _beginthread
DWORD WINAPI StartTftpTransfer (LPVOID pThreadArgs);
void TftpdMain (void *param);
void TftpdConsole (void *param);

#ifdef OLD
/////////////////////
// is a file handle open ? Ask before terminating app if a transfer is in progress
/////////////////////
BOOL IsActiveTransfer ()
{
struct LL_TftpInfo *pTftp;
  for ( pTftp=pTftpFirst ; pTftp!=NULL  && pTftp->r.hFile==INVALID_HANDLE_VALUE;  pTftp=pTftp->next );
return pTftp!=NULL; // we do not have parse all list, a transfer is active
} // IsActiveTransfer
#endif


////////////////////////////////////////////////////////////
// TFTP daemon --> Runs at main level
////////////////////////////////////////////////////////////
struct S_TftpdParam
{
    SOCKET  sListenerSocket;    // socket on port 69

}; // S_TftpdParam ;



void PopulateTftpdStruct (struct LL_TftpInfo *pTftp)
{
struct LL_TftpInfo *pTmp;
static DWORD TransferId=467;    // unique identifiant

    // init or reinit struct
    pTftp->s.dwTimeout = sSettings.Timeout;
    pTftp->s.dwPacketSize = TFTP_SEGSIZE;  // default
    pTftp->r.skt = INVALID_SOCKET;
    pTftp->r.hFile = INVALID_HANDLE_VALUE;
    pTftp->c.bMCast = FALSE;    // may be updated later
	pTftp->c.nOAckPort = 0;		// use classical port for OAck
    pTftp->tm.dwTransferId = TransferId++;

    // init statistics
    memset (& pTftp->st, 0, sizeof pTftp->st);
    time (& pTftp->st.StartTime);
    pTftp->st.dLastUpdate = pTftp->st.StartTime;
	pTftp->st.ret_code = TFTP_TRF_RUNNING;
    // count the transfers (base 0)
    for ( pTftp->st.dwTransfert=0, pTmp = pTftpFirst->next ;
          pTmp!=NULL ;
          pTmp = pTmp->next, pTftp->st.dwTransfert++ ) ;
    LOG (9, "Transfert #%d", pTftp->st.dwTransfert);

   // init MD5 structure
    pTftp->m.bInit = sSettings.bMD5;

    // clear buffers
    memset (& pTftp->b, 0, sizeof pTftp->b);
} // PopulateTftpdStruct

// Suppress structure item
struct LL_TftpInfo *TftpdDestroyThreadItem (struct LL_TftpInfo *pTftp)
{
struct LL_TftpInfo *pTmp=pTftp;

    LOG (9, "thread %d has exited", pTftp->tm.dwThreadHandle);

LogToMonitor ("removing thread %d (%p/%p/%p)\n", pTftp->tm.dwThreadHandleId, pTftp, pTftpFirst, pTftpFirst->next);
    // do not cancel permanent Thread
    if (! pTftp->tm.bPermanentThread )
    {
        if (pTftp!=pTftpFirst)
        {
              // search for the previous struct
            for (pTmp=pTftpFirst ; pTmp->next!=NULL && pTmp->next!=pTftp ; pTmp=pTmp->next);
            pTmp->next = pTftp->next;   // detach the struct from list
        }
        else pTftpFirst = pTmp = pTftpFirst->next;

        memset (pTftp, 0xAA, sizeof *pTftp); // fill with something is a good debugging tip
        free (pTftp);
    }

return pTmp;	// pointer on previous item
} // TftpdDestroyThreadItem


// --------------------------------------------------------
// Filter incoming request
// add-on created on 24 April 2008
// return TRUE if message should be filtered
// --------------------------------------------------------
int TftpMainFilter (struct sockaddr_in *from, char *data, int len)
{
static char LastMsg[PKTSIZE];
static int  LastMsgSize;
static time_t LastDate;
static struct sockaddr_in LastFrom;

	if (len > PKTSIZE) return TRUE;	// packet should really be dropped
	// test only duplicated packets
	if (    len==LastMsgSize  
		&&  memcmp (data, LastMsg, len)==0
		&&  LastFrom.sin_addr.s_addr == from->sin_addr.s_addr
		&&  LastFrom.sin_port == from->sin_port
		&&  LastFrom.sin_family == from->sin_family
		&&  time (NULL) == LastDate )
	{
		LOG (1, "Warning : received duplicated request from %s:%d", 
			     inet_ntoa (from->sin_addr), 
				 htons (from->sin_port) );
		Sleep (250);	// time for the first TFTP thread to start
		return FALSE;	// accept message nevertheless
	}
	// save last frame

	LastMsgSize = len;
	memcpy (LastMsg, data, len);
	LastFrom = *from;
	time (&LastDate);
	return FALSE; // packet is OK
} // TftpMainFilter


// activate a new thread and pass control to it 
int TftpdChooseNewThread (SOCKET sListenerSocket)
{
struct LL_TftpInfo *pTftp, *pTmp;
int             fromlen;
int             bNewThread;
int             Rc;
int             nThread=0;

    for (  pTmp = pTftpFirst ;  pTmp!=NULL ;  pTmp = pTmp->next)   nThread++;
    if (nThread >= TFTP_MAXTHREADS)
    {char dummy_buf [PKTSIZE];
     struct sockaddr_in  from;
        fromlen = sizeof from;
        // Read the connect datagram to empty queue
        Rc = recvfrom (sListenerSocket, dummy_buf, sizeof dummy_buf, 0,
                       (struct sockaddr *) & from, & fromlen);
        if (Rc>0) 
               LOG (1, "max number of threads reached, connection from %s dropped", inet_ntoa (from.sin_addr) );
        return -1;
    }

    // search a permanent thread in waiting state
    for (pTftp=pTftpFirst ; pTftp!=NULL  ; pTftp=pTftp->next)
        if ( pTftp->tm.bPermanentThread  &&  ! pTftp->tm.bActive )  break;
    bNewThread = (pTftp==NULL);

    if (bNewThread)
    {
        // search for the last thread struct
        for ( pTftp=pTftpFirst ;  pTftp!=NULL && pTftp->next!=NULL ; pTftp=pTftp->next );
        if (pTftp==NULL)   pTftp=pTftpFirst =calloc (1, sizeof *pTftpFirst);
        else               pTftp=pTftp->next=calloc (1, sizeof *pTftpFirst);
        // note due the calloc if thread has just been created
        //   pTftp->tm.dwThreadHandle == NULL ;
        pTftp->next = NULL ;
    }

    PopulateTftpdStruct (pTftp);

    // Read the connect datagram (since this use a "global socket" port 69 its done here)
    fromlen = sizeof pTftp->b.cnx_frame;
    Rc = recvfrom (sListenerSocket, pTftp->b.cnx_frame, sizeof pTftp->b.cnx_frame, 0,
               (struct sockaddr *)&pTftp->b.from, &fromlen);
    if (Rc < 0)
    {
        // the Tftp structure has been created --> suppress it
        LOG (0, "Error : RecvFrom returns %d: <%s>", WSAGetLastError(), LastErrorText());
        if (! pTftp->tm.bPermanentThread )
        {
            // search for the last thread struct
            for ( pTmp=pTftpFirst ;  pTmp->next!=pTftp ; pTmp=pTmp->next );
            pTmp->next = pTftp->next ; // remove pTftp from linked list
            free (pTftp);
        }
    }
	// should the message be silently dropped
    else if (TftpMainFilter (& pTftp->b.from, pTftp->b.cnx_frame, Rc))
	{
        // the Tftp structure has been created --> suppress it
        LOG (1, "Warning : Unaccepted request received from %s", inet_ntoa (pTftp->b.from.sin_addr));
        if (! pTftp->tm.bPermanentThread )
        {
            // search for the last thread struct
            for ( pTmp=pTftpFirst ;  pTmp->next!=pTftp ; pTmp=pTmp->next );
            pTmp->next = pTftp->next ; // remove pTftp from linked list
            free (pTftp);
        }
	}
	else	// message is accepted
    {
        LOG (1, "Connection received from %s on port %d",
                inet_ntoa (pTftp->b.from.sin_addr), ntohs (pTftp->b.from.sin_port) );
#if (defined DEBUG || defined DEB_TEST)
        BinDump (pTftp->b.cnx_frame, Rc, "Connect:");
#endif		

        // mark thread as started (will not be reused)
        pTftp->tm.bActive=TRUE ;

        // start new thread or wake up permanent one
        if (bNewThread)
        {
            // create the worker thread
            // pTftp->tm.dwThreadHandle = (HANDLE) _beginthread (StartTftpTransfer, 8192, (void *) pTftp);
            pTftp->tm.dwThreadHandle = CreateThread (NULL,
                                                     8192,
                                                     StartTftpTransfer,
                                                     pTftp,
                                                     0,
                                                     & pTftp->tm.dwThreadHandleId);
LogToMonitor ("Thread %d transfer %d started (records %p/%p)\n", pTftp->tm.dwThreadHandleId, pTftp->tm.dwTransferId, pTftpFirst, pTftp);
            LOG (9, "thread %d started", pTftp->tm.dwThreadHandle);

        }
        else                 // Start the thread
        {
    LogToMonitor ("waking up thread %d for transfer %d\n",
                   pTftp->tm.dwThreadHandleId,
                   pTftp->tm.dwTransferId );
            if (pTftp->tm.hEvent!=NULL)       SetEvent (pTftp->tm.hEvent);
        }
       // Put the multicast hook which adds the new client if the same mcast transfer
       // is already in progress

    } // recv ok --> thread has been started

return TRUE;
} // TftpdStartNewThread


void SendStatsToGui (void)
{
static struct S_TftpTrfStat sMsg;
struct LL_TftpInfo *pTftp;
int Ark;

   for ( Ark=0,  pTftp=pTftpFirst ;  Ark<SizeOfTab(sMsg.t)  &&  pTftp!=NULL ; pTftp=pTftp->next )
   {
      if (pTftp->tm.bActive )
      {
          sMsg.t[Ark].dwTransferId = pTftp->tm.dwTransferId;
          sMsg.t[Ark].stat = pTftp->st;
          Ark++ ;
      }
   }
   sMsg.nbTrf = Ark;
   time (& sMsg.dNow);
   //if (Ark>0)
        SendMsgRequest (  C_TFTP_TRF_STAT, 
                        & sMsg , 
                          sMsg.nbTrf * sizeof (sMsg.t[0]) + offsetof (struct S_TftpTrfStat, t[0]),
						  TRUE,		// block thread until msg sent
					      FALSE );		// if no GUI return
} // SendStatsToGui


////////////////////////////////////////////////////////////
// Init TFTP daemon
////////////////////////////////////////////////////////////

int CreatePermanentThreads (void)
{
int Ark;
struct LL_TftpInfo *pTftp;


    // inits socket
    ////////////////////////////////////////////
    // Create the permanents threads
    for ( Ark=0  ; Ark < TFTP_PERMANENTTHREADS ; Ark++ )
    {
        if (pTftpFirst==NULL)  pTftp=pTftpFirst= calloc (1, sizeof *pTftpFirst);
        else                   pTftp=pTftp->next=calloc (1, sizeof *pTftpFirst);
        pTftp->next = NULL;
        pTftp->tm.bPermanentThread = TRUE;
        pTftp->tm.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        pTftp->tm.N = Ark+1 ;
        //pTftp->tm.dwThreadHandle = (HANDLE) _beginthread (StartTftpTransfer, 4096, (void *) pTftp);
        pTftp->tm.dwThreadHandle = CreateThread (NULL,
                                                 8192,
                                                 StartTftpTransfer,
                                                 pTftp,
                                                 0,
                                                 & pTftp->tm.dwThreadHandleId);
        pTftp->r.hFile=INVALID_HANDLE_VALUE ;
    }

return TRUE;
}  // CreatePermanentThreads


int TftpdCleanup (SOCKET sListenerSocket)
{
struct LL_TftpInfo *pTftp, *pTmp;
   // suspend all threads
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next )
        if (pTftp->tm.dwThreadHandle!=NULL) SuspendThread (pTftp->tm.dwThreadHandle);

    if (WSAIsBlocking ())  WSACancelBlockingCall ();   // the thread is probably blocked into a select

    // then frees resources
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTmp )
    {
        pTmp=pTftp->next ;

        if (pTftp->r.skt!=INVALID_SOCKET)          closesocket (pTftp->r.skt);
        if (pTftp->r.hFile!=INVALID_HANDLE_VALUE)  CloseHandle(pTftp->r.hFile);
        if (pTftp->tm.hEvent!=NULL)                CloseHandle(pTftp->tm.hEvent);
        free (pTftp);
    }
    Sleep (100);
   // kill the threads
    for (pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next )
        if (pTftp->tm.dwThreadHandle!=NULL) TerminateThread (pTftp->tm.dwThreadHandle, 0);

     // close main window
     closesocket (sListenerSocket);
return TRUE;
} // TftpdCleanup


// a watch dog which reset the socket event if data are available
int ResetSockEvent (SOCKET s, HANDLE hEv)
{
long dwData;
int Rc;
   Rc = ioctlsocket ( s ,  FIONREAD, & dwData);
   if (dwData==0) ResetEvent (hEv);
return Rc;   
}


// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
void TftpdMain (void *param)
{
int Rc;
int parse;
// struct S_TftpdParam *pTftpd = (struct S_TftpdParam *) param;
HANDLE hSocketEvent;
struct LL_TftpInfo *pTftp;
// events : either socket event or wake up by another thread
enum { E_TFTP_SOCK=0, E_TFTP_WAKE, E_TFTP_EV_NB };
HANDLE tObjects [E_TFTP_EV_NB];


    // creates socket and starts permanent threads
    CreatePermanentThreads ();

    // create event for the incoming Socket
    hSocketEvent = WSACreateEvent();
    WSAEventSelect (tThreads[TH_TFTP].skt, hSocketEvent, FD_READ);

     tObjects[E_TFTP_SOCK] = hSocketEvent;
     tObjects[E_TFTP_WAKE] = tThreads[TH_TFTP].hEv;


    // stop only when TFTP is stopped and all thread have returned
    while ( tThreads[TH_TFTP].gRunning  )
    {

        // waits for either incoming connection or thread event
        Rc = WaitForMultipleObjects ( E_TFTP_EV_NB,
                                      tObjects,
                                      FALSE,
                                      sSettings.dwRefreshInterval );
#ifdef RT                                      
if (Rc!=WAIT_TIMEOUT) LogToMonitor ( "exit wait, object %d\n", Rc);
#endif
        if (! tThreads[TH_TFTP].gRunning ) break;

        switch (Rc)
        {
            case E_TFTP_WAKE :   // a thread has exited
								 // update table
				do
				{
					parse=FALSE;
					for ( pTftp=pTftpFirst ; pTftp!=NULL ; pTftp=pTftp->next ) 
					{
						if  (! pTftp->tm.bPermanentThread && ! pTftp->tm.bActive )
						{
if (pTftp==NULL) { LogToMonitor ("NULL POINTER pTftpFirst:%p\n", pTftpFirst); Sleep (5000); break; }
							CloseHandle (pTftp->tm.dwThreadHandle);
							pTftp = TftpdDestroyThreadItem (pTftp);
							parse = TRUE;
							break;
						} // thread no more active
					}
				} // parse all threads (due to race conditions, we can have only one event)
				while (parse);
                break;

			// we have received a message on the port 69
            case E_TFTP_SOCK :    // Socket Msg
				WSAEventSelect (tThreads[TH_TFTP].skt, 0, 0);
                TftpdChooseNewThread (tThreads[TH_TFTP].skt);
                ResetEvent( hSocketEvent );
                WSAEventSelect (tThreads[TH_TFTP].skt, hSocketEvent, FD_READ);
                // ResetSockEvent (sListenerSocket, hSocketEvent);
               break;

            case  WAIT_TIMEOUT :
                SendStatsToGui();
                // ResetSockEvent (sListenerSocket, hSocketEvent);
                break;
            case -1 :
 LogToMonitor ( "WaitForMultipleObjects error %d\n", GetLastError() );
                LOG (1, "WaitForMultipleObjects error %d", GetLastError());
                break;
        }   // switch
    } // endless loop

    // TftpdCleanup (sListenerSocket, hSemaphore);

LogToMonitor ("signalling worker threads\n");
    /////////////////////////////////
    // wait for end of worker threads
    for (pTftp=pTftpFirst ; pTftp!=NULL  ; pTftp=pTftp->next)
    {
        if (pTftp->tm.bActive)                nak (pTftp, ECANCELLED);
        else if (pTftp->tm.bPermanentThread)  SetEvent (pTftp->tm.hEvent);
    }
LogToMonitor ("waiting for worker threads\n");

    while ( pTftpFirst != NULL )
	{
        WaitForSingleObject (pTftpFirst->tm.dwThreadHandle, 10000);
		LogToMonitor ("End of thread %d\n", pTftpFirst->tm.dwThreadHandleId);
		pTftpFirst->tm.bPermanentThread = FALSE;
        TftpdDestroyThreadItem (pTftpFirst);
	}

    WSACloseEvent (hSocketEvent);

LogToMonitor ("main TFTP thread ends here\n");
_endthread ();
} // Tftpd main thread


