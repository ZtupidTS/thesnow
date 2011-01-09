// Copyright (C) 2010 Dolphin Project.

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

#include "../DSPMemoryMap.h"
#include "../DSPEmitter.h"
#include "../DSPStacks.h"
#include "../DSPAnalyzer.h"
#include "DSPJitUtil.h"
#include "x64Emitter.h"
#include "ABI.h"

using namespace Gen;

const int GetCodeSize(void(*jitCode)(const UDSPInstruction, DSPEmitter&), const UDSPInstruction opc, DSPEmitter &emitter)
{
	u16 pc = g_dsp.pc;
	const u8* ptr = emitter.GetCodePtr();
	jitCode(opc, emitter);
	//emitter.JMP(emitter.GetCodePtr());
	int size = (int)(emitter.GetCodePtr() - ptr);
	emitter.SetCodePtr((u8*)ptr);
	g_dsp.pc = pc;
	return size;
}

const u8* CheckCondition(DSPEmitter& emitter, u8 cond, u8 skipCodeSize)
{
	if (cond == 0xf) // Always true.
		return NULL;
	//emitter.INT3();
	FixupBranch skipCode2;
#ifdef _M_IX86 // All32
	emitter.MOV(16, R(EAX), M(&g_dsp.r.sr));
#else
	emitter.MOV(64, R(RAX), ImmPtr(&g_dsp.r.sr));
	emitter.MOV(16, R(EAX), MatR(RAX));
#endif
	switch(cond)
	{
	case 0x0: // GE - Greater Equal
	case 0x1: // L - Less
	case 0x2: // G - Greater
	case 0x3: // LE - Less Equal
		emitter.MOV(16, R(EDX), R(EAX));
		emitter.SHR(16, R(EDX), Imm8(3)); //SR_SIGN flag
		emitter.NOT(16, R(EDX));
		emitter.SHR(16, R(EAX), Imm8(1)); //SR_OVERFLOW flag
		emitter.NOT(16, R(EAX));
		emitter.XOR(16, R(EAX), R(EDX));
		emitter.TEST(16, R(EAX), Imm16(1));
		if (cond < 0x2)
			break;
		
		//LE: problem in here, half the tests fail
		skipCode2 = emitter.J_CC(CC_NE);
		//skipCode2 = emitter.J_CC((CCFlags)(CC_NE - (cond & 1)));
#ifdef _M_IX86 // All32
		emitter.MOV(16, R(EAX), M(&g_dsp.r.sr));
#else
		emitter.MOV(64, R(RAX), ImmPtr(&g_dsp.r.sr));
		emitter.MOV(16, R(EAX), MatR(RAX));
#endif
		emitter.TEST(16, R(EAX), Imm16(SR_ARITH_ZERO));
		break;
	case 0x4: // NZ - Not Zero
	case 0x5: // Z - Zero 
		emitter.TEST(16, R(EAX), Imm16(SR_ARITH_ZERO));
		break;
	case 0x6: // NC - Not carry
	case 0x7: // C - Carry 
		emitter.TEST(16, R(EAX), Imm16(SR_CARRY));
		break;
	case 0x8: // ? - Not over s32
	case 0x9: // ? - Over s32
		emitter.TEST(16, R(EAX), Imm16(SR_OVER_S32));
		break;
	case 0xa: // ?
	case 0xb: // ?
	{
		//full of fail, both
		emitter.TEST(16, R(EAX), Imm16(SR_OVER_S32 | SR_TOP2BITS));
		FixupBranch skipArithZero = emitter.J_CC(CC_E);
		emitter.TEST(16, R(EAX), Imm16(SR_ARITH_ZERO));
		FixupBranch setZero = emitter.J_CC(CC_NE);

		emitter.MOV(16, R(EAX), Imm16(1));
		FixupBranch toEnd = emitter.J();

		emitter.SetJumpTarget(skipArithZero);
		emitter.SetJumpTarget(setZero);
		emitter.XOR(16, R(EAX), R(EAX));
		emitter.SetJumpTarget(toEnd);
		emitter.SETcc(CC_E, R(EAX));
		emitter.TEST(8, R(EAX), R(EAX));
		break;
		//emitter.TEST(16, R(EAX), Imm16(SR_OVER_S32 | SR_TOP2BITS));
		//skipCode2 = emitter.J_CC((CCFlags)(CC_E + (cond & 1)));
		//emitter.TEST(16, R(EAX), Imm16(SR_ARITH_ZERO));
		//break;
	}
	case 0xc: // LNZ  - Logic Not Zero
	case 0xd: // LZ - Logic Zero
		emitter.TEST(16, R(EAX), Imm16(SR_LOGIC_ZERO));
		break;
	case 0xe: // 0 - Overflow
		emitter.TEST(16, R(EAX), Imm16(SR_OVERFLOW));
		break;
	}
	FixupBranch skipCode = cond == 0xe ? emitter.J_CC(CC_E) : emitter.J_CC((CCFlags)(CC_NE - (cond & 1)));
	const u8* res = emitter.GetCodePtr();
	emitter.NOP(skipCodeSize);
	emitter.SetJumpTarget(skipCode);
	if ((cond | 1) == 0x3) // || (cond | 1) == 0xb)
		emitter.SetJumpTarget(skipCode2);
	return res;
}

template <void(*jitCode)(const UDSPInstruction, DSPEmitter&)>
void ReJitConditional(const UDSPInstruction opc, DSPEmitter& emitter)
{
	static const int codeSize = GetCodeSize(jitCode, opc, emitter);
	//emitter.INT3();
	const u8* codePtr = CheckCondition(emitter, opc & 0xf, codeSize);
	//const u8* afterSkip = emitter.GetCodePtr();
	if (codePtr != NULL) 
		emitter.SetCodePtr((u8*)codePtr);
	jitCode(opc, emitter);
	//if (codePtr != NULL) 
	//{
	//	emitter.JMP(afterSkip + 4 + sizeof(void*));
	//	emitter.SetCodePtr((u8*)afterSkip);
	//	emitter.ADD(16, M(&g_dsp.pc), Imm8(1)); //4 bytes + pointer
	//}
}

void WriteBranchExit(DSPEmitter& emitter)
{
	//		ABI_RestoreStack(0);
	emitter.ABI_PopAllCalleeSavedRegsAndAdjustStack();
	if (DSPAnalyzer::code_flags[emitter.startAddr] & DSPAnalyzer::CODE_IDLE_SKIP)
	{
		emitter.MOV(16, R(EAX), Imm16(0x1000));
	}
	else
	{
		emitter.MOV(16, R(EAX), Imm16(emitter.blockSize[emitter.startAddr]));
	}
	emitter.RET();
}

void WriteBlockLink(DSPEmitter& emitter, u16 dest)
{
	// Jump directly to the called block if it has already been compiled.
	if (!(dest >= emitter.startAddr && dest <= emitter.compilePC))
	{
		if (emitter.blockLinks[dest] != 0 )
		{
#ifdef _M_IX86 // All32
			// Check if we have enough cycles to execute the next block
			emitter.MOV(16, R(ESI), M(&cyclesLeft));
			emitter.CMP(16, R(ESI), Imm16(emitter.blockSize[emitter.startAddr] + emitter.blockSize[dest]));
			FixupBranch notEnoughCycles = emitter.J_CC(CC_BE);

			emitter.SUB(16, R(ESI), Imm16(emitter.blockSize[emitter.startAddr]));
			emitter.MOV(16, M(&cyclesLeft), R(ESI));
			emitter.JMPptr(M(&emitter.blockLinks[dest]));

			emitter.SetJumpTarget(notEnoughCycles);
#else
			// Check if we have enough cycles to execute the next block
			emitter.CMP(16, R(R12), Imm16(emitter.blockSize[emitter.startAddr] + emitter.blockSize[dest]));
			FixupBranch notEnoughCycles = emitter.J_CC(CC_BE);

			emitter.SUB(16, R(R12), Imm16(emitter.blockSize[emitter.startAddr]));
			emitter.MOV(64, R(RAX), ImmPtr((void *)emitter.blockLinks[dest]));
			emitter.JMPptr(R(RAX));

			emitter.SetJumpTarget(notEnoughCycles);
#endif
		}
		else
		{
			// The destination has not been compiled yet.  Add it to the list
			// of blocks that this block is waiting on.
			emitter.unresolvedJumps[emitter.startAddr].push_back(dest);
		}
	}
}

void r_jcc(const UDSPInstruction opc, DSPEmitter& emitter)
{
	u16 dest = dsp_imem_read(emitter.compilePC + 1);
	const DSPOPCTemplate *opcode = GetOpTemplate(opc);

	// If the block is unconditional, attempt to link block
	if (opcode->uncond_branch)
		WriteBlockLink(emitter, dest);
#ifdef _M_IX86 // All32
	emitter.MOV(16, M(&(g_dsp.pc)), Imm16(dest));
#else
	emitter.MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	emitter.MOV(16, MatR(RAX), Imm16(dest));
#endif
	WriteBranchExit(emitter);
}
// Generic jmp implementation
// Jcc addressA
// 0000 0010 1001 cccc
// aaaa aaaa aaaa aaaa
// Jump to addressA if condition cc has been met. Set program counter to
// address represented by value that follows this "jmp" instruction.
// NOTE: Cannot use Default(opc) here because of the need to write branch exit
void DSPEmitter::jcc(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	MOV(16, M(&(g_dsp.pc)), Imm16(compilePC + 2));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 2));
#endif
	ReJitConditional<r_jcc>(opc, *this);
}

void r_jmprcc(const UDSPInstruction opc, DSPEmitter& emitter)
{
	u8 reg = (opc >> 5) & 0x7;
	u16 *regp = reg_ptr(reg);
	//reg can only be DSP_REG_ARx and DSP_REG_IXx now,
	//no need to handle DSP_REG_STx.
#ifdef _M_IX86 // All32
	emitter.MOV(16, R(EAX), M(regp));
	emitter.MOV(16, M(&g_dsp.pc), R(EAX));
#else
	emitter.MOV(64, R(RSI), ImmPtr(regp));
	emitter.MOV(16, R(RSI), MatR(RSI));
	emitter.MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	emitter.MOV(16, MatR(RAX), R(RSI));
#endif
	WriteBranchExit(emitter);
}
// Generic jmpr implementation
// JMPcc $R
// 0001 0111 rrr0 cccc
// Jump to address; set program counter to a value from register $R.
// NOTE: Cannot use Default(opc) here because of the need to write branch exit
void DSPEmitter::jmprcc(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), Imm16(compilePC + 1));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 1));
#endif
	ReJitConditional<r_jmprcc>(opc, *this);
}

void r_call(const UDSPInstruction opc, DSPEmitter& emitter)
{
	emitter.MOV(16, R(DX), Imm16(emitter.compilePC + 2));
	emitter.dsp_reg_store_stack(DSP_STACK_C);
	u16 dest = dsp_imem_read(emitter.compilePC + 1);
	const DSPOPCTemplate *opcode = GetOpTemplate(opc);

	// If the block is unconditional, attempt to link block
	if (opcode->uncond_branch)
		WriteBlockLink(emitter, dest);
#ifdef _M_IX86 // All32
	emitter.MOV(16, M(&(g_dsp.pc)), Imm16(dest));
#else
	emitter.MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	emitter.MOV(16, MatR(RAX), Imm16(dest));
#endif
	WriteBranchExit(emitter);
}
// Generic call implementation
// CALLcc addressA
// 0000 0010 1011 cccc
// aaaa aaaa aaaa aaaa
// Call function if condition cc has been met. Push program counter of
// instruction following "call" to $st0. Set program counter to address
// represented by value that follows this "call" instruction.
// NOTE: Cannot use Default(opc) here because of the need to write branch exit
void DSPEmitter::call(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	MOV(16, M(&(g_dsp.pc)), Imm16(compilePC + 2));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 2));
#endif
	ReJitConditional<r_call>(opc, *this);
}

void r_callr(const UDSPInstruction opc, DSPEmitter& emitter)
{
	u8 reg = (opc >> 5) & 0x7;
	u16 *regp = reg_ptr(reg);
	emitter.MOV(16, R(DX), Imm16(emitter.compilePC + 1));
	emitter.dsp_reg_store_stack(DSP_STACK_C);
#ifdef _M_IX86 // All32
	emitter.MOV(16, R(EAX), M(regp));
	emitter.MOV(16, M(&g_dsp.pc), R(EAX));
#else
	emitter.MOV(64, R(RSI), ImmPtr(regp));
	emitter.MOV(16, R(RSI), MatR(RSI));
	emitter.MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	emitter.MOV(16, MatR(RAX), R(RSI));
#endif
	WriteBranchExit(emitter);
}
// Generic callr implementation
// CALLRcc $R
// 0001 0111 rrr1 cccc
// Call function if condition cc has been met. Push program counter of 
// instruction following "call" to call stack $st0. Set program counter to 
// register $R.
// NOTE: Cannot use Default(opc) here because of the need to write branch exit
void DSPEmitter::callr(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), Imm16(compilePC + 1));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 1));
#endif
	ReJitConditional<r_callr>(opc, *this);
}

void r_ifcc(const UDSPInstruction opc, DSPEmitter& emitter)
{
#ifdef _M_IX86 // All32
	emitter.MOV(16, M(&g_dsp.pc), Imm16(emitter.compilePC + 1));
#else
	emitter.MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	emitter.MOV(16, MatR(RAX), Imm16(emitter.compilePC + 1));
#endif
}
// Generic if implementation
// IFcc
// 0000 0010 0111 cccc
// Execute following opcode if the condition has been met.
// NOTE: Cannot use Default(opc) here because of the need to write branch exit
void DSPEmitter::ifcc(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), Imm16((compilePC + 1) + opTable[compilePC + 1]->size));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16((compilePC + 1) + opTable[compilePC + 1]->size));
#endif
	ReJitConditional<r_ifcc>(opc, *this);
	WriteBranchExit(*this);
}

void r_ret(const UDSPInstruction opc, DSPEmitter& emitter)
{
	emitter.dsp_reg_load_stack(DSP_STACK_C);
#ifdef _M_IX86 // All32
	emitter.MOV(16, M(&g_dsp.pc), R(DX));
#else
	emitter.MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	emitter.MOV(16, MatR(RAX), R(DX));
#endif
	WriteBranchExit(emitter);
}

// Generic ret implementation
// RETcc
// 0000 0010 1101 cccc
// Return from subroutine if condition cc has been met. Pops stored PC
// from call stack $st0 and sets $pc to this location.
// NOTE: Cannot use Default(opc) here because of the need to write branch exit
void DSPEmitter::ret(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), Imm16(compilePC + 1));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 1));
#endif
	ReJitConditional<r_ret>(opc, *this);
}

// RTI
// 0000 0010 1111 1111
// Return from exception. Pops stored status register $sr from data stack
// $st1 and program counter PC from call stack $st0 and sets $pc to this
// location.
void DSPEmitter::rti(const UDSPInstruction opc)
{
//	g_dsp.r[DSP_REG_SR] = dsp_reg_load_stack(DSP_STACK_D);
	dsp_reg_load_stack(DSP_STACK_D);
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.r.sr), R(DX));
#else
	// MOV(64, R(R11), ImmPtr(&g_dsp.r));
	MOV(16, MDisp(R11, STRUCT_OFFSET(g_dsp.r, sr)), R(DX));
#endif
//	g_dsp.pc = dsp_reg_load_stack(DSP_STACK_C);
	dsp_reg_load_stack(DSP_STACK_C);
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), R(DX));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), R(DX));
#endif
}

// HALT
// 0000 0000 0020 0001 
// Stops execution of DSP code. Sets bit DSP_CR_HALT in register DREG_CR.
void DSPEmitter::halt(const UDSPInstruction opc)
{
#ifdef _M_IX86 // All32
	OR(16, M(&g_dsp.cr), Imm16(4));
#else
	MOV(64, R(RAX), ImmPtr(&g_dsp.cr));
	OR(16, MatR(RAX), Imm16(4));
#endif
	//	g_dsp.pc = dsp_reg_load_stack(DSP_STACK_C);
	dsp_reg_load_stack(DSP_STACK_C);
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), R(DX));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), R(DX));
#endif
}

// LOOP handling: Loop stack is used to control execution of repeated blocks of
// instructions. Whenever there is value on stack $st2 and current PC is equal
// value at $st2, then value at stack $st3 is decremented. If value is not zero
// then PC is modified with value from call stack $st0. Otherwise values from
// call stack $st0 and both loop stacks $st2 and $st3 are popped and execution
// continues at next opcode.
void DSPEmitter::HandleLoop()
{
#ifdef _M_IX86 // All32
	MOVZX(32, 16, EAX, M(&g_dsp.r.st[2]));
	MOVZX(32, 16, ECX, M(&g_dsp.r.st[3]));
#else
	// MOV(64, R(R11), ImmPtr(&g_dsp.r));
	MOVZX(32, 16, EAX, MDisp(R11, STRUCT_OFFSET(g_dsp.r, st[2])));
	MOVZX(32, 16, ECX, MDisp(R11, STRUCT_OFFSET(g_dsp.r, st[3])));
#endif

	CMP(32, R(RCX), Imm32(0));
	FixupBranch rLoopCntG = J_CC(CC_LE, true);
	CMP(16, R(RAX), Imm16(compilePC - 1));
	FixupBranch rLoopAddrG = J_CC(CC_NE, true);

#ifdef _M_IX86 // All32
	SUB(16, M(&(g_dsp.r.st[3])), Imm16(1));
	CMP(16, M(&(g_dsp.r.st[3])), Imm16(0));
#else
	SUB(16, MDisp(R11, STRUCT_OFFSET(g_dsp.r, st[3])), Imm16(1));
	CMP(16, MDisp(R11, STRUCT_OFFSET(g_dsp.r, st[3])), Imm16(0));
#endif
	
	FixupBranch loadStack = J_CC(CC_LE, true);
#ifdef _M_IX86 // All32
	MOVZX(32, 16, ECX, M(&(g_dsp.r.st[0])));
	MOV(16, M(&g_dsp.pc), R(RCX));
#else
	MOVZX(32, 16, RCX, MDisp(R11, STRUCT_OFFSET(g_dsp.r, st[0])));
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), R(RCX));
#endif
	FixupBranch loopUpdated = J(true);

	SetJumpTarget(loadStack);
	dsp_reg_load_stack(0);
	dsp_reg_load_stack(2);
	dsp_reg_load_stack(3);

	SetJumpTarget(loopUpdated);
	SetJumpTarget(rLoopAddrG);
	SetJumpTarget(rLoopCntG);

}

// LOOP $R
// 0000 0000 010r rrrr
// Repeatedly execute following opcode until counter specified by value
// from register $R reaches zero. Each execution decrement counter. Register
// $R remains unchanged. If register $R is set to zero at the beginning of loop
// then looped instruction will not get executed.
// Actually, this instruction simply prepares the loop stacks for the above.
// The looping hardware takes care of the rest.
void DSPEmitter::loop(const UDSPInstruction opc)
{
	u16 reg = opc & 0x1f;
	u16 *regp = reg_ptr(reg);
//	u16 cnt = g_dsp.r[reg];
#ifdef _M_IX86 // All32
	MOVZX(32, 16, EDX, M(regp));
#else
	// MOV(64, R(R11), ImmPtr(&g_dsp.r));
	MOVZX(32, 16, EDX, MDisp(R11, PtrOffset(regp, &g_dsp.r)));
#endif
	u16 loop_pc = compilePC + 1;

	CMP(16, R(EDX), Imm16(0));
	FixupBranch cnt = J_CC(CC_Z, true);
	dsp_reg_store_stack(3);
	MOV(16, R(RDX), Imm16(compilePC + 1));
	dsp_reg_store_stack(0);
	MOV(16, R(RDX), Imm16(loop_pc));
	dsp_reg_store_stack(2);

	SetJumpTarget(cnt);
#ifdef _M_IX86 // All32
	MOV(16, M(&(g_dsp.pc)), Imm16(compilePC + 1));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 1));
#endif
}

// LOOPI #I
// 0001 0000 iiii iiii
// Repeatedly execute following opcode until counter specified by
// immediate value I reaches zero. Each execution decrement counter. If
// immediate value I is set to zero at the beginning of loop then looped
// instruction will not get executed.
// Actually, this instruction simply prepares the loop stacks for the above.
// The looping hardware takes care of the rest.
void DSPEmitter::loopi(const UDSPInstruction opc)
{
	u16 cnt = opc & 0xff;
	u16 loop_pc = compilePC + 1;

	if (cnt) 
	{
		MOV(16, R(RDX), Imm16(compilePC + 1));
		dsp_reg_store_stack(0);
		MOV(16, R(RDX), Imm16(loop_pc));
		dsp_reg_store_stack(2);
		MOV(16, R(RDX), Imm16(cnt));
		dsp_reg_store_stack(3);

#ifdef _M_IX86 // All32
		MOV(16, M(&(g_dsp.pc)), Imm16(compilePC + 1));
#else
		MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
		MOV(16, MatR(RAX), Imm16(compilePC + 1));
#endif
	}
}


// BLOOP $R, addrA
// 0000 0000 011r rrrr
// aaaa aaaa aaaa aaaa
// Repeatedly execute block of code starting at following opcode until
// counter specified by value from register $R reaches zero. Block ends at
// specified address addrA inclusive, ie. opcode at addrA is the last opcode
// included in loop. Counter is pushed on loop stack $st3, end of block address
// is pushed on loop stack $st2 and repeat address is pushed on call stack $st0.
// Up to 4 nested loops are allowed.
void DSPEmitter::bloop(const UDSPInstruction opc)
{
	u16 reg = opc & 0x1f;
	u16* regp = reg_ptr(reg);
//	u16 cnt = g_dsp.r[reg];
#ifdef _M_IX86 // All32
	MOVZX(32, 16, EDX, M(regp));
#else
	// MOV(64, R(R11), ImmPtr(&g_dsp.r));
	MOVZX(32, 16, EDX, MDisp(R11, PtrOffset(regp, &g_dsp.r)));
#endif
	u16 loop_pc = dsp_imem_read(compilePC + 1);

	CMP(16, R(EDX), Imm16(0));
	FixupBranch cnt = J_CC(CC_Z, true);
	dsp_reg_store_stack(3);
	MOV(16, R(RDX), Imm16(compilePC + 2));
	dsp_reg_store_stack(0);
	MOV(16, R(RDX), Imm16(loop_pc));
	dsp_reg_store_stack(2);
#ifdef _M_IX86 // All32
	MOV(16, M(&(g_dsp.pc)), Imm16(compilePC + 2));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(compilePC + 2));
#endif
	FixupBranch exit = J();

	SetJumpTarget(cnt);
	//		g_dsp.pc = loop_pc;
	//		dsp_skip_inst();
#ifdef _M_IX86 // All32
	MOV(16, M(&g_dsp.pc), Imm16(loop_pc + opTable[loop_pc]->size));
#else
	MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
	MOV(16, MatR(RAX), Imm16(loop_pc + opTable[loop_pc]->size));
#endif
	WriteBranchExit(*this);
	SetJumpTarget(exit);
}

// BLOOPI #I, addrA
// 0001 0001 iiii iiii
// aaaa aaaa aaaa aaaa
// Repeatedly execute block of code starting at following opcode until
// counter specified by immediate value I reaches zero. Block ends at specified
// address addrA inclusive, ie. opcode at addrA is the last opcode included in
// loop. Counter is pushed on loop stack $st3, end of block address is pushed
// on loop stack $st2 and repeat address is pushed on call stack $st0. Up to 4
// nested loops are allowed.
void DSPEmitter::bloopi(const UDSPInstruction opc)
{
	u16 cnt = opc & 0xff;
//	u16 loop_pc = dsp_fetch_code();
	u16 loop_pc = dsp_imem_read(compilePC + 1);

	if (cnt) 
	{
		MOV(16, R(RDX), Imm16(compilePC + 2));
		dsp_reg_store_stack(0);
		MOV(16, R(RDX), Imm16(loop_pc));
		dsp_reg_store_stack(2);
		MOV(16, R(RDX), Imm16(cnt));
		dsp_reg_store_stack(3);

#ifdef _M_IX86 // All32
		MOV(16, M(&(g_dsp.pc)), Imm16(compilePC + 2));
#else
		MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
		MOV(16, MatR(RAX), Imm16(compilePC + 2));
#endif
	}
	else
	{
//		g_dsp.pc = loop_pc;
//		dsp_skip_inst();
#ifdef _M_IX86 // All32
		MOV(16, M(&g_dsp.pc), Imm16(loop_pc + opTable[loop_pc]->size));
#else
		MOV(64, R(RAX), ImmPtr(&(g_dsp.pc)));
		MOV(16, MatR(RAX), Imm16(loop_pc + opTable[loop_pc]->size));
#endif
		WriteBranchExit(*this);
	}
}
