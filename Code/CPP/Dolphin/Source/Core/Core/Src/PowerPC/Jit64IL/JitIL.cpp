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

#include <map>

#include "Common.h"
#include "x64Emitter.h"
#include "ABI.h"
#include "Thunk.h"
#include "../../HLE/HLE.h"
#include "../../Core.h"
#include "../../PatchEngine.h"
#include "../../CoreTiming.h"
#include "../PowerPC.h"
#include "../Profiler.h"
#include "../PPCTables.h"
#include "../PPCAnalyst.h"
#include "../../HW/Memmap.h"
#include "../../HW/GPFifo.h"
#include "../JitCommon/JitCache.h"
#include "JitIL.h"
#include "JitILAsm.h"
#include "JitIL_Tables.h"

using namespace Gen;
using namespace PowerPC;

// Dolphin's PowerPC->x86 JIT dynamic recompiler
// (Nearly) all code by ector (hrydgard)
// Features:
// * x86 & x64 support, lots of shared code.
// * Basic block linking
// * Fast dispatcher

// Unfeatures:
// * Does not recompile all instructions - sometimes falls back to inserting a CALL to the corresponding JIT function.

// Various notes below

// Register allocation
//   RAX - Generic quicktemp register
//   RBX - point to base of memory map
//   RSI RDI R12 R13 R14 R15 - free for allocation
//   RCX RDX R8 R9 R10 R11 - allocate in emergencies. These need to be flushed before functions are called.
//   RSP - stack pointer, do not generally use, very dangerous
//   RBP - ?

// IMPORTANT:
// Make sure that all generated code and all emulator state sits under the 2GB boundary so that
// RIP addressing can be used easily. Windows will always allocate static code under the 2GB boundary.
// Also make sure to use VirtualAlloc and specify EXECUTE permission.

// Open questions
// * Should there be any statically allocated registers? r3, r4, r5, r8, r0 come to mind.. maybe sp
// * Does it make sense to finish off the remaining non-jitted instructions? Seems we are hitting diminishing returns.
// * Why is the FPU exception handling not working 100%? Several games still get corrupted floating point state.
//   This can even be seen in one homebrew Wii demo - RayTracer.elf

// Other considerations
//
// Many instructions have shorter forms for EAX. However, I believe their performance boost
// will be as small to be negligible, so I haven't dirtied up the code with that. AMD recommends it in their
// optimization manuals, though.
//
// We support block linking. Reserve space at the exits of every block for a full 5-byte jmp. Save 16-bit offsets 
// from the starts of each block, marking the exits so that they can be nicely patched at any time.
//
// Blocks do NOT use call/ret, they only jmp to each other and to the dispatcher when necessary.
//
// All blocks that can be precompiled will be precompiled. Code will be memory protected - any write will mark
// the region as non-compilable, and all links to the page will be torn out and replaced with dispatcher jmps.
//
// Alternatively, icbi instruction SHOULD mark where we can't compile
//
// Seldom-happening events is handled by adding a decrement of a counter to all blr instructions (which are
// expensive anyway since we need to return to dispatcher, except when they can be predicted).

// TODO: SERIOUS synchronization problem with the video plugin setting tokens and breakpoints in dual core mode!!!
//       Somewhat fixed by disabling idle skipping when certain interrupts are enabled
//       This is no permanent reliable fix
// TODO: Zeldas go whacko when you hang the gfx thread

// Idea - Accurate exception handling
// Compute register state at a certain instruction by running the JIT in "dry mode", and stopping at the right place.
// Not likely to be done :P


// Optimization Ideas -
/*
  * Assume SP is in main RAM (in Wii mode too?) - partly done
  * Assume all floating point loads and double precision loads+stores are to/from main ram
    (single precision can be used in write gather pipe, specialized fast check added)
  * AMD only - use movaps instead of movapd when loading ps from memory?
  * HLE functions like floorf, sin, memcpy, etc - they can be much faster
  * ABI optimizations - drop F0-F13 on blr, for example. Watch out for context switching.
    CR2-CR4 are non-volatile, rest of CR is volatile -> dropped on blr.
	R5-R12 are volatile -> dropped on blr.
  * classic inlining across calls.
  
Low hanging fruit:
stfd -- guaranteed in memory
cmpl
mulli
stfs
stwu
lb/stzx

bcx - optimize!
bcctr
stfs
psq_st
addx
orx
rlwimix
fcmpo
DSP_UpdateARAMDMA
lfd
stwu
cntlzwx
bcctrx
WriteBigEData

TODO
lha
srawx
addic_rc
addex
subfcx
subfex

fmaddx
fmulx
faddx
fnegx
frspx
frsqrtex
ps_sum0
ps_muls0
ps_adds1

*/

static int CODE_SIZE = 1024*1024*16;

namespace CPUCompare
{
	extern u32 m_BlockStart;
}

void JitIL::Init()
{
	jo.optimizeStack = true;

	if (Core::g_CoreStartupParameter.bEnableDebugging)
	{
		jo.enableBlocklink = false;
		Core::g_CoreStartupParameter.bSkipIdle = false;
	}
	else
	{
		jo.enableBlocklink = true;  // Speed boost, but not 100% safe
	}

#ifdef _M_X64
	jo.enableFastMem = false;
#else
	jo.enableFastMem = false;
#endif
	jo.assumeFPLoadFromMem = true;
	jo.fpAccurateFcmp = false;
	jo.optimizeGatherPipe = true;
	jo.fastInterrupts = false;
	jo.accurateSinglePrecision = false;

	trampolines.Init();
	AllocCodeSpace(CODE_SIZE);

	blocks.Init();
	asm_routines.Init();
}

void JitIL::ClearCache()
{
	blocks.Clear();
	trampolines.ClearCodeSpace();
	ClearCodeSpace();
}

void JitIL::Shutdown()
{
	FreeCodeSpace();

	blocks.Shutdown();
	trampolines.Shutdown();
	asm_routines.Shutdown();
}


void JitIL::WriteCallInterpreter(UGeckoInstruction inst)
{
	if (js.isLastInstruction)
	{
		MOV(32, M(&PC), Imm32(js.compilerPC));
		MOV(32, M(&NPC), Imm32(js.compilerPC + 4));
	}
	Interpreter::_interpreterInstruction instr = GetInterpreterOp(inst);
	ABI_CallFunctionC((void*)instr, inst.hex);
	if (js.isLastInstruction)
	{
		MOV(32, R(EAX), M(&NPC));
		WriteRfiExitDestInEAX();
	}
}

void JitIL::unknown_instruction(UGeckoInstruction inst)
{
	//	CCPU::Break();
	PanicAlert("unknown_instruction %08x - Fix me ;)", inst.hex);
}

void JitIL::Default(UGeckoInstruction _inst)
{
	ibuild.EmitInterpreterFallback(
		ibuild.EmitIntConst(_inst.hex),
		ibuild.EmitIntConst(js.compilerPC));
}

void JitIL::HLEFunction(UGeckoInstruction _inst)
{
	ABI_CallFunctionCC((void*)&HLE::Execute, js.compilerPC, _inst.hex);
	MOV(32, R(EAX), M(&NPC));
	WriteExitDestInEAX(0);
}

void JitIL::DoNothing(UGeckoInstruction _inst)
{
	// Yup, just don't do anything.
}

void JitIL::NotifyBreakpoint(u32 em_address, bool set)
{
	int block_num = blocks.GetBlockNumberFromStartAddress(em_address);
	if (block_num >= 0)
	{
		blocks.DestroyBlock(block_num, false);
	}
}

static const bool ImHereDebug = false;
static const bool ImHereLog = false;
static std::map<u32, int> been_here;

static void ImHere()
{
	static FILE *f = 0;
	if (ImHereLog)
	{
		if (!f)
		{
#ifdef _M_X64
			f = fopen("log64.txt", "w");
#else
			f = fopen("log32.txt", "w");
#endif
		}
		fprintf(f, "%08x r0: %08x r5: %08x r6: %08x\n", PC, PowerPC::ppcState.gpr[0], PowerPC::ppcState.gpr[5], PowerPC::ppcState.gpr[6]); fflush(f);
	}
	if (been_here.find(PC) != been_here.end()) {
		been_here.find(PC)->second++;
		if ((been_here.find(PC)->second) & 1023)
			return;
	}
	DEBUG_LOG(DYNA_REC, "I'm here - PC = %08x , LR = %08x", PC, LR);
	//		printf("I'm here - PC = %08x , LR = %08x", PC, LR);
	been_here[PC] = 1;
}

void JitIL::Cleanup()
{
	if (jo.optimizeGatherPipe && js.fifoBytesThisBlock > 0)
		ABI_CallFunction((void *)&GPFifo::CheckGatherPipe);
}

void JitIL::WriteExit(u32 destination, int exit_num)
{
	Cleanup();
	SUB(32, M(&CoreTiming::downcount), js.downcountAmount > 127 ? Imm32(js.downcountAmount) : Imm8(js.downcountAmount)); 

	//If nobody has taken care of this yet (this can be removed when all branches are done)
	JitBlock *b = js.curBlock;
	b->exitAddress[exit_num] = destination;
	b->exitPtrs[exit_num] = GetWritableCodePtr();
	
	// Link opportunity!
	int block = blocks.GetBlockNumberFromStartAddress(destination);
	if (block >= 0 && jo.enableBlocklink) 
	{
		// It exists! Joy of joy!
		JMP(blocks.GetBlock(block)->checkedEntry, true);
		b->linkStatus[exit_num] = true;
	}
	else 
	{
		MOV(32, M(&PC), Imm32(destination));
		JMP(asm_routines.dispatcher, true);
	}
}

void JitIL::WriteExitDestInEAX(int exit_num) 
{
	MOV(32, M(&PC), R(EAX));
	Cleanup();
	SUB(32, M(&CoreTiming::downcount), js.downcountAmount > 127 ? Imm32(js.downcountAmount) : Imm8(js.downcountAmount)); 
	JMP(asm_routines.dispatcher, true);
}

void JitIL::WriteRfiExitDestInEAX() 
{
	MOV(32, M(&PC), R(EAX));
	Cleanup();
	SUB(32, M(&CoreTiming::downcount), js.downcountAmount > 127 ? Imm32(js.downcountAmount) : Imm8(js.downcountAmount)); 
	JMP(asm_routines.testExceptions, true);
}

void JitIL::WriteExceptionExit(u32 exception)
{
	Cleanup();
	OR(32, M(&PowerPC::ppcState.Exceptions), Imm32(exception));
	MOV(32, M(&PC), Imm32(js.compilerPC + 4));
	JMP(asm_routines.testExceptions, true);
}

void STACKALIGN JitIL::Run()
{
	CompiledCode pExecAddr = (CompiledCode)asm_routines.enterCode;
	pExecAddr();
	//Will return when PowerPC::state changes
}

void JitIL::SingleStep()
{
	CompiledCode pExecAddr = (CompiledCode)asm_routines.enterCode;
	pExecAddr();
}

void JitIL::Trace(PPCAnalyst::CodeBuffer *code_buf, u32 em_address)
{
	char regs[500] = "";
	char fregs[750] = "";

#ifdef JIT_LOG_GPR
	for (unsigned int i = 0; i < 32; i++)
	{
		char reg[50];
		sprintf(reg, "r%02d: %08x ", i, PowerPC::ppcState.gpr[i]);
		strncat(regs, reg, 500);
	}
#endif

#ifdef JIT_LOG_FPR
	for (unsigned int i = 0; i < 32; i++)
	{
		char reg[50];
		sprintf(reg, "f%02d: %016x ", i, riPS0(i));
		strncat(fregs, reg, 750);
	}
#endif	
	const PPCAnalyst::CodeOp &op = code_buf->codebuffer[0];
	char ppcInst[256];
	DisassembleGekko(op.inst.hex, em_address, ppcInst, 256);
	
	NOTICE_LOG(DYNA_REC, "JITIL PC: %08x SRR0: %08x SRR1: %08x CRfast: %02x%02x%02x%02x%02x%02x%02x%02x FPSCR: %08x MSR: %08x LR: %08x %s %s %08x %s", PC, SRR0, SRR1, PowerPC::ppcState.cr_fast[0], PowerPC::ppcState.cr_fast[1], PowerPC::ppcState.cr_fast[2], PowerPC::ppcState.cr_fast[3], PowerPC::ppcState.cr_fast[4], PowerPC::ppcState.cr_fast[5], PowerPC::ppcState.cr_fast[6], PowerPC::ppcState.cr_fast[7], PowerPC::ppcState.fpscr, PowerPC::ppcState.msr, PowerPC::ppcState.spr[8], regs, fregs, op.inst.hex, ppcInst);
}

void STACKALIGN JitIL::Jit(u32 em_address)
{
	if (GetSpaceLeft() < 0x10000 || blocks.IsFull() || Core::g_CoreStartupParameter.bJITNoBlockCache)
	{
		ClearCache();
	}
	int block_num = blocks.AllocateBlock(em_address);
	JitBlock *b = blocks.GetBlock(block_num);
	blocks.FinalizeBlock(block_num, jo.enableBlocklink, DoJit(em_address, &code_buffer, b));
}

const u8* JitIL::DoJit(u32 em_address, PPCAnalyst::CodeBuffer *code_buf, JitBlock *b)
{
	int blockSize = code_buf->GetSize();

	if (Core::g_CoreStartupParameter.bEnableDebugging)
	{
		// Comment out the following to disable breakpoints (speed-up)
		blockSize = 1;
		Trace(code_buf, em_address);
	}

	if (em_address == 0)
		PanicAlert("ERROR : Trying to compile at 0. LR=%08x", LR);

	int size;
	js.isLastInstruction = false;
	js.blockStart = em_address;
	js.fifoBytesThisBlock = 0;
	js.curBlock = b;
	js.cancel = false;

	//Analyze the block, collect all instructions it is made of (including inlining,
	//if that is enabled), reorder instructions for optimal performance, and join joinable instructions.
	b->exitAddress[0] = PPCAnalyst::Flatten(em_address, &size, &js.st, &js.gpa, &js.fpa, code_buf, blockSize);
	PPCAnalyst::CodeOp *ops = code_buf->codebuffer;

	const u8 *start = AlignCode4(); //TODO: Test if this or AlignCode16 make a difference from GetCodePtr
	b->checkedEntry = start;
	b->runCount = 0;

	// Downcount flag check. The last block decremented downcounter, and the flag should still be available.
	FixupBranch skip = J_CC(CC_NBE);
	MOV(32, M(&PC), Imm32(js.blockStart));
	JMP(asm_routines.doTiming, true);  // downcount hit zero - go doTiming.
	SetJumpTarget(skip);

	const u8 *normalEntry = GetCodePtr();
	b->normalEntry = normalEntry;
	
	if (ImHereDebug)
		ABI_CallFunction((void *)&ImHere); //Used to get a trace of the last few blocks before a crash, sometimes VERY useful

	if (js.fpa.any)
	{
		//This block uses FPU - needs to add FP exception bailout
		TEST(32, M(&PowerPC::ppcState.msr), Imm32(1 << 13)); //Test FP enabled bit
		FixupBranch b1 = J_CC(CC_NZ);
		MOV(32, M(&PC), Imm32(js.blockStart));
		JMP(asm_routines.fpException, true);
		SetJumpTarget(b1);
	}

	js.rewriteStart = (u8*)GetCodePtr();

	// Start up IR builder (structure that collects the
	// instruction processed by the JIT routines)
	ibuild.Reset();

	
	js.downcountAmount = js.st.numCycles;

	if (!Core::g_CoreStartupParameter.bEnableDebugging)
		js.downcountAmount += PatchEngine::GetSpeedhackCycles(em_address);

	// Translate instructions
	for (int i = 0; i < (int)size; i++)
	{
		js.compilerPC = ops[i].address;
		js.op = &ops[i];
		js.instructionNumber = i;
		if (i == (int)size - 1)
		{
			js.isLastInstruction = true;
			js.next_inst = 0;
		}
		else
		{
			// help peephole optimizations
			js.next_inst = ops[i + 1].inst;
			js.next_compilerPC = ops[i + 1].address;
		}

		if (!ops[i].skip)
		{
			JitILTables::CompileInstruction(ops[i]);
		}
	}

	// Perform actual code generation
	WriteCode();

	b->codeSize = (u32)(GetCodePtr() - normalEntry);
	b->originalSize = size;

#ifdef JIT_LOG_X86
	LogGeneratedX86(size, code_buf, normalEntry, b);
#endif

	return normalEntry;
}
