/*====================================================================

   filename:     gdsp_interpreter.cpp
   project:      GCemu
   created:      2004-6-18
   mail:		  duddie@walla.com

   Copyright (c) 2005 Duddie & Tratax

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   ====================================================================*/

#include "DSPTables.h"
#include "DSPHost.h"
#include "DSPCore.h"
#include "DSPAnalyzer.h"

#include "DSPHWInterface.h"
#include "DSPIntUtil.h"

namespace DSPInterpreter {

volatile u32 gdsp_running;

// NOTE: These have nothing to do with g_dsp.r[DSP_REG_CR].

// Hm, should instructions that change CR use this? Probably not (but they
// should call UpdateCachedCR())
void WriteCR(u16 val)
{
	// reset
	if (val & 1)
	{
		DSPCore_Reset();
		val &= ~1;
	}
	// init - can reset and init be done at the same time?
	else if (val == 4)
	{
		// this looks like a hack! OSInitAudioSystem ucode
		// should send this mail - not dsp core itself
		gdsp_mbox_write_h(GDSP_MBOX_DSP, 0x8054);
		gdsp_mbox_write_l(GDSP_MBOX_DSP, 0x4348);
		val |= 0x800;
	}

	// update cr
	g_dsp.cr = val;
}

// Hm, should instructions that read CR use this? (Probably not).
u16 ReadCR()
{
	if (g_dsp.pc & 0x8000)
	{
		g_dsp.cr |= 0x800;
	}
	else
	{
		g_dsp.cr &= ~0x800;
	}

	return g_dsp.cr;
}

void Step()
{
	DSPCore_CheckExceptions();

	g_dsp.step_counter++;

#if PROFILE
	g_dsp.err_pc = g_dsp.pc;

	ProfilerAddDelta(g_dsp.err_pc, 1);
	if (g_dsp.step_counter == 1)
	{
		ProfilerInit();
	}

	if ((g_dsp.step_counter & 0xFFFFF) == 0)
	{
		ProfilerDump(g_dsp.step_counter);
	}
#endif

	u16 opc = dsp_fetch_code();
	ExecuteInstruction(UDSPInstruction(opc));
	
	if (DSPAnalyzer::code_flags[g_dsp.pc - 1] & DSPAnalyzer::CODE_LOOP_END)
		HandleLoop();
}

// Used by thread mode.
void Run()
{
	int checkInterrupt = 0;
	gdsp_running = true;
	while (!(g_dsp.cr & CR_HALT) && gdsp_running)
	{
		if (jit)
			jit->RunForCycles(1);
		else {
			// Automatically let the other threads work if we're idle skipping
			if(DSPAnalyzer::code_flags[g_dsp.pc] & DSPAnalyzer::CODE_IDLE_SKIP)
				Common::YieldCPU();
			
			Step();
			
			// Turns out the less you check for external interrupts, the more 
			// sound you hear, and it becomes slower
			checkInterrupt++;
			if(checkInterrupt == 500) { // <-- Arbitrary number. TODO: tweak
				DSPCore_CheckExternalInterrupt();
				checkInterrupt = 0;
			}
		}
	}
	gdsp_running = false;
}

// This one has basic idle skipping, and checks breakpoints.
int RunCyclesDebug(int cycles)
{
	// First, let's run a few cycles with no idle skipping so that things can progress a bit.
	for (int i = 0; i < 8; i++)
	{
		if (g_dsp.cr & CR_HALT)
			return 0; 
		if (dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
		{
			DSPCore_SetState(DSPCORE_STEPPING);
			return cycles;
		}
		Step();
		cycles--;
		if (cycles < 0)
			return 0;
	}
	
	DSPCore_CheckExternalInterrupt();

	while (true)
	{
		// Next, let's run a few cycles with idle skipping, so that we can skip
		// idle loops.
		for (int i = 0; i < 8; i++)
		{
			if (g_dsp.cr & CR_HALT)
				return 0;
			if (dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
			{
				DSPCore_SetState(DSPCORE_STEPPING);
				return cycles;
			}
			// Idle skipping.
			if (DSPAnalyzer::code_flags[g_dsp.pc] & DSPAnalyzer::CODE_IDLE_SKIP)
				return 0;
			Step();
			cycles--;
			if (cycles < 0)
				return 0;
		}

		// Now, lets run some more without idle skipping. 
		for (int i = 0; i < 200; i++)
		{
			if (dsp_breakpoints.IsAddressBreakPoint(g_dsp.pc))
			{
				DSPCore_SetState(DSPCORE_STEPPING);
				return cycles;
			}
			Step();
			cycles--;
			if (cycles < 0)
				return 0;

			// We don't bother directly supporting pause - if the main emu pauses,
			// it just won't call this function anymore.
		}
	}
}

// Used by non-thread mode. Meant to be efficient.
int RunCycles(int cycles)
{
	// First, let's run a few cycles with no idle skipping so that things can
	// progress a bit.
	for (int i = 0; i < 8; i++)
	{
		if (g_dsp.cr & CR_HALT)
			return 0; 
		Step();
		cycles--;
		if (cycles < 0)
			return 0;
	}

	DSPCore_CheckExternalInterrupt();

	while (true)
	{
		// Next, let's run a few cycles with idle skipping, so that we can skip
		// idle loops.
		for (int i = 0; i < 8; i++)
		{
			if (g_dsp.cr & CR_HALT)
				return 0;
			// Idle skipping.
			if (DSPAnalyzer::code_flags[g_dsp.pc] & DSPAnalyzer::CODE_IDLE_SKIP)
				return 0;
			Step();
			cycles--;
			if (cycles < 0)
				return 0;
		}

		// Now, lets run some more without idle skipping. 
		for (int i = 0; i < 200; i++)	
		{
			Step();
			cycles--;
			if (cycles < 0)
				return 0;
			// We don't bother directly supporting pause - if the main emu pauses,
			// it just won't call this function anymore.
		}
	}
}

void Stop()
{
	gdsp_running = false;
}

}  // namespace
