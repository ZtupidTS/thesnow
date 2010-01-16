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

#include "Common.h"
#include "Thunk.h"
#include "../Core.h"
#include "HW.h"
#include "../PowerPC/PowerPC.h"
#include "CPU.h"
#include "DSP.h"
#include "DVDInterface.h"
#include "EXI.h"
#include "GPFifo.h"
#include "Memmap.h"
#include "ProcessorInterface.h"
#include "SI.h"
#include "AudioInterface.h"
#include "VideoInterface.h"
#include "WII_IPC.h"
#include "../PluginManager.h"
#include "../ConfigManager.h"
#include "../CoreTiming.h"
#include "SystemTimers.h"
#include "../IPC_HLE/WII_IPC_HLE.h"
#include "../State.h"
#include "../PowerPC/PPCAnalyst.h"

namespace HW
{
	void Init()
	{
		CoreTiming::Init();

		thunks.Init(); // not really hw, but this way we know it's inited early :P
		State_Init();

		// Init the whole Hardware
		AudioInterface::Init();
		VideoInterface::Init();
		SerialInterface::Init();
		ProcessorInterface::Init();
		Memory::Init();
		DSP::Init();
		DVDInterface::Init();
		GPFifo::Init();
		ExpansionInterface::Init();
		CCPU::Init();
		SystemTimers::Init();
		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bWii)
		{
			WII_IPCInterface::Init();
			WII_IPC_HLE_Interface::Init();
		}
	}

	void Shutdown()
	{
		SystemTimers::Shutdown();
		CCPU::Shutdown();
		ExpansionInterface::Shutdown();		
		DVDInterface::Shutdown();
		DSP::Shutdown();
		Memory::Shutdown();
		SerialInterface::Shutdown();
		AudioInterface::Shutdown();

		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bWii)
		{
			WII_IPCInterface::Shutdown();
			WII_IPC_HLE_Interface::Shutdown();
		}
		
		State_Shutdown();
		thunks.Shutdown();
		CoreTiming::Shutdown();
	}

	void DoState(PointerWrap &p)
	{
		Memory::DoState(p);
		VideoInterface::DoState(p);
		SerialInterface::DoState(p);
		ProcessorInterface::DoState(p);
		DSP::DoState(p);
		DVDInterface::DoState(p);
		GPFifo::DoState(p);
		ExpansionInterface::DoState(p);
		AudioInterface::DoState(p);
		if (SConfig::GetInstance().m_LocalCoreStartupParameter.bWii)
		{
			WII_IPCInterface::DoState(p);
			WII_IPC_HLE_Interface::DoState(p);
		}
	}
}
