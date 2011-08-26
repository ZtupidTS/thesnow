// isNoteBook.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "isNoteBook.h"
#ifndef _WIN64
#pragma comment(lib,"setupapi_XP_X86.lib")
#else
#pragma comment(lib,"setupapi_7_X64.lib")
#endif // _WIN32
// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 isNoteBook.h
int isNoteBook(int argc, char *argv[], char *envp[])
{
	 HDEVINFO hDevInfo;  
     SP_DEVINFO_DATA DeviceInfoData;  
     DWORD i;  
     // Create a HDEVINFO with all present devices.  
     hDevInfo = SetupDiGetClassDevs(NULL,  
         0, // Enumerator  
         0,  
         DIGCF_PRESENT | DIGCF_ALLCLASSES );  
       
     if (hDevInfo == INVALID_HANDLE_VALUE)  
     {  
         // Insert error handling here.  
         return -1;
     }  
       
     // Enumerate through all devices in Set.  
       
     TCHAR buffer[4096]={0};  
     DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);  
     for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,  
         &DeviceInfoData);i++)  
     {  
         DWORD DataT;  
         DWORD buffersize = 4096;  
           
         //   
         // Call function with null to begin with,   
         // then use the returned buffer size   
         // to Alloc the buffer. Keep calling until  
         // success or an unknown failure.  
         //   
         SetupDiGetDeviceRegistryProperty(  
             hDevInfo,  
             &DeviceInfoData,  
             SPDRP_HARDWAREID,  
             &DataT,  
             (PBYTE)buffer,  
             buffersize,  
             &buffersize);  
           
         if( wcscmp(buffer, L"ACPI\\ACPI0003")==0 )  
         {  
             SetupDiDestroyDeviceInfoList(hDevInfo);  
  //           printf("It is a laptop\n");  
             return 1;
         }  
     }  
       
       
     if ( GetLastError()!=NO_ERROR &&  
          GetLastError()!=ERROR_NO_MORE_ITEMS )  
     {  
         // Insert error handling here.  
         return -1;
     }  
       
     //  Cleanup  
     SetupDiDestroyDeviceInfoList(hDevInfo);  
    return 0;  
}

int HaveBattery()
{
	SYSTEM_POWER_STATUS pw;
	GetSystemPowerStatus(&pw);
	if (pw.BatteryFlag==128){
		return 0;
	}
	else
	{
	return 1;
	}
};

int BatteryLifeTime()
{
	SYSTEM_POWER_STATUS pw;
	GetSystemPowerStatus(&pw);
	return pw.BatteryLifeTime;
};

int BatteryLifeTimePercent()
{
	SYSTEM_POWER_STATUS pw;
	GetSystemPowerStatus(&pw);
	return pw.BatteryLifePercent;
};

int ACLineStatus()
{
	SYSTEM_POWER_STATUS pw;
	GetSystemPowerStatus(&pw);
	return pw.ACLineStatus;
};

int CPUNum(){
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
};

int CloseMe(){
	ExitProcess(0);
};

/*
LPCWSTR DialGetConnectionsList(){
	DWORD dwCb = 0;
	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwConnections = 0;
	LPRASCONN lpRasConn = NULL;
	LPRASDIALPARAMS lprasdialparams=NULL;
	LPBOOL lpfPassword=FALSE;
	std::wstring Fnstring=L"0";
	// Call RasEnumConnections with lpRasConn = NULL. dwCb is returned with the required buffer size and 
	// a return code of ERROR_BUFFER_TOO_SMALL
	dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

	if (dwRet == ERROR_BUFFER_TOO_SMALL){
		// Allocate the memory needed for the array of RAS structure(s).
		lpRasConn = (LPRASCONN) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
		if (lpRasConn == NULL){
		//	wprintf(L"HeapAlloc failed!\n");
			return L"-1";
		}
		// The first RASCONN structure in the array must contain the RASCONN structure size
		lpRasConn[0].dwSize = sizeof(RASCONN);

		// Call RasEnumConnections to enumerate active connections
		dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

		// If successful, print the names of the active connections.
		if (ERROR_SUCCESS == dwRet){
		//	wprintf(L"The following RAS connections are currently active:\n");
			for (DWORD i = 0; i < dwConnections; i++){
				//wprintf(L"%s\n", lpRasConn[i].szEntryName);
				Fnstring+=lpRasConn[i].szEntryName;
				Fnstring+=L";";
//				MessageBox(NULL,lpRasConn[i].szEntryName,L"",MB_OK);
//				RasHangUp(lpRasConn[i].hrasconn);
//				RasGetEntryDialParams(lpRasConn[i].szPhonebook,lprasdialparams,lpfPassword);
//				RasDial(NULL,lpRasConn[i].szPhonebook,lprasdialparams,NULL,NULL,NULL);
			}
		}
		//Deallocate memory for the connection buffer
		HeapFree(GetProcessHeap(), 0, lpRasConn);
		lpRasConn = NULL;
	}
	WCHAR *fnRet=new WCHAR;
	wcscpy(fnRet,Fnstring.c_str());
	return fnRet;
};

LPCWSTR DialHangUp(LPCWSTR dialName){
	DWORD dwCb = 0;
	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwConnections = 0;
	LPRASCONN lpRasConn = NULL;
	std::wstring Fnstring=L"0";
	// Call RasEnumConnections with lpRasConn = NULL. dwCb is returned with the required buffer size and 
	// a return code of ERROR_BUFFER_TOO_SMALL
	dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
MessageBox(NULL,L"999999",L"",MB_OK);
	if (dwRet == ERROR_BUFFER_TOO_SMALL){
		// Allocate the memory needed for the array of RAS structure(s).
		lpRasConn = (LPRASCONN) new LPRASCONN[dwCb];// HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
		if (lpRasConn == NULL){
			return L"-1";
		}
			MessageBox(NULL,L"999",L"",MB_OK);
		// The first RASCONN structure in the array must contain the RASCONN structure size
		lpRasConn[0].dwSize = sizeof(RASCONN);

		// Call RasEnumConnections to enumerate active connections
		dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
MessageBox(NULL,L"",L"",MB_OK);
		// If successful, print the names of the active connections.
		if (ERROR_SUCCESS == dwRet){
			for (DWORD i = 0; i < dwConnections; i++){
				if (wcscmp(lpRasConn[i].szEntryName,dialName))
				{
					MessageBox(NULL,lpRasConn[i].szEntryName,L"",MB_OK);
	//				RasHangUp(lpRasConn[i].hrasconn);
					Fnstring=L"1";
				}
			}
		}
		//Deallocate memory for the connection buffer
		HeapFree(GetProcessHeap(), 0, lpRasConn);
		lpRasConn = NULL;
	}
	WCHAR *fnRet=new WCHAR;
	wcscpy(fnRet,Fnstring.c_str());
	return fnRet;
};


LPCWSTR DialGetList()
{
	DWORD cbBuf;  
	DWORD cEntry;
	DWORD dwRet;  
	LPBYTE lpBuffer;  
	std::wstring Fnstring=L"";
//	char  szMessage[256];  

#pragma region //获得所有连接  
	LPRASENTRYNAME lpRasEntry = new RASENTRYNAME;
	RtlZeroMemory(lpRasEntry, sizeof(RASENTRYNAME) );
	lpRasEntry->dwSize = sizeof(RASENTRYNAME);  
	cbBuf = sizeof(RASENTRYNAME);  

	dwRet = RasEnumEntries(NULL, NULL, lpRasEntry, &cbBuf, &cEntry); //枚举连接，系统会把需要的缓冲区大小放在cuBuf中 
	delete lpRasEntry;

	lpBuffer = new BYTE[cbBuf];   //分配缓冲区
	lpRasEntry = (LPRASENTRYNAME)lpBuffer;  
	lpRasEntry->dwSize = sizeof(RASENTRYNAME);   //初始化dwSize参数 必须!!!!
	dwRet = RasEnumEntries(NULL, NULL, lpRasEntry, &cbBuf, &cEntry);   //枚举所有连接，连接详细信息保存在lpRasEntry所指向的缓冲区中， cEntry中为所获得的连接个数
	if (ERROR_BUFFER_TOO_SMALL == dwRet)   //判断枚举所有连接是否成功， 一般都会成功
	{  
		delete lpRasEntry;  
		lpRasEntry = NULL;  
		lpRasEntry = (LPRASENTRYNAME) new WCHAR[cbBuf];  
		if (NULL != lpRasEntry)  
		{  
			ZeroMemory(lpRasEntry, cbBuf);  
			lpRasEntry->dwSize = cbBuf;  
			dwRet = RasEnumEntries(NULL, NULL, lpRasEntry, &cbBuf, &cEntry);  
		}  
		else  
		{
			dwRet = ERROR_NOT_ENOUGH_MEMORY;
			return L"0";
		}
	}  
	if (0 != dwRet) // 其他错误  
	{  
			return L"-1";
	}  
	if (0 == cEntry) //枚举不到连接， 可能系统没有拨号相关的连接 
	{  
		delete lpRasEntry ; 
		return L"-2";
	}  
	BOOL bSet = FALSE;  
	if (0 == dwRet)  // 成功枚举连接  
	{  
		for (DWORD ndx = 0; ndx < cEntry; ndx++)  
		{
//			printf("%d   %s   %d   %s\n",lpRasEntry[ndx].dwSize, //打印枚举得到的连接信息
//				lpRasEntry[ndx].szEntryName,
//				lpRasEntry[ndx].dwFlags,
//				lpRasEntry[ndx].szPhonebookPath);
			Fnstring+=lpRasEntry[ndx].szEntryName;
			Fnstring+=L";";
		}  
		delete lpRasEntry ; 
	}
	WCHAR *fnRet=new WCHAR;
	wcscpy(fnRet,Fnstring.c_str());
	return fnRet;
}

*/