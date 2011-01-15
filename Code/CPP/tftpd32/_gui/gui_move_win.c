//////////////////////////////////////////////////////
//
// Projet Tftpd32.   Mars 2000 Ph.jounin
// File tftp_srv.c:  Display management
//
//
//////////////////////////////////////////////////////



#include "headers.h"

#define TOOLBAR_HEIGHT  0        // pas de fenêtre Toolbar


// The tabs
enum { ONGLET_TFTP_SERVER = 0,
       ONGLET_TFTP_CLIENT,
       ONGLET_DHCP_SERVER,
       ONGLET_SYSLOG_SERVER,
       ONGLET_SNTP_SERVER,
       ONGLET_DNS_SERVER,
       ONGLET_EVENTS_VIEWER,
       NB_ONGLETS };
// tab masks : 1>>Onglet
#define TAB_TFTP_SERVER     MakeMask(ONGLET_TFTP_SERVER)
#define TAB_TFTP_CLIENT     MakeMask(ONGLET_TFTP_CLIENT)
#define TAB_DHCP_SERVER     MakeMask(ONGLET_DHCP_SERVER)
#define TAB_SYSLOG_SERVER   MakeMask(ONGLET_SYSLOG_SERVER)
#define TAB_SNTP_SERVER     MakeMask(ONGLET_SNTP_SERVER)
#define TAB_DNS_SERVER      MakeMask(ONGLET_DNS_SERVER)
#define TAB_EVENTS_VIEWER   MakeMask(ONGLET_EVENTS_VIEWER)
#define TAB_NONE            MakeMask(NB_ONGLETS)
#define TAB_ALL            (DWORD) -1


// The sizing table
static struct S_Resize
{
    DWORD  idCtrl;                      // The control ID
    LONG   x, y, width, height;         // size of the window at "normal" size
    LONG   dx, dy, dwidth, dheight;     // proportyinal size increase (1 means 10%)
    DWORD  mView;                       // windows is displayed for Tabs ?
}
tResize [] =
{
// COMMON
    { IDC_TXT_BAD_SERVICES, 10,  60, 170,  30,    0,  0,  0,  0, TAB_NONE }, // visible only if services is 0
    { IDC_TAB_OPTION,        3,  36, 215, 102,    0,  0, 10, 10, TAB_ALL },
    { IDC_ABOUT_BUTTON,      6, 141,  62,  12,    0, 10,  1,  0, TAB_ALL },
    { IDC_SETTINGS_BUTTON,  80, 141,  62,  12,    4, 10,  2,  0, TAB_ALL },
    { IDC_TFTPD_HELP,      154, 141,  62,  12,    9, 10,  1,  0, TAB_ALL },
    { IDC_BASE_DIRECTORY,   63,   7, 110,  12,    0,  0,  7,  0, 0 },
    { IDC_CB_IP,            63,  22, 110,  41,    0,  0,  7,  0, TAB_ALL },
    { IDC_CB_DIR,           63,   7, 110,  41,    0,  0,  7,  0, TAB_ALL },
    { IDC_SHDIR_BUTTON,    180,  22,  37,  12,    8,  0,  2,  0, TAB_ALL },
    { IDC_BROWSE_BUTTON,   180,   7,  37,  12,    8,  0,  2,  0, TAB_ALL },
    { IDC_TXT_ADDRESS,       6,  23,  55,   9,    0,  0,  0,  0, TAB_ALL },
    { IDC_TXT_BASEDIR,       6,   8,  55,   9,    0,  0,  0,  0, TAB_ALL },
// TFTP SERVER
    { IDC_LV_TFTP,           6,  49, 210,  80,    0,  0, 10, 10, TAB_TFTP_SERVER },
// EVENTS VIEWER
    { IDC_LB_LOG,                 6,  49, 210,  68,    0,  0, 10, 10, TAB_EVENTS_VIEWER },
//    { IDC_CURRENT_ACTION,       135, 123,  76,  12,    2, 10,  8,  0, TAB_EVENTS_VIEWER },
    { IDC_TXT_CURACTION,         85, 125,  70,   9,    2, 10,  8,  0, TAB_EVENTS_VIEWER },
    { IDC_TFTP_CLEAR,             6, 123,  32,  12,    0, 10,  0,  0, TAB_EVENTS_VIEWER },
    { IDC_TFTP_COPYTOCLIP,       40, 123,  32,  12,    0, 10,  0,  0, TAB_EVENTS_VIEWER },
// TFTP CLIENT
    { IDC_TXT_CLIENTHOST,        10,  55,  20,  10,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENTPORT,       125,  55,  20,  10,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENT_LOCALFILE,  10,  69,  40,  10,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENT_REMOTEFILE, 10,  83,  40,  10,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_HOST,           34,  53,  70,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_PORT,          144,  53,  25,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_LOCALFILE,      54,  66,  95,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_REMOTEFILE,     54,  80,  95,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_BLOCK,           6, 115,  50,  18,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TXT_CLIENTBLOCKSIZE,   10,  94,  20,  20,    0,  0,  0,  0, TAB_TFTP_CLIENT },  
    { IDC_CB_DATASEG,            34,  94,  40,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },  

    { IDC_CLIENT_BROWSE,        155,  70,  15,   9,    0,  0,  0,  0, TAB_TFTP_CLIENT },  // suppressed
    { IDC_CLIENT_FULL_PATH,      87,  94,  86,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },  // suppressed

    { IDC_CLIENT_PROGRESS,       10, 134, 162,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_GET_BUTTON,     60, 116,  25,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_SEND_BUTTON,    90, 116,  25,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_CLIENT_BREAK_BUTTON,  147, 116,  25,  12,    0,  0,  0,  0, TAB_TFTP_CLIENT },
    { IDC_TEXT_TFTPCLIENT_HELP,  10, 135, 162,  24,    0,  0,  0,  0, 0 },
// DHCP SERVER
	{ IDC_LV_DHCP,                6,  49, 210,  68,    0,  0, 10, 10, TAB_DHCP_SERVER },

// SYSLOG SERVER
    { IDC_LB_SYSLOG,             6, 49, 210,  68,    0,  0, 10, 10, TAB_SYSLOG_SERVER },
    { IDC_SYSLOG_CLEAR,         16, 122, 70,  12,    0, 10,  0,  0, TAB_SYSLOG_SERVER },
    { IDC_SYSLOG_COPY,          90, 122, 70,  12,    0, 10,  0,  0, TAB_SYSLOG_SERVER },
//    { IDC_SYSLOG_SAVE,       126, 112, 80,  12,   10, 10,  0,  0, TAB_SYSLOG_SERVER },

// SNTP SERVER
    { IDC_TXT_SNTP,             20,  60, 135, 60,    0,   0, 0,  0, TAB_SNTP_SERVER },

// DNS SERVER
    { IDC_TXT_DNS,              20,  60, 135, 60,    0,   0, 0,  0, TAB_DNS_SERVER },
	{ IDC_LB_DNS,                6, 49, 210,  68,    0,  0, 10, 10, TAB_DNS_SERVER },

}; // tResize

static RECT SizeInit =    {40, 30, 40+230, 30+180 };
static RECT RectMinMax  = { 0, 0, 200, 192 };
static RECT RectTftpClient =  { 10, 50, 154, 53 };

///////////////////////////////////////////////
// Resize until ..
int TR_MinMaxInfo (HWND hDlgWnd, LPMINMAXINFO lpInfo)
{
    lpInfo->ptMinTrackSize.x = RectMinMax.right;
    lpInfo->ptMinTrackSize.y = RectMinMax.bottom;
return FALSE;
}   // TR_MinMaxInfo



///////////////////////////////////////////////
// Resize the main window --> resize each control
int TR_ResizeWindow (HWND hDlgWnd, BOOL bInit)
{
static BOOL bConvert2Physique=FALSE;
DWORD Ark;
RECT  RClient;
SIZE  SClient;
int  InflateX, InflateY;       // tenth

 // at first call : convert logical data to physical
  if (! bConvert2Physique)
  {
      bConvert2Physique = TRUE;
      for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
           MapDialogRect (hDlgWnd, (LPRECT) & tResize[Ark].x);
      MapDialogRect (hDlgWnd, & SizeInit);
      MapDialogRect (hDlgWnd, & RectMinMax);
      MapDialogRect (hDlgWnd, & RectTftpClient);
   } 

  if (bInit)
        MoveWindow (hDlgWnd, SizeInit.left, SizeInit.top, SizeInit.right, SizeInit.bottom, FALSE);

    // get window size and calculate the inflate rate
    GetWindowRect (hDlgWnd, & RClient);
    SClient.cx = RClient.right - RClient.left;
    SClient.cy = RClient.bottom - RClient.top;
    InflateX = 10 * ( SClient.cx - (SizeInit.right - SizeInit.left) );
    InflateY = 10 * ( SClient.cy - (SizeInit.bottom - SizeInit.top) );

    for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
    {
        MoveWindow (GetDlgItem (hDlgWnd, tResize[Ark].idCtrl),
                    tResize[Ark].x + (tResize[Ark].dx * InflateX) / 100,
                    tResize[Ark].y + (tResize[Ark].dy * InflateY) / 100,
                    tResize[Ark].width + (tResize[Ark].dwidth * InflateX) / 100,
                    tResize[Ark].height+ (tResize[Ark].dheight * InflateY) / 100,
                    TRUE);
    }

    InvalidateRect (hDlgWnd, NULL, FALSE);

return Ark;
} //TR_ResizeWindow




static int CR_Redisplay;
///////////////////////////////////////////////
// Note du changement dans la selection Tree
void CR_TreeSelectionHasChanged(void)
{
   CR_Redisplay = TRUE;
}

///////////////////////////////////////////////
// Initthe Tab control
int TR_InitTabControl (HWND hDlgWnd)
{
TC_ITEM TabCtrlItem;
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_OPTION);
int    nOnglet, Ark;
static const struct S_TabCtrlData
{
	int   tab;
	char *name;
	int   mask;
	int   service;	// TRUE if tab is the GUI of a service
} tTabCtrlData [] =
{
	ONGLET_TFTP_SERVER,   "Tftp 服务器" ,  TFTPD32_TFTP_SERVER,	   TRUE,
	ONGLET_TFTP_CLIENT,   "Tftp 客户端",   TFTPD32_TFTP_CLIENT,   FALSE,
	ONGLET_DHCP_SERVER,   "DHCP 服务器",   TFTPD32_DHCP_SERVER,    TRUE,
	ONGLET_SYSLOG_SERVER, "系统日志服务器", TFTPD32_SYSLOG_SERVER,  TRUE,
	ONGLET_SNTP_SERVER,   "SNTP 服务器",   TFTPD32_SNTP_SERVER,    TRUE,
	ONGLET_DNS_SERVER,    "DNS 服务器",    TFTPD32_DNS_SERVER,    TRUE,
	ONGLET_EVENTS_VIEWER, "日志查看器",    TFTPD32_ALL_SERVICES,  FALSE,
};  // tTabCtrlData
// Ordonated list of the Client Controls 
static const int tTftpClientCtrl [] = 
{ 
    IDC_CLIENT_HOST, IDC_CLIENT_PORT, IDC_CLIENT_LOCALFILE, IDC_CLIENT_REMOTEFILE,
    IDC_CLIENT_GET_BUTTON, IDC_CLIENT_SEND_BUTTON
};     

    // Create tabs and add them label
    // code is really not optimized, but it is working
    // and since it is called only once...
    TabCtrlItem.mask = TCIF_TEXT | TCIF_PARAM   ;
    
	for ( nOnglet = 0;  nOnglet < SizeOfTab (tTabCtrlData) ; nOnglet++ )
	{
		// for local tabs (tftp client & event viewer use the settings)
		// for remote services use the report sent by the console
		// and saved into sGuiSettings.uRunningServices
        if (   tTabCtrlData[nOnglet].mask 
			& (tTabCtrlData[nOnglet].service ? sGuiSettings.uRunningServices : sGuiSettings.uServices) )
		{
			TabCtrlItem.pszText = tTabCtrlData[nOnglet].name;
			TabCtrlItem.lParam  = tTabCtrlData[nOnglet].tab ;
            TabCtrl_InsertItem(hTabWnd, nOnglet, (LPARAM) &  TabCtrlItem);
		}	// tab should be displayed
	} // all tabs


    TR_ResizeWindow(hDlgWnd, TRUE);
    // First available tab is selected
    TabCtrl_SetCurSel (hTabWnd, 0);
    TR_ChangeTabControl (hDlgWnd);

	// Place Tab ctrl at bottom of Z-order and all client control at top
	// Local File control will then accept drag an drop
    // (set also the other windows just to ease navigation with TAB)
	SetWindowPos (hTabWnd, HWND_BOTTOM, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
    for ( Ark=SizeOfTab (tTftpClientCtrl) ; Ark>0 ; Ark-- )
	    SetWindowPos ( GetDlgItem (hDlgWnd, tTftpClientCtrl [Ark-1]), 
		               HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE );
return TRUE;
} // InitTabControl


///////////////////////////////////////////////
// Tab control has changed : hide/show controls
LRESULT TR_ChangeTabControl (HWND hDlgWnd)
{
HWND   hTabWnd = GetDlgItem (hDlgWnd, IDC_TAB_OPTION);
DWORD  nCurOnglet = TabCtrl_GetCurSel (hTabWnd);
DWORD  Ark;
DWORD  TabMask;
TC_ITEM TabCtrlItem;

 // get the selected tab id
 if (nCurOnglet != (DWORD)-1)
   {
          TabCtrlItem.mask = TCIF_PARAM   ;
          TabCtrl_GetItem (hTabWnd, nCurOnglet, & TabCtrlItem);
          TabMask = MakeMask (TabCtrlItem.lParam);
   }
  else    TabMask = TAB_NONE;

 // according to TabMask and tResize table hide/display controls
 for (Ark=0 ; Ark<SizeOfTab(tResize) ; Ark++)
        ShowWindow (GetDlgItem (hDlgWnd, tResize[Ark].idCtrl),
                    (tResize[Ark].mView & TabMask) ? SW_SHOW : SW_HIDE);

  // DHCP tab has been selected : display the settings
  if (TabMask == TAB_DHCP_SERVER)
  {
	 Gui_LoadDHCPConfig (hDlgWnd);
  }

return TabCtrlItem.lParam ;
} // ChangeTabControl


