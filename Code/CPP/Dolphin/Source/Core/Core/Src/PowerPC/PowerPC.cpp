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

#include <float.h>

#include "Common.h"
#include "Atomic.h"
#include "MathUtil.h"
#include "ChunkFile.h"

#include "../HW/Memmap.h"
#include "../HW/CPU.h"
#include "../Core.h"
#include "../CoreTiming.h"
#include "../HW/SystemTimers.h"

#include "Interpreter/Interpreter.h"
#include "JitCommon/JitBase.h"
#include "Jit64IL/JitIL.h"
#include "Jit64/Jit.h"
#include "PowerPC.h"
#include "PPCTables.h"
#include "CPUCoreBase.h"

#include "../Host.h"

CPUCoreBase *cpu_core_base;

namespace PowerPC
{

// STATE_TO_SAVE
PowerPCState GC_ALIGNED16(ppcState);
volatile CPUState state = CPU_STEPPING;

Interpreter * const interpreter = Interpreter::getInstance();
CoreMode mode;

BreakPoints breakpoints;
MemChecks memchecks;
PPCDebugInterface debug_interface;

void CompactCR()
{
	u32 new_cr = ppcState.cr_fast[0] << 28;
	for (int i = 1; i < 8; i++)
	{
		new_cr |= ppcState.cr_fast[i] << (28 - i * 4);
	}
	ppcState.cr = new_cr;
}

void ExpandCR()
{
	for (int i = 0; i < 8; i++)
	{
		ppcState.cr_fast[i] = (ppcState.cr >> (28 - i * 4)) & 0xF;
	}
}

void DoState(PointerWrap &p)
{
	rSPR(SPR_DEC) = SystemTimers::GetFakeDecrementer();
	*((u64 *)&TL) = SystemTimers::GetFakeTimeBase(); //works since we are little endian and TL comes first :)

	p.Do(ppcState);

	SystemTimers::DecrementerSet();
	SystemTimers::TimeBaseSet();
}

void ResetRegisters()
{
	for (int i = 0; i < 32; i++)
	{
		ppcState.gpr[i] = 0;
		riPS0(i) = 0;
		riPS1(i) = 0;
	}

	memset(ppcState.spr, 0, sizeof(ppcState.spr));
	/*
	0x00080200 = lonestar 2.0
	0x00088202 = lonestar 2.2
	0x70000100 = gekko 1.0
	0x00080100 = gekko 2.0
	0x00083203 = gekko 2.3a
	0x00083213 = gekko 2.3b
	0x00083204 = gekko 2.4
	0x00083214 = gekko 2.4e (8SE) - retail HW2
	*/
	ppcState.spr[SPR_PVR] = 0x00083214;
	ppcState.spr[SPR_HID1] = 0x80000000; // We're running at 3x the bus clock
	ppcState.spr[SPR_ECID_U] = 0x0d96e200;
	ppcState.spr[SPR_ECID_M] = 0x1840c00d;
	ppcState.spr[SPR_ECID_L] = 0x82bb08e8;

	ppcState.cr = 0;
	ppcState.fpscr = 0;
	ppcState.pc = 0;
	ppcState.npc = 0;
	ppcState.Exceptions = 0;

	TL = 0;
	TU = 0;
	SystemTimers::TimeBaseSet();

	// MSR should be 0x40, but we don't emulate BS1, so it would never be turned off :}
	ppcState.msr = 0;
	rDEC = 0xFFFFFFFF;
	SystemTimers::DecrementerSet();
}

void Init(int cpu_core)
{
	enum {
		FPU_PREC_24 = 0 << 8,
		FPU_PREC_53 = 2 << 8,
		FPU_PREC_64 = 3 << 8,
		FPU_PREC_MASK = 3 << 8,
	};
#ifdef _M_IX86
	// sets the floating-point lib to 53-bit
	// PowerPC has a 53bit floating pipeline only
	// eg: sscanf is very sensitive
#ifdef _WIN32
	_control87(_PC_53, MCW_PC);
#else
	unsigned short _mode;
	asm ("fstcw %0" : : "m" (_mode));
	_mode = (_mode & ~FPU_PREC_MASK) | FPU_PREC_53;
	asm ("fldcw %0" : : "m" (_mode));
#endif
#else
	//x64 doesn't need this - fpu is done with SSE
	//but still - set any useful sse options here
#endif

	ResetRegisters();
	PPCTables::InitTables(cpu_core);

	// We initialize the interpreter because
	// it is used on boot and code window independently.
	interpreter->Init();

	switch (cpu_core)
	{
	case 0:
		{
			cpu_core_base = interpreter;
			break;
		}
	case 1:
		{
			cpu_core_base = new Jit64();
			break;
		}
	case 2:
		{
			cpu_core_base = new JitIL();
			break;
		}
	default:
		{
			PanicAlert("Unrecognizable cpu_core: %d", cpu_core);
			break;
		}
	}

	if (cpu_core_base != interpreter)
	{
		jit = dynamic_cast<JitBase*>(cpu_core_base);
		jit->Init();
		mode = MODE_JIT;
	}
	else
	{
		mode = MODE_INTERPRETER;
	}
	state = CPU_STEPPING;

	ppcState.iCache.Reset();
}

void Shutdown()
{
	if (jit)
	{
		jit->Shutdown();
		delete jit;
		jit = NULL;
	}
	interpreter->Shutdown();
	cpu_core_base = NULL;
	state = CPU_POWERDOWN;
}

void SetMode(CoreMode new_mode)
{
	if (new_mode == mode)
		return;  // We don't need to do anything.

	mode = new_mode;

	switch (mode)
	{
	case MODE_INTERPRETER:  // Switching from JIT to interpreter
		jit->ClearCache();  // Remove all those nasty JIT patches.
		cpu_core_base = interpreter;
		break;

	case MODE_JIT:  // Switching from interpreter to JIT.
		// Don't really need to do much. It'll work, the cache will refill itself.
		cpu_core_base = jit;
		break;
	}
}

void SingleStep() 
{
	cpu_core_base->SingleStep();
}

void RunLoop()
{
	state = CPU_RUNNING;
	cpu_core_base->Run();
	Host_UpdateDisasmDialog();
}

CPUState GetState()
{
	return state;
}

volatile CPUState *GetStatePtr()
{
	return &state;
}

void Start()
{
	state = CPU_RUNNING;
	Host_UpdateDisasmDialog();
}

void Pause()
{
	state = CPU_STEPPING;
	Host_UpdateDisasmDialog();
}

void Stop()
{
	state = CPU_POWERDOWN;
	Host_UpdateDisasmDialog();
}

void CheckExceptions()
{
	// Read volatile data once
	u32 exceptions = ppcState.Exceptions;

	// This check is unnecessary in JIT mode. However, it probably doesn't really hurt.
	if (!exceptions)
		return;

	// Example procedure:
	// set SRR0 to either PC or NPC
	//SRR0 = NPC;
	// save specified MSR bits
	//SRR1 = MSR & 0x87C0FFFF;
	// copy ILE bit to LE
	//MSR |= (MSR >> 16) & 1;
	// clear MSR as specified
	//MSR &= ~0x04EF36; // 0x04FF36 also clears ME (only for machine check exception)
	// set to exception type entry point
	//NPC = 0x80000x00;

	if (exceptions & EXCEPTION_ISI)
	{
		SRR0 = NPC;
		// Page fault occurred
		SRR1 = (MSR & 0x87C0FFFF) | (1 << 30);
		MSR |= (MSR >> 16) & 1;
		MSR &= ~0x04EF36;
		NPC = 0x80000400;

		INFO_LOG(POWERPC, "EXCEPTION_ISI");
		Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_ISI);
	}
	else if (exceptions & EXCEPTION_PROGRAM)
	{
		SRR0 = PC;
		// say that it's a trap exception
		SRR1 = (MSR & 0x87C0FFFF) | 0x40000;
		MSR |= (MSR >> 16) & 1;
		MSR &= ~0x04EF36;
		NPC = 0x80000700;

		INFO_LOG(POWERPC, "EXCEPTION_PROGRAM");
		Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_PROGRAM);
	} 
	else if (exceptions & EXCEPTION_SYSCALL)
	{
		SRR0 = NPC;
		SRR1 = MSR & 0x87C0FFFF;
		MSR |= (MSR >> 16) & 1;
		MSR &= ~0x04EF36;
		NPC = 0x80000C00;

		INFO_LOG(POWERPC, "EXCEPTION_SYSCALL (PC=%08x)", PC);
		Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_SYSCALL);
	}
	else if (exceptions & EXCEPTION_FPU_UNAVAILABLE)
	{			
		//This happens a lot - Gamecube OS uses deferred FPU context switching
		SRR0 = PC;	// re-execute the instruction
		SRR1 = MSR & 0x87C0FFFF;
		MSR |= (MSR >> 16) & 1;
		MSR &= ~0x04EF36;
		NPC = 0x80000800;

		INFO_LOG(POWERPC, "EXCEPTION_FPU_UNAVAILABLE");
		Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_FPU_UNAVAILABLE);
	}
	else if (exceptions & EXCEPTION_DSI)
	{
		SRR0 = PC;
		SRR1 = MSR & 0x87C0FFFF;
		MSR |= (MSR >> 16) & 1;
		MSR &= ~0x04EF36;
		NPC = 0x80000300;
		//DSISR and DAR regs are changed in GenerateDSIException()

		INFO_LOG(POWERPC, "EXCEPTION_DSI");
		Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_DSI);
	} 
	else if (exceptions & EXCEPTION_ALIGNMENT)
	{
		//This never happens ATM
		// perhaps we can get dcb* instructions to use this :p
		SRR0 = PC;
		SRR1 = MSR & 0x87C0FFFF;
		MSR |= (MSR >> 16) & 1;
		MSR &= ~0x04EF36;
		NPC = 0x80000600;

		//TODO crazy amount of DSISR options to check out

		INFO_LOG(POWERPC, "EXCEPTION_ALIGNMENT");
		Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_ALIGNMENT);
	}

	// EXTERNAL INTERRUPT
	else if (MSR & 0x0008000) //hacky...the exception shouldn't be generated if EE isn't set...
	{
		if (exceptions & EXCEPTION_EXTERNAL_INT)
		{
			// Pokemon gets this "too early", it hasn't a handler yet
			SRR0 = NPC;
			SRR1 = MSR & 0x87C0FFFF;
			MSR |= (MSR >> 16) & 1;
			MSR &= ~0x04EF36;
			NPC = 0x80000500;

			INFO_LOG(POWERPC, "EXCEPTION_EXTERNAL_INT");
			Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_EXTERNAL_INT);

			_dbg_assert_msg_(POWERPC, (SRR1 & 0x02) != 0, "GEKKO", "EXTERNAL_INT unrecoverable???");
		}
		else if (exceptions & EXCEPTION_DECREMENTER)
		{
			SRR0 = NPC;
			SRR1 = MSR & 0x87C0FFFF;
			MSR |= (MSR >> 16) & 1;
			MSR &= ~0x04EF36;
			NPC = 0x80000900;

			INFO_LOG(POWERPC, "EXCEPTION_DECREMENTER");
			Common::AtomicAnd(ppcState.Exceptions, ~EXCEPTION_DECREMENTER);
		}
		else
		{
			_dbg_assert_msg_(POWERPC, 0, "Unknown EXT interrupt: Exceptions == %08x", exceptions);
			ERROR_LOG(POWERPC, "Unknown EXTERNAL INTERRUPT exception: Exceptions == %08x", exceptions);
		}
	}
}

void CheckBreakPoints()
{
	if (PowerPC::breakpoints.IsAddressBreakPoint(PC))
	{
		PowerPC::Pause();
		if (PowerPC::breakpoints.IsTempBreakPoint(PC))
			PowerPC::breakpoints.Remove(PC);
	}
}

void OnIdle(u32 _uThreadAddr)
{
	u32 nextThread = Memory::Read_U32(_uThreadAddr);
	//do idle skipping
	if (nextThread == 0)
		CoreTiming::Idle();
}

void OnIdleIL()
{
	CoreTiming::Idle();
}

}  // namespace


// FPSCR update functions

void UpdateFPRF(double dvalue)
{
	FPSCR.FPRF = MathUtil::ClassifyDouble(dvalue);
	//if (FPSCR.FPRF == 0x11)
	//	PanicAlert("QNAN alert");
}

void UpdateFEX() {
	FPSCR.FEX = (FPSCR.XX & FPSCR.XE) |
		        (FPSCR.ZX & FPSCR.ZE) |
				(FPSCR.UX & FPSCR.UE) |
				(FPSCR.OX & FPSCR.OE) |
				(FPSCR.VX & FPSCR.VE);
}
