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

#ifndef _COMMANDPROCESSOR_H
#define _COMMANDPROCESSOR_H

#include "Common.h"
#include "pluginspecs_video.h"

class PointerWrap;

extern bool MT;

namespace CommandProcessor
{

extern SCPFifoStruct fifo; //This one is shared between gfx thread and emulator thread

// internal hardware addresses
enum
{
	STATUS_REGISTER				= 0x00,
	CTRL_REGISTER				= 0x02,
	CLEAR_REGISTER				= 0x04,
	PERF_SELECT					= 0x06,
	FIFO_TOKEN_REGISTER			= 0x0E,
	FIFO_BOUNDING_BOX_LEFT		= 0x10,
	FIFO_BOUNDING_BOX_RIGHT		= 0x12,
	FIFO_BOUNDING_BOX_TOP		= 0x14,
	FIFO_BOUNDING_BOX_BOTTOM	= 0x16,
	FIFO_BASE_LO				= 0x20,
	FIFO_BASE_HI				= 0x22,
	FIFO_END_LO					= 0x24,
	FIFO_END_HI					= 0x26,
	FIFO_HI_WATERMARK_LO		= 0x28,
	FIFO_HI_WATERMARK_HI		= 0x2a,
	FIFO_LO_WATERMARK_LO		= 0x2c,
	FIFO_LO_WATERMARK_HI		= 0x2e,
	FIFO_RW_DISTANCE_LO			= 0x30,
	FIFO_RW_DISTANCE_HI			= 0x32,
	FIFO_WRITE_POINTER_LO		= 0x34,
	FIFO_WRITE_POINTER_HI		= 0x36,
	FIFO_READ_POINTER_LO		= 0x38,
	FIFO_READ_POINTER_HI		= 0x3A,
	FIFO_BP_LO					= 0x3C,
	FIFO_BP_HI					= 0x3E,
	XF_RASBUSY_L				= 0x40,
	XF_RASBUSY_H				= 0x42,
	XF_CLKS_L					= 0x44,
	XF_CLKS_H					= 0x46,
	XF_WAIT_IN_L				= 0x48,
	XF_WAIT_IN_H				= 0x4a,
	XF_WAIT_OUT_L				= 0x4c,
	XF_WAIT_OUT_H				= 0x4e,
	VCACHE_METRIC_CHECK_L		= 0x50,
	VCACHE_METRIC_CHECK_H		= 0x52,
	VCACHE_METRIC_MISS_L		= 0x54,
	VCACHE_METRIC_MISS_H		= 0x56,
	VCACHE_METRIC_STALL_L		= 0x58,
	VCACHE_METRIC_STALL_H		= 0x5A,
	CLKS_PER_VTX_IN_L			= 0x60,
	CLKS_PER_VTX_IN_H			= 0x62,
	CLKS_PER_VTX_OUT			= 0x64,
};

enum
{
	GATHER_PIPE_SIZE = 32,
    INT_CAUSE_CP =  0x800
};

// Fifo Status Register
union UCPStatusReg
{
	struct
	{
		unsigned OverflowHiWatermark	:	1;
		unsigned UnderflowLoWatermark	:	1;
		unsigned ReadIdle				:	1;
		unsigned CommandIdle			:	1;
		unsigned Breakpoint				:	1;
		unsigned						:	11;
	};
	u16 Hex;
	UCPStatusReg() {Hex = 0; }
	UCPStatusReg(u16 _hex) {Hex = _hex; }
};

// Fifo Control Register
union UCPCtrlReg
{
	struct
	{
		unsigned GPReadEnable			:	1;
		unsigned BPEnable				:	1;
		unsigned FifoOverflowIntEnable	:	1;
		unsigned FifoUnderflowIntEnable	:	1;
		unsigned GPLinkEnable			:	1;
		unsigned BPInit					:	1;
		unsigned						:	10;
	};
	u16 Hex;
	UCPCtrlReg() {Hex = 0; }
	UCPCtrlReg(u16 _hex) {Hex = _hex; }
};

// Fifo Clear Register
union UCPClearReg
{
	struct
	{
		unsigned ClearFifoOverflow		:	1;
		unsigned ClearFifoUnderflow		:	1;
		unsigned ClearMetrices			:	1;
		unsigned						:	13;
	};
	u16 Hex;
	UCPClearReg() {Hex = 0; }
	UCPClearReg(u16 _hex) {Hex = _hex; }
};

// Init
void Init();
void Shutdown();
void DoState(PointerWrap &p);

// Read
void Read16(u16& _rReturnValue, const u32 _Address);
void Write16(const u16 _Data, const u32 _Address);
void Read32(u32& _rReturnValue, const u32 _Address);
void Write32(const u32 _Data, const u32 _Address);

// for CGPFIFO
void CatchUpGPU();
void GatherPipeBursted();
void UpdateFifoRegister();
void UpdateInterrupts(bool active);
void UpdateInterruptsFromVideoPlugin(bool active);
void SetFifoIdleFromVideoPlugin();

bool AllowIdleSkipping();

// for DC GP watchdog hack
void IncrementGPWDToken();
void WaitForFrameFinish();

void FifoCriticalEnter();
void FifoCriticalLeave();

} // namespace CommandProcessor

#endif // _COMMANDPROCESSOR_H

