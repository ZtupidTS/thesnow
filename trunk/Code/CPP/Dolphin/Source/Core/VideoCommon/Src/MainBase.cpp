
#include "MainBase.h"
#include "VideoState.h"
#include "VideoConfig.h"
#include "RenderBase.h"
#include "FramebufferManagerBase.h"
#include "TextureCacheBase.h"
#include "VertexLoaderManager.h"
#include "CommandProcessor.h"
#include "PixelEngine.h"
#include "Atomic.h"
#include "Fifo.h"
#include "BPStructs.h"
#include "OnScreenDisplay.h"
#include "VideoBackendBase.h"
#include "ConfigManager.h"

bool s_BackendInitialized = false;

volatile u32 s_swapRequested = false;
u32 s_efbAccessRequested = false;
volatile u32 s_FifoShuttingDown = false;

static volatile struct
{
	u32 xfbAddr;
	FieldType field;
	u32 fbWidth;
	u32 fbHeight;
} s_beginFieldArgs;

static struct
{
	EFBAccessType type;
	u32 x;
	u32 y;
	u32 Data;
} s_accessEFBArgs;

static u32 s_AccessEFBResult = 0;

void VideoBackendHardware::EmuStateChange(EMUSTATE_CHANGE newState)
{
	EmulatorState((newState == EMUSTATE_CHANGE_PLAY) ? true : false);
}

// Enter and exit the video loop
void VideoBackendHardware::Video_EnterLoop()
{
	RunGpuLoop();
}

void VideoBackendHardware::Video_ExitLoop()
{
	ExitGpuLoop();
	s_FifoShuttingDown = true;
}

void VideoBackendHardware::Video_SetRendering(bool bEnabled)
{
	Fifo_SetRendering(bEnabled);
}

// Run from the graphics thread (from Fifo.cpp)
void VideoFifo_CheckSwapRequest()
{
	if(g_ActiveConfig.bUseXFB)
	{
		if (Common::AtomicLoadAcquire(s_swapRequested))
		{
			EFBRectangle rc;
			g_renderer->Swap(s_beginFieldArgs.xfbAddr, s_beginFieldArgs.field, s_beginFieldArgs.fbWidth, s_beginFieldArgs.fbHeight,rc);
			Common::AtomicStoreRelease(s_swapRequested, false);
		}
	}
}

// Run from the graphics thread (from Fifo.cpp)
void VideoFifo_CheckSwapRequestAt(u32 xfbAddr, u32 fbWidth, u32 fbHeight)
{
	if (g_ActiveConfig.bUseXFB)
	{
		if(Common::AtomicLoadAcquire(s_swapRequested))
		{
			u32 aLower = xfbAddr;
			u32 aUpper = xfbAddr + 2 * fbWidth * fbHeight;
			u32 bLower = s_beginFieldArgs.xfbAddr;
			u32 bUpper = s_beginFieldArgs.xfbAddr + 2 * s_beginFieldArgs.fbWidth * s_beginFieldArgs.fbHeight;

			if (addrRangesOverlap(aLower, aUpper, bLower, bUpper))
				VideoFifo_CheckSwapRequest();
		}
	}
}

// Run from the CPU thread (from VideoInterface.cpp)
void VideoBackendHardware::Video_BeginField(u32 xfbAddr, FieldType field, u32 fbWidth, u32 fbHeight)
{
	if (s_BackendInitialized && g_ActiveConfig.bUseXFB)
	{
		if (!SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread)
			VideoFifo_CheckSwapRequest();
		s_beginFieldArgs.xfbAddr = xfbAddr;
		s_beginFieldArgs.field = field;
		s_beginFieldArgs.fbWidth = fbWidth;
		s_beginFieldArgs.fbHeight = fbHeight;
	}
}

// Run from the CPU thread (from VideoInterface.cpp)
void VideoBackendHardware::Video_EndField()
{
	if (s_BackendInitialized)
	{
		Common::AtomicStoreRelease(s_swapRequested, true);
	}
}

void VideoBackendHardware::Video_AddMessage(const char* pstr, u32 milliseconds)
{
	OSD::AddMessage(pstr, milliseconds);
}

void VideoBackendHardware::Video_ClearMessages()
{
	OSD::ClearMessages();
}

// Screenshot
bool VideoBackendHardware::Video_Screenshot(const char *_szFilename)
{
	Renderer::SetScreenshot(_szFilename);
	return true;
}

void VideoFifo_CheckEFBAccess()
{
	if (Common::AtomicLoadAcquire(s_efbAccessRequested))
	{
		s_AccessEFBResult = g_renderer->AccessEFB(s_accessEFBArgs.type, s_accessEFBArgs.x, s_accessEFBArgs.y, s_accessEFBArgs.Data);

		Common::AtomicStoreRelease(s_efbAccessRequested, false);
	}
}

u32 VideoBackendHardware::Video_AccessEFB(EFBAccessType type, u32 x, u32 y, u32 InputData)
{
	if (s_BackendInitialized)
	{
		s_accessEFBArgs.type = type;
		s_accessEFBArgs.x = x;
		s_accessEFBArgs.y = y;
		s_accessEFBArgs.Data = InputData;

		Common::AtomicStoreRelease(s_efbAccessRequested, true);

		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread)
		{
			while (Common::AtomicLoadAcquire(s_efbAccessRequested) && !s_FifoShuttingDown)
				//Common::SleepCurrentThread(1);
				Common::YieldCPU();
		}
		else
			VideoFifo_CheckEFBAccess();

		return s_AccessEFBResult;
	}

	return 0;
}

static volatile u32 s_doStateRequested = false;
 
static volatile struct
{
	unsigned char **ptr;
	int mode;
} s_doStateArgs;

// Depending on the threading mode (DC/SC) this can be called 
// from either the GPU thread or the CPU thread
void VideoFifo_CheckStateRequest()
{
	if (Common::AtomicLoadAcquire(s_doStateRequested))
	{
		// Clear all caches that touch RAM
		//TextureCache::Invalidate(false);
		VertexLoaderManager::MarkAllDirty();

		PointerWrap p(s_doStateArgs.ptr, s_doStateArgs.mode);
		VideoCommon_DoState(p);

		// Refresh state.
		if (s_doStateArgs.mode == PointerWrap::MODE_READ)
		{
			BPReload();
			RecomputeCachedArraybases();
		}

		Common::AtomicStoreRelease(s_doStateRequested, false);
	}
}

// Run from the CPU thread
void VideoBackendHardware::DoState(PointerWrap& p)
{
	s_doStateArgs.ptr = p.ptr;
	s_doStateArgs.mode = p.mode;
	Common::AtomicStoreRelease(s_doStateRequested, true);
	if (SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread)
	{
		while (Common::AtomicLoadAcquire(s_doStateRequested) && !s_FifoShuttingDown)
			Common::YieldCPU();
	}
	else
		VideoFifo_CheckStateRequest();
}

void VideoBackendHardware::RunLoop(bool enable)
{
	VideoCommon_RunLoop(enable);
}

void VideoFifo_CheckAsyncRequest()
{
	VideoFifo_CheckSwapRequest();
	VideoFifo_CheckEFBAccess();
}

void VideoBackendHardware::Video_GatherPipeBursted()
{
	CommandProcessor::GatherPipeBursted();
}

bool VideoBackendHardware::Video_IsPossibleWaitingSetDrawDone()
{
	return CommandProcessor::isPossibleWaitingSetDrawDone;
}

void VideoBackendHardware::Video_AbortFrame()
{
	CommandProcessor::AbortFrame();
}

readFn16 VideoBackendHardware::Video_CPRead16()
{
	return CommandProcessor::Read16;
}
writeFn16 VideoBackendHardware::Video_CPWrite16()
{
	return CommandProcessor::Write16;
}

readFn16  VideoBackendHardware::Video_PERead16()
{
	return PixelEngine::Read16;
}
writeFn16 VideoBackendHardware::Video_PEWrite16()
{
	return PixelEngine::Write16;
}
writeFn32 VideoBackendHardware::Video_PEWrite32()
{
	return PixelEngine::Write32;
}
