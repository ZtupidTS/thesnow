//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File threading.h:    Manage threads
//
// source released under artistic license (see license.txt)
//
//////////////////////////////////////////////////////

#include "async_log.h"


// Starts one thread per service (Tftpd, Sntpd, dhcpd)
enum e_Threads { TH_CONSOLE, 
                 TH_ASYNCSAVEKEY, 
				 TH_SCHEDULER,
				 TH_DHCP, 
				 TH_TFTP, 
				 TH_SNTP, 
				 TH_DNS,
				 TH_SYSLOG, 
				 TH_NUMBER };

// Events created for main threads
struct S_ThreadMonitoring
{
    int gRunning;       // thread status
    HANDLE tTh;         // thread handle
    HANDLE hEv;         // wake up event
    SOCKET  skt;        // Listening SOCKET
}  
tThreads [TH_NUMBER];


// threads started by StartAllThreads
void TftpdConsole (void *param);
void ListenDhcpMessage (void *param);
void TftpdMain (void *param);
void SntpdProc (void *param);
void SyslogProc (void *param);
void AsyncSaveKeyBckgProc (void *param);
void Scheduler (void *param);
void ListenDNSMessage (void * param);


// Threads management : birth, life and death
int  StartAllWorkerThreads (void);
int  StartWorkerThreads (BOOL bSoft);
int  WakeUpThread (int Idx);
void TerminateWorkerThreads (BOOL bSoft);
int GetRunningThreads (void);

// Access to console
int SendMsgRequest (int type,				// msg type
					const void *msg_stuff,	// data
					int size,				// size of data
					BOOL bBlocking,			// block thread until msg sent
					BOOL bRetain );			// retain msg if GUI not connected

BOOL Tftpd32ReadSettings (void);
BOOL Tftpd32SaveSettings (void);
BOOL Tftpd32DestroySettings (void);

// Send the IP interfaces
int	AnswerIPList (void);

// Complex actions handled by console thread
void SendDirectoryContent (void);
