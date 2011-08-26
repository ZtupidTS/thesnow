// isVM.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

//********************************************************************
//      Filename: Main.cpp
//        Author: Chinafish
//      Modifier: Chinafish
//       Created: 2008-5-15 15:07
//       Updated: 2008-5-15 15:07
//            QQ: 149200849
//           MSN: china_fish@msn.com    
//       Purpose: �жϳ����Ƿ��������������
//=====================================================
//  Copyright(C) 2004-2008 by Chinafish. All Rights Reserved.
//********************************************************************

DWORD __forceinline IsInsideVPC_exceptionFilter(LPEXCEPTION_POINTERS ep);
bool IsInsideVPC();
bool IsInsideVMWare();
int CheckVPC();
/*
// �����汾ʹ��MiniPE (3.5KB)
#ifndef _DEBUG

#pragma comment(linker, "/ENTRY:EntryPoint")
#pragma comment(linker, "/SECTION:VPC,")
#pragma comment(linker, "/MERGE:.data=VPC")

int EntryPoint()
{
    CheckVPC();
    ExitProcess(0);
}
#else
int WINAPI WinMain(IN HINSTANCE hInstance, IN HINSTANCE hPrevInstance, IN LPSTR lpCmdLine, IN int nShowCmd )
{
    return CheckVPC();
}
#endif // _DEBUG
*/
int fnisVM()
{
	if(IsInsideVPC())
		return 1;
	else if(IsInsideVMWare())
		return 2;
	else
		return 0;
}
/*
int CheckVPC()
{
    if(IsInsideVPC())
        MessageBox(NULL, "�����������Microsoft Virtual PC��!", "��ʾ", MB_OK|MB_ICONINFORMATION);
    else if(IsInsideVMWare())
        MessageBox(NULL, "�����������VMWare��!", "��ʾ", MB_OK|MB_ICONINFORMATION);
    else
        MessageBox(NULL, "������ʵ�ĵ�����!", "��ʾ", MB_OK|MB_ICONINFORMATION);

    return 0;
}
*/
DWORD __forceinline IsInsideVPC_exceptionFilter(LPEXCEPTION_POINTERS ep)
{
    PCONTEXT ctx = ep->ContextRecord;
    ctx->Ebx = -1;
    ctx->Eip += 4;
    return EXCEPTION_CONTINUE_EXECUTION;
}

bool IsInsideVPC()
{
    bool rc = false;
    __try
    {
        _asm push ebx
        _asm mov ebx, 0 // It will stay ZERO if VPC is running
        _asm mov eax, 1 // VPC function number
        _asm __emit 0Fh
        _asm __emit 3Fh
        _asm __emit 07h
        _asm __emit 0Bh
        _asm test ebx, ebx
        _asm setz [rc]
        _asm pop ebx
    }
    // The except block shouldn't get triggered if VPC is running!!
    __except(IsInsideVPC_exceptionFilter(GetExceptionInformation()))
    {
    }

    return rc;
}

bool IsInsideVMWare()
{
    bool rc = true;

    __try
    {
        __asm
        {
            push edx
            push ecx
            push ebx
            mov eax, 'VMXh'
            mov ebx, 0
            mov ecx, 10
            mov edx, 'VX'
            in eax, dx
            cmp ebx, 'VMXh'
            setz [rc]
            pop ebx
            pop ecx
            pop edx
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = false;
    }

    return rc;
} 