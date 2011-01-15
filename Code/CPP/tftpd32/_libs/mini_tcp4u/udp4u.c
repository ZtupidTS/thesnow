/*
 * From Tcp4u v 3.31         Last Revision 08/12/1997  3.31-01
 *
 *===========================================================================
 *
 * Project: Tcp4u,      Library for tcp protocol
 * File:    tcp4.c
 *
 *===========================================================================
 * Source released under GPL license
 */

#include <windows.h>
#include <winsock.h>
#include <time.h>

#include "tcp4u.h"
#include "../log/LogToMonitor.h"


// send data using Udp
int UdpSend (int nFromPort, struct sockaddr_in *sa_to, const char *data, int len)
{
SOCKET s;
struct sockaddr_in sa_from;
int    Rc;
int True=1;

   s = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (s == INVALID_SOCKET)  return TCP4U_ERROR;
   // REUSEADDR option in order to allow thread to open 69 port
   Rc = setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) & True, sizeof True);
   LogToMonitor (Rc==0 ? "UdpSend: Port %d may be reused" : "setsockopt error", nFromPort);

   // populate sa_from
   sa_from.sin_family = AF_INET;
   sa_from.sin_addr.s_addr = htonl (INADDR_ANY); // will be changed by sendto
   sa_from.sin_port = htons (nFromPort);
   Rc = bind (s, & sa_from, sizeof sa_from);
   LogToMonitor ("UdpSend bind returns %d (error %d)", Rc, GetLastError ());
   if (Rc<0) { closesocket (s); return TCP4U_BINDERROR; }

   Rc = sendto (s, data, len, 0, sa_to, sizeof *sa_to);
   LogToMonitor ("sendto returns %d", Rc);
   closesocket (s);
return Rc;
} // UdpSend

