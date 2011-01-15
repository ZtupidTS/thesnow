//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File bootp.c:    Manage BOOTP/DHCP protocols
//
// Released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"
#include <stdio.h>          // sscanf is used


///////////////////////////////////////////
//  DHCP configuration management
///////////////////////////////////////////

// Utilitaire : translate a dialog item text into a in_addr struct
struct in_addr DlgItem2Address (HWND hWnd, int nDlgItem, const char *szDescr, BOOL bStrict)
{
char             szBuf[120];
struct in_addr   dwAddr;
   memset (szBuf, 0, sizeof szBuf);
   GetDlgItemText (hWnd, nDlgItem,  szBuf, sizeof szBuf - 1);
   dwAddr.s_addr = inet_addr (szBuf);
   if (bStrict   &&  dwAddr.s_addr==INADDR_ANY)
   {
    wsprintf (szBuf, "字段 %s 含有无效格式", szDescr);
      MY_WARNING (szBuf);
   } // Erreur dans un champ
return dwAddr;
} // DlgItem2Address


int Gui_LoadDHCPConfig (HWND hMainWnd)
{
   // Display values
   SetDlgItemText (hMainWnd, IDC_DHCP_ADDRESS_POOL,  inet_ntoa (sGuiParamDHCP.dwAddr));
   SetDlgItemText (hMainWnd, IDC_DHCP_MASK,          inet_ntoa (sGuiParamDHCP.dwMask));
   SetDlgItemText (hMainWnd, IDC_DHCP_DNS_SERVER,    inet_ntoa (sGuiParamDHCP.dwDns));
   SetDlgItemText (hMainWnd, IDC_DHCP_DEFAULT_ROUTER, inet_ntoa (sGuiParamDHCP.dwGateway));
   SetDlgItemText (hMainWnd, IDC_DHCP_BOOT_FILE,     sGuiParamDHCP.szBootFile);
   SetDlgItemInt  (hMainWnd, IDC_DHCP_POOL_SIZE,     sGuiParamDHCP.nPoolSize, FALSE);
   SetDlgItemText (hMainWnd, IDC_DHCP_DOMAINNAME,    sGuiParamDHCP.szDomainName);

   // first value in the GUI
   SetDlgItemInt  (hMainWnd, IDC_DHCP_ADDOPTION_NB,  sGuiParamDHCP.t[0].nAddOption, FALSE);
   SetDlgItemText (hMainWnd, IDC_DHCP_ADDOPTION_VALUE, sGuiParamDHCP.t[0].szAddOption);
return TRUE;   
} //  Gui_LoadDHCPConfig



// Save configuration either in INI file (if it exists) or in registry
int Gui_DHCPSaveConfig (HWND hWnd)
{
struct S_DHCP_Param  sNewParamDHCP;      // New param
// HWND hMainWnd = GetParent (hWnd);     // hWnd is no more a sub-wnd
HWND hMainWnd = hWnd;
INT   Ark;

     memset (& sNewParamDHCP, 0, sizeof sNewParamDHCP);
     // save parameters not assigned by GUI
     for (Ark=1 ; Ark < SizeOfTab (sGuiParamDHCP.t) ; Ark++)    sNewParamDHCP.t[Ark] = sGuiParamDHCP.t[Ark];
     sNewParamDHCP.nLease = sGuiParamDHCP.nLease;          

     sNewParamDHCP.nPoolSize = GetDlgItemInt (hMainWnd, IDC_DHCP_POOL_SIZE, NULL, FALSE);
     sNewParamDHCP.dwAddr =    DlgItem2Address (hMainWnd, IDC_DHCP_ADDRESS_POOL, "地址池", sNewParamDHCP.nPoolSize!=0);
     sNewParamDHCP.dwMask =    DlgItem2Address (hMainWnd, IDC_DHCP_MASK, "子网掩码", sNewParamDHCP.nPoolSize!=0);
     sNewParamDHCP.dwDns  =    DlgItem2Address (hMainWnd, IDC_DHCP_DNS_SERVER, "DNS 服务器", FALSE);
     sNewParamDHCP.dwGateway=  DlgItem2Address (hMainWnd, IDC_DHCP_DEFAULT_ROUTER, "默认路由", FALSE);
     GetDlgItemText (hMainWnd, IDC_DHCP_BOOT_FILE,   sNewParamDHCP.szBootFile, sizeof sNewParamDHCP.szBootFile - 1);
     GetDlgItemText (hMainWnd, IDC_DHCP_DOMAINNAME,  sNewParamDHCP.szDomainName, sizeof sNewParamDHCP.szDomainName - 1);

     // from the GUI only one user option is available
     sNewParamDHCP.t[0].nAddOption = GetDlgItemInt (hMainWnd, IDC_DHCP_ADDOPTION_NB, NULL, FALSE);
     GetDlgItemText (hMainWnd,
                     IDC_DHCP_ADDOPTION_VALUE,
                     sNewParamDHCP.t[0].szAddOption,
                     sizeof sNewParamDHCP.t[0].szAddOption - 1);

     // Change 30 oct 2002 : Gateway && DNS can be empty
     if (     sNewParamDHCP.dwAddr.s_addr==INADDR_ANY || sNewParamDHCP.dwMask.s_addr==INADDR_ANY
          // ||  sNewParamDHCP.dwDns.s_addr ==INADDR_ANY
          // || sNewParamDHCP.dwGateway.s_addr==INADDR_ANY
     )
     return FALSE;

     // load again (warkaround for a LCC bug)
     sNewParamDHCP.nPoolSize = GetDlgItemInt (hMainWnd, IDC_DHCP_POOL_SIZE, NULL, FALSE);
     if (sNewParamDHCP.nPoolSize == 0)   MY_WARNING ("DHCP 池为空\nDHCP 服务器将只能分配\n静态地址");

	 if (memcmp (& sGuiParamDHCP, & sNewParamDHCP, sizeof sNewParamDHCP) !=0)
	 {
		sGuiParamDHCP = sNewParamDHCP;
		return TRUE;
	 }
return FALSE;
} // Gui_DHCPSaveConfig




