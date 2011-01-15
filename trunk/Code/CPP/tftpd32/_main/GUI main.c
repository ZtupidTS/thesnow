//////////////////////////////////////////////////////
//
// Projet TFTPD32.   Mai 98 Ph.jounin
// File gui_main.c:  The MAIN program
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"

//////////////////////////////////////////////////////
//
//  Starts services and main window
//
//////////////////////////////////////////////////////


// A few global variables
char      szTftpd32Help [MAX_PATH];     // Full Path for Help file

/* ----------------------------- */
/* WinMain                       */
/* ----------------------------- */
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
WSADATA               WSAData;
int                   Rc;


  // ------------------------------------------
  // Start the App
  // ------------------------------------------
     	 
     Rc = WSAStartup (MAKEWORD(2,0), & WSAData);
     if (Rc != 0)
     {
         CMsgBox (NULL, 
				  GetLastError()==WSAVERNOTSUPPORTED ?
					    "����: Tftpd32 ��Ҫ winsock 2 ���ϵİ汾" :
						"����: ���ܳ�ʼ�� Winsocket", 
				   APPLICATION, 
				   MB_OK | MB_ICONERROR);
         return FALSE;
     }


     // opens Gui
     GuiMain (hInstance, hPrevInstance,lpszCmdLine, nCmdShow);

     WSACleanup ();
return 0;
} /* WinMain */


