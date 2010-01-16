/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2007-2008 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include <afxmt.h>

#include "DiskMark.h"
#include "DiskMarkDlg.h"
#include "DiskBench.h"

#include <winioctl.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define DISK_TEST_MAX_BUFFER_SIZE  1024*1024
#define DISK_TEST_TIME             6*1000

static char* buf = NULL;
static CString TestFilePath;
static CString TestFileDir;
static HANDLE hFile;
static int BufSize;
static int Loop;
static int DiskTestNumber;
static unsigned int DiskTestSize;

static void Init(void* dlg);
static void Sequential(void* dlg);
static void Random(void* dlg, int size, double &readScore, double &writeScore);
static UINT Exit(void* dlg);
static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static BOOL WaitFlag;

/** Reference 
Xorshift RNGs
George Marsaglia
The Florida State University
http://www.jstatsoft.org/v08/i14/paper
*/
unsigned long xor128()
{
	static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
	unsigned long t;
	t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
}



UINT ExecDiskBenchAll(LPVOID dlg)
{
	Init(dlg);
	if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){Sequential(dlg);}
	if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		Random(dlg, 512, 
			(((CDiskMarkDlg*)dlg)->m_RandomRead512KBScore), 
			(((CDiskMarkDlg*)dlg)->m_RandomWrite512KBScore));}
	if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		Random(dlg, 4, 
			(((CDiskMarkDlg*)dlg)->m_RandomRead4KBScore), 
			(((CDiskMarkDlg*)dlg)->m_RandomWrite4KBScore));}
	return Exit(dlg);
}

UINT ExecDiskBenchSequential(LPVOID dlg)
{
	Init(dlg);
	if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){Sequential(dlg);}
	return Exit(dlg);
}

UINT ExecDiskBenchRandom512KB(LPVOID dlg)
{
	Init(dlg);
	if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		Random(dlg, 512, 
			(((CDiskMarkDlg*)dlg)->m_RandomRead512KBScore), 
			(((CDiskMarkDlg*)dlg)->m_RandomWrite512KBScore));}
	return Exit(dlg);
}

UINT ExecDiskBenchRandom4KB(LPVOID dlg)
{
	Init(dlg);
	if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		Random(dlg, 4, 
			(((CDiskMarkDlg*)dlg)->m_RandomRead4KBScore), 
			(((CDiskMarkDlg*)dlg)->m_RandomWrite4KBScore));}
	return Exit(dlg);
}

void Init(void* dlg)
{
	BOOL FlagArc;
	BOOL result;
	static CString cstr;
	TCHAR drive;
	
	drive = ((CDiskMarkDlg*)dlg)->m_ValueTestDrive.GetAt(0);

	DiskTestNumber = ((CDiskMarkDlg*)dlg)->m_IndexTestNumber + 1;
	DiskTestSize   = _tstoi(((CDiskMarkDlg*)dlg)->m_ValueTestSize) * 1024 * 1024;

	BufSize = DISK_TEST_MAX_BUFFER_SIZE;
	Loop = DiskTestSize / BufSize;

// GetVolumeInformation (IS_COMPRESSED)
	TCHAR RootPath[4];
	wsprintf(RootPath, _T("%c:\\"), drive);

	TestFileDir.Format(_T("%sCrystalDiskMark%08X"), RootPath, timeGetTime());
	CreateDirectory(TestFileDir, NULL);
	TestFilePath.Format(_T("%s\\CrystalDiskMark%08X.tmp"), TestFileDir, timeGetTime());

	DWORD FileSystemFlags;
	GetVolumeInformation(RootPath, NULL, NULL, NULL, NULL, &FileSystemFlags, NULL, NULL);
	if( FileSystemFlags & FS_VOL_IS_COMPRESSED ){
		FlagArc = TRUE;
	}else{
		FlagArc = FALSE;
	}

// Check Disk Capacity //
	OSVERSIONINFO osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&osVersionInfo);

	ULARGE_INTEGER FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes;
	GetDiskFreeSpaceEx(RootPath, &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);
	if(TotalNumberOfFreeBytes.HighPart == 0 && DiskTestSize > TotalNumberOfFreeBytes.LowPart )
	{
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCapacityError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return ;
	}

// Test Write
	DWORD writesize;
	TCHAR data[] = _T("123");
	HANDLE hFile = ::CreateFile(TestFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE){
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return ;
	}

	result = WriteFile(hFile, data, sizeof(TCHAR)*4, &writesize, NULL);
	CloseHandle(hFile);
	DeleteFile(TestFilePath);

	if(result == 0){
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskWriteError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return ;
	}

// Preapare Test File
	int i;

	buf = (char*)VirtualAlloc(NULL, BufSize, MEM_COMMIT, PAGE_READWRITE);
	if(buf == NULL){
		AfxMessageBox(_T("Failed VirtualAlloc()."));
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return ;
	}

	// Fill buffer with random data
	int temp = BufSize / 2;
	for(i=0;i< temp;i++){
		memset((buf + i*2), xor128(), 2);
	}

	cstr = "Preparing...";
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM)&cstr, 0);

	hFile = ::CreateFile(TestFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if(hFile == INVALID_HANDLE_VALUE){
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return ;
	}

// COMPRESSION_FORMAT_NONE
	USHORT lpInBuffer = COMPRESSION_FORMAT_NONE;
	DWORD lpBytesReturned = 0;
	DeviceIoControl(hFile, FSCTL_SET_COMPRESSION, (LPVOID) &lpInBuffer,
				sizeof(USHORT), NULL, 0, (LPDWORD)&lpBytesReturned, NULL);

	for(i = 0;i < Loop;i++){
		if(((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
			result = WriteFile(hFile, buf, BufSize, &writesize, NULL);
		}else{
			CloseHandle(hFile);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}
	}
	CloseHandle(hFile);

	if(result == 0){
		AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskWriteError);
		((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
		return ;
	}
}

void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if(idEvent == TIMER_ID)
	{
		WaitFlag = FALSE;
		KillTimer(hwnd, idEvent);
	}
}

UINT Exit(void* dlg)
{
	if(buf != NULL){
		VirtualFree(buf, DISK_TEST_MAX_BUFFER_SIZE, MEM_DECOMMIT);
		buf = NULL;
	}
	DeleteFile(TestFilePath);
	RemoveDirectory(TestFileDir);

	static CString cstr;
	cstr = _T("");
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM)&cstr, 0);
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_EXIT_BENCHMARK, 0, 0);

	((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
	((CDiskMarkDlg*)dlg)->m_WinThread = NULL;

	return 0;
}

void Sequential(void* dlg)
{
	static CString cstr;
	int count;
	double score, maxScore;
	int i, j;
	DWORD start, finish;
	DWORD readSize, writeSize;
	BOOL result;

//////////////////////////////////////////////
// Read Test
//////////////////////////////////////////////
	if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		return ;
	}

	score = 0.0;
	maxScore = 0.0;
	for(j = 1; j <= DiskTestNumber; j++){
		cstr.Format(_T("Sequential Read %d/%d"), j, DiskTestNumber);
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM)&cstr, 0);

		hFile = ::CreateFile(TestFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(hFile == INVALID_HANDLE_VALUE){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}

		count = 0;
		WaitFlag = TRUE;
		SetTimer(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), TIMER_ID, DISK_TEST_TIME, TimerProc);
		start = timeGetTime();
		do{
			for(i = 0; i < Loop; i++)
			{
				result = ReadFile(hFile, buf, BufSize, &readSize, NULL);
				if(result)
				{
					count++;
				}
				else
				{
					CloseHandle(hFile);
					AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskReadError);
					((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
					return ;
				}
			}
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		}while(WaitFlag);
		CloseHandle(hFile);
		finish = timeGetTime();

		if(result == 0){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskReadError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}
		score = count * (BufSize / 1000.0) / (finish - start);
		if(score > maxScore){
			maxScore = score;
		}
		((CDiskMarkDlg*)dlg)->m_SequentialReadScore = score;
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
		if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
			return ;
		}
	}
	((CDiskMarkDlg*)dlg)->m_SequentialReadScore = maxScore;
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);

//////////////////////////////////////////////
// Write Test
//////////////////////////////////////////////
	if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		return ;
	}
	score = 0.0;
	maxScore = 0.0;
	for(j = 1; j <= DiskTestNumber; j++){
		cstr.Format(_T("Sequential Write %d/%d"), j, DiskTestNumber);
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM)&cstr, 0);

		hFile = ::CreateFile(TestFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if(hFile == INVALID_HANDLE_VALUE ){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}
		count = 0;

		WaitFlag = TRUE;
		SetTimer(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), TIMER_ID, DISK_TEST_TIME, TimerProc);
		start = timeGetTime();
		do{
			for(i = 0; i < Loop; i++)
			{
				result = WriteFile(hFile, buf, BufSize, &writeSize, NULL);
				if(result)
				{
					count++;
				}
				else
				{
					FlushFileBuffers(hFile);
					CloseHandle(hFile);
					AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskWriteError);
					((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
					return ;
				}
			}
			SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		}while(WaitFlag);
		FlushFileBuffers(hFile);
		CloseHandle(hFile);

		finish = timeGetTime();
		if(result == 0){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskWriteError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}

		score = count * (BufSize / 1000.0) / (finish - start);
		if(score > maxScore){
			maxScore = score;
		}
		((CDiskMarkDlg*)dlg)->m_SequentialWriteScore = score;
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
		if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
			return ;
		}
	}
	((CDiskMarkDlg*)dlg)->m_SequentialWriteScore = maxScore;
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
}

void Random(void* dlg, int size, double &readScore, double &writeScore)
{
	static CString cstr;
	int count;
	double score, maxScore;
	DWORD start, finish;
	int i, j;
	DWORD readSize, writeSize;
	int bufSize;
	int loop;
	int split;
	BOOL result;

	score = 0.0;
	maxScore = 0.0;

// init
	if(size == 512){	
		bufSize = size * 1024;
		loop = 20;
		split = 2 * (DiskTestSize / 1024 / 1024);
	}else if(size == 4){
		bufSize = size * 1024;
		loop = 160;
		split = 256 * (DiskTestSize / 1024 / 1024);
	}else{
		return ;
	}

//////////////////////////////////////////////
// Read Test
//////////////////////////////////////////////
	if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		return ;
	}
	for(j = 1; j <= DiskTestNumber; j++){
		cstr.Format(_T("Random Read %dKB %d/%d"), size, j, DiskTestNumber);
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM)&cstr, 0);

		hFile = ::CreateFile(TestFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_RANDOM_ACCESS, NULL);
		if(hFile == INVALID_HANDLE_VALUE ){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}

		count = 0;
		WaitFlag = TRUE;
		SetTimer(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), TIMER_ID, DISK_TEST_TIME, TimerProc);
		start = timeGetTime();
		do{
			for(i = 0; i < loop; i++)
			{
				SetFilePointer(hFile, (xor128() % split) * bufSize, NULL, FILE_BEGIN);
				result = ReadFile(hFile, buf, bufSize, &readSize, NULL);
				if(result)
				{
					count++;
				}
				else
				{
					CloseHandle(hFile);
					AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskReadError);
					((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
					return ;
				}
			}
		}while(WaitFlag);
		CloseHandle(hFile);
		finish = timeGetTime();
		if(result == 0){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskReadError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}

		score = count * (bufSize / 1000.0) / (finish - start);
		if(score > maxScore){
			maxScore = score;
		}
		readScore = score;
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
		if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
			return ;
		}
	}
	readScore = maxScore;
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);

//////////////////////////////////////////////
// Write Test
//////////////////////////////////////////////
	if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
		return ;
	}
	score = 0.0;
	maxScore = 0.0;
	for(j = 1; j <= DiskTestNumber; j++){
		cstr.Format(_T("Random Write %dKB %d/%d"), size, j, DiskTestNumber);
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_MESSAGE, (WPARAM)&cstr, 0);

		hFile = ::CreateFile(TestFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING|FILE_FLAG_RANDOM_ACCESS, NULL);
		if(hFile == INVALID_HANDLE_VALUE ){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskCreateFileError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}

		count = 0;
		WaitFlag = TRUE;
		SetTimer(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), TIMER_ID, DISK_TEST_TIME, TimerProc);
		start = timeGetTime();
		do{
			for(i = 0; i < loop; i++)
			{
				SetFilePointer(hFile, (xor128() % split) * bufSize, NULL, FILE_BEGIN);
				result = WriteFile(hFile, buf, bufSize, &writeSize, NULL);
				if(result)
				{
					count++;
				}
				else
				{
					FlushFileBuffers(hFile);
					CloseHandle(hFile);
					AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskWriteError);
					((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
					return ;
				}
			}
		}while(WaitFlag);
		FlushFileBuffers(hFile);
		CloseHandle(hFile);
		if(result == 0){
			AfxMessageBox(((CDiskMarkDlg*)dlg)->m_MesDiskWriteError);
			((CDiskMarkDlg*)dlg)->m_DiskBenchStatus = FALSE;
			return ;
		}
		finish = timeGetTime();
		score = count * (bufSize / 1000.0) / (finish - start);
		if(score > maxScore){
			maxScore = score;
		}
		writeScore = score;
		::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
		if(! ((CDiskMarkDlg*)dlg)->m_DiskBenchStatus){
			return ;
		}
	}

	writeScore = maxScore;
	::PostMessage(((CDiskMarkDlg*)dlg)->GetSafeHwnd(), WM_USER_UPDATE_SCORE, 0, 0);
}
