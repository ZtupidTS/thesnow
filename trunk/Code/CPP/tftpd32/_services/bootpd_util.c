//////////////////////////////////////////////////////
//
// Projet TFTPD32.  January 2006 Ph.jounin
// Projet DHCPD32.  January 2006 Ph.jounin
// File bootpd_util.c:    Manage BOOTP/DHCP protocols
//
// source released under European Union Public License
//
//////////////////////////////////////////////////////

#include "headers.h"

#include <stdio.h>          // sscanf is used
#include <process.h>        // endthread + beginthread

#include "threading.h"
#include "bootpd_functions.h"


//////////////////////////////////////////////////////////////////////////////////////////////
// AddOption Translation
// Translates the option values into a value
// Use the SnmpCmd syntax ("a ip @" "s string", "i integer", "x string", "u unsigned")
//////////////////////////////////////////////////////////////////////////////////////////////
int TranslateParam2Value (void *buffer, int len, const char *opt_val, struct in_addr ip, const char *tMac)
{
char sz[256];
   if (opt_val[1] == ' ')
   {
	   switch (opt_val [0])
	   {
			case 'a' : // IP address
				* (unsigned long *) buffer = inet_addr (opt_val+2);
				return sizeof (unsigned long);
			case 's' : // string
					lstrcpyn (sz, opt_val+2, len);
					TranslateExp (sz, buffer, ip, tMac);
					((char *) buffer) [len-1] = 0;
					return lstrlen (buffer);
			case 'i' :	// integer 
			    * (unsigned long *) buffer = atoi (opt_val+2);
				return sizeof (unsigned long);
			case '0' :
			case 'x' :	// hex digit
				sscanf(opt_val+2, "%x", buffer);
				return sizeof (unsigned long);
			case 'u' :	// hex digit
				sscanf(opt_val+2, "%u", buffer);
				return sizeof (unsigned long);
	   } // switch opt_val[0]
   }
   // non trouve --> conserver la chaine
	lstrcpyn (sz, opt_val, len);
	TranslateExp (sz, buffer, ip, tMac);
	( (char *) buffer) [len-1] = 0;
	return lstrlen (buffer);
} // TranslateParam2Value




///////////////////////////////////////////
//  DHCP configuration management
///////////////////////////////////////////

// The data to translate the registry entries into the sParamDHCP struct
// DHCP parameters in configuration file or registry
static struct
{
   char *szEntry;
   void *pValue;
   int   nType;
   int   nBufSize;
}
tDHCPd32Entry[] =
{
    KEY_DHCP_POOL,      & sParamDHCP.dwAddr.s_addr,    REG_DWORD, sizeof sParamDHCP.dwAddr.s_addr,
    KEY_DHCP_POOLSIZE,  & sParamDHCP.nPoolSize,        REG_DWORD, sizeof sParamDHCP.nPoolSize,
    KEY_DHCP_BOOTFILE,    sParamDHCP.szBootFile,       REG_SZ,    sizeof sParamDHCP.szBootFile,
    KEY_DHCP_DNS,       & sParamDHCP.dwDns.s_addr,     REG_DWORD, sizeof sParamDHCP.dwDns.s_addr,
    KEY_DHCP_MASK,      & sParamDHCP.dwMask.s_addr,    REG_DWORD, sizeof sParamDHCP.dwMask.s_addr,
    KEY_DHCP_DEFROUTER, & sParamDHCP.dwGateway.s_addr, REG_DWORD, sizeof sParamDHCP.dwGateway.s_addr,
    KEY_DHCP_DOMAINNAME,  sParamDHCP.szDomainName,     REG_SZ,    sizeof sParamDHCP.szDomainName,
    KEY_DHCP_LEASE_TIME,& sParamDHCP.nLease,           REG_DWORD, sizeof sParamDHCP.nLease,

}; // tDHCPd32Entry



// Save configuration either in INI file (if it exists) or in registry
int DHCPSaveConfig ( const struct S_DHCP_Param  *pNewParamDHCP )
{
INT   Ark;
char szBuf[64];

     // allocate new array, but keep pointers
     if ( sParamDHCP.nPoolSize!=pNewParamDHCP->nPoolSize )
	 {
         tFirstIP = realloc (tFirstIP, sizeof (tFirstIP[0]) * pNewParamDHCP->nPoolSize);
         tMAC     = realloc (tMAC,     sizeof (tMAC[0]) * pNewParamDHCP->nPoolSize);
		      // do not complain if pool is empty
         if (pNewParamDHCP->nPoolSize!=0 && (tFirstIP==NULL || tMAC==NULL) )
		{
			SVC_ERROR ("Can not allocate memory");
			return FALSE;
		}
	 }
     
     nAllocatedIP = min (nAllocatedIP, pNewParamDHCP->nPoolSize);

     sParamDHCP = *pNewParamDHCP;

     for (Ark=0 ; Ark<SizeOfTab (tDHCPd32Entry) ; Ark++)
        AsyncSaveKey (  TFTPD32_DHCP_KEY,
                        tDHCPd32Entry [Ark].szEntry,
                        tDHCPd32Entry [Ark].pValue,
                        tDHCPd32Entry [Ark].nBufSize,
                        tDHCPd32Entry [Ark].nType,
                        szTftpd32IniFile );
 
 // custom items
    for (Ark=0 ; Ark < SizeOfTab (sParamDHCP.t) ; Ark++)
    {
       wsprintf (szBuf, "%s%d", KEY_DHCP_USER_OPTION_NB, Ark+1);
       AsyncSaveKey  (TFTPD32_DHCP_KEY, szBuf, & sParamDHCP.t[Ark].nAddOption,
                 sizeof sParamDHCP.t[Ark].nAddOption, REG_DWORD, szTftpd32IniFile);
       wsprintf (szBuf, "%s%d", KEY_DHCP_USER_OPTION_VALUE, Ark+1);
       AsyncSaveKey  (TFTPD32_DHCP_KEY,  szBuf, sParamDHCP.t[Ark].szAddOption,
                 sizeof sParamDHCP.t[Ark].szAddOption, REG_SZ, szTftpd32IniFile);
    }

return TRUE;
} // DHCPSaveConfig

// read configuration either from INI file (if it exists) or from the registry
int DHCPReadConfig ( void )
{
int   Ark;
char szBuf[128];

   memset (& sParamDHCP, 0, sizeof sParamDHCP);
   sParamDHCP.nLease = DHCP_DEFAULT_LEASE_TIME;

   for (Ark=0 ; Ark<SizeOfTab (tDHCPd32Entry) ; Ark++)
        ReadKey (  TFTPD32_DHCP_KEY, 
                   tDHCPd32Entry [Ark].szEntry,
                   tDHCPd32Entry [Ark].pValue,
                   tDHCPd32Entry [Ark].nBufSize,
                   tDHCPd32Entry [Ark].nType,
                   szTftpd32IniFile );
    // custom items
   for (Ark=0 ; Ark < SizeOfTab (sParamDHCP.t) ; Ark++)
   {
      wsprintf (szBuf, "%s%d", KEY_DHCP_USER_OPTION_NB, Ark+1);
      ReadKey  (TFTPD32_DHCP_KEY, szBuf, & sParamDHCP.t[Ark].nAddOption, 
                sizeof sParamDHCP.t[Ark].nAddOption, REG_DWORD, szTftpd32IniFile);

       wsprintf (szBuf, "%s%d", KEY_DHCP_USER_OPTION_VALUE, Ark+1);
       ReadKey  (TFTPD32_DHCP_KEY,  szBuf, sParamDHCP.t[Ark].szAddOption, 
                 sizeof sParamDHCP.t[Ark].szAddOption, REG_SZ, szTftpd32IniFile);
   }

   if ( sParamDHCP.nPoolSize!=0 )
   {
	  tFirstIP = malloc (sParamDHCP.nPoolSize * sizeof *tFirstIP[0]) ;
	  tMAC = malloc (sParamDHCP.nPoolSize * sizeof *tMAC[0]) ; 
	  if (tFirstIP == NULL  ||  tMAC == NULL ) 
	   {
			SVC_ERROR ("Can not allocate memory");
			return FALSE;
	   }
	   LoadLeases ();
   }

   if (sParamDHCP.nLease==0)
   {  
      sParamDHCP.nLease=DHCP_DEFAULT_LEASE_TIME;
      LOG (12, "%d, Lease time not specified, set to 2 days", GetCurrentThreadId ());
   }

return TRUE;
} // DHCPReadConfig


///////////////////////////////////////////
//  translation
///////////////////////////////////////////
// translate $IP$ and $MAC$ keywords
char *TranslateExp (const char *exp, char *to, struct in_addr ip, const char *tMac)
{
char *q;
int  Ark;
char sz [256];		// somewhat larger that DHCP_FILE_LEN (128 bytes)

	// truncate input
	Ark = strnlen ( to, DHCP_FILE_LEN -1 );  	to [Ark] = 0;

// LOG (1, "bootp file fmt is <%s>\n", exp);
// LOG (1, "rqst file is <%s>\n", to);
// LOG (1, "IP <%s>\n", inet_ntoa (ip));
// LOG (1, "MAC is <%s>\n", haddrtoa (tMac, 6, '.'));


    if ( (q=strstr (exp, "$IP$")) != NULL )
    {
       lstrcpyn (sz, exp, 1 + q - exp);
       lstrcat (sz, inet_ntoa (ip) );
       lstrcat (sz, q + sizeof "$IP$" - 1);
       lstrcpyn (to, sz, DHCP_FILE_LEN - 1);
    }
    else if ( (q=strstr (exp, "$MAC$")) != NULL )
    {
       lstrcpyn (sz, exp, 1 + q - exp);
       lstrcat (sz, haddrtoa (tMac, 6, '.') );
       lstrcat (sz, q + sizeof "$MAC$" - 1);
       lstrcpyn (to, sz, DHCP_FILE_LEN - 1);
    }
    else if ( (q=strstr (exp, "$BootFileName$")) != NULL )
    {
	   lstrcpyn (sz, exp, 1 + q - exp);
	   lstrcat (sz, to);
       lstrcat (sz, q + sizeof "$BootFileName$" - 1);
	   // replace to now
       lstrcpyn (to, sz, DHCP_FILE_LEN - 1);
    }
    else lstrcpyn (to, exp, DHCP_FILE_LEN - 1);

    // truncate 
    to [DHCP_FILE_LEN-1]=0;
return to;
} // TranslateExp 



///////////////////////////////////////////
//  DHCP database management
///////////////////////////////////////////
// Returns true if the macaddr is empty
int IsMacEmpty(struct LL_IP* pcur)
{
	return (pcur->sMacAddr[0] == 0) && (pcur->sMacAddr[1] == 0) && (pcur->sMacAddr[2] == 0) &&
		   (pcur->sMacAddr[3] == 0) && (pcur->sMacAddr[4] == 0) && (pcur->sMacAddr[5] == 0);
}

// Comparison between two struct. Serves to sort Array
int QsortCompare (const void *p1, const void *p2)
{
	DWORD addr1 = ntohl((*(const struct LL_IP**)p1)->dwIP.s_addr);
	DWORD addr2 = ntohl((*(const struct LL_IP**)p2)->dwIP.s_addr);

	if(addr1 < addr2)
		return -1;
	if(addr1 == addr2)
		return 0;
	return 1;
} // QsortCompare

int MACCompare (const void* p1, const void* p2)
{
const struct LL_IP  *ip1 = *(const struct LL_IP **) p1;
const struct LL_IP  *ip2 = *(const struct LL_IP **) p2;

return memcmp(ip1->sMacAddr, ip2->sMacAddr, 6);
}

//Increment the number of allocated entries
void IncNumAllocated()
{
   ++nAllocatedIP;
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, KEY_LEASE_NUMLEASES, &nAllocatedIP, sizeof(nAllocatedIP), REG_DWORD, szTftpd32IniFile);
}

//Decrement the number of allocated entries
void DecNumAllocated()
{
   --nAllocatedIP;
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, KEY_LEASE_NUMLEASES, &nAllocatedIP, sizeof(nAllocatedIP), REG_DWORD, szTftpd32IniFile);
}

//Set the number of allocated entries explicitly
void SetNumAllocated(int n)
{
   nAllocatedIP = n;
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, KEY_LEASE_NUMLEASES, &nAllocatedIP, sizeof(nAllocatedIP), REG_DWORD, szTftpd32IniFile);
}

//Set the IP address to a new one
void SetIP(struct LL_IP* pCur, DWORD newip)
{
   char* addr;
   char key [_MAX_PATH];
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_IP);
   
   pCur->dwIP.s_addr = newip;
   addr = inet_ntoa(pCur->dwIP);
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, addr, strlen(addr), REG_SZ, szTftpd32IniFile);
}

//Set the MAC address to a new one
void SetMacAddr(struct LL_IP* pCur, const unsigned char *pMac, int nMacLen)
{
   char key [_MAX_PATH];
   char* macaddr;
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_MAC);

   memset(pCur->sMacAddr, 0, sizeof pCur->sMacAddr);
   memcpy(pCur->sMacAddr, pMac, min (sizeof pCur->sMacAddr, nMacLen));
   macaddr = haddrtoa(pCur->sMacAddr, 6, ':');
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, macaddr, strlen(macaddr), REG_SZ, szTftpd32IniFile);
}

//Set the MAC address to all zeros
void ZeroMacAddr(struct LL_IP* pCur)
{
   char key [_MAX_PATH];
   char* macaddr;
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_MAC);

   memset(pCur->sMacAddr, 0, sizeof pCur->sMacAddr);
   macaddr = haddrtoa(pCur->sMacAddr, 6, ':');
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, macaddr, strlen(macaddr), REG_SZ, szTftpd32IniFile);
}

//Set tAllocated to the current time
void SetAllocTime(struct LL_IP* pCur)
{
   char* t;
   char key [_MAX_PATH];
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_ALLOC);

   time(&pCur->tAllocated);
   t = timetoa(pCur->tAllocated);
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, t, strlen(t)+1, REG_SZ, szTftpd32IniFile);
}

//Zero tAllocated
void ZeroAllocTime(struct LL_IP* pCur)
{
   char* t;
   char key [_MAX_PATH];
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_ALLOC);

   pCur->tAllocated = 0;
   t = timetoa(pCur->tAllocated);
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, t, strlen(t) + 1, REG_SZ, szTftpd32IniFile);
}

//Set tRenewed to the current time
void SetRenewTime(struct LL_IP* pCur)
{
   char* t;
   char key [_MAX_PATH];
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_RENEW);

   time(&pCur->tRenewed);
   t = timetoa(pCur->tRenewed);
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, t, strlen(t) + 1, REG_SZ, szTftpd32IniFile);
}

//Force tRenewed to a specific time
void ForceRenewTime(struct LL_IP* pCur, time_t newtime)
{
   char* t;
   char key [_MAX_PATH];
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_RENEW);

   pCur->tRenewed = newtime;
   t = timetoa(pCur->tRenewed);
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, t, strlen(t) + 1, REG_SZ, szTftpd32IniFile);
}

//Zero tRenewed
void ZeroRenewTime(struct LL_IP* pCur)
{
   char* t;
   char key [_MAX_PATH];
   sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, pCur->dwAllocNum, KEY_LEASE_RENEW);

   pCur->tRenewed = 0;
   t = timetoa(pCur->tRenewed);
   if (sSettings.bPersLeases)
      AsyncSaveKey(TFTPD32_DHCP_KEY, key, t, strlen(t) + 1, REG_SZ, szTftpd32IniFile);
}

//Completely renumbers and rewrites the lease list from current membory.  
void ReorderLeases()
{
   int i;
   AsyncSaveKey (TFTPD32_DHCP_KEY, 
				 KEY_LEASE_NUMLEASES, 
				 & nAllocatedIP, 
				 sizeof(nAllocatedIP), 
				 REG_DWORD, 
				 szTftpd32IniFile);

   for(i = 0; i < nAllocatedIP; ++i)
   {
      char key [_MAX_PATH];
      char* macaddr = haddrtoa(tFirstIP[i]->sMacAddr, 6, ':');
      char* addr = inet_ntoa(tFirstIP[i]->dwIP);
      char* alloc = timetoa(tFirstIP[i]->tAllocated);
      char* renew = timetoa(tFirstIP[i]->tRenewed);

      tFirstIP[i]->dwAllocNum = i;
      sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, i, KEY_LEASE_MAC);
      SaveKey(TFTPD32_DHCP_KEY, key, macaddr, strlen(macaddr) + 1, REG_SZ, szTftpd32IniFile);
      sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, i, KEY_LEASE_IP);
      SaveKey(TFTPD32_DHCP_KEY, key, addr, strlen(addr) + 1, REG_SZ, szTftpd32IniFile);
      sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, i, KEY_LEASE_ALLOC);
      SaveKey(TFTPD32_DHCP_KEY, key, alloc, strlen(alloc) + 1, REG_SZ, szTftpd32IniFile);
      sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, i, KEY_LEASE_RENEW);
      if (sSettings.bPersLeases)
         AsyncSaveKey(TFTPD32_DHCP_KEY, key, renew, strlen(renew) + 1, REG_SZ, szTftpd32IniFile);
   }
}

//Read in and initialize the leases
void LoadLeases(void)
{
   //We need to make sure the leases we load actually fit in the address pool, so we'll be
   //tracking the index to the lease file and the index to the allocated list
   int leaseindex, allocindex;
 
   // From Nick : I realized that there was a race condition in that code, 
   // particularly with the reading and saving of KEY_LEASE_NUMLEASES
   // I’ve added a function, which LoadLeases calls immediately on entry:
   WaitForMsgQueueToFinish (LL_ID_SETTINGS);

   nAllocatedIP = 0;
   ReadKey(TFTPD32_DHCP_KEY, KEY_LEASE_NUMLEASES, &nAllocatedIP, sizeof(nAllocatedIP), REG_DWORD, szTftpd32IniFile);

   if (nAllocatedIP > sParamDHCP.nPoolSize)
   {
      SVC_WARNING ("The pool size is too small for the number of leases, ignoring extra leases");
      nAllocatedIP = sParamDHCP.nPoolSize;
   }

   allocindex = 0;
   for(leaseindex = 0; leaseindex < nAllocatedIP; ++leaseindex)
   {
     char key [_MAX_PATH];
     char tmpval [_MAX_PATH];

     tFirstIP[allocindex] = malloc (sizeof(struct LL_IP));
     memset(tFirstIP[allocindex], 0, sizeof(struct LL_IP));

     tFirstIP[allocindex]->dwAllocNum = leaseindex;
     sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, leaseindex, KEY_LEASE_MAC);
     if(ReadKey(TFTPD32_DHCP_KEY, key, tmpval, _MAX_PATH, REG_SZ, szTftpd32IniFile))
        atohaddr(tmpval, tFirstIP[allocindex]->sMacAddr, 6);
     sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, leaseindex, KEY_LEASE_IP);
     if(ReadKey(TFTPD32_DHCP_KEY, key, tmpval, _MAX_PATH, REG_SZ, szTftpd32IniFile))
        tFirstIP[allocindex]->dwIP.s_addr = inet_addr(tmpval);
     sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, leaseindex, KEY_LEASE_ALLOC);
     if(ReadKey(TFTPD32_DHCP_KEY, key, tmpval, _MAX_PATH, REG_SZ, szTftpd32IniFile))     
        tFirstIP[allocindex]->tAllocated = atotime(tmpval);
     sprintf(key, "%s%d%s", KEY_LEASE_PREFIX, leaseindex, KEY_LEASE_RENEW);
     if(ReadKey(TFTPD32_DHCP_KEY, key, tmpval, _MAX_PATH, REG_SZ, szTftpd32IniFile))
        tFirstIP[allocindex]->tRenewed = atotime(tmpval);

    // fix errors in date conversion (registry modified at hand)
    if (tFirstIP[allocindex]->tAllocated == -1) tFirstIP[allocindex]->tAllocated = 0;
    if (tFirstIP[allocindex]->tRenewed   == -1) tFirstIP[allocindex]->tRenewed   = 0;

     //If the address doesn't fit in the pool, don't add it after all
	 //Since we are assuming the leases were written in order, do a quick check for dups
	 //and invalid macaddrs
     if((!AddrFitsPool(&tFirstIP[allocindex]->dwIP)) || (IsMacEmpty(tFirstIP[allocindex])) ||
		((allocindex > 0) && (tFirstIP[allocindex]->dwIP.s_addr == tFirstIP[allocindex - 1]->dwIP.s_addr)))
     {
        free(tFirstIP[allocindex]);
        tFirstIP[allocindex] = NULL;
     }
     else
	 {
		tMAC[allocindex] = tFirstIP[allocindex];  //Copy to cross index
        ++allocindex;   //Move on to the next one
	 }
   }

   if(allocindex != nAllocatedIP)
      SetNumAllocated(allocindex);

    // ensure that data base is sorted (especially if we've dropped some leases in the load)
    qsort (tMAC, nAllocatedIP, sizeof *tMAC, MACCompare);
    qsort (tFirstIP, nAllocatedIP, sizeof *tFirstIP, QsortCompare);
    ReorderLeases();

} // LoadLeases


//Free the Lease memory
void FreeLeases(BOOL freepool)
{
	int Ark;

	//Free the individual elements
	for(Ark = 0; Ark < nAllocatedIP; ++Ark)
	{
		free(tFirstIP[Ark]);
		tFirstIP[Ark] = NULL;
		tMAC[Ark] = NULL;  //Yes, they don't point to the same thing, but we're clearing everyone out.
	}
	if(freepool)
	{
		if(tFirstIP)
		{
			free(tFirstIP);
			tFirstIP = NULL;
		}
		if(tMAC)
		{
			free(tMAC);
			tMAC = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Data Base Queries
// Retrieve a DHCP item in the data base 
//////////////////////////////////////////////////////////////////////////////////////////////


// Search in the list by IP
struct LL_IP *DHCPSearchByIP (const struct in_addr *pAddr, BOOL* wasexpired)
{
//	int  Ark;
	struct LL_IP** pArk;
	struct LL_IP key, *pkey;
	time_t expire;
	time(&expire);
	expire -= sParamDHCP.nLease * 60; //Adjust now so there's just a simple compare to tRenewed
	
	key.dwIP.s_addr = pAddr->s_addr;
	pkey = &key;
	pArk = bsearch(&pkey, tFirstIP, nAllocatedIP, sizeof(*tFirstIP), QsortCompare);
	if(pArk)
		*wasexpired = ((*pArk)->tRenewed == 0) || (expire > (*pArk)->tRenewed);
	if(pArk)
		return *pArk;
	return NULL;

#if 0
	for (Ark = 0 ;
		 Ark<nAllocatedIP && ! (tFirstIP[Ark]->dwIP.s_addr==pAddr->s_addr) ;
		 Ark++ );

	if(Ark < nAllocatedIP)
	{
		*wasexpired = (tFirstIP[Ark]->tRenewed==0) || (expire > tFirstIP[Ark]->tRenewed);
		return tFirstIP[Ark];
	}
	return NULL;
#endif
} // DHCPSearchByIP


// Search in the list by Mac Address
struct LL_IP *DHCPSearchByMacAddress (const unsigned char *pMac, int nMacLen)
{
	struct LL_IP** pArk;
	struct LL_IP key, *pkey;

	nMacLen = min (nMacLen, 6);
	memcpy(key.sMacAddr, pMac, nMacLen);
	pkey = &key;
	pArk = bsearch(&pkey, tMAC, nAllocatedIP, sizeof(*tMAC), MACCompare);
	if(pArk)
		return *pArk;
	return NULL;

#if 0
int Ark;
   nMacLen = min (nMacLen, sizeof tFirstIP[0]->sMacAddr);
    for (Ark=0 ;
         Ark<nAllocatedIP && memcmp (tFirstIP[Ark]->sMacAddr, pMac, nMacLen)!=0 ;
         Ark++);
return Ark<nAllocatedIP ? tFirstIP[Ark] : NULL ;
#endif
} // DHCPSearchByMacAddress


#if 0 //We are no longer using the registry
// Search in configuration file/registry by Mac Address
struct LL_IP *DHCPSearchByRegistry (const unsigned char *pMac, int nMacLen)
{
int           Rc;
HKEY          hKey;
char          szIP[20];
DWORD dwSize;

   if (nMacLen!=6) return NULL; // work only for Ethernet and Token Ring
   szIP[0] = 0;

   Rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,    // Key handle at root level.
                      TFTPD32_DHCP_KEY,      // Path name of child key.
                      0,                        // Reserved.
                      KEY_READ,                // Requesting read access.
                    & hKey) == ERROR_SUCCESS;                    // Address of key to be returned.
   
   if (Rc)  READKEY (haddrtoa(pMac, nMacLen), szIP);
   CloseHandle (hKey);

   if (isdigit (szIP[0]))    // entry has been found
        return DHCPReallocItem (NULL, inet_addr (szIP), pMac, nMacLen);
return NULL;
} // DHCPSearchByRegistry
#endif


