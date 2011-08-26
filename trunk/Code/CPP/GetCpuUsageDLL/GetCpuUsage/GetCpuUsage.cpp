// GetCpuUsage.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <stdio.h>
#include <winternl.h>

#define MAXCPU 32
typedef NTSTATUS (WINAPI *NTQSI)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);

void setcolor(int color, int bgcolor) {
	color &= 0xf;
	bgcolor &= 0xf;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (bgcolor << 4) | color);
}

void gotoxy(int x, int y)
{
	COORD Cur= {(SHORT)x, (SHORT) y};

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}

#define GRAPHSIZE 32
typedef struct {
	int index;
	int graph[GRAPHSIZE];
} Graph;
#define HALFRECT 0xDC
#define FULLRECT 0xDB

void draw_graph(Graph* graph, int x, int y) {
	static int color[11] = {7, 2, 2, 2, 14, 14, 14, 14, 12, 12, 12};
	static int bgcolor[11] = {0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 7};
	int index;
	int i, j, usage;

	for(i = 10; i >= 0; i--) {
		index = graph->index;
		gotoxy(x, y + 10 - i);
		setcolor(color[i], bgcolor[i]);
		for(j = 0; j < GRAPHSIZE; j++) {
			usage = graph->graph[index];
			if(usage / 10 == i) {
				if(usage % 10 < 5)
					putchar(HALFRECT);
				else
					putchar(FULLRECT);
			}
			else if(usage / 10 > i) {
				putchar(FULLRECT);
			}
			else {
				putchar(' ');
			}
			index = (index + 1) % GRAPHSIZE;
		}
	}
}


int DrawCpuUsage() {
	NTSTATUS retval;
	SYSTEM_BASIC_INFORMATION sbi;
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ppsi[MAXCPU], ppsi_old[MAXCPU];
	ULONG outlen;
	NTQSI ntqsi;
	int usage, i;
	Graph graph[MAXCPU];


	ntqsi = (NTQSI)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
	if(ntqsi == NULL) {
		printf("function open failed: %s\n", "NtQuerySystemInformation");
		return 1;
	}

	retval = ntqsi(SystemBasicInformation, &sbi, sizeof(sbi), &outlen);
	if(retval) {
		printf("failed\n");
		return 2;
	}
	if(sbi.NumberOfProcessors > MAXCPU) {
		printf("too many cpu's\nupper bound at %d", MAXCPU);
		return 3;
	}

	SetConsoleOutputCP(437);
	memset(ppsi_old, 0, sizeof(ppsi_old));
	memset(graph, 0, sizeof(graph));

	while(1) {
		ntqsi(SystemProcessorPerformanceInformation, ppsi, sizeof(ppsi), &outlen);
		if(outlen % sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)) {
			printf("returned unexpected size\n");
			continue;
		}

		for(i = 0; i < sbi.NumberOfProcessors; i++) {
			usage = 100 - (int)(((ppsi[i].IdleTime.QuadPart - ppsi_old[i].IdleTime.QuadPart) * 100) /
				((ppsi[i].KernelTime.QuadPart + ppsi[i].UserTime.QuadPart) -
				(ppsi_old[i].KernelTime.QuadPart + ppsi_old[i].UserTime.QuadPart))
				);
			graph[i].graph[graph[i].index] = usage;
			graph[i].index = (graph[i].index + 1) % GRAPHSIZE;
		}
		for(i = 0; i < sbi.NumberOfProcessors; i++) {
			int x, y;

			x = (i % 2) * (GRAPHSIZE + 3) + 2;
			y = (i / 2) * 12;
			gotoxy(x, y);
			printf("CPU%d  %d%% ", i, graph[i].graph[(graph[i].index + GRAPHSIZE - 1) % GRAPHSIZE]);
			draw_graph(graph + i, x, y + 1);
		}

		memcpy(ppsi_old, ppsi, outlen);

		Sleep(500);
	}
	SetConsoleOutputCP(0);
	return 0;
}


int GetCpuUsage(int num=0) {
	NTSTATUS retval;
	SYSTEM_BASIC_INFORMATION sbi;
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ppsi[MAXCPU], ppsi_old[MAXCPU];
	ULONG outlen;
	NTQSI ntqsi;
	int usage, i;
	int allusage=0;

	ntqsi = (NTQSI)GetProcAddress(LoadLibrary(L"ntdll.dll"), "NtQuerySystemInformation");
	if(ntqsi == NULL) {
//		printf("function open failed: %s\n", "NtQuerySystemInformation");
		return -1;
	}

	retval = ntqsi(SystemBasicInformation, &sbi, sizeof(sbi), &outlen);
	if(retval) {
//		printf("failed\n");
		return -2;
	}
	if(sbi.NumberOfProcessors > MAXCPU) {
//		printf("too many cpu's\nupper bound at %d", MAXCPU);
		return -3;
	}

	memset(ppsi_old, 0, sizeof(ppsi_old));
	ntqsi(SystemProcessorPerformanceInformation, ppsi, sizeof(ppsi), &outlen);
	if(outlen % sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)) {
//		printf("returned unexpected size\n");
		return -4;
	}
	memcpy(ppsi_old, ppsi, outlen);
	Sleep(500);
	ntqsi(SystemProcessorPerformanceInformation, ppsi, sizeof(ppsi), &outlen);
	if(outlen % sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)) {
//		printf("returned unexpected size\n");
		return -4;
	}
	if (num>sbi.NumberOfProcessors)	{num=0;	}
	if (num!=0)
	{
		usage = 100 - (int)(((ppsi[num].IdleTime.QuadPart - ppsi_old[num].IdleTime.QuadPart) * 100) /
			((ppsi[num].KernelTime.QuadPart + ppsi[num].UserTime.QuadPart) -
			(ppsi_old[num].KernelTime.QuadPart + ppsi_old[num].UserTime.QuadPart))
			);
		return usage;
	}
	else
	{
		for(i = 0; i < sbi.NumberOfProcessors; i++) {
			usage = 100 - (int)(((ppsi[i].IdleTime.QuadPart - ppsi_old[i].IdleTime.QuadPart) * 100) /
				((ppsi[i].KernelTime.QuadPart + ppsi[i].UserTime.QuadPart) -
				(ppsi_old[i].KernelTime.QuadPart + ppsi_old[i].UserTime.QuadPart))
				);
			allusage+=usage;
	}
		allusage=allusage/(sbi.NumberOfProcessors);
		return allusage;
	}
}

