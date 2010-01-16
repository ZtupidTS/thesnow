//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File tftp_sec.c:   Settings
//
//////////////////////////////////////////////////////

// registry key :
//       HKEY_LOCAL_MACHINE\SOFTWARE\TFTPD32

// some shortcurts
#define ISDLG_CHECKED(hWnd,Ctrl) \
 (SendDlgItemMessage (hWnd, Ctrl, BM_GETCHECK, 0, 0) == BST_CHECKED)
#define CHECK_DLG_IF(hWnd, Ctrl, Flag) \
     SendDlgItemMessage (hWnd, Ctrl, BM_SETCHECK, (Flag) ? BST_CHECKED : BST_UNCHECKED, 0);
#define UNCHECK_DLG(hWnd, Ctrl) \
     SendDlgItemMessage (hWnd, Ctrl, BM_SETCHECK, BST_UNCHECKED, 0);


#include <stdio.h>
#include "headers.h"


#define  RESET_DEFAULT_TEXT  "Reset current configuration\nand destroy registry entries ?"



/////////////////////////////////////////////////////////
//  Settings Window management
/////////////////////////////////////////////////////////

int CALLBACK SettingsProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
static struct S_Tftpd32Settings sNewSettings;
char        szBrowsePath [MAX_PATH];
char        szSyslogFileName [MAX_PATH];
char        sz [32];

  switch (message)
  {
       case WM_INITDIALOG :
            // Deactivate Browse Button
            if ( IsGuiConnectedToRemoteService () )
                Button_Enable (GetDlgItem (hWnd, IDC_BUTTON_BROWSE), FALSE);
           // Directory
           SetDlgItemText (hWnd, IDC_BASEDIR, sGuiSettings.szBaseDirectory);
        // Services
           CHECK_DLG_IF (hWnd, IDC_CHECK_TFTP_SERVER,  sGuiSettings.uServices  & TFTPD32_TFTP_SERVER);
           CHECK_DLG_IF (hWnd, IDC_CHECK_TFTP_CLIENT,  sGuiSettings.uServices  & TFTPD32_TFTP_CLIENT);
           CHECK_DLG_IF (hWnd, IDC_CHECK_SYSLOG_SERVER,sGuiSettings.uServices  & TFTPD32_SYSLOG_SERVER);
           CHECK_DLG_IF (hWnd, IDC_CHECK_DHCP_SERVER,  sGuiSettings.uServices  & TFTPD32_DHCP_SERVER);
           CHECK_DLG_IF (hWnd, IDC_CHECK_DNS_SERVER,   sGuiSettings.uServices  & TFTPD32_DNS_SERVER);
           CHECK_DLG_IF (hWnd, IDC_CHECK_SNTP_SERVER,  sGuiSettings.uServices  & TFTPD32_SNTP_SERVER);
          // Syslog Save option
           CHECK_DLG_IF (hWnd, IDC_CHECK_SAVE_SYSLOG, sGuiSettings.szSyslogFile[0]!=0);
           SetDlgItemText (hWnd, IDC_SYSLOG_FILE, sGuiSettings.szSyslogFile);
           Edit_Enable (GetDlgItem (hWnd, IDC_SYSLOG_FILE), sGuiSettings.szSyslogFile[0]!=0);
           CHECK_DLG_IF (hWnd, IDC_CHECK_PIPE_SYSLOG, sGuiSettings.bSyslogPipe);

           // DHCP Persistant leases + Address + ping
           CHECK_DLG_IF (hWnd, IDC_CHECK_PERS_LEASES, sGuiSettings.bPersLeases);
           CHECK_DLG_IF (hWnd, IDC_CHECK_PING, sGuiSettings.bPing);
//	       FillCBLocalIP (GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP), FALSE, sGuiSettings.szDHCPLocalIP);
		   CopyCBContent ( GetDlgItem (GetParent (hWnd), IDC_CB_IP),
						   GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP),
						   sGuiSettings.szDHCPLocalIP );
           CHECK_DLG_IF (hWnd, IDC_CHECK_LOCALIP_DHCP, sGuiSettings.szDHCPLocalIP[0]!=0);
           ComboBox_Enable (GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP), ISDLG_CHECKED (hWnd, IDC_CHECK_LOCALIP_DHCP) );

           // security Level -> Radio Box
           CHECK_DLG_IF (hWnd, IDC_RADIO_SECNONE, sGuiSettings.SecurityLvl==SECURITY_NONE);
           CHECK_DLG_IF (hWnd, IDC_RADIO_SECSTD,  sGuiSettings.SecurityLvl==SECURITY_STD);
           CHECK_DLG_IF (hWnd, IDC_RADIO_SECHIGH, sGuiSettings.SecurityLvl==SECURITY_HIGH);
           CHECK_DLG_IF (hWnd, IDC_RADIO_SECRO,   sGuiSettings.SecurityLvl==SECURITY_READONLY);
           // Timeouts -> group timeout
           SetDlgItemInt (hWnd, IDC_TIMEOUT, sGuiSettings.Timeout, FALSE);
           SetDlgItemInt (hWnd, IDC_MAXRETRANSMIT, sGuiSettings.Retransmit, FALSE);
           SetDlgItemInt (hWnd, IDC_PORT, sGuiSettings.Port, FALSE);

           // Advanced Options
           CHECK_DLG_IF (hWnd, IDC_CHECK_HIDE,     sGuiSettings.bHide);
           CHECK_DLG_IF (hWnd, IDC_CHECK_PROGRESS, sGuiSettings.bProgressBar);
           CHECK_DLG_IF (hWnd, IDC_CHECK_NEGOCIATE,sGuiSettings.bNegociate);
           CHECK_DLG_IF (hWnd, IDC_CHECK_PXE,      sGuiSettings.bPXECompatibility);
           CHECK_DLG_IF (hWnd, IDC_CHECK_DIRTEXT,  sGuiSettings.bDirText);
           CHECK_DLG_IF (hWnd, IDC_CHECK_MD5,      sGuiSettings.bMD5);
           CHECK_DLG_IF (hWnd, IDC_CHECK_UNIX,     sGuiSettings.bUnixStrings);
           CHECK_DLG_IF (hWnd, IDC_CHECK_BEEP,     sGuiSettings.bBeep);
           CHECK_DLG_IF (hWnd, IDC_CHECK_VROOT,    sGuiSettings.bVirtualRoot);
           SetDlgItemInt (hWnd, IDC_WINSIZE, sGuiSettings.WinSize, FALSE);

           // Limitations des acces
//           FillCBLocalIP (GetDlgItem (hWnd, IDC_CB_LOCALIP), FALSE, sGuiSettings.szLocalIP);
		   CopyCBContent ( GetDlgItem (GetParent (hWnd), IDC_CB_IP),
						   GetDlgItem (hWnd, IDC_CB_LOCALIP),
						   sGuiSettings.szLocalIP );
           CHECK_DLG_IF (hWnd, IDC_CHECK_LOCALIP, sGuiSettings.szLocalIP[0]!=0);
           ComboBox_Enable (GetDlgItem (hWnd, IDC_CB_LOCALIP), ISDLG_CHECKED (hWnd, IDC_CHECK_LOCALIP) );

           // Fenêtre d'anticipation
           CHECK_DLG_IF (hWnd, IDC_CHECK_WINSIZE, sGuiSettings.WinSize>0);
           Edit_Enable (GetDlgItem (hWnd, IDC_WINSIZE), sGuiSettings.WinSize > 0);
           // local ports
           if (sGuiSettings.nTftpLowPort!=0 && sGuiSettings.nTftpHighPort>=sGuiSettings.nTftpLowPort)
           {char sz[64];
              wsprintf (sz, "%d:%d", sGuiSettings.nTftpLowPort, sGuiSettings.nTftpHighPort);
              SetDlgItemText (hWnd, IDC_LOCAL_PORTS, sz);
           }

           // Save Settings
           sNewSettings = sGuiSettings;
           CenterChildWindow (hWnd, CCW_VISIBLE);
           break;

       case WM_COMMAND :
           switch (wParam)
           {
                case IDC_CHECK_PXE :
                // uncheck other choice
                UNCHECK_DLG (hWnd, IDC_CHECK_NEGOCIATE);
                   if ( ISDLG_CHECKED (hWnd, wParam) )
                {
                    CHECK_DLG_IF (hWnd, IDC_CHECK_VROOT, TRUE);
                    CHECK_DLG_IF (hWnd, IDC_CHECK_UNIX, TRUE);
                }
                break;

                case IDC_CHECK_NEGOCIATE :
             // uncheck other choice
                UNCHECK_DLG (hWnd, IDC_CHECK_PXE);
                break;


                case IDC_CHECK_TFTP_SERVER :
                case IDC_CHECK_DNS_SERVER :
                case IDC_CHECK_TFTP_CLIENT :
                case IDC_CHECK_SYSLOG_SERVER :
                   if (!   (   ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_SERVER)
                             || ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_CLIENT)
                             || ISDLG_CHECKED (hWnd, IDC_CHECK_SYSLOG_SERVER)
                             || ISDLG_CHECKED (hWnd, IDC_CHECK_DHCP_SERVER) 
                             || ISDLG_CHECKED (hWnd, IDC_CHECK_DNS_SERVER) 
                             || ISDLG_CHECKED (hWnd, IDC_CHECK_SNTP_SERVER) 
                            )
                    )
                   {
                     // re-check dlg item
                       CHECK_DLG_IF (hWnd, wParam, TRUE);;
                    MY_WARNING ("At least one service have to be checked");
                 }
                  break;
                case IDC_CHECK_LOCALIP :
                    ComboBox_Enable ( GetDlgItem (hWnd, IDC_CB_LOCALIP), ISDLG_CHECKED (hWnd, wParam) );
                    break;

                case IDC_CHECK_LOCALIP_DHCP :
                    ComboBox_Enable ( GetDlgItem (hWnd, IDC_CB_LOCALIP_DHCP), ISDLG_CHECKED (hWnd, wParam) );
                    break;


               case IDC_CHECK_SAVE_SYSLOG :
                   Edit_Enable ( GetDlgItem (hWnd, IDC_SYSLOG_FILE), ISDLG_CHECKED (hWnd, wParam) );
                  if (ISDLG_CHECKED (hWnd, wParam))
                  {
                     GetDlgItemText (hWnd, IDC_SYSLOG_FILE, szSyslogFileName, sizeof szSyslogFileName);
                     if (szSyslogFileName[0] == 0)
                           SetDlgItemText (hWnd, IDC_SYSLOG_FILE, DEFAULT_SYSLOG_FILE);
                   }
                  break;

                case IDC_CHECK_WINSIZE :
                   if ( ISDLG_CHECKED (hWnd, wParam) )
                    {
                          MY_WARNING ("Use an anticipation window acclerates transfers\nOn the other hand, it may not work with your TFTP client.");
                       Edit_Enable ( GetDlgItem (hWnd, IDC_WINSIZE), TRUE );
                    }
                  else
                   {
                      sNewSettings.WinSize = 0;
                      SetDlgItemInt (hWnd, IDC_WINSIZE, 0, FALSE);
                       Edit_Enable ( GetDlgItem (hWnd, IDC_WINSIZE), FALSE );
                 }
                    break;

                case IDC_BUTTON_BROWSE :
                  GetDlgItemText (hWnd, IDC_BASEDIR, szBrowsePath, sizeof szBrowsePath);
                     // Do not change dir, wait for OK button
                     if (MyBrowseWindow (hWnd, szBrowsePath, szBrowsePath[0]!='.'))
                            SetDlgItemText (hWnd, IDC_BASEDIR, szBrowsePath);
                     break;

                case IDC_BUTTON_DEFAULT :
                     if (CMsgBox (hWnd, RESET_DEFAULT_TEXT, "Tftpd32", MB_YESNOCANCEL | MB_ICONQUESTION) == IDYES)
                     {
                         PostMessage (GetParent (hWnd), WM_DESTROY_SETTINGS, 0, 0);  // destroy all Keys
                         MY_WARNING ("Settings entries have been deleted.\nYou have to restart Tftpd32 in order to use the new specifications");
                         EndDialog (hWnd, 0);         // and exit window -> do not save in reg
                     }
                     break;

                case IDOK :
                    // Directory
                    GetDlgItemText (hWnd, IDC_BASEDIR, sNewSettings.szBaseDirectory, sizeof sNewSettings.szBaseDirectory);
                  // Services
                    sNewSettings.uServices = 0;
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_SERVER)  )  sNewSettings.uServices |= TFTPD32_TFTP_SERVER;
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_TFTP_CLIENT)  )  sNewSettings.uServices |= TFTPD32_TFTP_CLIENT;
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_SYSLOG_SERVER))  sNewSettings.uServices |= TFTPD32_SYSLOG_SERVER;
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_DHCP_SERVER))    sNewSettings.uServices |= TFTPD32_DHCP_SERVER;
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_DNS_SERVER))     sNewSettings.uServices |= TFTPD32_DNS_SERVER;
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_SNTP_SERVER))    sNewSettings.uServices |= TFTPD32_SNTP_SERVER;

                    // test if settings have changed
                    // Syslog options
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_SAVE_SYSLOG) )
                    {
                        GetDlgItemText (hWnd, IDC_SYSLOG_FILE, sNewSettings.szSyslogFile, sizeof sNewSettings.szSyslogFile);
                    }
                    else
                    {
                       sNewSettings.szSyslogFile[0] = 0;
                    }
                    sNewSettings.bSyslogPipe = ISDLG_CHECKED (hWnd, IDC_CHECK_PIPE_SYSLOG);

                    // DHCP options 
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_LOCALIP_DHCP))
                         ComboBox_GetText (GetDlgItem(hWnd, IDC_CB_LOCALIP_DHCP), sNewSettings.szDHCPLocalIP , sizeof sNewSettings.szDHCPLocalIP );
                    else  sNewSettings.szDHCPLocalIP[0]=0;
                    sNewSettings.bPersLeases = ISDLG_CHECKED (hWnd, IDC_CHECK_PERS_LEASES);
                    sNewSettings.bPing       = ISDLG_CHECKED (hWnd, IDC_CHECK_PING);

                    // security Level -> Radio Box
                    if (ISDLG_CHECKED (hWnd, IDC_RADIO_SECNONE))      sNewSettings.SecurityLvl = SECURITY_NONE;
                    else if (ISDLG_CHECKED (hWnd, IDC_RADIO_SECRO))   sNewSettings.SecurityLvl = SECURITY_READONLY;
                    else if (ISDLG_CHECKED (hWnd, IDC_RADIO_SECHIGH)) sNewSettings.SecurityLvl = SECURITY_HIGH;
                    else                                             sNewSettings.SecurityLvl = SECURITY_STD;
                   // Timeouts -> group timeout
                    sNewSettings.Timeout = GetDlgItemInt (hWnd, IDC_TIMEOUT, NULL, FALSE);
                    sNewSettings.Retransmit = GetDlgItemInt (hWnd, IDC_MAXRETRANSMIT, NULL, FALSE);
                    sNewSettings.Port = GetDlgItemInt (hWnd, IDC_PORT, NULL, FALSE);

                    // Advanced Options
                    sNewSettings.bHide       = ISDLG_CHECKED (hWnd, IDC_CHECK_HIDE);
                    sNewSettings.bProgressBar= ISDLG_CHECKED (hWnd, IDC_CHECK_PROGRESS);
                    sNewSettings.WinSize     = GetDlgItemInt (hWnd, IDC_WINSIZE, NULL, FALSE);
                    sNewSettings.bNegociate  = ISDLG_CHECKED (hWnd, IDC_CHECK_NEGOCIATE);
                    sNewSettings.bDirText    = ISDLG_CHECKED (hWnd, IDC_CHECK_DIRTEXT);
                    sNewSettings.bMD5        = ISDLG_CHECKED (hWnd, IDC_CHECK_MD5);
                    sNewSettings.bUnixStrings= ISDLG_CHECKED (hWnd, IDC_CHECK_UNIX);
                    sNewSettings.bBeep       = ISDLG_CHECKED (hWnd, IDC_CHECK_BEEP);
                    sNewSettings.bVirtualRoot= ISDLG_CHECKED (hWnd, IDC_CHECK_VROOT);
                    sNewSettings.bPXECompatibility = ISDLG_CHECKED (hWnd, IDC_CHECK_PXE);
                    // Sécurité sur l'accès
                    if (ISDLG_CHECKED (hWnd, IDC_CHECK_LOCALIP))
                         ComboBox_GetText (GetDlgItem(hWnd, IDC_CB_LOCALIP), sNewSettings.szLocalIP, sizeof sNewSettings.szLocalIP);
                    else sNewSettings.szLocalIP [0] = 0;
                    // local ports
                    GetDlgItemText (hWnd, IDC_LOCAL_PORTS, sz, sizeof sz);
                    // parse the string, lcc stdio lib does not handle the format syntax %d%*[-:]
                    if (   sscanf_s (sz, "%d:%d", & sNewSettings.nTftpLowPort, & sNewSettings.nTftpHighPort) != 2
		                && sscanf_s (sz, "%d-%d", & sNewSettings.nTftpLowPort, & sNewSettings.nTftpHighPort) != 2 )
                        sNewSettings.nTftpLowPort = sNewSettings.nTftpHighPort = 0;
                    // validate data
                    if (sNewSettings.Timeout==0  ||  sNewSettings.Retransmit==0  || sNewSettings.Port==0)
                    {
                        MY_WARNING ("Timeouts and Ports should be numerical and can not be 0");
                        break;
                    }

                    // field Base directory
                    if ( IsGuiConnectedToRemoteService () )
                          PostMessage (hWnd, WM_TFTP_CHG_WORKING_DIR, 0, 0);
                    else
                    {
                        if (IsValidDirectory (sNewSettings.szBaseDirectory) )
                        {HWND hCBWnd = GetDlgItem (GetParent(hWnd), IDC_CB_DIR);
                              GetFullPathName ( sNewSettings.szBaseDirectory, 
                                                sizeof szBrowsePath,
                                                szBrowsePath, 
                                                NULL );
                              TftpDir_AddEntry (hCBWnd, szBrowsePath);
                              lstrcpy (sNewSettings.szWorkingDirectory, sNewSettings.szBaseDirectory); 
                              // transfer to daemon
                              PostMessage (hWnd, WM_TFTP_CHG_WORKING_DIR, 0, 0);
                        }
                        else   MY_WARNING ("Can not change directory");
                    }

                 // some change need a restart
                    if (   sGuiSettings.Port      != sNewSettings.Port
                        || sGuiSettings.uServices != sNewSettings.uServices
                        || sGuiSettings.bHide     != sNewSettings.bHide
                        || lstrcmp (sGuiSettings.szDHCPLocalIP, sNewSettings.szDHCPLocalIP) != 0
                        || lstrcmp (sGuiSettings.szLocalIP, sNewSettings.szLocalIP) != 0
                     )
					{
                        MY_WARNING ("You have to restart Tftpd32\nin order to apply the new settings");
					}

                    sGuiSettings = sNewSettings;
                    PostMessage (GetParent (hWnd), WM_SAVE_SETTINGS, 0, 0);
                    EndDialog (hWnd, 0);
                    break;

                case IDCANCEL :
                    EndDialog (hWnd, -1);
                    break;

                case IDC_TFTPD_HELP:
                    // WinHelp(hWnd, szTftpd32Help, HELP_CONTENTS, 0);
                    ShellExecute (hWnd, "open", szTftpd32Help,  NULL, NULL, SW_NORMAL);
                    break;
           }
           break;

       case WM_CLOSE :
       case WM_DESTROY :
            EndDialog (hWnd, -1);
            break;

  } // switch

return FALSE;
} // SettingsProc



/////////////////////////////////////////////////////////
// Save window position
/////////////////////////////////////////////////////////
BOOL Tftpd32SaveWindowPos (HWND hMainWnd)
{
RECT  R;
HKEY  hKey=INVALID_HANDLE_VALUE;
DWORD dwState;
INT   Rc;
char  sz[128];

    GetWindowRect (hMainWnd, &R);
    wsprintf (sz, "%d %d %d %d ", R.left, R.top, R.right, R.bottom);

    Rc = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                         TFTPD32_MAIN_KEY,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_WRITE,
                         NULL,
                        &hKey,
                        &dwState) == ERROR_SUCCESS;
    Rc &=      SAVEKEY (KEY_WINDOW_POS,sz,REG_SZ);
    if (hKey!=INVALID_HANDLE_VALUE)  RegCloseKey (hKey);
return Rc;
} // Tftpd32SaveWindowPos



BOOL Tftpd32RetrieveWindowPos (HWND hMainWnd)
{
RECT  R;
HKEY  hKey=INVALID_HANDLE_VALUE;
DWORD dwSize;
INT   Rc, Ark=0;
char  sz[128], *pCur, *pNext;


   Rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,    // Key handle at root level.
                      TFTPD32_MAIN_KEY,      // Path name of child key.
                      0,                      // Reserved.
                      KEY_READ,                // Requesting read access.
                    & hKey);                 // Address of key to be returned.
    // Lire les données dans cette entrée
    READKEY (KEY_WINDOW_POS, sz);
    if (hKey!=INVALID_HANDLE_VALUE)
            RegCloseKey (hKey);

  // parse entry
 for (sz [sizeof sz - 1] = 0, pNext=pCur=sz ; *pNext!=0 ; pNext++)
      if (*pNext==' ')
       {
          *pNext=0;
          switch (Ark++)
         {
             case 0 : R.left   = atoi(pCur); break;
             case 1 : R.top    = atoi(pCur); break;
             case 2 : R.right  = atoi(pCur); break;
             case 3 : R.bottom = atoi(pCur); break;
         }
          pCur=pNext+1;
      }

 // The window should be visible (inside the screen)
    if (Ark == 4
        && R.left + 150 < GetSystemMetrics(SM_CXSCREEN)
        && R.top  + 150 < GetSystemMetrics(SM_CYSCREEN)
        && R.left < R.right
        && R.top  < R.bottom
        && R.left >= 0
        && R.top  >= 0
     )
    {
      MoveWindow (hMainWnd, R.left, R.top, R.right-R.left, R.bottom-R.top, TRUE);
    }
return Rc;
} // Tftpd32RetrieveWindowPos






