// GetCPU.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "cpu.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CPU cpu;
    int sys;
    int process;
    TKTime upTime;

    for( ;; )
    {
        Sleep( 200 );
        process = cpu.GetUsage( &sys, &upTime );
        wprintf( _T("Process : %d\tSystem  : %d\n"), process, sys );
    }

    return 0;
}

