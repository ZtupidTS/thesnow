/*
 * From Tcp4u v 3.31         Last Revision 08/12/1997  3.31-01
 *
 *===========================================================================
 *
 * Project: Tcp4u,      Library for tcp protocol
 * File:    tcp4.c
 *
 *===========================================================================
* Source released under under European Union Public License
 */


#include <windows.h>
#include <winsock.h>
#include <time.h>

#include "tcp4u.h"
#include "..\log\logtomonitor.h"


struct in_addr Tcp4uGetIPAddr (LPCSTR szHost)
{
struct hostent *  lpHostEnt;
struct in_addr    sin_addr;

  sin_addr.s_addr = inet_addr (szHost); /* doted address */
  if (sin_addr.s_addr==INADDR_NONE)     /* si pas une doted address  */
  {                      /* regarder le fichier hosts */
      lpHostEnt = gethostbyname (szHost);
      if (lpHostEnt!=NULL)
         memcpy (& sin_addr.s_addr, lpHostEnt->h_addr, lpHostEnt->h_length);
    }
return sin_addr;
} /* Tcp4uGetIPAddr */


SOCKET TcpGetListenSocket (LPCSTR szService, unsigned short * pPort)
{
struct sockaddr_in   saSockAddr; /* specifications pour le Accept */
int                  sockaddr_size = sizeof (struct sockaddr);
SOCKET               ListenSock;
int                  Rc;
struct servent     * lpServEnt;


   ListenSock = INVALID_SOCKET;
   /* --- 1er champ de saSockAddr : Port */
   lpServEnt =  (szService==NULL) ? NULL : getservbyname (szService, "tcp") ;
   saSockAddr.sin_port = (lpServEnt!=NULL) ? lpServEnt->s_port : htons (*pPort);
   
  /* create socket */
  ListenSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (ListenSock == INVALID_SOCKET)  return INVALID_SOCKET;

  saSockAddr.sin_family = AF_INET;
  saSockAddr.sin_addr.s_addr=INADDR_ANY;
  /* Bind name to socket */
  Rc =   bind (ListenSock,(struct sockaddr *) & saSockAddr, sockaddr_size);
  if (Rc==SOCKET_ERROR)
  {
	  Rc = GetLastError ();
      closesocket (ListenSock);
	  SetLastError (Rc);
      return  INVALID_SOCKET;
  }
  Rc = listen (ListenSock, 1);
  if (Rc==SOCKET_ERROR)
  {
	  Rc = GetLastError ();
      closesocket (ListenSock);
	  SetLastError (Rc);
      return  INVALID_SOCKET;
  }
  Rc = getsockname (ListenSock, (struct sockaddr *) & saSockAddr, & sockaddr_size );
  if (Rc==0)
	 *pPort = htons (saSockAddr.sin_port);
return ListenSock;
}  /* TcpGetListenSock */



// --------------------
// Recv
// --------------------

int TcpRecv (SOCKET s, LPSTR szBuf, unsigned uBufSize, 
                     unsigned uTimeOut, HANDLE hLogFile)
{
int             Rc, nUpRc;  /* Return Code of select and recv */
struct timeval  TO;         /* Time Out structure             */
struct timeval *pTO;        /* Time Out structure             */
fd_set          ReadMask;   /* select mask                    */
DWORD           dummy;

  if (s==INVALID_SOCKET)  return TCP4U_ERROR;

  FD_ZERO (& ReadMask);     /* mise a zero du masque */
  FD_SET (s, & ReadMask);   /* Attente d'evenement en lecture */

  /* detail des modes */
  switch (uTimeOut)
  {
      case  TCP4U_WAITFOREVER : pTO = NULL; 
                                break;
      case  TCP4U_DONTWAIT    : TO.tv_sec = TO.tv_usec=0 ; 
                                pTO = & TO;
                                break;
      /* Otherwise  uTimeout is really the Timeout */
      default :                 TO.tv_sec = (long) uTimeOut;
                                TO.tv_usec=0;
                                pTO = & TO;
                                break;
  }
  /* s+1 normally unused but better for a lot of bugged TCP Stacks */
  Rc = select (s+1, & ReadMask, NULL, NULL, pTO);
  if (Rc<0) 
  {
	 LogToMonitor ("select returns error %d\n", WSAGetLastError());
     return  TCP4U_ERROR;
  }
  if (Rc==0)
     return  TCP4U_TIMEOUT;  /* timeout en reception           */

  if (szBuf==NULL  ||  uBufSize==0)  
	return TCP4U_SUCCESS;
  
  Rc = recv (s, szBuf, uBufSize, 0);  /* chgt 11/01/95 */
 
  switch (Rc)
  {
       case SOCKET_ERROR : 
		 	  LogToMonitor ("recv returns error %d\n", WSAGetLastError());
              nUpRc = TCP4U_ERROR ; 
              break;
       case 0            : 
              nUpRc = TCP4U_SOCKETCLOSED ; 
              break;
       default :
              if (hLogFile!=INVALID_HANDLE_VALUE)    WriteFile (hLogFile, szBuf, Rc, &dummy, NULL);
              nUpRc = Rc;
              break;
  } /* translation des codes d'erreurs */
return nUpRc;
} /* TcpRecv */


int TcpSend (SOCKET s, LPCSTR szBuf, unsigned uBufSize, HANDLE hLogFile)
{
int      Rc;
unsigned Total;
DWORD    dummy;

  if (s==INVALID_SOCKET)  return TCP4U_ERROR;
  if (hLogFile!=INVALID_HANDLE_VALUE)    WriteFile (hLogFile, szBuf, uBufSize, &dummy, NULL);

  for ( Total = 0, Rc = 1 ;  Total < uBufSize  &&  Rc > 0 ;  Total += Rc)
  {
      Rc = send (s, & szBuf[Total], uBufSize-Total, 0);
  }
  
return Total>=uBufSize ? TCP4U_SUCCESS :  TCP4U_ERROR;
} /* TcpSend */


int TcpPPSend (SOCKET s, LPCSTR szBuf, unsigned uBufSize, HANDLE hLogFile)
{
int      Rc;
unsigned Total;
DWORD    dummy;
unsigned short usSize = htons (uBufSize);

  if (s==INVALID_SOCKET)  return TCP4U_ERROR;
  if (uBufSize > 0x7FFF)  return TCP4U_OVERFLOW;
  if (hLogFile!=INVALID_HANDLE_VALUE)    WriteFile (hLogFile, szBuf, uBufSize, &dummy, NULL);

  // send msg length
  Rc = send (s, (char *) & usSize, sizeof usSize, 0);
  for ( Total = 0 ;  Total < uBufSize  &&  Rc > 0 ;  Total += Rc)
  {
      Rc = send (s, & szBuf[Total], uBufSize-Total, 0);
  }
  if (Rc<0) 
		LogToMonitor ("send returns error %d\n", WSAGetLastError());
 
return Total>=uBufSize ? TCP4U_SUCCESS :  TCP4U_ERROR;
} /* TcpPPSend */



int TcpPPRecv (SOCKET s, LPSTR szBuf, unsigned uBufSize, int uTimeOut, HANDLE hLogFile)
{
unsigned short usToBeReceived=0; 
int            usUpRc, usReceived;
int            Rc;

  usReceived = 0;
  usUpRc=1;
  // get number of byte to be expected
  Rc = TcpRecv (s, (LPSTR) & usToBeReceived, sizeof usToBeReceived, uTimeOut, hLogFile);
  if (Rc!=sizeof usToBeReceived)  return Rc;
  // put in machine ordrer
  usToBeReceived = ntohs (usToBeReceived);
  
  if (usToBeReceived > 0x7FFF)  return TCP4U_OVERFLOW;
  
  // loop while msg is shorter than expected or timeout
  while (usUpRc>0  &&  usReceived < usToBeReceived  )
  {
      usUpRc = TcpRecv (s, szBuf, usToBeReceived - usReceived, uTimeOut, hLogFile);
      usReceived += usUpRc;
      szBuf += usUpRc;
  }

return usUpRc>0 ? usReceived : usUpRc ;
} /* TcpPPRecv */



SOCKET TcpConnect (LPCSTR szHost,
                   LPCSTR szService, 
                   unsigned short nPort)
{
int                   Rc;
struct sockaddr_in    saSockAddr;
struct servent     *  lpServEnt;
SOCKET                connect_skt;

  /* --- 1er champ de saSockAddr : Port */
  lpServEnt = (szService==NULL) ?  NULL :  getservbyname (szService, "tcp") ;  
  saSockAddr.sin_port = lpServEnt!=NULL ?   lpServEnt->s_port : htons(nPort);
  /* --- 2eme champ de saSockAddr : Addresse serveur */
  saSockAddr.sin_addr = Tcp4uGetIPAddr (szHost);
  if (saSockAddr.sin_addr.s_addr==INADDR_NONE)   return INVALID_SOCKET;
  /* --- Dernier champ : liaison connectie */
  saSockAddr.sin_family      = AF_INET; /* on utilise le mode connecte TCP */
  /* --- creation de la socket */
  if ( (connect_skt = socket (PF_INET, SOCK_STREAM, 0))==SOCKET_ERROR)
       return  SOCKET_ERROR;
  /* --- connect retourne INVALID_SOCKET ou numero valide */
  Rc = connect (connect_skt,(struct sockaddr *) & saSockAddr, sizeof saSockAddr);
  /* --- enregistrement dans notre table */
return Rc==0 ? connect_skt : SOCKET_ERROR;
}  /* TcpConnect */

