
#ifndef _VIDEOCOMMON_MAINBASE_H_
#define _VIDEOCOMMON_MAINBASE_H_

#include "CommonTypes.h"

extern bool s_PluginInitialized;
extern u32 s_efbAccessRequested;
extern volatile u32 s_FifoShuttingDown;
extern volatile u32 s_swapRequested;

void VideoFifo_CheckEFBAccess();
void VideoFifo_CheckSwapRequestAt(u32 xfbAddr, u32 fbWidth, u32 fbHeight);

#endif
