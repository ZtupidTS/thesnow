#include <Windows.h>
#include <TCHAR.h>
//#include <Winternl.h>

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

//typedef UNICODE_STRING OBJECT_NAME_INFORMATION;
//typedef UNICODE_STRING *POBJECT_NAME_INFORMATION;

#ifndef NTSTATUS 
typedef LONG NTSTATUS; 
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0) 
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L) 
#endif 

#ifndef SYSTEM_INFORMATION_CLASS 
typedef enum _SYSTEM_INFORMATION_CLASS { 
	SystemBasicInformation, // 0 
	SystemProcessorInformation, // 1 
	SystemPerformanceInformation, // 2 
	SystemTimeOfDayInformation, // 3 
	SystemNotImplemented1, // 4 
	SystemProcessesAndThreadsInformation, // 5 
	SystemCallCounts, // 6 
	SystemConfigurationInformation, // 7 
	SystemProcessorTimes, // 8 
	SystemGlobalFlag, // 9 
	SystemNotImplemented2, // 10 
	SystemModuleInformation, // 11 
	SystemLockInformation, // 12 
	SystemNotImplemented3, // 13 
	SystemNotImplemented4, // 14 
	SystemNotImplemented5, // 15 
	SystemHandleInformation, // 16 
	SystemObjectInformation, // 17 
	SystemPagefileInformation, // 18 
	SystemInstructionEmulationCounts, // 19 
	SystemInvalidInfoClass1, // 20 
	SystemCacheInformation, // 21 
	SystemPoolTagInformation, // 22 
	SystemProcessorStatistics, // 23 
	SystemDpcInformation, // 24 
	SystemNotImplemented6, // 25 
	SystemLoadImage, // 26 
	SystemUnloadImage, // 27 
	SystemTimeAdjustment, // 28 
	SystemNotImplemented7, // 29 
	SystemNotImplemented8, // 30 
	SystemNotImplemented9, // 31 
	SystemCrashDumpInformation, // 32 
	SystemExceptionInformation, // 33 
	SystemCrashDumpStateInformation, // 34 
	SystemKernelDebuggerInformation, // 35 
	SystemContextSwitchInformation, // 36 
	SystemRegistryQuotaInformation, // 37 
	SystemLoadAndCallImage, // 38 
	SystemPrioritySeparation, // 39 
	SystemNotImplemented10, // 40 
	SystemNotImplemented11, // 41 
	SystemInvalidInfoClass2, // 42 
	SystemInvalidInfoClass3, // 43 
	SystemTimeZoneInformation, // 44 
	SystemLookasideInformation, // 45 
	SystemSetTimeSlipEvent, // 46 
	SystemCreateSession, // 47 
	SystemDeleteSession, // 48 
	SystemInvalidInfoClass4, // 49 
	SystemRangeStartInformation, // 50 
	SystemVerifierInformation, // 51 
	SystemAddVerifier, // 52 
	SystemSessionProcessesInformation // 53 
} SYSTEM_INFORMATION_CLASS; 
#endif 

#ifndef HANDLEINFO 
typedef struct HandleInfo{ 
	ULONG Pid; 
	USHORT ObjectType; 
	USHORT HandleValue; 
	PVOID ObjectPointer; 
	ULONG AccessMask; 
} HANDLEINFO, *PHANDLEINFO; 
#endif 

#ifndef SYSTEMHANDLEINFO 
typedef struct SystemHandleInfo { 
	ULONG nHandleEntries; 
	HANDLEINFO HandleInfo[1]; 
} SYSTEMHANDLEINFO, *PSYSTEMHANDLEINFO; 
#endif 


#ifndef STATUS_INFO_LENGTH_MISMATCH 
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L) 
#endif 

#ifndef OBJECT_INFORMATION_CLASS 
typedef enum _OBJECT_INFORMATION_CLASS {
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectAllInformation,
	ObjectDataInformation
} OBJECT_INFORMATION_CLASS, *POBJECT_INFORMATION_CLASS;
#endif 



#ifndef OBJECT_BASIC_INFORMATION 
typedef struct _OBJECT_BASIC_INFORMATION 
{ 
	ULONG Unknown1; 
	ACCESS_MASK DesiredAccess; 
	ULONG HandleCount; 
	ULONG ReferenceCount; 
	ULONG PagedPoolQuota; 
	ULONG NonPagedPoolQuota; 
	BYTE Unknown2[32]; 
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION; 
#endif 



typedef DWORD (CALLBACK* NTQUERYOBJECT)(HANDLE,DWORD, PVOID, DWORD,PDWORD); 
NTQUERYOBJECT NtQueryObject;
typedef DWORD (WINAPI *ZWQUERYSYSTEMINFORMATION)(DWORD, PVOID, DWORD, PDWORD);


typedef struct _SYSTEM_PROCESS_INFORMATION
{
	DWORD NextEntryDelta;
	DWORD ThreadCount;
	DWORD Reserved1[6];
	FILETIME ftCreateTime;
	FILETIME ftUserTime;
	FILETIME ftKernelTime;
	UNICODE_STRING ProcessName;
	DWORD BasePriority;
	DWORD ProcessId;
	DWORD InheritedFromProcessId;
	DWORD HandleCount;
	DWORD Reserved2[2];
	DWORD VmCounters;
	DWORD dCommitCharge;
	PVOID ThreadInfos[1];
}SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

#define SystemProcessesAndThreadsInformation 5

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG ProcessId;
	UCHAR ObjectTypeNumber;
	UCHAR Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
}SYSTEM_HANDLE_INFORMATION,*PSYSTEM_HANDLE_INFORMATION;

typedef enum _POOL_TYPE {
	NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed,
	DontUseThisType,
	NonPagedPoolCacheAligned,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS
} POOL_TYPE;

typedef struct _OBJECT_TYPE_INFORMATION {

	UNICODE_STRING TypeName; ULONG TotalNumberOfHandles; ULONG TotalNumberOfObjects; WCHAR Unused1[8]; ULONG HighWaterNumberOfHandles; ULONG HighWaterNumberOfObjects; WCHAR Unused2[8]; ACCESS_MASK InvalidAttributes; GENERIC_MAPPING GenericMapping; ACCESS_MASK ValidAttributes; BOOLEAN SecurityRequired; BOOLEAN MaintainHandleCount; USHORT MaintainTypeList; POOL_TYPE PoolType; ULONG DefaultPagedPoolCharge; ULONG DefaultNonPagedPoolCharge;

} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;


typedef struct _OBJECT_ALL_INFORMATION {

	ULONG NumberOfObjectsTypes; OBJECT_TYPE_INFORMATION ObjectTypeInformation[1];

} OBJECT_ALL_INFORMATION, *POBJECT_ALL_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION {
	UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

HANDLE GetProcessKernelObject(DWORD ProcessId,LPTSTR MuTexName){
	HMODULE hNtDll = NULL;
	ZWQUERYSYSTEMINFORMATION pfnZwQuerySystemInformation = NULL;
	NTQUERYOBJECT pfnNtQueryObject = NULL;
	PSYSTEM_HANDLE_INFORMATION pSysHandleInfo = NULL;
	POBJECT_ALL_INFORMATION pAllInfo =NULL;
	POBJECT_NAME_INFORMATION pNameInfo = NULL;


	ULONG nNumberHandle =0;
	NTSTATUS ntStatus = 0;
	ULONG ulSize,ulCount;
	char cBuffer[0x80000*2],cInfoBuffer[0x10000];
//	LPVOID pBuffer=malloc(0x80000);
	hNtDll = GetModuleHandle(TEXT("ntdll.dll"));
	pfnZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)GetProcAddress(hNtDll,"ZwQuerySystemInformation");
	pfnNtQueryObject = (NTQUERYOBJECT)GetProcAddress(hNtDll,"NtQueryObject");

	ntStatus = pfnZwQuerySystemInformation(SystemHandleInformation,cBuffer,sizeof(cBuffer),&ulSize);
//	ntStatus = pfnZwQuerySystemInformation(SystemHandleInformation,pBuffer,sizeof(pBuffer),&ulSize);
//	cBuffer=static_cast <char *>(pBuffer);
//	if (pfnNtQueryObject)
//	{
//		MessageBox(NULL,L"pfnNtQueryObject LOAD OK!",L"TEST",MB_OK);
//	}
	if(NT_SUCCESS(ntStatus))
	{
		DWORD n = ulSize/sizeof(SYSTEM_HANDLE_INFORMATION);
		nNumberHandle = *(PULONG)cBuffer;
		pSysHandleInfo = (PSYSTEM_HANDLE_INFORMATION)(cBuffer +4);
		ulCount = 0;

		for(ULONG i=0;i!=nNumberHandle;++i)
		{

			if(pSysHandleInfo->ProcessId != ProcessId) continue;
			wchar_t *status_t=new wchar_t[255];
			_ltow(pSysHandleInfo->ProcessId,status_t,10);
			MessageBox(NULL,L"TESTED!",status_t,MB_OK);
			ntStatus = pfnNtQueryObject((HANDLE)pSysHandleInfo->Handle,ObjectAllInformation,cInfoBuffer,0x10000,&ulSize);
			ntStatus = pfnNtQueryObject((HANDLE)pSysHandleInfo->Handle,ObjectNameInformation,cInfoBuffer,0x10000,&ulSize);
			if(NT_SUCCESS(ntStatus))
			{
				pAllInfo = (POBJECT_ALL_INFORMATION)cInfoBuffer;
				pNameInfo = (POBJECT_NAME_INFORMATION)cInfoBuffer;
				if(_tcsstr(pNameInfo->Name.Buffer,MuTexName) !=NULL)
				{
					MessageBox(NULL,pNameInfo->Name.Buffer,L"TEST	OK",MB_OK);
					return (HWND)pSysHandleInfo->Handle;
				}else
				{
					MessageBox(NULL,pNameInfo->Name.Buffer,L"TEST",MB_OK);
				}
			}
			else
			{
				MessageBox(NULL,L"pfnNtQueryObject FAILED!",L"TEST",MB_OK);
			}
		}
	}
	else
	{
		LONG status=NTSTATUS(ntStatus);
		wchar_t *status_text=new wchar_t[255];
		_ltow(status,status_text,10);
		MessageBox(NULL,L"pfnZwQuerySystemInformation FAILED!",status_text,MB_OK);
	}
	return NULL;
}

int CloseMuTex(DWORD ProcessId,LPTSTR MuTexName){
	HANDLE hMuTex = GetProcessKernelObject(ProcessId,MuTexName);
	if (hMuTex)
	{
		CloseHandle(hMuTex);
		return 1;
	}
	MessageBox(NULL,L"¸Â±­¾ßµÄ",MuTexName,MB_OK);
	return 0;
};