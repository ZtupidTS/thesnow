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

#include "ABI.h"
#include "x64Emitter.h"

#include "../../HW/Memmap.h"

#include "../PowerPC.h"
#include "../../CoreTiming.h"
#include "MemoryUtil.h"

#include "ABI.h"
#include "Jit.h"
#include "../JitCommon/JitCache.h"

#include "../../HW/GPFifo.h"
#include "../../Core.h"
#include "JitAsm.h"

using namespace Gen;

//static int temp32; // unused?

//TODO - make an option
//#if _DEBUG
static bool enableDebug = false; 
//#else
//		bool enableDebug = false; 
//#endif

//static bool enableStatistics = false; //unused?

//GLOBAL STATIC ALLOCATIONS x86
//EAX - ubiquitous scratch register - EVERYBODY scratches this

//GLOBAL STATIC ALLOCATIONS x64
//EAX - ubiquitous scratch register - EVERYBODY scratches this
//RBX - Base pointer of memory
//R15 - Pointer to array of block pointers 

Jit64AsmRoutineManager asm_routines;

// PLAN: no more block numbers - crazy opcodes just contain offset within
// dynarec buffer
// At this offset - 4, there is an int specifying the block number.


void Jit64AsmRoutineManager::Generate()
{
	enterCode = AlignCode16();
	ABI_PushAllCalleeSavedRegsAndAdjustStack();
#ifndef _M_IX86
	// Two statically allocated registers.
	MOV(64, R(RBX), Imm64((u64)Memory::base));
	MOV(64, R(R15), Imm64((u64)jit->GetBlockCache()->GetCodePointers())); //It's below 2GB so 32 bits are good enough
#endif

	const u8 *outerLoop = GetCodePtr();
		ABI_CallFunction(reinterpret_cast<void *>(&CoreTiming::Advance));
		FixupBranch skipToRealDispatch = J(); //skip the sync and compare first time
	 
		dispatcher = GetCodePtr();
			// The result of slice decrementation should be in flags if somebody jumped here
			// IMPORTANT - We jump on negative, not carry!!!
			FixupBranch bail = J_CC(CC_BE);

			if (Core::g_CoreStartupParameter.bEnableDebugging)
			{
				ABI_CallFunction(reinterpret_cast<void *>(&PowerPC::CheckBreakPoints));
				TEST(32, M((void*)PowerPC::GetStatePtr()), Imm32(0xFFFFFFFF));
				FixupBranch noBreakpoint = J_CC(CC_Z);
				ABI_PopAllCalleeSavedRegsAndAdjustStack();
				RET();
				SetJumpTarget(noBreakpoint);
			}

			SetJumpTarget(skipToRealDispatch);

			dispatcherNoCheck = GetCodePtr();
			MOV(32, R(EAX), M(&PowerPC::ppcState.pc));
			dispatcherPcInEAX = GetCodePtr();

			FixupBranch needinst = J(true);
			const u8* haveinst = GetCodePtr();

			TEST(32, R(EAX), Imm32(0xFC));
			FixupBranch notfound = J_CC(CC_NZ);
				BSWAP(32, EAX);
				//IDEA - we have 26 bits, why not just use offsets from base of code?
				if (enableDebug)
				{
					ADD(32, M(&PowerPC::ppcState.DebugCount), Imm8(1));
				}
				//grab from list and jump to it
#ifdef _M_IX86
				MOV(32, R(EDX), ImmPtr(jit->GetBlockCache()->GetCodePointers()));
				JMPptr(MComplex(EDX, EAX, 4, 0));
#else
				JMPptr(MComplex(R15, RAX, 8, 0));
#endif
			SetJumpTarget(notfound);

			//Ok, no block, let's jit
#ifdef _M_IX86
			ABI_AlignStack(4);
			PUSH(32, M(&PowerPC::ppcState.pc));
			CALL(reinterpret_cast<void *>(&Jit));
			ABI_RestoreStack(4);
#else
			MOV(32, R(ABI_PARAM1), M(&PowerPC::ppcState.pc));
			CALL((void *)&Jit);
#endif
			JMP(dispatcherNoCheck); // no point in special casing this

			//FP blocks test for FPU available, jump here if false
			fpException = AlignCode4(); 
			MOV(32, R(EAX), M(&PC));
			MOV(32, M(&NPC), R(EAX));
			OR(32, M(&PowerPC::ppcState.Exceptions), Imm32(EXCEPTION_FPU_UNAVAILABLE));
			ABI_CallFunction(reinterpret_cast<void *>(&PowerPC::CheckExceptions));
			MOV(32, R(EAX), M(&NPC));
			MOV(32, M(&PC), R(EAX));
			JMP(dispatcher);

		SetJumpTarget(bail);
		doTiming = GetCodePtr();

		ABI_CallFunction(reinterpret_cast<void *>(&CoreTiming::Advance));
		
		testExceptions = GetCodePtr();
		TEST(32, M(&PowerPC::ppcState.Exceptions), Imm32(0xFFFFFFFF));
		FixupBranch skipExceptions = J_CC(CC_Z);
			MOV(32, R(EAX), M(&PC));
			MOV(32, M(&NPC), R(EAX));
			ABI_CallFunction(reinterpret_cast<void *>(&PowerPC::CheckExceptions));
			MOV(32, R(EAX), M(&NPC));
			MOV(32, M(&PC), R(EAX));
		SetJumpTarget(skipExceptions);
		
		TEST(32, M((void*)PowerPC::GetStatePtr()), Imm32(0xFFFFFFFF));
		J_CC(CC_Z, outerLoop, true);

	//Landing pad for drec space
	ABI_PopAllCalleeSavedRegsAndAdjustStack();
	RET();


	SetJumpTarget(needinst);
#ifdef JIT_UNLIMITED_ICACHE
			
			TEST(32, R(EAX), Imm32(JIT_ICACHE_VMEM_BIT));
			FixupBranch vmem = J_CC(CC_NZ);
			TEST(32, R(EAX), Imm32(JIT_ICACHE_EXRAM_BIT));
			FixupBranch exram = J_CC(CC_NZ);

			AND(32, R(EAX), Imm32(JIT_ICACHE_MASK));
#ifdef _M_IX86
			MOV(32, R(EAX), MDisp(EAX, (u32)jit->GetBlockCache()->GetICache()));
#else
			MOV(64, R(RSI), Imm64((u64)jit->GetBlockCache()->GetICache()));
			MOV(32, R(EAX), MComplex(RSI, EAX, SCALE_1, 0));
#endif

			FixupBranch getinst = J();
			SetJumpTarget(exram);
						
			AND(32, R(EAX), Imm32(JIT_ICACHEEX_MASK));
#ifdef _M_IX86
			MOV(32, R(EAX), MDisp(EAX, (u32)jit->GetBlockCache()->GetICacheEx()));
#else
			MOV(64, R(RSI), Imm64((u64)jit->GetBlockCache()->GetICacheEx()));
			MOV(32, R(EAX), MComplex(RSI, EAX, SCALE_1, 0));
#endif

			FixupBranch getinst2 = J();
			SetJumpTarget(vmem);
						
			AND(32, R(EAX), Imm32(JIT_ICACHE_MASK));
#ifdef _M_IX86
			MOV(32, R(EAX), MDisp(EAX, (u32)jit->GetBlockCache()->GetICacheVMEM()));
#else
			MOV(64, R(RSI), Imm64((u64)jit->GetBlockCache()->GetICacheVMEM()));
			MOV(32, R(EAX), MComplex(RSI, EAX, SCALE_1, 0));
#endif

			SetJumpTarget(getinst2);
			SetJumpTarget(getinst);
#else
#ifdef _M_IX86
			AND(32, R(EAX), Imm32(Memory::MEMVIEW32_MASK));
			MOV(32, R(EBX), Imm32((u32)Memory::base));
			MOV(32, R(EAX), MComplex(EBX, EAX, SCALE_1, 0));
#else
			MOV(32, R(EAX), MComplex(RBX, RAX, SCALE_1, 0));
#endif
#endif	
	JMP(haveinst, true);


	breakpointBailout = GetCodePtr();
	//Landing pad for drec space
	ABI_PopAllCalleeSavedRegsAndAdjustStack();
	RET();

	GenerateCommon();
}

void Jit64AsmRoutineManager::GenerateCommon()
{
	fifoDirectWrite8 = AlignCode4();
	GenFifoWrite(8);
	fifoDirectWrite16 = AlignCode4();
	GenFifoWrite(16);
	fifoDirectWrite32 = AlignCode4();
	GenFifoWrite(32);
	fifoDirectWriteFloat = AlignCode4();
	GenFifoFloatWrite();
	fifoDirectWriteXmm64 = AlignCode4(); 
	GenFifoXmm64Write();

	GenQuantizedLoads();
	GenQuantizedStores();
	GenQuantizedSingleStores();

	//CMPSD(R(XMM0), M(&zero), 
	// TODO

	// Fast write routines - special case the most common hardware write
	// TODO: use this.
	// Even in x86, the param values will be in the right registers.
	/*
	const u8 *fastMemWrite8 = AlignCode16();
	CMP(32, R(ABI_PARAM2), Imm32(0xCC008000));
	FixupBranch skip_fast_write = J_CC(CC_NE, false);
	MOV(32, EAX, M(&m_gatherPipeCount));
	MOV(8, MDisp(EAX, (u32)&m_gatherPipe), ABI_PARAM1);
	ADD(32, 1, M(&m_gatherPipeCount));
	RET();
	SetJumpTarget(skip_fast_write);
	CALL((void *)&Memory::Write_U8);*/
}
