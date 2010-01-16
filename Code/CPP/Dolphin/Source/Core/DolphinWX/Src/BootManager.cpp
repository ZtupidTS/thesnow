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




// =======================================================
// File description
// -------------
/* Purpose of this file: Collect boot settings for Core::Init()

   Call sequence: This file has one of the first function called when a game is booted,
   the boot sequence in the code is:
   
	DolphinWX:	GameListCtrl.cpp	OnActivated
				BootManager.cpp		BootCore
	Core		Core.cpp			Init		Thread creation
									EmuThread	Calls CBoot::BootUp
				Boot.cpp			CBoot::BootUp()
									CBoot::EmulatedBS2_Wii() / GC() or Load_BS2()
 */
// =============





// Includes
// ----------------
#include <string>
#include <vector>

#include "Globals.h"
#include "Common.h"
#include "IniFile.h"
#include "BootManager.h"
#include "ISOFile.h"
#include "Volume.h"
#include "VolumeCreator.h"
#include "ConfigManager.h"
#include "SysConf.h"
#include "Core.h"
#if defined(HAVE_WX) && HAVE_WX
	#include "ConfigMain.h"
	#include "Frame.h"
	#include "CodeWindow.h"
	#include "Setup.h"
#endif


#if defined(HAVE_WX) && HAVE_WX
extern CFrame* main_frame;
#endif

namespace BootManager
{
#ifdef _WIN32
	extern "C" HINSTANCE wxGetInstance();
#endif

// Boot the ISO or file
bool BootCore(const std::string& _rFilename)
{
	SCoreStartupParameter& StartUp = SConfig::GetInstance().m_LocalCoreStartupParameter;

	// Use custom settings for debugging mode
	#if defined(HAVE_WX) && HAVE_WX		
		if (main_frame->g_pCodeWindow)
		{
			//StartUp.bCPUThread = code_frame->UseDualCore();
			StartUp.bUseJIT = !main_frame->g_pCodeWindow->UseInterpreter();
			StartUp.bBootToPause = main_frame->g_pCodeWindow->BootToPause();
			StartUp.bAutomaticStart = main_frame->g_pCodeWindow->AutomaticStart();
			StartUp.bJITUnlimitedCache = main_frame->g_pCodeWindow->UnlimitedJITCache();
			StartUp.bJITBlockLinking = main_frame->g_pCodeWindow->JITBlockLinking();
		}
		else
		{
			//StartUp.bCPUThread = false;
			//StartUp.bUseJIT = true;
		}		
		StartUp.bEnableDebugging = main_frame->g_pCodeWindow ? true : false; // RUNNING_DEBUG
	#endif 

	StartUp.m_BootType = SCoreStartupParameter::BOOT_ISO;
	StartUp.m_strFilename = _rFilename;
	SConfig::GetInstance().m_LastFilename = _rFilename;
	StartUp.bRunCompareClient = false;
	StartUp.bRunCompareServer = false;

	#ifdef _WIN32
		StartUp.hInstance = wxGetInstance();
		#ifdef _M_X64
			StartUp.bUseFastMem = true;
		#endif
	#endif

	// If for example the ISO file is bad we return here
	if (!StartUp.AutoSetup(SCoreStartupParameter::BOOT_DEFAULT)) return false;

	// ====================================================
	// Load game specific settings
	IniFile game_ini;
	std::string unique_id = StartUp.GetUniqueID();
	StartUp.m_strGameIni = FULL_GAMECONFIG_DIR + unique_id + ".ini";
	if (unique_id.size() == 6 && game_ini.Load(StartUp.m_strGameIni.c_str()))
	{
		// General settings
		game_ini.Get("Core", "CPUOnThread",			&StartUp.bCPUThread, StartUp.bCPUThread);
		game_ini.Get("Core", "SkipIdle",			&StartUp.bSkipIdle, StartUp.bSkipIdle);
		game_ini.Get("Core", "OptimizeQuantizers",	&StartUp.bOptimizeQuantizers, StartUp.bOptimizeQuantizers);
		game_ini.Get("Core", "EnableFPRF",			&StartUp.bEnableFPRF, StartUp.bEnableFPRF);
		game_ini.Get("Core", "TLBHack",				&StartUp.iTLBHack, StartUp.iTLBHack);
		// Wii settings
		if (StartUp.bWii)
		{
			// Flush possible changes to SYSCONF to file
			SConfig::GetInstance().m_SYSCONF->Save();
		}
	} 

	// Run the game
	// --------------
#if defined(HAVE_WX) && HAVE_WX
	if(main_frame)
	{
		// Save the window handle of the eventual parent to the rendering window
		StartUp.hMainWindow = main_frame->GetRenderHandle();

		// Now that we know if we have a Wii game we can run this
		main_frame->ModifyStatusBar();
	}
#endif
	// Init the core
	if (!Core::Init())
	{
		PanicAlert("Couldn't init the core.\nCheck your configuration.");
		return false;
	}

#if defined(HAVE_WX) && HAVE_WX
	// Boot to pause or not
	Core::SetState((main_frame->g_pCodeWindow && StartUp.bBootToPause) ? Core::CORE_PAUSE : Core::CORE_RUN);
#else
	Core::SetState(Core::CORE_RUN);
#endif
	// =====================	

	return true;
}

void Stop()
{
	Core::Stop();
}



} // namespace
