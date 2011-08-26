/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2007 hiyohiyo, All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#define WM_USER_UPDATE_SCORE	(WM_USER+1)
#define WM_USER_UPDATE_MESSAGE	(WM_USER+2)
#define WM_USER_EXIT_BENCHMARK	(WM_USER+3)

#define TIMER_ID 5963

// Common Message
#define ALL_0X00_0FILL _T("<All 0x00, 0Fill>")
#define ALL_0XFF_1FILL _T("<All 0xFF, 1Fill>")

enum TEST_DATA_TYPE
{
	TEST_DATA_DEFAULT = 0,
	TEST_DATA_ALL0X00,
	TEST_DATA_ALL0XFF,
};

UINT ExecDiskBenchAll(void* dlg);
UINT ExecDiskBenchSequential(void* dlg);
UINT ExecDiskBenchRandom512KB(void* dlg);
UINT ExecDiskBenchRandom4KB(void* dlg);
UINT ExecDiskBenchRandom4KB32QD(void* dlg);