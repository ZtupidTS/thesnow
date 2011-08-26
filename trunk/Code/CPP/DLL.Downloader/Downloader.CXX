#include <shlobj.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
DWORD ThreadCases;
HMODULE HMO , NETHMO = LoadLibrary("Wininet.dll");
class EncoderParameter
{
public:
GUID    Guid;
ULONG   NumberOfValues;
ULONG   Type;
VOID*   Value;
};
class EncoderParameters
{
public:
UINT Count;
EncoderParameter Parameter[1];
};
typedef struct {
UINT32 GdiplusVersion;
int DebugEventCallback;
BOOL SuppressBackgroundThread;
BOOL SuppressExternalCodecs;
} Gdiplus;
class GpImage {};
class GpGraphics {};
Gdiplus Gdiplusi;
int Status,token,output;

typedef struct {
HGLOBAL hMemory;
DWORD TotalReadTimes;
DWORD NumberOfBytesRead;
DWORD FileSize;
DWORD IsComplete;
DWORD IsError;} NetInfoR;
typedef struct {
DWORD TotalWriteTimes;
DWORD NumberOfBytesWrite;
DWORD FileSize;
DWORD IsComplete;
DWORD IsError;} NetInfoW;
typedef struct {
DWORD TotalCopyTimes;
DWORD NumberOfBytesCopy;
DWORD FileSize;
DWORD IsComplete;
DWORD IsError;} FileInfo;
typedef struct {
DWORD IsComplete;
DWORD IsError;} ImageInfo;
#ifdef __cplusplus
extern "C"
{
#endif
__declspec(dllexport) DWORD WINAPI  FtpMemGetFile(HANDLE iConnect,LPCWSTR RemoteFile,LPCWSTR LocalFile,LPCWSTR CurrentDir,DWORD BytesNumberAtaTime,DWORD iFlag,DWORD MemSize);
__declspec(dllexport) DWORD WINAPI FtpMemPutFile(HANDLE iConnect,LPCWSTR RemoteFile,LPCWSTR LocalFile,LPCWSTR CurrentDir,DWORD CreateDir,DWORD BytesNumberAtaTime,DWORD iFlag,HGLOBAL hMemory);
__declspec(dllexport) DWORD WINAPI DownloadFile(LPCWSTR Url,LPCWSTR FilePath,DWORD BytesNumberAtaTime,DWORD iFlag,DWORD MemSize);
__declspec(dllexport) DWORD WINAPI File_Copy(LPCWSTR SourceFile,LPCWSTR DestFile,DWORD BytesNumberAtaTime,DWORD iFlag);
__declspec(dllexport) DWORD WINAPI SaveImage(HBITMAP hBmp,CHAR* NewImageName,int iQuality,bool DeletImage);
__declspec(dllexport) bool WINAPI FtpGetFileInfo(DWORD FtpGetNO,NetInfoR &NInfo);
__declspec(dllexport) bool WINAPI PauseFtpGet(DWORD FtpGetNO,bool Pause);
__declspec(dllexport) bool WINAPI CancelFtpGet(DWORD FtpGetNO);
__declspec(dllexport) bool WINAPI FtpPutFileInfo(DWORD FtpPutNO,NetInfoW &NInfo);
__declspec(dllexport) bool WINAPI PauseFtpPut(DWORD FtpPutNO,bool Pause);
__declspec(dllexport) bool WINAPI CancelFtpPut(DWORD FtpPutNO);
__declspec(dllexport) bool WINAPI DownloadGetInfo(DWORD DownlNO,NetInfoR &NInfo);
__declspec(dllexport) bool WINAPI PauseDownload(DWORD DownlNO,bool Pause);
__declspec(dllexport) bool WINAPI CancelDownload(DWORD DownlNO);
__declspec(dllexport) bool WINAPI File_CopyGetInfo(DWORD CopyNO,FileInfo &FInfo);
__declspec(dllexport) bool WINAPI PauseCopyFile(DWORD CopyNO,bool Pause);
__declspec(dllexport) bool WINAPI CancelCopyFile(DWORD CopyNO);
__declspec(dllexport) bool WINAPI SaveImageGetInfo(DWORD SaveNO,ImageInfo &IInfo);
__declspec(dllexport) bool WINAPI FtpCreate_Dir(HANDLE hConnect,LPCWSTR Dir,LPCWSTR CurrentDir);
#ifdef __cplusplus
}
#endif

DWORD FtpGetCount = 0;
HANDLE GetThreadH = NULL;
std::vector<DWORD> GetTestArray;
std::vector<HANDLE> GetConnectArray;
std::vector<HANDLE> GetFileArray;
std::vector<HANDLE> GetOFileArray;
std::vector<HGLOBAL> GethMemoryArray;
std::vector<BYTE*> GetBytePtrArray;
std::vector<std::wstring> GetRemoteFileArray;
std::vector<std::wstring> GetCurrentDirArray;
std::vector<DWORD> GetFileSizeArray;
std::vector<DWORD> GetByteAtaTimeArray;
std::vector<DWORD> GetBytesReadArray;
std::vector<DWORD> GetTotalReadTimesArray;
std::vector<DWORD> GetCompleteArray;
std::vector<DWORD> GetErrorArray;
std::vector<DWORD> GetMemSizeArray;
std::vector<DWORD> GetNewDownlArray;
std::vector<DWORD> GetPauseArray;
std::vector<DWORD> GetMovePosArray;

DWORD FtpPutCount = 0;
HANDLE PutThreadH = NULL;
std::vector<DWORD> PutiTestArray;
std::vector<DWORD> PutCreateDirArray;
std::vector<HANDLE> PutConnectArray;
std::vector<HANDLE> PutOFileArray;
std::vector<HANDLE> PutFileArray;
std::vector<HGLOBAL> PuthMemoryArray;
std::vector<BYTE*> PutBytePtrArray;
std::vector<std::wstring> PutRemoteFileArray;
std::vector<std::wstring> PutCurrentDirArray;
std::vector<DWORD> PutFileSizeArray;
std::vector<DWORD> PutBytesAtaTimeArray;
std::vector<DWORD> PutBytesToWrite;
std::vector<DWORD> PutTotalWriteTimesArray;
std::vector<DWORD> PutCompleteArray;
std::vector<DWORD> PutErrorArray;
std::vector<DWORD> PutNewuploadArray;
std::vector<DWORD> PutPauseArray;
std::vector<DWORD> PutMovePosArray;

DWORD DownlCount = 0;
HANDLE DownlThreadH = NULL;
HANDLE DownlOInte = NULL;
std::vector<DWORD> DownliTestArray;
std::vector<HANDLE> DownlFileArray;
std::vector<HANDLE> DownlOUrlArray;
std::vector<DWORD> DownlMovePosArray;
std::vector<DWORD> DownlFileSizeArray;
std::vector<DWORD> DownlByteAtaTimeArray;
std::vector<DWORD> DownlBytesReadArray;
std::vector<DWORD> DownlTotalReadTimesArray;
std::vector<DWORD> DownlCompleteArray;
std::vector<DWORD> DownlErrorArray;
std::vector<BYTE*> DownlBytePtrArray; 
std::vector<HGLOBAL> DownlhMemoryArray;
std::vector<DWORD> DownlPauseArray;
std::vector<std::wstring> DownlStrUrlArray;
std::vector<DWORD> DownlNewDownlArray;
std::vector<DWORD> DownlMemSizeArray;

DWORD CopyCount = 0;
HANDLE CopyThreadH = NULL;
std::vector<HANDLE> CopyShFileArray;
std::vector<HANDLE> CopyDhFileArray;
std::vector<DWORD> CopyFileSizeArray;
std::vector<DWORD> CopyMovePosArray;
std::vector<DWORD> CopyBytesAtaTimeArray;
std::vector<DWORD> CopyBytesArray;
std::vector<DWORD> CopyTotalTimesArray;
std::vector<DWORD> CopyCompleteArray;
std::vector<DWORD> CopyErrorArray;
std::vector<DWORD> CopyPauseArray;
std::vector<DWORD> CopyNewCopyArray;

DWORD SaveImageCount = 0;
HANDLE SaveThrHD = NULL;
std::vector<GpImage*> SaveImageArray;
std::vector<WCHAR*> SaveNewImageNameArray;
std::vector<CLSID> SaveGUIDImageArray;
std::vector<EncoderParameters> SaveParameterArray;
std::vector<DWORD> SaveCompleteArray;
std::vector<DWORD> SaveErrorArray;
std::vector<DWORD> SaveNewSaveArray;

DWORD WINAPI ThreadProc(LPVOID lpParameter);
HANDLE Internet_Open(LPCWSTR lpszAgent,DWORD dwAccessType,LPCWSTR lpszProxyName,LPCWSTR lpszProxyBypass,DWORD dwFlags);
HANDLE Internet_OpenUrl(HANDLE hInternet,LPCWSTR lpszUrl,LPCWSTR lpszHeaders,DWORD dwHeadersLength,DWORD dwFlags,LPVOID dwContext);
HANDLE Internet_Connect(HANDLE hInternet,LPCWSTR lpszServerName,WORD nServerPort,LPCTSTR lpszUsername,LPCTSTR lpszPassword,DWORD dwService,DWORD dwFlags,LPVOID dwContext);
HANDLE FtpOpen_File(HANDLE hConnect,LPCWSTR lpszFileName,DWORD dwAccess,DWORD dwFlags,LPVOID dwContext);
DWORD FtpGetFileFileSize(HANDLE hFile,LPDWORD lpdwFileSizeHigh);
BOOL Internet_ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD dwNumberOfBytesToRead,LPDWORD lpdwNumberOfBytesRead);
BOOL Internet_WriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD dwNumberOfBytesToWrite,LPDWORD lpdwNumberOfBytesWritten);
BOOL HttpQueryInfo(HANDLE hRequest,DWORD dwInfoLevel,LPVOID lpvBuffer,LPDWORD lpdwBufferLength,LPDWORD lpdwIndex);
BOOL InternetQuery_DataAvailable(HANDLE hFile,LPDWORD lpdwNumberOfBytesAvailable,DWORD dwFlags,LPVOID dwContext);
BOOL Internet_CloseHandle(HANDLE hInternet);
BOOL FtpCreate_Directory(HANDLE hConnect,LPCWSTR lpszDirectory);
BOOL SHCreate_DirectoryExW(HWND hwnd,LPCWSTR pszPath,SECURITY_ATTRIBUTES *psa);
BOOL FtpSetCurrent_Directory(HANDLE hConnect,LPCWSTR lpszDirectory);
BOOL FtpGetCurrent_Directory(HANDLE hConnect,LPCWSTR lpszCurrentDirectory,LPDWORD lpdwCurrentDirectory);
void atThreadLibraryExit(void);

extern "C"
BOOL WINAPI DllMain(HANDLE hinstDLL,DWORD dwReason, LPVOID lpvReserved)
{
atexit(atThreadLibraryExit);
return 1;
}

DWORD WINAPI  FtpMemGetFile(HANDLE iConnect,LPCWSTR RemoteFile,LPCWSTR LocalFile,LPCWSTR CurrentDir,DWORD BytesNumberAtaTime,DWORD iFlag,DWORD MemSize)
{

DWORD FBOOL = (iFlag != 1 && iFlag != 2);
if (BytesNumberAtaTime <= 0 || !iConnect || FBOOL) return 0;

HANDLE GetFile;
DWORD GetTest;
if (iFlag == 1)
{
GetTest = 1;
GetFile = CreateFileW(LocalFile,GENERIC_WRITE,
0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);    
if (GetFile == INVALID_HANDLE_VALUE) return 0;
} else {
GetTest = 0;
}

GetConnectArray.push_back(iConnect);
GetFileArray.push_back(GetFile);
GetRemoteFileArray.push_back(RemoteFile);
GetCurrentDirArray.push_back(CurrentDir);
GetTestArray.push_back(GetTest);
GetFileSizeArray.push_back(0);
GetByteAtaTimeArray.push_back(BytesNumberAtaTime);
GetOFileArray.push_back(0);
GetBytesReadArray.push_back(0);
GetTotalReadTimesArray.push_back(0);
GetCompleteArray.push_back(0);
GetErrorArray.push_back(0);
GethMemoryArray.push_back(0);
GetBytePtrArray.push_back(0);
GetMemSizeArray.push_back(MemSize);
GetNewDownlArray.push_back(1);
GetPauseArray.push_back(0);
GetMovePosArray.push_back(0);

DWORD ThreadID;
FtpGetCount++;
ThreadCases = 1;
if (!GetThreadH)
{
GetThreadH = CreateThread(NULL,0,ThreadProc,NULL,0,&ThreadID);
}

return FtpGetCount;
}

bool WINAPI FtpGetFileInfo(DWORD FtpGetNO,NetInfoR &NInfo)
{

if (FtpGetNO < 1 || FtpGetNO > FtpGetCount) return 0;

FtpGetNO--;
NInfo.TotalReadTimes = GetTotalReadTimesArray[FtpGetNO];
NInfo.NumberOfBytesRead = GetBytesReadArray[FtpGetNO];
NInfo.FileSize = GetFileSizeArray[FtpGetNO];
NInfo.IsComplete = GetCompleteArray[FtpGetNO];
NInfo.IsError = GetErrorArray[FtpGetNO];
if ((!GetTestArray[FtpGetNO]) && (GetCompleteArray[FtpGetNO] == 1))
NInfo.hMemory = GethMemoryArray[FtpGetNO];

return 1;
}

bool WINAPI PauseFtpGet(DWORD FtpGetNO,bool Pause)
{

if (FtpGetNO < 1 || FtpGetNO > FtpGetCount || 
GetCompleteArray[FtpGetNO - 1]) return 0;
FtpGetNO--;

if (Pause)
{
GetPauseArray[FtpGetNO] = 1;
} else {
GetPauseArray[FtpGetNO] = 0;
}

return 1;
}

bool WINAPI CancelFtpGet(DWORD FtpGetNO)
{

if (FtpGetNO < 1 || FtpGetNO > FtpGetCount || 
GetCompleteArray[FtpGetNO - 1]) return 0;

PauseFtpGet(FtpGetNO,1);
FtpGetNO--;
CloseHandle(GetFileArray[FtpGetNO]);
Internet_CloseHandle(GetOFileArray[FtpGetNO]);
GlobalFree(GethMemoryArray[FtpGetNO]);
GetCompleteArray[FtpGetNO] = 2;

return 1;

}

DWORD WINAPI FtpMemPutFile(HANDLE iConnect,LPCWSTR RemoteFile,LPCWSTR LocalFile,LPCWSTR CurrentDir,DWORD CreateDir,DWORD BytesNumberAtaTime,DWORD iFlag,HGLOBAL hMemory)
{

DWORD FBOOL = (iFlag != 1 && iFlag != 2);
if (BytesNumberAtaTime <= 0 || !iConnect || FBOOL) return 0;

DWORD PutTest , LoDWORD , HiDWORD , FileSize;
HANDLE PutFile;
BYTE* BytePtr;
BYTE* iBytePtr;
HGLOBAL ihMemory;

if (iFlag == 1)
{
PutTest = 1;
PutFile = CreateFileW(LocalFile,GENERIC_READ,FILE_SHARE_READ,
NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
if (PutFile == INVALID_HANDLE_VALUE) return 0;
LoDWORD = GetFileSize(PutFile,&HiDWORD);
FileSize = LoDWORD;
} else {
PutTest = 0;
FileSize = GlobalSize(hMemory);
BytePtr = (BYTE*) GlobalLock(hMemory);
if (!BytePtr) return 0;
ihMemory = GlobalAlloc(GPTR,FileSize);
iBytePtr = (BYTE*) GlobalLock(ihMemory);
CopyMemory(iBytePtr,BytePtr,FileSize);
}

PutConnectArray.push_back(iConnect);
PutRemoteFileArray.push_back(RemoteFile);
PutCurrentDirArray.push_back(CurrentDir);
PutiTestArray.push_back(PutTest);
PutFileArray.push_back(PutFile);
PutOFileArray.push_back(0);
PuthMemoryArray.push_back(ihMemory);
PutBytePtrArray.push_back(iBytePtr);
PutFileSizeArray.push_back(FileSize);
PutMovePosArray.push_back(0);
PutBytesAtaTimeArray.push_back(BytesNumberAtaTime);
PutBytesToWrite.push_back(0);
PutTotalWriteTimesArray.push_back(0);
PutCompleteArray.push_back(0);
PutErrorArray.push_back(0);
PutPauseArray.push_back(0);
PutNewuploadArray.push_back(1);
PutCreateDirArray.push_back(CreateDir);

DWORD ThreadID;
FtpPutCount++;
ThreadCases = 2;
if (!PutThreadH)
{
PutThreadH = CreateThread(NULL,0,ThreadProc,NULL,0,&ThreadID);
}

return FtpPutCount;
}

bool WINAPI FtpPutFileInfo(DWORD FtpPutNO,NetInfoW &NInfo)
{

if (FtpPutNO < 1 || FtpPutNO > FtpPutCount) return 0;
FtpPutNO--;
NInfo.TotalWriteTimes = PutTotalWriteTimesArray[FtpPutNO];
NInfo.NumberOfBytesWrite = PutBytesToWrite[FtpPutNO];
NInfo.FileSize = PutFileSizeArray[FtpPutNO];
NInfo.IsComplete = PutCompleteArray[FtpPutNO];
NInfo.IsError = PutErrorArray[FtpPutNO];

return 1;
}


bool WINAPI PauseFtpPut(DWORD FtpPutNO,bool Pause)
{

if (FtpPutNO < 1 || FtpPutNO > FtpPutCount || 
PutCompleteArray[FtpPutNO - 1]) return 0;
FtpPutNO--;

if (Pause)
{
PutPauseArray[FtpPutNO] = 1;
} else {
PutPauseArray[FtpPutNO] = 0;
}

return 1;
}

bool WINAPI CancelFtpPut(DWORD FtpPutNO)
{
if (FtpPutNO < 1 || FtpPutNO > FtpPutCount || 
PutCompleteArray[FtpPutNO - 1]) return 0;

PauseFtpPut(FtpPutNO,1);
FtpPutNO--;

CloseHandle(PutFileArray[FtpPutNO]);
Internet_CloseHandle(PutOFileArray[FtpPutNO]);
GlobalFree(PuthMemoryArray[FtpPutNO]);
PutCompleteArray[FtpPutNO] = 2;

return 1;
}

DWORD WINAPI DownloadFile(LPCWSTR Url,LPCWSTR FilePath,DWORD BytesNumberAtaTime,DWORD iFlag,DWORD MemSize)
{

DWORD DBOOL = (iFlag != 1 && iFlag != 2);
if (BytesNumberAtaTime <= 0 || DBOOL) return 0;

if (!DownlOInte)
{
DownlOInte = Internet_Open((LPCWSTR) "OPEN",NULL,NULL,NULL,0);
if(!DownlOInte) return 0;
}

DWORD DownlTest;
HANDLE DownlFile;
if (iFlag == 1)
{
DownlTest = 1;
DownlFile = CreateFileW(FilePath,GENERIC_WRITE,
0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);          
if (DownlFile == INVALID_HANDLE_VALUE) return 0;
} else {
DownlTest = 0;
}

DownlStrUrlArray.push_back(Url);
DownliTestArray.push_back(DownlTest);
DownlFileArray.push_back(DownlFile);
DownlOUrlArray.push_back(0);
DownlMovePosArray.push_back(0);
DownlBytePtrArray.push_back(0);
DownlhMemoryArray.push_back(0);
DownlFileSizeArray.push_back(0);
DownlByteAtaTimeArray.push_back(BytesNumberAtaTime);
DownlBytesReadArray.push_back(0);
DownlTotalReadTimesArray.push_back(0);
DownlCompleteArray.push_back(0);
DownlErrorArray.push_back(0);
DownlPauseArray.push_back(0);
DownlMemSizeArray.push_back(MemSize);
DownlNewDownlArray.push_back(1);

DWORD ThreadID;
DownlCount++;
ThreadCases = 3;
if (!DownlThreadH)
{
DownlThreadH = CreateThread(NULL,0,ThreadProc,NULL,0,&ThreadID);
}

return DownlCount;

}

bool WINAPI DownloadGetInfo(DWORD DownlNO,NetInfoR &NInfo)
{

if (DownlNO < 1 || DownlNO > DownlCount) return 0;
DownlNO--;
NInfo.TotalReadTimes = DownlTotalReadTimesArray[DownlNO];
NInfo.NumberOfBytesRead = DownlBytesReadArray[DownlNO];
NInfo.FileSize = DownlFileSizeArray[DownlNO];
NInfo.IsComplete = DownlCompleteArray[DownlNO];
NInfo.IsError = DownlErrorArray[DownlNO];
if ((!DownliTestArray[DownlNO]) && (DownlCompleteArray[DownlNO] == 1))
NInfo.hMemory = DownlhMemoryArray[DownlNO];
return 1;

}

bool WINAPI PauseDownload(DWORD DownlNO,bool Pause)
{

if (DownlNO < 1 || DownlNO > DownlCount ||
DownlCompleteArray[DownlNO - 1]) return 0;
DownlNO--;

if (Pause != 0)
{
DownlPauseArray[DownlNO] = 1;
} else {
DownlPauseArray[DownlNO] = 0;
}

return 1;
}

bool WINAPI CancelDownload(DWORD DownlNO)
{
if (DownlNO < 1 || DownlNO > DownlCount || 
DownlCompleteArray[DownlNO - 1]) return 0;

PauseDownload(DownlNO,1);
DownlNO--;
CloseHandle(DownlFileArray[DownlNO]);
Internet_CloseHandle(DownlOUrlArray[DownlNO]);
GlobalFree(DownlhMemoryArray[DownlNO]);
DownlCompleteArray[DownlNO] = 2;

return 1;
}

DWORD WINAPI File_Copy(LPCWSTR SourceFile,LPCWSTR DestFile,DWORD BytesNumberAtaTime,DWORD iFlag)
{

HANDLE HSFile , HDFile;
DWORD LoDWORD , HiDWORD , FileSize , iTest;
DWORD RT = 0 , nBufferLength;
int Pos = 0,TestPeriodslash = 0,iNo = 0;
std::wstring iDir = DestFile;
DWORD CdirLength = MAX_PATH * 2;
WCHAR Cdir[MAX_PATH * 2];

iTest = (iFlag == 0 || iFlag == 1 || iFlag == 8 || iFlag == 9);
if (BytesNumberAtaTime <= 0 || !iTest || iDir.length() < 1 || 
iDir.length() > MAX_PATH) return 0;

HSFile = CreateFileW(SourceFile,GENERIC_READ,FILE_SHARE_READ,
NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);                        
if (HSFile == INVALID_HANDLE_VALUE) return 0;

LoDWORD = GetFileSize(HSFile,&HiDWORD);
FileSize = LoDWORD;

if (iFlag)
{

if (iFlag != 1)
{

if (iDir[0] == (char) 92) iDir = iDir.substr(1,iDir.length());
if (iDir[iDir.length() - 1] == (char) 92) iDir = iDir.substr(0,iDir.length() - 1);
DestFile = iDir.c_str();

if (iDir[0] == (char) 92 ||
iDir[0] == (char) 58) return FALSE;

if (iDir.length() > 2)
{
if (iDir[1] == (char) 58 &&
iDir[2] != (char) 92) return FALSE;
}

if (iDir.length() == 2)
{
if (iDir[1] == (char) 58) return FALSE;
}

if (iDir.length() == 3)
{
if (iDir[1] == (char) 58 && iDir[2] == (char) 92) 
return FALSE;
}

if (iDir.length() > 1) 
{
Pos = 0;
if (iDir[1] == (char) 58) Pos = 2;
}else{
Pos = 0;
}

if (
iDir.find ( L"<" , 0 ) != std::string::npos || 
iDir.find ( L">" , 0 ) != std::string::npos || 
iDir.find ( L":" ,Pos) != std::string::npos || 
iDir.find ( L'"' , 0 ) != std::string::npos || 
iDir.find ( L"/" , 0 ) != std::string::npos ||
iDir.find ( L"|" , 0 ) != std::string::npos ||
iDir.find ( L"\\\\",0) != std::string::npos ||
iDir.find ( L"?" , 0 ) != std::string::npos ) 
return FALSE;

for (int i= iDir.length() - 1; i >= 0 ; i--) 
{
if (iDir[i] == (char) 46 && !TestPeriodslash) TestPeriodslash = 1;
if (iDir[i] == (char) 92 && !TestPeriodslash) TestPeriodslash = 2;
}

if (TestPeriodslash)
{

for (int i= iDir.length() - 1; i >= 0 ; i--)
{
if (iDir[i] == (char) 92)
{
TestPeriodslash = 3;
iNo = i;
break;
}}

if (TestPeriodslash == 3) 
{

iDir = iDir.substr(0,iNo);

if (iDir.length() > 1)
{

if (iDir[1] != (char) 58 ) 
{
RT = GetCurrentDirectoryW(CdirLength,Cdir);
if (!RT) return 0;
iDir = std::wstring(L"\\") + iDir;
iDir = std::wstring(Cdir) + iDir;
}

} else {
RT = GetCurrentDirectoryW(CdirLength,Cdir);
if (!RT) return 0;
iDir = std::wstring(L"\\") + iDir;
iDir = std::wstring(Cdir) + iDir;
}

RT = SHCreate_DirectoryExW(NULL,iDir.c_str(),NULL);
if (RT && RT != ERROR_ALREADY_EXISTS)
return 0;
}}

}

HDFile = CreateFileW(DestFile,GENERIC_WRITE
,0,NULL,CREATE_ALWAYS,NULL,NULL);
if (HDFile == INVALID_HANDLE_VALUE) return 0;

} else {

HDFile = CreateFileW(DestFile,GENERIC_WRITE
,0,NULL,CREATE_NEW,NULL,NULL);
if (HDFile == INVALID_HANDLE_VALUE) return 0;

}

CopyShFileArray.push_back(HSFile);
CopyDhFileArray.push_back(HDFile);
CopyFileSizeArray.push_back(FileSize);
CopyMovePosArray.push_back(0);
CopyBytesAtaTimeArray.push_back(BytesNumberAtaTime);
CopyBytesArray.push_back(0);
CopyTotalTimesArray.push_back(0);
CopyCompleteArray.push_back(0);
CopyErrorArray.push_back(0);
CopyPauseArray.push_back(0);
CopyNewCopyArray.push_back(1);

DWORD ThreadID;
CopyCount++;
ThreadCases = 4;
if (!CopyThreadH)
{
CopyThreadH = CreateThread(NULL,0,ThreadProc,NULL,0,&ThreadID);
}

return CopyCount;
}

bool WINAPI File_CopyGetInfo(DWORD CopyNO,FileInfo &FInfo)
{

if (CopyNO < 1 || CopyNO > CopyCount) return 0;
CopyNO--;
FInfo.TotalCopyTimes = CopyTotalTimesArray[CopyNO];
FInfo.NumberOfBytesCopy = CopyBytesArray[CopyNO];
FInfo.FileSize = CopyFileSizeArray[CopyNO];
FInfo.IsComplete = CopyCompleteArray[CopyNO];
FInfo.IsError = CopyErrorArray[CopyNO];

return 1;
}

bool WINAPI PauseCopyFile(DWORD CopyNO,bool Pause)
{

if (CopyNO < 1 || CopyNO > CopyCount || 
CopyCompleteArray[CopyNO - 1]) return 0;
CopyNO--;

if (Pause)
{
CopyPauseArray[CopyNO] = 1;
} else {
CopyPauseArray[CopyNO] = 0;
}

return 1;
}

bool WINAPI CancelCopyFile(DWORD CopyNO)
{
if (CopyNO < 1 || CopyNO > CopyCount || 
CopyCompleteArray[CopyNO - 1]) return 0;

PauseCopyFile(CopyNO,1);
CopyNO--;
CloseHandle(CopyShFileArray[CopyNO]);
CloseHandle(CopyDhFileArray[CopyNO]);
CopyCompleteArray[CopyNO] = 2;

return 1;
}

DWORD WINAPI SaveImage(HBITMAP hBmp,CHAR* NewImageName,int iQuality,bool DeletImage)
{

GpImage* hbmReturn;
CLSID GUIDImage;
EncoderParameters encoderParams;
CLSID iEncoderQuality;
int ImageType = 0 , Nowide;
WCHAR* lpWideCharStr = new WCHAR[600];
  
std::string Ext = (std::string) NewImageName;
if (Ext.length() < 4) return 0;
Ext = Ext.substr(Ext.length() - 4,4);
for (int i = 0; i < Ext.length(); i++) {Ext[i] = toupper(Ext[i]);}
if (Ext == std::string(".BMP")) ImageType = 1;
if (Ext == std::string(".JPG")) ImageType = 2;
if (Ext == std::string(".PNG")) ImageType = 3;
if (Ext == std::string(".GIF")) ImageType = 4;
if (Ext == std::string(".TIF")) ImageType = 5;
if (!ImageType) return 0;
if (!HMO)
{
HMO = LoadLibrary("gdiplus.dll");
if (!HMO) return 0;
Gdiplusi.GdiplusVersion = 1;
Gdiplusi.DebugEventCallback = 0;
Gdiplusi.SuppressBackgroundThread = 0;
Gdiplusi.SuppressExternalCodecs = 0;
typedef int (CALLBACK* parametersA) (LPVOID,LPVOID,LPVOID);
parametersA ProcA = (parametersA) GetProcAddress(HMO,_T("GdiplusStartup"));
Status = ProcA(&token,&Gdiplusi,&output);
if (Status != 0) return 0;
}

typedef int (CALLBACK* parametersB) (HBITMAP,int,GpImage**);
parametersB ProcB = (parametersB) GetProcAddress(HMO,_T("GdipCreateBitmapFromHBITMAP"));
Status = ProcB(hBmp,0,&hbmReturn);
if (Status != 0) return 0;

if (DeletImage) DeleteObject((HGDIOBJ) hBmp);

switch (ImageType)
{
case 1://bmp
CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}",&GUIDImage);
break;
case 2://jpg  
CLSIDFromString(L"{557CF401-1A04-11D3-9A73-0000F81EF32E}",&GUIDImage);
break;
case 3://png
CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}",&GUIDImage);
break;
case 4://gif
CLSIDFromString(L"{557CF402-1A04-11D3-9A73-0000F81EF32E}",&GUIDImage);
break;
case 5://tif
CLSIDFromString(L"{557CF405-1A04-11D3-9A73-0000F81EF32E}",&GUIDImage);
break;
}

CLSIDFromString(L"{1D5BE4B5-FA4A-452D-9CDD-5DB35105E7EB}",&iEncoderQuality);
encoderParams.Count = 1;
encoderParams.Parameter[0].NumberOfValues = 1;
encoderParams.Parameter[0].Guid  = iEncoderQuality;
encoderParams.Parameter[0].Type  = 4;
encoderParams.Parameter[0].Value = &iQuality;

memset(lpWideCharStr,0,sizeof(lpWideCharStr));
Nowide = MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(CHAR*) NewImageName,-1,lpWideCharStr,0);
MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(CHAR*) NewImageName,Nowide,lpWideCharStr,600 * sizeof(WCHAR));

SaveImageArray.push_back(hbmReturn);
SaveNewImageNameArray.push_back(lpWideCharStr);
SaveGUIDImageArray.push_back(GUIDImage);
SaveParameterArray.push_back(encoderParams);
SaveCompleteArray.push_back(0);
SaveErrorArray.push_back(0);
SaveNewSaveArray.push_back(1);

DWORD ThreadID;
SaveImageCount++;
ThreadCases = 5;
if (!SaveThrHD)
{
SaveThrHD = CreateThread(NULL,0,ThreadProc,NULL,0,&ThreadID);
}

return SaveImageCount;

}

bool WINAPI SaveImageGetInfo(DWORD SaveNO,ImageInfo &IInfo)
{

if (SaveNO < 1 || SaveNO > SaveImageCount) return 0;
SaveNO--;
IInfo.IsComplete = SaveCompleteArray[SaveNO];
IInfo.IsError = SaveErrorArray[SaveNO];
return 1;

}




DWORD WINAPI ThreadProc(LPVOID lpParameter)
{

DWORD LocalThreadCases = ThreadCases;

if (LocalThreadCases == 1)
{

DWORD BytesRead,MovePos = 0,Err,MemSize;
DWORD BytesAtaTime,FileSize,GetTest;
DWORD LoDWORD,HiDWORD;
DWORD CdirLength = MAX_PATH * 2;
WCHAR Cdir[MAX_PATH * 2];
HANDLE FtpGetOFile,GetFile,iConnect;
BYTE* BytePtr;
HGLOBAL hMemory;
bool RT;
LPCWSTR RemoteFile , CurrentDir;

while(1)
{


for (int i = 0; i < FtpGetCount; i++)
{

if (GetCompleteArray[i] || 
GetPauseArray[i]) continue;

iConnect = GetConnectArray[i];
GetTest = GetTestArray[i];
FtpGetOFile = GetOFileArray[i];
GetFile = GetFileArray[i];
BytesAtaTime = GetByteAtaTimeArray[i];
FileSize = GetFileSizeArray[i];
MovePos = GetMovePosArray[i];
MemSize = GetMemSizeArray[i];
BytePtr = GetBytePtrArray[i];

if (GetNewDownlArray[i])
{

CurrentDir = GetCurrentDirArray[i].c_str();
if (std::wstring(CurrentDir).length())
{

RT = FtpSetCurrent_Directory(iConnect,CurrentDir);
Err = GetLastError();
if (!RT)
{
CloseHandle(GetFile);
GetErrorArray[i] = Err;
GetCompleteArray[i] = 1;
GetNewDownlArray[i] = 0;
continue;
}

}

RemoteFile = GetRemoteFileArray[i].c_str();
FtpGetOFile = FtpOpen_File(iConnect,RemoteFile,GENERIC_READ,0,NULL);
Err = GetLastError();
if (!FtpGetOFile)
{
CloseHandle(GetFile);
GetErrorArray[i] = Err;
GetCompleteArray[i] = 1;
GetNewDownlArray[i] = 0;
continue;
}

LoDWORD = FtpGetFileFileSize(FtpGetOFile,&HiDWORD);
FileSize = LoDWORD;

if (!GetTest)
{
if (MemSize) FileSize = MemSize;
hMemory = GlobalAlloc(GPTR,FileSize);
BytePtr = (BYTE*) GlobalLock(hMemory);
GetBytePtrArray[i] = BytePtr;
GethMemoryArray[i] = hMemory;
}

GetFileSizeArray[i] = FileSize;
GetOFileArray[i] = FtpGetOFile;
GetNewDownlArray[i] = 0;
continue;

}

if (GetTest)
{

BytesRead = min(BytesAtaTime,FileSize - MovePos);
BYTE* iBytePtr = new BYTE[BytesRead];
if (GetPauseArray[i]) continue;
RT = Internet_ReadFile(FtpGetOFile,iBytePtr,BytesRead,&BytesRead);
Err = GetLastError();

if(!RT || BytesRead <= 0)
{
CloseHandle(GetFile);
Internet_CloseHandle(FtpGetOFile);
GetErrorArray[i] = Err;
GetCompleteArray[i] = 1;
continue;
}

WriteFile(GetFile,iBytePtr,BytesRead,&BytesRead,NULL);
GetMovePosArray[i] += BytesRead;
GetBytesReadArray[i] += BytesRead;
GetTotalReadTimesArray[i]++;

} else {

if (MovePos == FileSize)
{
Internet_CloseHandle(FtpGetOFile);
GetErrorArray[i] = Err;
GetCompleteArray[i] = 1;
continue;
}

BytesRead = min(BytesAtaTime,FileSize - MovePos);
BYTE* iBytePtr = new BYTE[BytesRead];
if (GetPauseArray[i]) continue;
RT = Internet_ReadFile(FtpGetOFile,iBytePtr,BytesRead,&BytesRead);
Err = GetLastError();

if(!RT || BytesRead <= 0)
{
Internet_CloseHandle(FtpGetOFile);
GetErrorArray[i] = Err;
GetCompleteArray[i] = 1;
continue;
}

BYTE* OutPtr = BytePtr + MovePos;
CopyMemory(OutPtr,iBytePtr,BytesRead);
GetMovePosArray[i] += BytesRead;
GetBytesReadArray[i] += BytesRead;
GetTotalReadTimesArray[i]++;

}}

}}

if (LocalThreadCases == 2)
{

DWORD BytesToWrite , MovePos = 0 , Err;
DWORD PutTest , BytesAtaTime , FileSize;
HANDLE PutOFile , PutFile , iConnect;
BYTE* BytePtr;
LPCWSTR RemoteFile , Dir , CurrentDir;
bool RT;

while(1)
{

for (int i = 0; i < FtpPutCount; i++)
{

if (PutCompleteArray[i] || 
PutPauseArray[i]) continue;

PutTest = PutiTestArray[i];
PutOFile = PutOFileArray[i];
PutFile = PutFileArray[i];
BytePtr = PutBytePtrArray[i];
BytesAtaTime = PutBytesAtaTimeArray[i];
FileSize = PutFileSizeArray[i];
MovePos = PutMovePosArray[i];
RemoteFile = PutRemoteFileArray[i].c_str();
CurrentDir = PutCurrentDirArray[i].c_str();
if (PutNewuploadArray[i]) 
{

iConnect = PutConnectArray[i];

if (PutCreateDirArray[i])
{

RT = FtpCreate_Dir(iConnect,RemoteFile,CurrentDir);
Err = GetLastError();

if (!RT)
{
GlobalFree(PuthMemoryArray[i]);
PutNewuploadArray[i] = 0;
PutErrorArray[i] = Err;
PutCompleteArray[i] = 1;
continue;
}

}

PutOFile = FtpOpen_File(iConnect,RemoteFile,GENERIC_WRITE,0,NULL);
Err = GetLastError();

if (!PutOFile)
{
CloseHandle(PutFile);
GlobalFree(PuthMemoryArray[i]);
PutNewuploadArray[i] = 0;
PutErrorArray[i] = Err;
PutCompleteArray[i] = 1;
continue;
}

PutOFileArray[i] = PutOFile;
PutNewuploadArray[i] = 0;
continue;

}

if (PutTest)
{

BytesToWrite = min(BytesAtaTime,FileSize - MovePos);

if (MovePos == FileSize)
{
CloseHandle(PutFile);
Internet_CloseHandle(PutOFile);
PutErrorArray[i] = Err;
PutCompleteArray[i] = 1;
continue;
}

BYTE* iBytePtr = new BYTE[BytesToWrite];
SetFilePointer(PutFile,MovePos,NULL,0);
if (PutPauseArray[i]) continue;
RT = ReadFile(PutFile,iBytePtr,BytesToWrite,&BytesToWrite, NULL);
if (PutPauseArray[i]) continue;
RT = Internet_WriteFile(PutOFile,iBytePtr,BytesToWrite,&BytesToWrite);
Err = GetLastError();

if(!RT || BytesToWrite <= 0)
{
CloseHandle(PutFile);
Internet_CloseHandle(PutOFile);
PutErrorArray[i] = Err;
PutCompleteArray[i] = 1;
continue;
}

PutMovePosArray[i] += BytesToWrite;
PutBytesToWrite[i] += BytesToWrite;
PutTotalWriteTimesArray[i]++;

} else {

BytesToWrite = min(BytesAtaTime,FileSize - MovePos);

if (MovePos == FileSize) 
{
Internet_CloseHandle(PutOFile);
PutErrorArray[i] = Err;
PutCompleteArray[i] = 1;
continue;
}

BYTE* iBytePtr = new BYTE[BytesToWrite];
BYTE* OutPtr = BytePtr + MovePos;
CopyMemory(iBytePtr,OutPtr,BytesToWrite);
RT = Internet_WriteFile(PutOFile,iBytePtr,BytesToWrite,&BytesToWrite);
Err = GetLastError();

if(!RT || BytesToWrite <= 0)
{
Internet_CloseHandle(PutOFile);
PutErrorArray[i] = Err;
PutCompleteArray[i] = 1;
continue;
}

PutMovePosArray[i] += BytesToWrite;
PutBytesToWrite[i] += BytesToWrite;
PutTotalWriteTimesArray[i]++;

}}

}}

if (LocalThreadCases == 3)
{

DWORD BytesRead , MovePos = 0 , Err;
DWORD FileSize , DownlTest , ByteAtaTime;
DWORD MemSize;
HANDLE DownlOUrl , DownlFile;
LPCWSTR Url;
bool RT;
HGLOBAL hMemory;
BYTE* BytePtr;

while(1)
{

for (int i = 0; i < DownlCount; i++)
{

if (DownlCompleteArray[i] || 
DownlPauseArray[i]) continue;

DownlOUrl = DownlOUrlArray[i];
DownlFile = DownlFileArray[i];
FileSize = DownlFileSizeArray[i];
DownlTest = DownliTestArray[i];
ByteAtaTime = DownlByteAtaTimeArray[i];
MemSize = DownlMemSizeArray[i];
MovePos = DownlMovePosArray[i];
BytePtr = DownlBytePtrArray[i];

if (DownlNewDownlArray[i])
{
DWORD BufferLength = 2000;
WCHAR BufferFileSize[2000];
Url = DownlStrUrlArray[i].c_str();
DownlOUrl = Internet_OpenUrl(DownlOInte,Url,NULL,NULL,NULL,NULL);
Err = GetLastError();
RT = HttpQueryInfo(DownlOUrl,5,BufferFileSize,&BufferLength,NULL);
if (!Err) Err = GetLastError();
if(!DownlOUrl || !RT)
{
CloseHandle(DownlFile);
Internet_CloseHandle(DownlOUrl);
DownlErrorArray[i] = Err;
DownlCompleteArray[i] = 1;
DownlNewDownlArray[i] = 0;
continue;
}
FileSize = _wtoi(BufferFileSize);
if (!DownlTest)
{
if (MemSize) FileSize = MemSize;
hMemory = GlobalAlloc(GPTR,FileSize);
BytePtr = (BYTE*) GlobalLock(hMemory);
DownlhMemoryArray[i] = hMemory;
DownlBytePtrArray[i] = BytePtr;
}
DownlOUrlArray[i] = DownlOUrl;
DownlNewDownlArray[i] = 0;
DownlFileSizeArray[i] = FileSize;
continue;
}

if (DownlTest)
{

BytesRead = min(ByteAtaTime,FileSize - MovePos);
BYTE* iBytePtr = new BYTE[BytesRead];
if (DownlPauseArray[i]) continue;
RT = Internet_ReadFile(DownlOUrl,iBytePtr,BytesRead,&BytesRead);
Err = GetLastError();

if(!RT || BytesRead <= 0)
{
CloseHandle(DownlFile);
Internet_CloseHandle(DownlOUrl);
DownlErrorArray[i] = Err;
DownlCompleteArray[i] = 1;
continue;
}

WriteFile(DownlFile,iBytePtr,BytesRead,&BytesRead,NULL);
DownlMovePosArray[i] += BytesRead;
DownlBytesReadArray[i] += BytesRead;
DownlTotalReadTimesArray[i]++;

} else {

if (MovePos == FileSize)
{
Internet_CloseHandle(DownlOUrl);
DownlErrorArray[i] = Err;
DownlCompleteArray[i] = 1;
continue;
}

BytesRead = min(ByteAtaTime,FileSize - MovePos);
BYTE* iBytePtr = new BYTE[BytesRead];
if (DownlPauseArray[i]) continue;
RT = Internet_ReadFile(DownlOUrl,iBytePtr,BytesRead,&BytesRead);
Err = GetLastError();

if(!RT || BytesRead <= 0 )
{
Internet_CloseHandle(DownlOUrl);
DownlErrorArray[i] = Err;
DownlCompleteArray[i] = 1;
continue;
}

BYTE* OutPtr = BytePtr + MovePos;
CopyMemory(OutPtr,iBytePtr,BytesRead);
DownlMovePosArray[i] += BytesRead;
DownlBytesReadArray[i] += BytesRead;
DownlTotalReadTimesArray[i]++;

}}

}}

if (LocalThreadCases == 4)
{

HANDLE HSFile , HDFile;
DWORD BytesRead , MovePos = 0 , Err;
DWORD BytesAtaTime , FileSize ,CompleteAll;
BYTE* BytePtr;
bool RT;

while(1)
{

for (int i = 0; i < CopyCount; i++)
{

if (CopyCompleteArray[i] || 
CopyPauseArray[i] ) continue;

HSFile = CopyShFileArray[i];
HDFile = CopyDhFileArray[i];
BytesAtaTime = CopyBytesAtaTimeArray[i];
FileSize = CopyFileSizeArray[i];
MovePos = CopyMovePosArray[i];

BytesRead = min(BytesAtaTime,FileSize - MovePos);

if (MovePos == FileSize)
{
CloseHandle(HSFile);
CloseHandle(HDFile);
CopyErrorArray[i] = Err;
CopyCompleteArray[i] = 1;
continue;
}

BytePtr = new BYTE[BytesRead];
if (CopyPauseArray[i]) continue;
RT = ReadFile(HSFile,BytePtr,BytesRead,&BytesRead,NULL);
Err = GetLastError();

if(!RT || BytesRead <= 0)
{
CloseHandle(HSFile);
CloseHandle(HDFile);
CopyErrorArray[i] = Err;
CopyCompleteArray[i] = 1;
continue;
}

RT = WriteFile(HDFile,BytePtr,BytesRead,&BytesRead,NULL);
CopyMovePosArray[i] += BytesRead;
CopyBytesArray[i] += BytesRead;
CopyTotalTimesArray[i]++;

}

}}

if (LocalThreadCases == 5)
{

GpImage* ihbmReturn;
WCHAR* ilpWideCharStr;
CLSID iGUIDImage;
EncoderParameters iencoderParams;

while(1)
{

for (int i = 0; i < SaveImageCount; i++)
{

if (SaveCompleteArray[i]) continue;

ihbmReturn = SaveImageArray[i];
ilpWideCharStr = SaveNewImageNameArray[i];
iGUIDImage = SaveGUIDImageArray[i];
iencoderParams = SaveParameterArray[i];

typedef int (CALLBACK* parametersC) (GpImage*,WCHAR*,CLSID*,EncoderParameters*);
parametersC ProcC = (parametersC) GetProcAddress(HMO,_T("GdipSaveImageToFile"));
Status = ProcC(ihbmReturn,ilpWideCharStr,&iGUIDImage,&iencoderParams);
typedef int (CALLBACK* parametersE) (GpImage*);
parametersE ProcE = (parametersE) GetProcAddress(HMO,_T("GdipDisposeImage"));
ProcE(ihbmReturn);
DeleteObject((HGDIOBJ) ihbmReturn);

SaveCompleteArray[i] = 1;
SaveErrorArray[i] = Status;

}

}}

return 0;
}


HANDLE Internet_Open(LPCWSTR lpszAgent,DWORD dwAccessType,LPCWSTR lpszProxyName,LPCWSTR lpszProxyBypass,DWORD dwFlags)
{

typedef HANDLE (CALLBACK* parameters) (LPCWSTR,DWORD,LPCWSTR,LPCWSTR,DWORD);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetOpenW"));
return Proc(lpszAgent,dwAccessType,lpszProxyName,lpszProxyBypass,dwFlags);

}

HANDLE Internet_Connect(HANDLE hInternet,LPCWSTR lpszServerName,WORD nServerPort,LPCWSTR lpszUsername,LPCWSTR lpszPassword,DWORD dwService,DWORD dwFlags,LPVOID dwContext)
{

typedef HANDLE (CALLBACK* parameters) (HANDLE,LPCWSTR,WORD,LPCWSTR,LPCWSTR,DWORD,DWORD,LPVOID);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetConnectW"));
return Proc(hInternet,lpszServerName,nServerPort,lpszUsername,lpszPassword,dwService,dwFlags,dwContext);

}

HANDLE FtpOpen_File(HANDLE hConnect,LPCWSTR lpszFileName,DWORD dwAccess,DWORD dwFlags,LPVOID dwContext)
{

typedef HANDLE (CALLBACK* parameters) (HANDLE,LPCWSTR,DWORD,DWORD,LPVOID);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("FtpOpenFileW"));
return Proc(hConnect,lpszFileName,dwAccess,dwFlags,dwContext);

}

BOOL Internet_WriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD dwNumberOfBytesToWrite,LPDWORD lpdwNumberOfBytesWritten)
{

typedef BOOL (CALLBACK* parameters) (HANDLE,LPCVOID,DWORD,LPDWORD);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetWriteFile"));
return Proc(hFile,lpBuffer,dwNumberOfBytesToWrite,lpdwNumberOfBytesWritten);

}

HANDLE Internet_OpenUrl(HANDLE hInternet,LPCWSTR lpszUrl,LPCWSTR lpszHeaders,DWORD dwHeadersLength,DWORD dwFlags,LPVOID dwContext)
{

typedef HANDLE (CALLBACK* parameters) (HANDLE,LPCWSTR,LPCWSTR,DWORD,DWORD,LPVOID);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetOpenUrlW"));
return Proc(hInternet,lpszUrl,lpszHeaders,dwHeadersLength,dwFlags,dwContext);

}

BOOL HttpQueryInfo(HANDLE hRequest,DWORD dwInfoLevel,LPVOID lpvBuffer,LPDWORD lpdwBufferLength,LPDWORD lpdwIndex)
{

typedef BOOL (CALLBACK* parameters) (HANDLE,DWORD,LPVOID,LPDWORD,LPDWORD);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("HttpQueryInfoW"));
return Proc(hRequest,dwInfoLevel,lpvBuffer,lpdwBufferLength,lpdwIndex);

}

DWORD FtpGetFileFileSize(HANDLE hFile,LPDWORD lpdwFileSizeHigh)
{

typedef DWORD (CALLBACK* parameters) (HANDLE,LPDWORD);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("FtpGetFileSize"));
return Proc(hFile,lpdwFileSizeHigh);

}

BOOL Internet_ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD dwNumberOfBytesToRead,LPDWORD lpdwNumberOfBytesRead)
{

typedef BOOL (CALLBACK* parameters) (HANDLE,LPVOID,DWORD,LPDWORD);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetReadFile"));
return Proc(hFile,lpBuffer,dwNumberOfBytesToRead,lpdwNumberOfBytesRead);

}

BOOL Internet_CloseHandle(HANDLE hInternet)
{

typedef BOOL (CALLBACK* parameters) (HANDLE);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetCloseHandle"));
return Proc(hInternet);

}

BOOL InternetQuery_DataAvailable(HANDLE hFile,LPDWORD lpdwNumberOfBytesAvailable,DWORD dwFlags,LPVOID dwContext)
{
typedef BOOL (CALLBACK* parameters) (HANDLE,LPDWORD,DWORD,LPVOID);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("InternetQueryDataAvailable"));
return Proc(hFile,lpdwNumberOfBytesAvailable,dwFlags,dwContext);
}


BOOL FtpSetCurrent_Directory(HANDLE hConnect,LPCWSTR lpszDirectory)
{
typedef BOOL (CALLBACK* parameters) (HANDLE,LPCWSTR);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("FtpSetCurrentDirectoryW"));
return Proc(hConnect,lpszDirectory);
}


BOOL FtpGetCurrent_Directory(HANDLE hConnect,LPCWSTR lpszCurrentDirectory,LPDWORD lpdwCurrentDirectory)
{
typedef BOOL (CALLBACK* parameters) (HANDLE,LPCWSTR,LPDWORD);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("FtpGetCurrentDirectoryW"));
return Proc(hConnect,lpszCurrentDirectory,lpdwCurrentDirectory);
}

BOOL FtpCreate_Directory(HANDLE hConnect,LPCWSTR lpszDirectory)
{
typedef BOOL (CALLBACK* parameters) (HANDLE,LPCWSTR);
parameters Proc = (parameters) GetProcAddress(NETHMO,_T("FtpCreateDirectoryW"));
return Proc(hConnect,lpszDirectory);
}


BOOL SHCreate_DirectoryExW(HWND hwnd,LPCWSTR pszPath,SECURITY_ATTRIBUTES *psa)
{
HMODULE SHHMO = LoadLibrary("Shell32.dll");
typedef BOOL (CALLBACK* parameters) (HWND,LPCWSTR,SECURITY_ATTRIBUTES*);
parameters Proc = (parameters) GetProcAddress(SHHMO,_T("SHCreateDirectoryExW"));
return Proc(hwnd,pszPath,psa);
}


bool WINAPI FtpCreate_Dir(HANDLE hConnect,LPCWSTR Dir,LPCWSTR CurrentDir)
{

std::wstring iDir = Dir,nDir;
DWORD CdirLength = MAX_PATH * 2;
WCHAR Cdir[MAX_PATH * 2];
int Pos = 0 , Err = 0 , TestPeriodslash = 0 , iNo = 0;
bool RT;

if (iDir.length() < 1 || iDir.length() > MAX_PATH
|| !hConnect ) return FALSE;

if (
iDir.find ( L"<" , 0 ) != std::string::npos || 
iDir.find ( L">" , 0 ) != std::string::npos || 
iDir.find ( L":" , 0 ) != std::string::npos || 
iDir.find ( L'"' , 0 ) != std::string::npos || 
iDir.find ( L"\\", 0 ) != std::string::npos ||
iDir.find ( L"|" , 0 ) != std::string::npos ||
iDir.find ( L"//", 0 ) != std::string::npos ||
iDir.find ( L"?" , 0 ) != std::string::npos ) 
return FALSE;

if (iDir[0] == (char) 47) iDir = iDir.substr(1,iDir.length());
if (iDir[iDir.length() - 1] == (char) 47) iDir = iDir.substr(0,iDir.length() - 1);

for (int i= iDir.length() - 1; i >= 0 ; i--) 
{
if (iDir[i] == (char) 46 && !TestPeriodslash) TestPeriodslash = 1;
if (iDir[i] == (char) 47 && !TestPeriodslash) TestPeriodslash = 2;
}

if (!std::wstring(CurrentDir).length())
{

RT = FtpGetCurrent_Directory(hConnect,Cdir,&CdirLength);
if (!RT) return FALSE;
CurrentDir = Cdir;

} else {

RT = FtpGetCurrent_Directory(hConnect,Cdir,&CdirLength);
if (!RT) return FALSE;

if (std::wstring(Cdir) != std::wstring(L"/"))
{
CurrentDir = 
(std::wstring(Cdir) + 
std::wstring(L"/") + 
std::wstring(CurrentDir)).c_str();
}

RT = FtpSetCurrent_Directory(hConnect,CurrentDir);
if (!RT) return FALSE;

}

if (!TestPeriodslash)
{
return TRUE;
}

if (TestPeriodslash)
{

for (int i= iDir.length() - 1; i >= 0 ; i--)
{

if (iDir[i] == (char) 47)
{
TestPeriodslash = 3;
iNo = i;
break;
}

}

if (TestPeriodslash != 3) 
{

return TRUE;

} else {

iDir = iDir.substr(0,iNo);

}

}

if (iDir[iDir.length() - 1] != (char) 47) iDir = iDir + std::wstring(L"/");

for (int j=0; j < iDir.length(); j++) 
{

if (iDir[j] == (char) 47) 
{

nDir = iDir.substr(Pos,j - Pos);
RT = FtpCreate_Directory(hConnect,nDir.c_str());
Err = GetLastError();
if (!RT && Err != 12003) return FALSE; //12003 ERROR_INTERNET_EXTENDED_ERROR
RT = FtpSetCurrent_Directory(hConnect,nDir.c_str());
if (!RT) return FALSE;
Pos = j + 1;

}

}

RT = FtpSetCurrent_Directory(hConnect,L"/");
if (!RT) return FALSE;

RT = FtpSetCurrent_Directory(hConnect,CurrentDir);
if (!RT) return FALSE;

return TRUE;
}

void atThreadLibraryExit(void)
{
if (DownlOInte) Internet_CloseHandle(DownlOInte);
if (GetThreadH) CloseHandle(GetThreadH);
if (PutThreadH) CloseHandle(PutThreadH);
if (DownlThreadH) CloseHandle(DownlThreadH);
if (CopyThreadH) CloseHandle(CopyThreadH);
if (SaveThrHD) CloseHandle(SaveThrHD);
}
