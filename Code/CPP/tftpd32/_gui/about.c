//////////////////////////////////////////////////////
//
// Projet TFTPD32.  Mai 98 Ph.jounin
// File about.c:    Display license
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////


#include "headers.h"

#ifdef _M_X64
#  define TFTPD_STRING   "TFTPD64 v3.51 Build " __DATE__ " " __TIME__
#  define TFTPD_ABOUT_TITLE   "About TFTPD64"
#else
#  define TFTPD_STRING   "TFTPD32 v3.51 Build " __DATE__ " " __TIME__
#  define TFTPD_ABOUT_TITLE   "About TFTPD32"
#endif

//#define INTERIM_VERSION

#ifdef INTERIM_VERSION
   const char LICENSE_TFTPD32 [] =
            "Tftpd32 and Tftpd64 are\r\n"
			"copyrighted by Ph. Jounin\r\n"
            "This in an Interim Release\r\n"
            "Please do not distribute\r\n"
            "NO WARRANTY\r\n\r\n"
            "Full credits will be given into release version\r\n"
            "Offical site: http://tftpd32.jounin.net";
#elif defined BAE_SYSTEMS_LEN_WHITE
const char LICENSE_TFTPD32 [] =
"\r\n\r\n"
"          Tftpd32 Standalone Edition\r\n\r\n"
"       copyrighted 2007 by Philippe Jounin,\r\n"
"        provided by BAE Systems Australia\r\n"
"     for the Australian Department of Defence\r\n\r\n";
#elif defined STANDALONE_EDITION
const char LICENSE_TFTPD32 [] =
"TFTPD32  TFTPD64 版权所有 1998-2010 by Philippe Jounin (philippe@jounin.net) "
"发布于欧洲联盟公共许可 1.1 "
"(参考早期帮助文件或者 EUPL-EN.pdf 中记录的完整许可文本)\r\n\r\n"
"官方网站: http://tftpd32.jounin.net\r\n\r\n"
"Tftpd32 和 TFTPD64 使用了下列版权或者贡献\r\n"
"  - RSA 数据安全中的MD5部分\r\n"
"  - Mike Muss 的 IP 校检计算\r\n"
"  - DHCP lease persistance is from Nick Wagner\r\n" 
"  - DHCP 头文件来自互联网软件\r\n    Consortium\r\n";
#elif defined SERVICE_EDITION
#  ifdef _M_X64
const char LICENSE_TFTPD32 [] =
"Tftpd64 Service Edition copyrighted 2007-2010 by Philippe Jounin\r\n";
#  else
const char LICENSE_TFTPD32 [] =
"Tftpd32 Service Edition copyrighted 2007-2010 by Philippe Jounin\r\n";
#  endif
#endif



LRESULT CALLBACK AboutProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
  switch (message)
  {
       case WM_INITDIALOG :
		   SetWindowText (hWnd, TFTPD_ABOUT_TITLE);
           SetDlgItemText (hWnd, IDC_TFTPD_STRING, TFTPD_STRING);
           SetDlgItemText (hWnd, IDC_ABOUT_TFTPD32, LICENSE_TFTPD32);
           CenterChildWindow (hWnd, CCW_INSIDE | CCW_VISIBLE);
           break;

       case WM_COMMAND :
           switch (wParam)
           {
                case IDOK :
                    EndDialog (hWnd, 0);
                    break;
           }
           break;

       case WM_CLOSE :
#if (defined INTERIM_VERSION  && defined STANDALONE_VERSION)
            sSettings.LogLvl=15;
#endif // INTERIM_VERSION
       case WM_DESTROY :
            EndDialog (hWnd, 0);
            break;

  } // switch

return FALSE;
} // AboutProc


