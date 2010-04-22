//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Feb 99 By  Ph.jounin
// File start_threads.c:  Thread management
//
// The main function of the service
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "../_common/headers.h"
#include <process.h>
#include "threading.h"



void StartTftpd32Services (void *param)
{
char sz[_MAX_PATH];

     // read log level (env var TFTP_LOG)
	if (GetEnvironmentVariable (TFTP_LOG, sz, sizeof sz)!=0)
          sSettings.LogLvl = atoi (sz);
    else  sSettings.LogLvl = TFTPD32_DEF_LOG_LEVEL;

    // Get the path in order to find the help file
    if (GetEnvironmentVariable (TFTP_INI, sz, sizeof sz)!=0)
          SetIniFileName (sz, szTftpd32IniFile);
    else  SetIniFileName (INI_FILE, szTftpd32IniFile);

    // Read settings (tftpd32.ini)
    Tftpd32ReadSettings ();

    // starts worker threads
    StartWorkerThreads (FALSE);

} // StartTftpd32Services

void StopTftpd32Services (void)
{
   TerminateWorkerThreads (FALSE);
}