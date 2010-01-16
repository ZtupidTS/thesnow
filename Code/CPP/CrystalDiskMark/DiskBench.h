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

UINT ExecDiskBenchAll(void* dlg);
UINT ExecDiskBenchSequential(void* dlg);
UINT ExecDiskBenchRandom512KB(void* dlg);
UINT ExecDiskBenchRandom4KB(void* dlg);