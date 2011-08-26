
#include <windows.h>
#include <psapi.h>
#include <string>
#include <iostream>
//#include <io.h>
//#include <stdio.h>
//#include <stdlib.h>
#include "resource.h"

#pragma comment(lib, "Psapi.lib")

//提升进程访问权限
void enableDebugPriv()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue)) {
		CloseHandle(hToken);
		return;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
		CloseHandle(hToken);
}

//根据进程名称取得进程ID,如果有多个运行实例则返回第一个枚举出来的进程ID 
DWORD getSpecifiedProcessId(const char* pszProcessName) 
{
	DWORD processId[1024], cbNeeded, dwProcessesCount;
	HANDLE hProcess;
	HMODULE hMod;

	char szProcessName[MAX_PATH] = "UnknownProcess";
	DWORD dwArrayInBytes = sizeof(processId)*sizeof(DWORD);

	if (!EnumProcesses(processId, dwArrayInBytes, &cbNeeded))
		return 0;
	//计算数组中的元素个数
	dwProcessesCount = cbNeeded / sizeof(DWORD);
	enableDebugPriv();

	for (UINT i = 0; i < dwProcessesCount; i++) {
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId[i]);
		if (!hProcess) {
			continue;
		} else {
			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
				GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName));
				if (!_stricmp(szProcessName, pszProcessName)) {
					CloseHandle(hProcess);
					return processId[i];
				}
			}
		}
	}

	CloseHandle(hProcess);
	return 0;
}

int GETDLL(){
	HRSRC res=::FindResource(NULL, MAKEINTRESOURCE(IDR_KILLSELF), "KillSelf");
	HGLOBAL gl=::LoadResource(NULL,res);
	LPVOID lp=::LockResource(gl); // 查找，加载，锁定资源
	char *strFileName= "c:\\KILL.dll";

	//判断文件是否存在
//	if( (_access( strFileName, 0 )) == -1 )
	if (INVALID_HANDLE_VALUE == CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))
	{
		HANDLE fp=::CreateFile(strFileName,
			GENERIC_WRITE,
			FILE_SHARE_READ ,
			NULL,
			CREATE_NEW, //创建新文件，如目标文件已存在则调用失败
			0,
			NULL);
		DWORD aa;
		if (!::WriteFile (fp,lp,::SizeofResource(NULL,res),&aa,NULL))
			return 0; //sizeofResource 得到资源文件的大小
		::CloseHandle (fp); //关闭句柄
		::FreeResource (gl); //释放内存
	}
	return 1;
};

int main(int argc, char* argv[])
{
	std::string strProcessName;
	std::cout << "KILL YOU (C) 2010 thesnoW\r\n\0";
	if (argc > 1)
	{
		strProcessName=argv[1];
	} 
	else
	{
		std::cout << "Please input the name of target process !(If you won't,Press Ctrl+C)" << std::endl;
		//等待输入进程名称
		std::cin >> strProcessName;
	}

	GETDLL();
	//在这里为了简单起见，使用了绝对路径
	char szDllPath[MAX_PATH] = "c:\\KILL.dll";
	char szFileName[MAX_PATH] = "c:\\KILL.dll";
	//提升进程访问权限
	enableDebugPriv();

	if (strProcessName.empty()) {
		MessageBox(NULL, "The target process name is invalid !", "Notice", MB_ICONSTOP);
		return -1;
	}
	//根据进程名称得到进程ID
	DWORD dwTargetProcessId = getSpecifiedProcessId(strProcessName.c_str());
	if (!dwTargetProcessId)
	{
		std::cout << "Process not exist or Process is 64bit program!\r\n\0";
		DeleteFile("c:\\KILL.dll");
		return -1;
	}
	HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwTargetProcessId);

	if (!hTargetProcess) {
		MessageBox(NULL, "Open target process failed !", "Notice", MB_ICONSTOP);
		return -1;
	}
	//计算DLL文件名称所占的存储空间
	int memorySize = (strlen(szDllPath) + 1) * sizeof(char);
	//在目标进程中开辟存储空间，用来存放DLL的文件名称
	char* pszFileNameRemote = (char*)VirtualAllocEx(hTargetProcess,
		0, memorySize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!pszFileNameRemote) {
		MessageBox(NULL, "Alloc dll name string in target process failed !", "Notice", MB_ICONSTOP);
		return -1;
	}
	//将DLL的文件名写入目标进程地址空间
	if (!WriteProcessMemory(hTargetProcess, pszFileNameRemote,
		(LPVOID)szFileName, memorySize, NULL)) {
			MessageBox(NULL, "Write dll name string to target process failed !","Notice", MB_ICONSTOP);
			DeleteFile("c:\\KILL.dll");
			return -1;
	}

	PTHREAD_START_ROUTINE pfnStartAddr =
		(PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

	HANDLE hRemoteThread = CreateRemoteThread(hTargetProcess,
		NULL, 0, pfnStartAddr, pszFileNameRemote, 0, NULL);

	WaitForSingleObject(hRemoteThread, INFINITE);
	VirtualFreeEx(hTargetProcess, 0, memorySize, NULL);

	if (hRemoteThread)
		CloseHandle(hTargetProcess);
	Sleep(500);
	DeleteFile("c:\\KILL.dll");
	return 0;
} 