//////////////////////////////////////////////////////
//
// Projet TFTPD32.  April 2007 Ph.jounin
//                  A free TFTP server for Windows
// File ip_thread.c: periodically checks server interfaces staus
//
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "../_common/headers.h"
#include "threading.h"

static struct S_IPAddressList old_if;

// called by console thread
int AnswerIPList (void)
{
   // change old_if structure 
   old_if.nb_addr++;
   // wake up scheduler thread 
   WakeUpThread (TH_SCHEDULER);
return 0;
} // 

// -----------------------
// called by scheduler
// -----------------------
int PoolNetworkInterfaces (void)
{
struct S_IPAddressList        new_if;
int    Ark;
char  szMySelf [128];
struct hostent *host;

   // list all interfaces
   memset (& new_if, 0, sizeof new_if);
   if ( gethostname (szMySelf, sizeof szMySelf) == 0 )
   {
	   host = gethostbyname (szMySelf);
	   if (host != NULL  &&  host->h_addr_list!=NULL)
	   {
			for ( Ark=0 ; Ark<MAX_IP_ITF && host->h_addr_list[Ark]!=NULL ;  Ark++)
				new_if.ent[Ark].address = * (struct in_addr *) host->h_addr_list[Ark];
			new_if.nb_addr = Ark;
	   } // gethostbyname OK
   } // gethostname OK

   // signal a change
   if (memcmp (&new_if, &old_if, sizeof old_if)!=0)
   {
	   old_if = new_if ;
	   SendMsgRequest (  C_REPLY_GET_INTERFACES, 
						& old_if, 
						  sizeof old_if,
						  FALSE,
						  FALSE );
   }
return TRUE;
} // PoolNetworkInterfaces



// ---------------------------------------------------------------
//a thread which wake up periodically to check interfaces status
// ---------------------------------------------------------------
void Scheduler (void *param)
{
    do
    {
		// wake up every 30 seconds
        WaitForSingleObject (tThreads[TH_SCHEDULER].hEv, 30000);

		PoolNetworkInterfaces ();
    }
    while ( tThreads[TH_SCHEDULER].gRunning );

	LogToMonitor ("end of ip pooling thread\n");
_endthread ();        
} // ListIPInterfaces


