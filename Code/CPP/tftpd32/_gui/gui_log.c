//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin - Jan 2003
// File qui_log.c:  Log management (tab event viewer)
//
//////////////////////////////////////////////////////


#include "headers.h"
#include <process.h>
#include <stdio.h>


// ---------------------------------------------------------------
// GUI  management
// ---------------------------------------------------------------
/* ------------------------------------------------- */
/* Manage a list box as a log window                 */
/* ------------------------------------------------- */
void LB_LOG (HWND hListBox, const char *szTxt)
{
int        Ark;
int        dwMaxExtent = 0;
SIZE       sTextSize = {0, 0};
HDC        hDC;
DWORD      dwMaxMsg = 200;
char       szBuf[LOGSIZE + 30];


#define CROLLBAR
#ifdef CROLLBAR
   // reduce number of messages
   while ( SendMessage (hListBox, LB_GETCOUNT, 0, 0) > (long) dwMaxMsg )
      SendMessage (hListBox, LB_DELETESTRING, 0, 0);

    Ark = SendMessage (hListBox, LB_ADDSTRING, 0, (LPARAM) szTxt);

    // extent horizontal scrollbar
    dwMaxExtent = 0;
    hDC = GetDC (hListBox);
    for (Ark= SendMessage (hListBox, LB_GETCOUNT, 0, 0);
         Ark>=0 ;
         Ark --)
         {
            SendMessage (hListBox, LB_GETTEXT, Ark-1, (LPARAM) szBuf);
            GetTextExtentPoint32 (hDC, szBuf, lstrlen (szBuf), & sTextSize);
            // Logical to Phycal units
            LPtoDP (hDC, (POINT *)  & sTextSize, 1);
            if (sTextSize.cx > dwMaxExtent)  dwMaxExtent = sTextSize.cx ;
         }
    // add a 10% reduction (20% should still be OK)
    dwMaxExtent *= 9;
    dwMaxExtent /= 10;
    SendMessage (hListBox, LB_SETHORIZONTALEXTENT, dwMaxExtent, 0);
    ReleaseDC (hListBox, hDC);
#endif
} // LB_LOG

//////////////////////////////////////
// Log Thread management
//////////////////////////////////////


