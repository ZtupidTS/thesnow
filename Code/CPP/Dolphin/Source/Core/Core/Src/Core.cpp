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
#include "CommonPaths.h"
#include "StringUtil.h"
#include "MathUtil.h"
#include "MemoryUtil.h"

#include "Console.h"
#include "Core.h"
#include "CPUDetect.h"
#include "CoreTiming.h"
#include "Boot/Boot.h"
#include "FifoPlayer/FifoPlayer.h"

#include "HW/Memmap.h"
#include "HW/ProcessorInterface.h"
#include "HW/GPFifo.h"
#include "HW/CPU.h"
#include "HW/GCPad.h"
#include "HW/Wiimote.h"
#include "HW/HW.h"
#include "HW/DSP.h"
#include "HW/GPFifo.h"
#include "HW/AudioInterface.h"
#include "HW/VideoInterface.h"
#include "HW/SystemTimers.h"

#include "IPC_HLE/WII_IPC_HLE_Device_usb.h"

#include "PowerPC/PowerPC.h"
#include "PowerPC/JitCommon/JitBase.h"

#include "DSPEmulator.h"
#include "ConfigManager.h"
#include "VideoBackendBase.h"
#include "OnScreenDisplay.h"
#ifdef _WIN32
#include "EmuWindow.h"
#endif

#include "VolumeHandler.h"
#include "FileMonitor.h"

#include "MemTools.h"
#include "Host.h"
#include "LogManager.h"

#include "State.h"
#include "OnFrame.h"

// TODO: ugly, remove
bool g_aspect_wide;

namespace Core
{

// Declarations and definitions
Common::Timer Timer;
volatile u32 DrawnFrame = 0;
u32 DrawnVideo = 0;

// Function forwarding
const char *Callback_ISOName(void);
void Callback_WiimoteInterruptChannel(int _number, u16 _channelID, const void* _pData, u32 _Size);

// Function declarations
void EmuThread();

void Stop();

bool g_bStopping = false;
bool g_bHwInit = false;
bool g_bRealWiimote = false;
void *g_pWindowHandle = NULL;
std::string g_stateFileName;
std::thread g_EmuThread;

static std::thread g_cpu_thread;

SCoreStartupParameter g_CoreStartupParameter;

std::string GetStateFileName() { return g_stateFileName; }
void SetStateFileName(std::string val) { g_stateFileName = val; }

// Display messages and return values

// Formatted stop message
std::string StopMessage(bool bMainThread, std::string Message)
{
	return StringFromFormat("Stop [%s %i]\t%s\t%s",
		bMainThread ? "Main Thread" : "Video Thread", Common::CurrentThreadId(), MemUsage().c_str(), Message.c_str());
}

// 
bool PanicAlertToVideo(const char* text, bool yes_no)
{
	DisplayMessage(text, 3000);
	return true;
}

void DisplayMessage(const char *message, int time_in_ms)
{
	SCoreStartupParameter& _CoreParameter =
		SConfig::GetInstance().m_LocalCoreStartupParameter;

	g_video_backend->Video_AddMessage(message, time_in_ms);
	if (_CoreParameter.bRenderToMain &&
		SConfig::GetInstance().m_InterfaceStatusbar) {
			Host_UpdateStatusBar(message);
	} else
		Host_UpdateTitle(message);
}

void Callback_DebuggerBreak()
{
	CCPU::Break();
}

void *GetWindowHandle()
{
	return g_pWindowHandle;
}

bool IsRunning()
{
	return (GetState() != CORE_UNINITIALIZED) || g_bHwInit;
}

bool IsRunningInCurrentThread()
{
	return IsRunning() && ((!g_cpu_thread.joinable()) || g_cpu_thread.get_id() == std::this_thread::get_id());
}

bool IsCPUThread()
{
	return ((!g_cpu_thread.joinable()) || g_cpu_thread.get_id() == std::this_thread::get_id());
}

// This is called from the GUI thread. See the booting call schedule in
// BootManager.cpp
bool Init()
{
	const SCoreStartupParameter& _CoreParameter =
		SConfig::GetInstance().m_LocalCoreStartupParameter;

	if (g_EmuThread.joinable())
	{
		PanicAlertT("Emu Thread already running");
		return false;
	}

	g_CoreStartupParameter = _CoreParameter;

	INFO_LOG(OSREPORT, "Starting core = %s mode",
		g_CoreStartupParameter.bWii ? "Wii" : "Gamecube");
	INFO_LOG(OSREPORT, "CPU Thread separate = %s",
		g_CoreStartupParameter.bCPUThread ? "Yes" : "No");

	Host_UpdateMainFrame(); // Disable any menus or buttons at boot

	g_aspect_wide = _CoreParameter.bWii;
	if (g_aspect_wide) 
	{
		IniFile gameIni;
		gameIni.Load(_CoreParameter.m_strGameIni.c_str());
		gameIni.Get("Wii", "Widescreen", &g_aspect_wide,
			!!SConfig::GetInstance().m_SYSCONF->
				GetData<u8>("IPL.AR"));
	}

	// g_pWindowHandle is first the m_Panel handle,
	// then it is updated to the render window handle,
	// within g_video_backend->Initialize()
	g_pWindowHandle = Host_GetRenderHandle();

	// Start the emu thread 
	g_EmuThread = std::thread(EmuThread);

	return true;
}

// Called from GUI thread
void Stop()  // - Hammertime!
{
	if (PowerPC::GetState() == PowerPC::CPU_POWERDOWN)
		return;

	const SCoreStartupParameter& _CoreParameter =
		SConfig::GetInstance().m_LocalCoreStartupParameter;

	g_bStopping = true;

	g_video_backend->EmuStateChange(EMUSTATE_CHANGE_STOP);

	INFO_LOG(CONSOLE, "Stop [Main Thread]\t\t---- Shutting down ----");

	// Stop the CPU
	INFO_LOG(CONSOLE, "%s", StopMessage(true, "Stop CPU").c_str());
	PowerPC::Stop();

	// Kick it if it's waiting (code stepping wait loop)
	CCPU::StepOpcode();

	if (_CoreParameter.bCPUThread)
	{
		// Video_EnterLoop() should now exit so that EmuThread()
		// will continue concurrently with the rest of the commands
		// in this function. We no longer rely on Postmessage.
		INFO_LOG(CONSOLE, "%s", StopMessage(true, "Wait for Video Loop to exit ...").c_str());
		
		g_video_backend->Video_ExitLoop();
	}

	INFO_LOG(CONSOLE, "%s", StopMessage(true, "Stopping Emu thread ...").c_str());
	
	g_EmuThread.join();	// Wait for emuthread to close.

	INFO_LOG(CONSOLE, "%s", StopMessage(true, "Main Emu thread stopped").c_str());

#ifdef _WIN32
	EmuWindow::Close();
#endif

	// Clear on screen messages that haven't expired
	g_video_backend->Video_ClearMessages();

	// Close the trace file
	Core::StopTrace();
	
	// Reload sysconf file in order to see changes committed during emulation
	if (_CoreParameter.bWii)
		SConfig::GetInstance().m_SYSCONF->Reload();

	INFO_LOG(CONSOLE, "Stop [Main Thread]\t\t---- Shutdown complete ----");
	Frame::g_InputCounter = 0;
	g_bStopping = false;
}

// Create the CPU thread, which is a CPU + Video thread in Single Core mode.
void CpuThread()
{
	const SCoreStartupParameter& _CoreParameter =
		SConfig::GetInstance().m_LocalCoreStartupParameter;

	if (_CoreParameter.bCPUThread)
	{
		Common::SetCurrentThreadName("CPU thread");
	}
	else
	{
		Common::SetCurrentThreadName("CPU-GPU thread");
		g_video_backend->Video_Prepare();
	}

	if (_CoreParameter.bLockThreads)
		Common::SetCurrentThreadAffinity(1);  // Force to first core

	if (_CoreParameter.bUseFastMem)
		EMM::InstallExceptionHandler(); // Let's run under memory watch

	if (!g_stateFileName.empty())
		State::LoadAs(g_stateFileName);

	// Enter CPU run loop. When we leave it - we are done.
	CCPU::Run();

	return;
}

void FifoPlayerThread()
{
	const SCoreStartupParameter& _CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;

	if (_CoreParameter.bCPUThread)
	{
		Common::SetCurrentThreadName("FIFO player thread");
	}
	else
	{
		g_video_backend->Video_Prepare();
		Common::SetCurrentThreadName("FIFO-GPU thread");
	}

	if (_CoreParameter.bLockThreads)
		Common::SetCurrentThreadAffinity(1);  // Force to first core

	// Enter CPU run loop. When we leave it - we are done.
	if (FifoPlayer::GetInstance().Open(_CoreParameter.m_strFilename))
	{
		FifoPlayer::GetInstance().Play();
		FifoPlayer::GetInstance().Close();
	}

	return;
}

// Initalize and create emulation thread
// Call browser: Init():g_EmuThread().
// See the BootManager.cpp file description for a complete call schedule.
void EmuThread()
{
	const SCoreStartupParameter& _CoreParameter =
		SConfig::GetInstance().m_LocalCoreStartupParameter;

	Common::SetCurrentThreadName("Emuthread - Starting");

	if (_CoreParameter.bLockThreads)
	{
		if (cpu_info.num_cores > 3)	// Force to third, non-HT core
			Common::SetCurrentThreadAffinity(4);
		else				// Force to second core
			Common::SetCurrentThreadAffinity(2);
	}

	DisplayMessage(cpu_info.brand_string, 8000);
	DisplayMessage(cpu_info.Summarize(), 8000);
	DisplayMessage(_CoreParameter.m_strFilename, 3000);

	HW::Init();	

	if (!g_video_backend->Initialize(g_pWindowHandle))
	{
		PanicAlert("Failed to initialize video backend!");
		return;
	}

	OSD::AddMessage(("Dolphin " + g_video_backend->GetName() + " Video Backend.").c_str(), 5000);

	if (!DSP::GetDSPEmulator()->Initialize(g_pWindowHandle,
				_CoreParameter.bWii, _CoreParameter.bDSPThread))
	{
		HW::Shutdown();
		g_video_backend->Shutdown();
		PanicAlert("Failed to initialize DSP emulator!");
		return;
	}

	Pad::Initialize(g_pWindowHandle);
	// Load and Init Wiimotes - only if we are booting in wii mode	
	if (g_CoreStartupParameter.bWii)
	{
		Wiimote::Initialize(g_pWindowHandle);

		// Activate wiimotes which don't have source set to "None"
		for (unsigned int i = 0; i != MAX_WIIMOTES; ++i)
			if (g_wiimote_sources[i])
				GetUsbPointer()->AccessWiiMote(i | 0x100)->
					Activate(true);
	}

	// The hardware is initialized.
	g_bHwInit = true;

	// Boot to pause or not
	Core::SetState(_CoreParameter.bBootToPause ? Core::CORE_PAUSE : Core::CORE_RUN);

	// Load GCM/DOL/ELF whatever ... we boot with the interpreter core
	PowerPC::SetMode(PowerPC::MODE_INTERPRETER);

	CBoot::BootUp();

	// Setup our core, but can't use dynarec if we are compare server
	if (_CoreParameter.iCPUCore && (!_CoreParameter.bRunCompareServer ||
					_CoreParameter.bRunCompareClient))
		PowerPC::SetMode(PowerPC::MODE_JIT);
	else
		PowerPC::SetMode(PowerPC::MODE_INTERPRETER);

	// Update the window again because all stuff is initialized
	Host_UpdateDisasmDialog();
	Host_UpdateMainFrame();

	// Determine the cpu thread function
	void (*cpuThreadFunc)(void);
	if (_CoreParameter.m_BootType == SCoreStartupParameter::BOOT_DFF)
		cpuThreadFunc = FifoPlayerThread;
	else
		cpuThreadFunc = CpuThread;

	// ENTER THE VIDEO THREAD LOOP
	if (_CoreParameter.bCPUThread)
	{
		// This thread, after creating the EmuWindow, spawns a CPU
		// thread, and then takes over and becomes the video thread
		Common::SetCurrentThreadName("Video thread");

		g_video_backend->Video_Prepare();

		// Spawn the CPU thread
		g_cpu_thread = std::thread(cpuThreadFunc);

		// become the GPU thread
		g_video_backend->Video_EnterLoop();

		// We have now exited the Video Loop
		INFO_LOG(CONSOLE, "%s", StopMessage(false, "Video Loop Ended").c_str());
	}
	else // SingleCore mode
	{
		// The spawned CPU Thread also does the graphics.
		// The EmuThread is thus an idle thread, which sleeps while
		// waiting for the program to terminate. Without this extra
		// thread, the video backend window hangs in single core mode
		// because noone is pumping messages.
		Common::SetCurrentThreadName("Emuthread - Idle");

		// Spawn the CPU+GPU thread
		g_cpu_thread = std::thread(cpuThreadFunc);

		while (PowerPC::GetState() != PowerPC::CPU_POWERDOWN)
		{
			g_video_backend->PeekMessages();
			Common::SleepCurrentThread(20);
		}
	}

	// Wait for g_cpu_thread to exit
	INFO_LOG(CONSOLE, "%s", StopMessage(true, "Stopping CPU-GPU thread ...").c_str());

	g_cpu_thread.join();

	INFO_LOG(CONSOLE, "%s", StopMessage(true, "CPU thread stopped.").c_str());

	VolumeHandler::EjectVolume();
	FileMon::Close();

	// Stop audio thread - Actually this does nothing when using HLE
	// emulation, but stops the DSP Interpreter when using LLE emulation.
	DSP::GetDSPEmulator()->DSP_StopSoundStream();
	
	// We must set up this flag before executing HW::Shutdown()
	g_bHwInit = false;
	INFO_LOG(CONSOLE, "%s", StopMessage(false, "Shutting down HW").c_str());
	HW::Shutdown();
	INFO_LOG(CONSOLE, "%s", StopMessage(false, "HW shutdown").c_str());
	Pad::Shutdown();
	Wiimote::Shutdown();
	g_video_backend->Shutdown();
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
		PanicAlertT("Invalid state");
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

static std::string GenerateScreenshotName()
{
	const std::string& gameId = SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID();
	std::string path = File::GetUserPath(D_SCREENSHOTS_IDX) + gameId + DIR_SEP_CHR;

	if (!File::CreateFullPath(path))
	{
		// fallback to old-style screenshots, without folder.
		path = File::GetUserPath(D_SCREENSHOTS_IDX);
	}

	//append gameId, path only contains the folder here.
	path += gameId;

	std::string name;
	for (int i = 1; File::Exists(name = StringFromFormat("%s-%d.png", path.c_str(), i)); ++i)
	{}

	return name;
}

void SaveScreenShot()
{
	const bool bPaused = (GetState() == CORE_PAUSE);

	SetState(CORE_PAUSE);

	g_video_backend->Video_Screenshot(GenerateScreenshotName().c_str());
	
	if (!bPaused)
		SetState(CORE_RUN);
}

// Apply Frame Limit and Display FPS info
// This should only be called from VI
void VideoThrottle()
{
	u32 TargetVPS = (SConfig::GetInstance().m_Framelimit > 1) ?
		SConfig::GetInstance().m_Framelimit * 5 : VideoInterface::TargetRefreshRate;

	// Disable the frame-limiter when the throttle (Tab) key is held down
	if (SConfig::GetInstance().m_Framelimit && !Host_GetKeyState('\t'))
	{
		u32 frametime = ((SConfig::GetInstance().b_UseFPS)? Common::AtomicLoad(DrawnFrame) : DrawnVideo) * 1000 / TargetVPS;

		u32 timeDifference = (u32)Timer.GetTimeDifference();
		if (timeDifference < frametime) {
			Common::SleepCurrentThread(frametime - timeDifference - 1);
		}

		while ((u32)Timer.GetTimeDifference() < frametime)
			Common::YieldCPU();
			//Common::SleepCurrentThread(1);
	}

	// Update info per second
	u32 ElapseTime = (u32)Timer.GetTimeDifference();
	if ((ElapseTime >= 1000 && DrawnVideo > 0) || Frame::g_bFrameStep)
	{
		SCoreStartupParameter& _CoreParameter = SConfig::GetInstance().m_LocalCoreStartupParameter;

		u32 FPS = Common::AtomicLoad(DrawnFrame) * 1000 / ElapseTime;
		u32 VPS = DrawnVideo * 1000 / ElapseTime;
		u32 Speed = VPS * 100 / VideoInterface::TargetRefreshRate;
		
		// Settings are shown the same for both extended and summary info
		std::string SSettings = StringFromFormat("%s %s", cpu_core_base->GetName(),	_CoreParameter.bCPUThread ? "DC" : "SC");

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
		std::string SFPS;
		if (Frame::g_recordfd)
			SFPS = StringFromFormat("VI: %u - Frame: %u - FPS: %u - VPS: %u - SPEED: %u%%", Frame::g_frameCounter, Frame::g_InputCounter, FPS, VPS, Speed);
		else
			SFPS = StringFromFormat("FPS: %u - VPS: %u - SPEED: %u%%", FPS, VPS, Speed);
		#endif

		// This is our final "frame counter" string
		std::string SMessage = StringFromFormat("%s | %s",
			SSettings.c_str(), SFPS.c_str());
		std::string TMessage = StringFromFormat("%s | ", svn_rev_str) +
			SMessage;

		// Show message
		g_video_backend->UpdateFPSDisplay(SMessage.c_str()); 

		if (_CoreParameter.bRenderToMain &&
			SConfig::GetInstance().m_InterfaceStatusbar) {
			Host_UpdateStatusBar(SMessage.c_str());
			Host_UpdateTitle(svn_rev_str);
		} else
			Host_UpdateTitle(TMessage.c_str());
		

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
bool ShouldSkipFrame(int skipped)
{
	const u32 TargetFPS = (SConfig::GetInstance().m_Framelimit > 1)
		? SConfig::GetInstance().m_Framelimit * 5
		: VideoInterface::TargetRefreshRate;
	const u32 frames = Common::AtomicLoad(DrawnFrame);
	const bool fps_slow = !(Timer.GetTimeDifference() < (frames + skipped) * 1000 / TargetFPS);

	return fps_slow;
}

// --- Callbacks for backends / engine ---

// Should be called from GPU thread when a frame is drawn
void Callback_VideoCopiedToXFB(bool video_update)
{
	if(video_update)
		Common::AtomicIncrement(DrawnFrame);
	Frame::FrameUpdate();
}

// Callback_ISOName: Let the DSP emulator get the game name
//
const char *Callback_ISOName()
{
	SCoreStartupParameter& params =
		SConfig::GetInstance().m_LocalCoreStartupParameter;
	if (params.m_strName.length() > 0)
		return params.m_strName.c_str();
	else	
		return "";
}

} // Core
