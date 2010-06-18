// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/


#ifdef _WIN32
	#include <windows.h>
#endif

#include "Setup.h" // Common
#include "Atomic.h"
#include "Thread.h"
#include "Timer.h"
#include "Common.h"
#include "StringUtil.h"
#include "MathUtil.h"
#include "MemoryUtil.h"

#include "Console.h"
#include "Core.h"
#include "CPUDetect.h"
#include "CoreTiming.h"
#include "Boot/Boot.h"

#include "HW/Memmap.h"
#include "HW/ProcessorInterface.h"
#include "HW/GPFifo.h"
#include "HW/CPU.h"
#include "HW/GCPad.h"
#include "HW/HW.h"
#include "HW/DSP.h"
#include "HW/GPFifo.h"
#include "HW/AudioInterface.h"
#include "HW/VideoInterface.h"
#include "HW/SystemTimers.h"

#include "PowerPC/PowerPC.h"
#include "PowerPC/JitCommon/JitBase.h"

#include "PluginManager.h"
#include "ConfigManager.h"

#include "VolumeHandler.h"
#include "FileMonitor.h"

#include "MemTools.h"
#include "Host.h"
#include "LogManager.h"

#include "State.h"
#include "OnFrame.h"

namespace Core
{

// Declarations and definitions
Common::Timer Timer;
volatile u32 DrawnFrame = 0;
u32 DrawnVideo = 0;

// Function forwarding
void Callback_VideoRequestWindowSize(int& x, int& y, int& width, int& height);
void Callback_VideoLog(const TCHAR* _szMessage, int _bDoBreak);
void Callback_VideoCopiedToXFB(bool video_update);
void Callback_DSPLog(const TCHAR* _szMessage, int _v);
const char *Callback_ISOName(void);
void Callback_DSPInterrupt();
void Callback_PADLog(const TCHAR* _szMessage);
void Callback_WiimoteLog(const TCHAR* _szMessage, int _v);
void Callback_WiimoteInterruptChannel(int _number, u16 _channelID, const void* _pData, u32 _Size);
bool Callback_RendererHasFocus(void);

// For keyboard shortcuts.
void Callback_CoreMessage(int Id);
TPeekMessages Callback_PeekMessages = NULL;
TUpdateFPSDisplay g_pUpdateFPSDisplay = NULL;

// Function declarations
THREAD_RETURN EmuThread(void *pArg);

void Stop();

bool g_bStopping = false;
bool g_bHwInit = false;
bool g_bRealWiimote = false;
HWND g_pWindowHandle = NULL;
#if defined(HAVE_X11) && HAVE_X11
void *g_pXWindow = NULL;
#endif
Common::Thread* g_EmuThread = NULL;

static Common::Thread* cpuThread = NULL;

SCoreStartupParameter g_CoreStartupParameter;

// This event is set when the emuthread starts.
Common::Event emuThreadGoing;
Common::Event cpuRunloopQuit;



// Display messages and return values

// Formatted stop message
std::string StopMessage(bool bMainThread, std::string Message)
{
	return StringFromFormat("Stop [%s %i]\t%s\t%s",
		bMainThread ? "Main Thread" : "Video Thread", Common::Thread::CurrentId(), MemUsage().c_str(), Message.c_str());
}

// 
bool PanicAlertToVideo(const char* text, bool yes_no)
{
	DisplayMessage(text, 3000);
	return true;
}

void DisplayMessage(const std::string &message, int time_in_ms)
{
	CPluginManager::GetInstance().GetVideo()->Video_AddMessage(message.c_str(),
															   time_in_ms);
}

void DisplayMessage(const char *message, int time_in_ms)
{
	CPluginManager::GetInstance().GetVideo()->Video_AddMessage(message, 
															   time_in_ms);
}

void Callback_DebuggerBreak()
{
	CCPU::Break();
}

void *GetWindowHandle()
{
	return g_pWindowHandle;
}

#if defined HAVE_X11 && HAVE_X11
void *GetXWindow()
{
	return g_pXWindow;
}
#endif
	 
bool GetRealWiimote()
{
	return g_bRealWiimote;
}

bool isRunning()
{
	return (GetState() != CORE_UNINITIALIZED) || g_bHwInit;
}

bool IsRunningInCurrentThread()
{
	return isRunning() && ((cpuThread == NULL) || cpuThread->IsCurrentThread());
}

// This is called from the GUI thread. See the booting call schedule in
// BootManager.cpp
bool Init()
{
	if (g_EmuThread != NULL)
	{
		PanicAlert("Emu Thread already running");
		return false;
	}

	// Get a handle to the current instance of the plugin manager
	CPluginManager &pManager = CPluginManager::GetInstance();
	SCoreStartupParameter &_CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;

	Common::InitThreading();

	g_CoreStartupParameter = _CoreParameter;
	// FIXME DEBUG_LOG(BOOT, dump_params());
	Host_SetWaitCursor(true);

	// Load all needed plugins 
	if (!pManager.InitPlugins())
		return false;

	emuThreadGoing.Init();

	// Start the emu thread 
	g_EmuThread = new Common::Thread(EmuThread, NULL);

	// Wait until the emu thread is running
	emuThreadGoing.MsgWait();
	emuThreadGoing.Shutdown();

	Host_SetWaitCursor(false);
	return true;
}

// Called from GUI thread or VI thread (why VI??? That must be bad. Window
// close? TODO: Investigate.)
void Stop()  // - Hammertime!
{
	const SCoreStartupParameter& _CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;
	g_bStopping = true;
	CPluginManager::GetInstance().EmuStateChange(PLUGIN_EMUSTATE_STOP);

	WARN_LOG(CONSOLE, "Stop [Main Thread]\t\t---- Shutting down ----");	

	// This must be done a while before freeing the dll to not crash wx around
	// MSWWindowProc and DefWindowProc, will investigate further
	Host_Message(AUDIO_DESTROY);
	Host_Message(VIDEO_DESTROY);

	Host_SetWaitCursor(true);  // hourglass!
	if (PowerPC::GetState() == PowerPC::CPU_POWERDOWN)
		return;

	// Stop the CPU
	WARN_LOG(CONSOLE, "%s", StopMessage(true, "Stop CPU").c_str());
	PowerPC::Stop();
	CCPU::StepOpcode();  // Kick it if it's waiting (code stepping wait loop)

	if (_CoreParameter.bCPUThread)
	{
		// Video_EnterLoop() should now exit so that EmuThread() will continue
		// concurrently with the rest of the commands in this function. We no
		// longer rely on Postmessage.
		NOTICE_LOG(CONSOLE, "%s", StopMessage(true, "Wait for Video Loop to exit ...").c_str());
		CPluginManager::GetInstance().GetVideo()->Video_ExitLoop();

		// Wait until the CPU finishes exiting the main run loop
		cpuRunloopQuit.Wait();
	}

	// Close the trace file
	Core::StopTrace();

	// Update mouse pointer
	Host_SetWaitCursor(false);

	WARN_LOG(CONSOLE, "%s", StopMessage(true, "Stopping Emu thread ...").c_str());
	g_EmuThread->WaitForDeath();
	delete g_EmuThread;  // Wait for emuthread to close.
	g_EmuThread = 0;
}

// Create the CPU thread. which would be a CPU + Video thread in Single Core mode.

THREAD_RETURN CpuThread(void *pArg)
{
	CPluginManager &Plugins = CPluginManager::GetInstance();
	const SCoreStartupParameter& _CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;

	if (_CoreParameter.bCPUThread)
	{
		Common::SetCurrentThreadName("CPU thread");
	}
	else
	{
		CPluginManager::GetInstance().GetVideo()->Video_Prepare();
		Common::SetCurrentThreadName("CPU-GPU thread");
	}

	if (_CoreParameter.bLockThreads)
		Common::Thread::SetCurrentThreadAffinity(1);  // Force to first core

	if (_CoreParameter.bUseFastMem)
	{
		#ifdef _M_X64
			// Let's run under memory watch
			EMM::InstallExceptionHandler();
		#else
			PanicAlert("32-bit platforms do not support fastmem yet. Report this bug.");
		#endif
	}

	// Enter CPU run loop. When we leave it - we are done.
	CCPU::Run();

	// The shutdown function of OpenGL is not thread safe
	// So we have to call the shutdown from the thread that started it.
	if (!_CoreParameter.bCPUThread)
	{
		Plugins.ShutdownVideoPlugin();
	}

	cpuRunloopQuit.Set();
	return 0;
}


// Initalize plugins and create emulation thread
// Call browser: Init():g_EmuThread(). See the BootManager.cpp file description for a complete call schedule.
THREAD_RETURN EmuThread(void *pArg)
{
	Host_UpdateMainFrame(); // Disable any menus or buttons at boot

	cpuRunloopQuit.Init();

	Common::SetCurrentThreadName("Emuthread - starting");
	const SCoreStartupParameter& _CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;

	CPluginManager &Plugins = CPluginManager::GetInstance();
	if (_CoreParameter.bLockThreads)
		Common::Thread::SetCurrentThreadAffinity(2);  // Force to second core

	INFO_LOG(OSREPORT, "Starting core = %s mode", _CoreParameter.bWii ? "Wii" : "Gamecube");
	INFO_LOG(OSREPORT, "CPU Thread separate = %s", _CoreParameter.bCPUThread ? "Yes" : "No");

	HW::Init();	

	emuThreadGoing.Set();

	// Load the VideoPlugin
	SVideoInitialize VideoInitialize;
	VideoInitialize.pGetMemoryPointer			= Memory::GetPointer;
	VideoInitialize.pSetInterrupt				= ProcessorInterface::SetInterrupt;
	VideoInitialize.pRegisterEvent				= CoreTiming::RegisterEvent;
	VideoInitialize.pScheduleEvent_Threadsafe	= CoreTiming::ScheduleEvent_Threadsafe;
	// This is first the m_Panel handle, then it is updated to have the new window handle
	VideoInitialize.pWindowHandle				= _CoreParameter.hMainWindow;
	VideoInitialize.pLog						= Callback_VideoLog;
	VideoInitialize.pSysMessage					= Host_SysMessage;
	VideoInitialize.pRequestWindowSize			= Callback_VideoRequestWindowSize;
	VideoInitialize.pCopiedToXFB				= Callback_VideoCopiedToXFB;
	VideoInitialize.pPeekMessages				= NULL;
	VideoInitialize.pUpdateFPSDisplay			= NULL;
	VideoInitialize.pMemoryBase					= Memory::base;
	VideoInitialize.pCoreMessage				= Callback_CoreMessage;
	VideoInitialize.bWii						= _CoreParameter.bWii;
	VideoInitialize.bOnThread					= _CoreParameter.bCPUThread;
	VideoInitialize.Fifo_CPUBase				= &ProcessorInterface::Fifo_CPUBase;
	VideoInitialize.Fifo_CPUEnd					= &ProcessorInterface::Fifo_CPUEnd;
	VideoInitialize.Fifo_CPUWritePointer		= &ProcessorInterface::Fifo_CPUWritePointer;
	bool aspectWide = _CoreParameter.bWii;
	if (aspectWide) 
	{
		IniFile gameIni;
		gameIni.Load(_CoreParameter.m_strGameIni.c_str());
		gameIni.Get("Wii", "Widescreen", &aspectWide, !!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.AR"));
	}
	VideoInitialize.bAutoAspectIs16_9			= aspectWide;

	Plugins.GetVideo()->Initialize(&VideoInitialize); // Call the dll

	// Under linux, this is an X11 Display, not a HWND!
	g_pWindowHandle			= (HWND)VideoInitialize.pWindowHandle;
#if defined(HAVE_X11) && HAVE_X11
	g_pXWindow					= (void *)VideoInitialize.pXWindow;
#endif
	Callback_PeekMessages	= VideoInitialize.pPeekMessages;
	g_pUpdateFPSDisplay		= VideoInitialize.pUpdateFPSDisplay;

	// Load and init DSPPlugin	
	DSPInitialize dspInit;
	dspInit.hWnd					= g_pWindowHandle;
	dspInit.pARAM_Read_U8			= (u8  (__cdecl *)(const u32))DSP::ReadARAM; 
	dspInit.pARAM_Write_U8			= (void (__cdecl *)(const u8, const u32))DSP::WriteARAM; 
	dspInit.pGetARAMPointer			= DSP::GetARAMPtr;
	dspInit.pGetMemoryPointer		= Memory::GetPointer;
	dspInit.pLog					= Callback_DSPLog;
	dspInit.pName					= Callback_ISOName;
	dspInit.pDebuggerBreak			= Callback_DebuggerBreak;
	dspInit.pGenerateDSPInterrupt	= Callback_DSPInterrupt;
	dspInit.pGetAudioStreaming		= AudioInterface::Callback_GetStreaming;
	dspInit.pGetSampleRate			= AudioInterface::Callback_GetSampleRate;
	dspInit.pEmulatorState			= (int *)PowerPC::GetStatePtr();
	dspInit.bWii					= _CoreParameter.bWii;
	dspInit.bOnThread				= _CoreParameter.bDSPThread;

	Plugins.GetDSP()->Initialize((void *)&dspInit);
	
	GCPad_Init(g_pWindowHandle);

	// Load and Init WiimotePlugin - only if we are booting in wii mode	
	if (_CoreParameter.bWii)
	{
		SWiimoteInitialize WiimoteInitialize;
		WiimoteInitialize.hWnd			= g_pWindowHandle;
#if defined(HAVE_X11) && HAVE_X11
		WiimoteInitialize.pXWindow	= g_pXWindow;
#endif
		WiimoteInitialize.ISOId			= Ascii2Hex(_CoreParameter.m_strUniqueID);
		WiimoteInitialize.pLog			= Callback_WiimoteLog;
		WiimoteInitialize.pWiimoteInterruptChannel = Callback_WiimoteInterruptChannel;
		WiimoteInitialize.pRendererHasFocus	= Callback_RendererHasFocus;
		// Wait for Wiiuse to find the number of connected Wiimotes
		Plugins.GetWiimote()->Initialize((void *)&WiimoteInitialize);
	}

	// The hardware is initialized.
	g_bHwInit = true;

	DisplayMessage("CPU: " + cpu_info.Summarize(), 8000);
	DisplayMessage(_CoreParameter.m_strFilename, 3000);

	// Load GCM/DOL/ELF whatever ... we boot with the interpreter core
	PowerPC::SetMode(PowerPC::MODE_INTERPRETER);
	CBoot::BootUp();

	if (g_pUpdateFPSDisplay != NULL)
		g_pUpdateFPSDisplay(("Loading " + _CoreParameter.m_strFilename).c_str());
	Host_UpdateTitle(("Loading " + _CoreParameter.m_strFilename).c_str());

	// Setup our core, but can't use dynarec if we are compare server
	if (_CoreParameter.iCPUCore && (!_CoreParameter.bRunCompareServer || _CoreParameter.bRunCompareClient))
		PowerPC::SetMode(PowerPC::MODE_JIT);
	else
		PowerPC::SetMode(PowerPC::MODE_INTERPRETER);

	// Spawn the CPU thread
	_dbg_assert_(HLE, cpuThread == NULL);
	// ENTER THE VIDEO THREAD LOOP
	if (_CoreParameter.bCPUThread)
	{
		// This thread, after creating the EmuWindow, spawns a CPU thread,
		// and then takes over and becomes the video thread

		Plugins.GetVideo()->Video_Prepare(); // wglMakeCurrent
		cpuThread = new Common::Thread(CpuThread, pArg);
		Common::SetCurrentThreadName("Video thread");

		if (g_pUpdateFPSDisplay != NULL)
			g_pUpdateFPSDisplay(("Loaded " + _CoreParameter.m_strFilename).c_str());
		Host_UpdateTitle(("Loaded " + _CoreParameter.m_strFilename).c_str());

		// Update the window again because all stuff is initialized
		Host_UpdateDisasmDialog();
		Host_UpdateMainFrame();

		Plugins.GetVideo()->Video_EnterLoop();
	}
	else // SingleCore mode
	{
		// the spawned CPU Thread also does the graphics.  the EmuThread is
		// thus an idle thread, which sleep wait for the program to terminate.
		// Without this extra thread, the video plugin window hangs in single
		// core mode since noone is pumping messages.

		cpuThread = new Common::Thread(CpuThread, pArg);
		Common::SetCurrentThreadName("Emuthread - Idle");

		if (g_pUpdateFPSDisplay != NULL)
			g_pUpdateFPSDisplay(("Loaded " + _CoreParameter.m_strFilename).c_str());
		Host_UpdateTitle(("Loaded " + _CoreParameter.m_strFilename).c_str());

		// Update the window again because all stuff is initialized
		Host_UpdateDisasmDialog();
		Host_UpdateMainFrame();

		// TODO(ector) : investigate using GetMessage instead .. although
		// then we lose the powerdown check. ... unless powerdown sends a message :P
		while (PowerPC::GetState() != PowerPC::CPU_POWERDOWN)
		{
			if (Callback_PeekMessages)
				Callback_PeekMessages();
			Common::SleepCurrentThread(20);
		}

		// Wait for CpuThread to exit
		NOTICE_LOG(CONSOLE, "%s", StopMessage(true, "Stopping CPU-GPU thread ...").c_str());
		cpuRunloopQuit.Wait();
		NOTICE_LOG(CONSOLE, "%s", StopMessage(true, "CPU thread stopped.").c_str());
		// On unix platforms, the Emulation main thread IS the CPU & video
		// thread So there's only one thread, imho, that's much better than on
		// windows :P
		//CpuThread(pArg);
	}

	// We have now exited the Video Loop
	NOTICE_LOG(CONSOLE, "%s", StopMessage(false, "Stop() and Video Loop Ended").c_str());

	// At this point, the CpuThread has already returned in SC mode.
	// But it may still be waiting in Dual Core mode.
	if (cpuThread)
	{
		// There is a CPU thread - join it.
		cpuThread->WaitForDeath();
		delete cpuThread;
		// Returns after game exited
		cpuThread = NULL;
	}

	VolumeHandler::EjectVolume();
	FileMon::Close();

	// Stop audio thread - Actually this does nothing on HLE plugin.
	// But stops the DSP Interpreter on LLE plugin.
	Plugins.GetDSP()->DSP_StopSoundStream();
	
	// We must set up this flag before executing HW::Shutdown()
	g_bHwInit = false;
	NOTICE_LOG(CONSOLE, "%s", StopMessage(false, "Shutting down HW").c_str());
	HW::Shutdown();
	NOTICE_LOG(CONSOLE, "%s", StopMessage(false, "HW shutdown").c_str());

	WARN_LOG(CONSOLE, "%s", StopMessage(false, "Shutting down plugins").c_str());
	// In single core mode, this has already been called.
	if (_CoreParameter.bCPUThread)
		Plugins.ShutdownVideoPlugin();

	GCPad_Deinit();
	Plugins.ShutdownPlugins();

	NOTICE_LOG(CONSOLE, "%s", StopMessage(false, "Plugins shutdown").c_str());

	NOTICE_LOG(CONSOLE, "%s", StopMessage(true, "Main thread stopped").c_str());
	NOTICE_LOG(CONSOLE, "Stop [Main Thread]\t\t---- Shutdown complete ----");

	cpuRunloopQuit.Shutdown();
	g_bStopping = false;
	return 0;
}

// Set or get the running state

void SetState(EState _State)
{
	switch (_State)
	{
	case CORE_UNINITIALIZED:
		Stop();
		break;
	case CORE_PAUSE:
		CCPU::EnableStepping(true);  // Break
		break;
	case CORE_RUN:
		CCPU::EnableStepping(false);
		break;
	default:
		PanicAlert("Invalid state");
		break;
	}
}

EState GetState()
{
	if (g_bHwInit)
	{
		if (CCPU::IsStepping())
			return CORE_PAUSE;
		else if (g_bStopping)
			return CORE_STOPPING;
		else
			return CORE_RUN;
	}
	return CORE_UNINITIALIZED;
}

static inline std::string GenerateScreenshotName()
{
	int index = 1;
	std::string tempname, name;
	std::string gameId = SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID();
	tempname = std::string(File::GetUserPath(D_SCREENSHOTS_IDX)) + gameId + DIR_SEP_CHR;

	if (!File::CreateFullPath(tempname.c_str())) {
		//fallback to old-style screenshots, without folder.
		tempname = std::string(File::GetUserPath(D_SCREENSHOTS_IDX));
	}
	//append gameId, tempname only contains the folder here.
	tempname += gameId;

	do
		name = StringFromFormat("%s-%d.png", tempname.c_str(), index++);
	while (File::Exists(name.c_str()));

	return name;
}

void ScreenShot(const std::string& name)
{
	bool bPaused = (GetState() == CORE_PAUSE);

	SetState(CORE_PAUSE);
	CPluginManager::GetInstance().GetVideo()->Video_Screenshot(name.c_str());
	if(!bPaused)
		SetState(CORE_RUN);
}

void ScreenShot()
{
	ScreenShot(GenerateScreenshotName());
}

// Apply Frame Limit and Display FPS info
// This should only be called from VI
void VideoThrottle()
{
	u32 TargetVPS = (SConfig::GetInstance().m_Framelimit > 1) ?
		SConfig::GetInstance().m_Framelimit * 5 : VideoInterface::TargetRefreshRate;

	// When frame limit is NOT off
	if (SConfig::GetInstance().m_Framelimit)
	{
		// Make the limiter a bit loose
		u32 frametime = ((SConfig::GetInstance().b_UseFPS)? Common::AtomicLoad(DrawnFrame) : DrawnVideo) * 1000 / TargetVPS;
		while ((u32)Timer.GetTimeDifference() < frametime)
			Common::YieldCPU();
			//Common::SleepCurrentThread(1);
	}

	// Update info per second
	u32 ElapseTime = (u32)Timer.GetTimeDifference();
	if (ElapseTime >= 1000 && DrawnVideo > 0)
	{
		SCoreStartupParameter& _CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;

		u32 FPS = Common::AtomicLoad(DrawnFrame) * 1000 / ElapseTime;
		u32 VPS = DrawnVideo * 1000 / ElapseTime;
		u32 Speed = VPS * 100 / VideoInterface::TargetRefreshRate;
		
		// Settings are shown the same for both extended and summary info
		std::string SSettings = StringFromFormat("%s %s",
		#ifdef _M_IX86
					_CoreParameter.iCPUCore ? jit->GetName() : "Int32", 
		#else
					_CoreParameter.iCPUCore ? jit->GetName() : "Int64", 
		#endif
		_CoreParameter.bCPUThread ? "DC" : "SC");

		// Use extended or summary information. The summary information does not print the ticks data,
		// that's more of a debugging interest, it can always be optional of course if someone is interested.
		//#define EXTENDED_INFO
		#ifdef EXTENDED_INFO
			u64 newTicks = CoreTiming::GetTicks();
			u64 newIdleTicks = CoreTiming::GetIdleTicks();
	 
			u64 diff = (newTicks - ticks) / 1000000;
			u64 idleDiff = (newIdleTicks - idleTicks) / 1000000;
	 
			ticks = newTicks;
			idleTicks = newIdleTicks;	 
			
			float TicksPercentage = (float)diff / (float)(SystemTimers::GetTicksPerSecond() / 1000000) * 100;

			std::string SFPS = StringFromFormat("FPS: %u - VPS: %u - SPEED: %u%%", FPS, VPS, Speed);
			SFPS += StringFromFormat(" | CPU: %s%i MHz [Real: %i + IdleSkip: %i] / %i MHz (%s%3.0f%%)",
					_CoreParameter.bSkipIdle ? "~" : "",
					(int)(diff),
					(int)(diff - idleDiff),
					(int)(idleDiff),
					SystemTimers::GetTicksPerSecond() / 1000000,
					_CoreParameter.bSkipIdle ? "~" : "",
					TicksPercentage);

		#else	// Summary information
		std::string SFPS = StringFromFormat("FPS: %u - VPS: %u - SPEED: %u%%", FPS, VPS, Speed);
		#endif

		// This is our final "frame counter" string
		std::string SMessage = StringFromFormat("%s | %s", SSettings.c_str(), SFPS.c_str());

		// Show message
		if (g_pUpdateFPSDisplay != NULL)
			g_pUpdateFPSDisplay(SMessage.c_str()); 
		Host_UpdateTitle(SMessage.c_str());

		Host_UpdateStatusBar(SMessage.c_str());

		// Reset counter
		Timer.Update();
		Common::AtomicStore(DrawnFrame, 0);
		DrawnVideo = 0;
	}

	DrawnVideo++;
}

// Executed from GPU thread
// reports if a frame should be skipped or not
// depending on the framelimit set
bool report_slow(int skipped)
{
	u32 TargetFPS = (SConfig::GetInstance().m_Framelimit > 1) ? SConfig::GetInstance().m_Framelimit * 5
		: VideoInterface::TargetRefreshRate;
	u32 frames = Common::AtomicLoad(DrawnFrame);
	bool fps_slow = (Timer.GetTimeDifference() < (frames + skipped) * 1000 / TargetFPS) ? false : true;

	return fps_slow;
}

// --- Callbacks for plugins / engine ---

// Callback_VideoLog
// WARNING - THIS IS EXECUTED FROM VIDEO THREAD
void Callback_VideoLog(const TCHAR *_szMessage, int _bDoBreak)
{
	INFO_LOG(VIDEO, _szMessage);
}

// Should be called from GPU thread when a frame is drawn
void Callback_VideoCopiedToXFB(bool video_update)
{
	if(video_update)
		Common::AtomicIncrement(DrawnFrame);
	Frame::FrameUpdate();
}

// Ask the host for the desired window size
void Callback_VideoRequestWindowSize(int& x, int& y, int& width, int& height)
{
	Host_RequestWindowSize(x, y, width, height);
}

// Callback_DSPLog
// WARNING - THIS MAY BE EXECUTED FROM DSP THREAD
	void Callback_DSPLog(const TCHAR* _szMessage, int _v)
{
	GENERIC_LOG(LogTypes::AUDIO, (LogTypes::LOG_LEVELS)_v, _szMessage);
}


// Callback_DSPInterrupt
// WARNING - THIS MAY BE EXECUTED FROM DSP THREAD
void Callback_DSPInterrupt()
{
	DSP::GenerateDSPInterruptFromPlugin(DSP::INT_DSP);
}


// Callback_PADLog 
//
void Callback_PADLog(const TCHAR* _szMessage)
{
	// FIXME add levels
	INFO_LOG(SERIALINTERFACE, _szMessage);
}


// Callback_ISOName: Let the DSP plugin get the game name
//
const char *Callback_ISOName()
{
	SCoreStartupParameter& params = SConfig::GetInstance().m_LocalCoreStartupParameter;
	if (params.m_strName.length() > 0)
		return params.m_strName.c_str();
	else	
		return "";
}

// Called from ANY thread!
// Pass the message on to the host
void Callback_CoreMessage(int Id)
{
	Host_Message(Id);
}

// Callback_WiimoteLog
//
void Callback_WiimoteLog(const TCHAR* _szMessage, int _v)
{
	GENERIC_LOG(LogTypes::WIIMOTE, (LogTypes::LOG_LEVELS)_v, _szMessage);
}

// Check to see if the renderer window has focus
bool Callback_RendererHasFocus(void)
{
	return Host_RendererHasFocus();
}

} // Core
