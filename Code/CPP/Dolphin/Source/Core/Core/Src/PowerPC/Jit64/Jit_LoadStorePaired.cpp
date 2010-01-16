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

// TODO(ector): Tons of pshufb optimization of the loads/stores, for SSSE3+, possibly SSE4, only.
// Should give a very noticable speed boost to paired single heavy code.

#include "Common.h"

#include "Thunk.h"
#include "../PowerPC.h"
#include "../../Core.h"
#include "../../HW/GPFifo.h"
#include "../../HW/Memmap.h"
#include "../PPCTables.h"
#include "CPUDetect.h"
#include "x64Emitter.h"
#include "ABI.h"

#include "Jit.h"
#include "JitAsm.h"
#include "JitRegCache.h"

const u8 GC_ALIGNED16(pbswapShuffle2x4[16]) = {3, 2, 1, 0, 7, 6, 5, 4, 8, 9, 10, 11, 12, 13, 14, 15};
const u8 GC_ALIGNED16(pbswapShuffleNoop[16]) = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

static double GC_ALIGNED16(psTemp[2]) = {1.0, 1.0};
static u64 GC_ALIGNED16(temp64);

// TODO(ector): Improve 64-bit version
static void WriteDual32(u64 value, u32 address)
{
	Memory::Write_U32((u32)(value >> 32), address);
	Memory::Write_U32((u32)value, address + 4);
}

const double GC_ALIGNED16(m_quantizeTableD[]) =
{
	(1 <<  0),	(1 <<  1),	(1 <<  2),	(1 <<  3),
	(1 <<  4),	(1 <<  5),	(1 <<  6),	(1 <<  7),
	(1 <<  8),	(1 <<  9),	(1 << 10),	(1 << 11),
	(1 << 12),	(1 << 13),	(1 << 14),	(1 << 15),
	(1 << 16),	(1 << 17),	(1 << 18),	(1 << 19),
	(1 << 20),	(1 << 21),	(1 << 22),	(1 << 23),
	(1 << 24),	(1 << 25),	(1 << 26),	(1 << 27),
	(1 << 28),	(1 << 29),	(1 << 30),	(1 << 31),
	1.0 / (1ULL << 32),	1.0 / (1 << 31),	1.0 / (1 << 30),	1.0 / (1 << 29),
	1.0 / (1 << 28),	1.0 / (1 << 27),	1.0 / (1 << 26),	1.0 / (1 << 25),
	1.0 / (1 << 24),	1.0 / (1 << 23),	1.0 / (1 << 22),	1.0 / (1 << 21),
	1.0 / (1 << 20),	1.0 / (1 << 19),	1.0 / (1 << 18),	1.0 / (1 << 17),
	1.0 / (1 << 16),	1.0 / (1 << 15),	1.0 / (1 << 14),	1.0 / (1 << 13),
	1.0 / (1 << 12),	1.0 / (1 << 11),	1.0 / (1 << 10),	1.0 / (1 <<  9),
	1.0 / (1 <<  8),	1.0 / (1 <<  7),	1.0 / (1 <<  6),	1.0 / (1 <<  5),
	1.0 / (1 <<  4),	1.0 / (1 <<  3),	1.0 / (1 <<  2),	1.0 / (1 <<  1),
}; 

const double GC_ALIGNED16(m_dequantizeTableD[]) =
{
	1.0 / (1 <<  0),	1.0 / (1 <<  1),	1.0 / (1 <<  2),	1.0 / (1 <<  3),
	1.0 / (1 <<  4),	1.0 / (1 <<  5),	1.0 / (1 <<  6),	1.0 / (1 <<  7),
	1.0 / (1 <<  8),	1.0 / (1 <<  9),	1.0 / (1 << 10),	1.0 / (1 << 11),
	1.0 / (1 << 12),	1.0 / (1 << 13),	1.0 / (1 << 14),	1.0 / (1 << 15),
	1.0 / (1 << 16),	1.0 / (1 << 17),	1.0 / (1 << 18),	1.0 / (1 << 19),
	1.0 / (1 << 20),	1.0 / (1 << 21),	1.0 / (1 << 22),	1.0 / (1 << 23),
	1.0 / (1 << 24),	1.0 / (1 << 25),	1.0 / (1 << 26),	1.0 / (1 << 27),
	1.0 / (1 << 28),	1.0 / (1 << 29),	1.0 / (1 << 30),	1.0 / (1 << 31),
	(1ULL << 32),	(1 << 31),		(1 << 30),		(1 << 29),
	(1 << 28),		(1 << 27),		(1 << 26),		(1 << 25),
	(1 << 24),		(1 << 23),		(1 << 22),		(1 << 21),
	(1 << 20),		(1 << 19),		(1 << 18),		(1 << 17),
	(1 << 16),		(1 << 15),		(1 << 14),		(1 << 13),
	(1 << 12),		(1 << 11),		(1 << 10),		(1 <<  9),
	(1 <<  8),		(1 <<  7),		(1 <<  6),		(1 <<  5),
	(1 <<  4),		(1 <<  3),		(1 <<  2),		(1 <<  1),
};  

// The big problem is likely instructions that set the quantizers in the same block.
// We will have to break block after quantizers are written to.
void Jit64::psq_st(UGeckoInstruction inst)
{
	INSTRUCTION_START
	JITDISABLE(LoadStorePaired)
	js.block_flags |= BLOCK_USE_GQR0 << inst.I;

	if (js.blockSetsQuantizers || !Core::GetStartupParameter().bOptimizeQuantizers)
	{
		Default(inst);
		return;
	}
	if (!inst.RA)
	{
		// This really should never happen. Unless we change this to also support stwux
		Default(inst);
		return;
	}

	const UGQR gqr(rSPR(SPR_GQR0 + inst.I));
	const EQuantizeType stType = static_cast<EQuantizeType>(gqr.ST_TYPE);
	int stScale = gqr.ST_SCALE;
	bool update = inst.OPCD == 61;

	int offset = inst.SIMM_12;
	int a = inst.RA;
	int s = inst.RS; // Fp numbers

	if (inst.W) {
		// PanicAlert("W=1: stType %i stScale %i update %i", (int)stType, (int)stScale, (int)update); 
		// It's fairly common that games write stuff to the pipe using this. Then, it's pretty much only
		// floats so that's what we'll work on.
		switch (stType)
		{
		case QUANTIZE_FLOAT:
			{
			// This one has quite a bit of optimization potential.
			if (gpr.R(a).IsImm())
			{
				PanicAlert("Imm: %08x", gpr.R(a).offset);
			}
			gpr.FlushLockX(ABI_PARAM1, ABI_PARAM2);
			gpr.Lock(a);
			fpr.Lock(s);
			if (update)
				gpr.LoadToX64(a, true, true);
			MOV(32, R(ABI_PARAM2), gpr.R(a));
			if (offset)
				ADD(32, R(ABI_PARAM2), Imm32((u32)offset));
			TEST(32, R(ABI_PARAM2), Imm32(0x0C000000));
			if (update && offset)
				MOV(32, gpr.R(a), R(ABI_PARAM2));
			CVTSD2SS(XMM0, fpr.R(s));
			MOVD_xmm(M(&temp64), XMM0);
			MOV(32, R(ABI_PARAM1), M(&temp64));
			FixupBranch argh = J_CC(CC_NZ);
			BSWAP(32, ABI_PARAM1);
#ifdef _M_X64
			MOV(32, MComplex(RBX, ABI_PARAM2, SCALE_1, 0), R(ABI_PARAM1));
#else
			MOV(32, R(EAX), R(ABI_PARAM2));
			AND(32, R(EAX), Imm32(Memory::MEMVIEW32_MASK));
			MOV(32, MDisp(EAX, (u32)Memory::base), R(ABI_PARAM1));
#endif
			FixupBranch skip_call = J();
			SetJumpTarget(argh);
			ABI_CallFunctionRR(thunks.ProtectFunction((void *)&Memory::Write_U32, 2), ABI_PARAM1, ABI_PARAM2); 
			SetJumpTarget(skip_call);
			gpr.UnlockAll();
			gpr.UnlockAllX();
			fpr.UnlockAll();
			return;
			}
		default:
			Default(inst);
			return;
		}
		return;
	}

	if (stType == QUANTIZE_FLOAT)
	{
		if (gpr.R(a).IsImm() && !update && cpu_info.bSSSE3)
		{
			u32 addr = (u32)(gpr.R(a).offset + offset);
			if (addr == 0xCC008000) {
				// Writing to FIFO. Let's do fast method.
				CVTPD2PS(XMM0, fpr.R(s));
				PSHUFB(XMM0, M((void*)&pbswapShuffle2x4));
				CALL((void*)asm_routines.fifoDirectWriteXmm64);
				js.fifoBytesThisBlock += 8;
				return;
			}
		}

		gpr.FlushLockX(ABI_PARAM1, ABI_PARAM2);
		gpr.Lock(a);
		fpr.Lock(s);
		if (update)
			gpr.LoadToX64(a, true, true);
		MOV(32, R(ABI_PARAM2), gpr.R(a));
		if (offset)
			ADD(32, R(ABI_PARAM2), Imm32((u32)offset));
		TEST(32, R(ABI_PARAM2), Imm32(0x0C000000));
		if (update && offset)
			MOV(32, gpr.R(a), R(ABI_PARAM2));
		CVTPD2PS(XMM0, fpr.R(s));
		SHUFPS(XMM0, R(XMM0), 1);
		MOVQ_xmm(M(&temp64), XMM0);
#ifdef _M_X64
		MOV(64, R(ABI_PARAM1), M(&temp64));
		FixupBranch argh = J_CC(CC_NZ);
		BSWAP(64, ABI_PARAM1);
		MOV(64, MComplex(RBX, ABI_PARAM2, SCALE_1, 0), R(ABI_PARAM1));
		FixupBranch arg2 = J();
		SetJumpTarget(argh);
		CALL(thunks.ProtectFunction((void *)&WriteDual32, 0));
#else
		FixupBranch argh = J_CC(CC_NZ);
		MOV(32, R(ABI_PARAM1), M(((char*)&temp64) + 4));
		BSWAP(32, ABI_PARAM1);
		AND(32, R(ABI_PARAM2), Imm32(Memory::MEMVIEW32_MASK));
		MOV(32, MDisp(ABI_PARAM2, (u32)Memory::base), R(ABI_PARAM1));
		MOV(32, R(ABI_PARAM1), M(&temp64));
		BSWAP(32, ABI_PARAM1);
		MOV(32, MDisp(ABI_PARAM2, 4+(u32)Memory::base), R(ABI_PARAM1));
		FixupBranch arg2 = J();
		SetJumpTarget(argh);
		MOV(32, R(ABI_PARAM1), M(((char*)&temp64) + 4));
		ABI_CallFunctionRR(thunks.ProtectFunction((void *)&Memory::Write_U32, 2), ABI_PARAM1, ABI_PARAM2); 
		MOV(32, R(ABI_PARAM1), M(((char*)&temp64)));
		ADD(32, R(ABI_PARAM2), Imm32(4));
		ABI_CallFunctionRR(thunks.ProtectFunction((void *)&Memory::Write_U32, 2), ABI_PARAM1, ABI_PARAM2); 
#endif
		SetJumpTarget(arg2);
		gpr.UnlockAll();
		gpr.UnlockAllX();
		fpr.UnlockAll();
	}
	else if (stType == QUANTIZE_U8)
	{
		gpr.FlushLockX(ABI_PARAM1, ABI_PARAM2);
		gpr.Lock(a);
		fpr.Lock(s);
		if (update)
			gpr.LoadToX64(a, true, update);
		MOV(32, R(ABI_PARAM2), gpr.R(a));
		if (offset)
			ADD(32, R(ABI_PARAM2), Imm32((u32)offset));
		if (update && offset)
			MOV(32, gpr.R(a), R(ABI_PARAM2));
		MOVAPD(XMM0, fpr.R(s));
		MOVDDUP(XMM1, M((void*)&m_quantizeTableD[stScale]));
		MULPD(XMM0, R(XMM1));
		CVTPD2DQ(XMM0, R(XMM0));
		PACKSSDW(XMM0, R(XMM0));
		PACKUSWB(XMM0, R(XMM0));
		MOVD_xmm(M(&temp64), XMM0);
		MOV(16, R(ABI_PARAM1), M(&temp64));
#ifdef _M_X64
		MOV(16, MComplex(RBX, ABI_PARAM2, SCALE_1, 0), R(ABI_PARAM1));
#else
		MOV(32, R(EAX), R(ABI_PARAM2));
		AND(32, R(EAX), Imm32(Memory::MEMVIEW32_MASK));
		MOV(16, MDisp(EAX, (u32)Memory::base), R(ABI_PARAM1));
#endif
		if (update)
			MOV(32, gpr.R(a), R(ABI_PARAM2));
		gpr.UnlockAll();
		gpr.UnlockAllX();
		fpr.UnlockAll();
	} 
	else if (stType == QUANTIZE_S16)
	{
		gpr.FlushLockX(ABI_PARAM1, ABI_PARAM2);
		gpr.Lock(a);
		fpr.Lock(s);
		if (update)
			gpr.LoadToX64(a, true, update);
		MOV(32, R(ABI_PARAM2), gpr.R(a));
		if (offset)
			ADD(32, R(ABI_PARAM2), Imm32((u32)offset));
		if (update)
			MOV(32, gpr.R(a), R(ABI_PARAM2));
		MOVAPD(XMM0, fpr.R(s));
		MOVDDUP(XMM1, M((void*)&m_quantizeTableD[stScale]));
		MULPD(XMM0, R(XMM1));
		SHUFPD(XMM0, R(XMM0), 1);
		CVTPD2DQ(XMM0, R(XMM0));
		PACKSSDW(XMM0, R(XMM0));
		MOVD_xmm(M(&temp64), XMM0);
		MOV(32, R(ABI_PARAM1), M(&temp64));
		SafeWriteRegToReg(ABI_PARAM1, ABI_PARAM2, 32, 0);
		gpr.UnlockAll();
		gpr.UnlockAllX();
		fpr.UnlockAll();
	}
	else {
		// Dodger uses this.
        // mario tennis
		//PanicAlert("st %i:%i", stType, inst.W);
		Default(inst);
	}
}

void Jit64::psq_l(UGeckoInstruction inst)
{
	INSTRUCTION_START
	JITDISABLE(LoadStorePaired)

	js.block_flags |= BLOCK_USE_GQR0 << inst.I;

	if (js.blockSetsQuantizers || !Core::GetStartupParameter().bOptimizeQuantizers)
	{
		Default(inst);
		return;
	}

	const UGQR gqr(rSPR(SPR_GQR0 + inst.I));
	const EQuantizeType ldType = static_cast<EQuantizeType>(gqr.LD_TYPE);
	int ldScale = gqr.LD_SCALE;
	bool update = inst.OPCD == 57;
	if (!inst.RA || inst.W)
	{
		// 0 1 during load
		//PanicAlert("ld:%i %i", ldType, (int)inst.W);
		Default(inst);
		return;
	}
	int offset = inst.SIMM_12;
	switch (ldType) {
		case QUANTIZE_FLOAT:  // We know this is from RAM, so we don't need to check the address.
			{
#ifdef _M_X64
			gpr.LoadToX64(inst.RA, true, update);
			fpr.LoadToX64(inst.RS, false);
			if (cpu_info.bSSSE3) {
				X64Reg xd = fpr.R(inst.RS).GetSimpleReg();
				MOVQ_xmm(xd, MComplex(RBX, gpr.R(inst.RA).GetSimpleReg(), 1, offset));
				PSHUFB(xd, M((void *)pbswapShuffle2x4));
				CVTPS2PD(xd, R(xd));
			} else {
				MOV(64, R(RAX), MComplex(RBX, gpr.R(inst.RA).GetSimpleReg(), 1, offset));
				BSWAP(64, RAX);
				MOV(64, M(&psTemp[0]), R(RAX));
				X64Reg r = fpr.R(inst.RS).GetSimpleReg();
				CVTPS2PD(r, M(&psTemp[0]));
				SHUFPD(r, R(r), 1);
			}
			if (update && offset != 0)
				ADD(32, gpr.R(inst.RA), Imm32(offset));
			break;
#else
			if (cpu_info.bSSSE3) {
				gpr.LoadToX64(inst.RA, true, update);
				fpr.LoadToX64(inst.RS, false);
				X64Reg xd = fpr.R(inst.RS).GetSimpleReg();
				MOV(32, R(EAX), gpr.R(inst.RA));
				AND(32, R(EAX), Imm32(Memory::MEMVIEW32_MASK));
				MOVQ_xmm(xd, MDisp(EAX, (u32)Memory::base + offset));
				PSHUFB(xd, M((void *)pbswapShuffle2x4));
				CVTPS2PD(xd, R(xd));
			} else {
				gpr.FlushLockX(ECX);
				gpr.LoadToX64(inst.RA, true, update);
				// This can probably be optimized somewhat.
				LEA(32, ECX, MDisp(gpr.R(inst.RA).GetSimpleReg(), offset));
				AND(32, R(ECX), Imm32(Memory::MEMVIEW32_MASK));
				MOV(32, R(EAX), MDisp(ECX, (u32)Memory::base));
				BSWAP(32, RAX);
				MOV(32, M(&psTemp[0]), R(RAX));
				MOV(32, R(EAX), MDisp(ECX, (u32)Memory::base + 4));
				BSWAP(32, RAX);
				MOV(32, M(((float *)&psTemp[0]) + 1), R(RAX));
				fpr.LoadToX64(inst.RS, false, true);
				X64Reg r = fpr.R(inst.RS).GetSimpleReg();
				CVTPS2PD(r, M(&psTemp[0]));
				gpr.UnlockAllX();
			}
			if (update && offset != 0)
				ADD(32, gpr.R(inst.RA), Imm32(offset));
			break;
#endif
			}
		case QUANTIZE_U8:
			{
			gpr.LoadToX64(inst.RA, true, update);
#ifdef _M_X64
			MOVZX(32, 16, EAX, MComplex(RBX, gpr.R(inst.RA).GetSimpleReg(), 1, offset));
#else
			LEA(32, EAX, MDisp(gpr.R(inst.RA).GetSimpleReg(), offset));
			AND(32, R(EAX), Imm32(Memory::MEMVIEW32_MASK));
			MOVZX(32, 16, EAX, MDisp(EAX, (u32)Memory::base));
#endif
			MOV(32, M(&temp64), R(EAX));
			MOVD_xmm(XMM0, M(&temp64));
			// SSE4 optimization opportunity here.
			PXOR(XMM1, R(XMM1));
			PUNPCKLBW(XMM0, R(XMM1));
			PUNPCKLWD(XMM0, R(XMM1));
			CVTDQ2PD(XMM0, R(XMM0));
			fpr.LoadToX64(inst.RS, false, true);
			X64Reg r = fpr.R(inst.RS).GetSimpleReg();
			MOVDDUP(r, M((void *)&m_dequantizeTableD[ldScale]));
			MULPD(r, R(XMM0));
			if (update && offset != 0)
				ADD(32, gpr.R(inst.RA), Imm32(offset));
			}
			break;
		case QUANTIZE_S16:
			{
			gpr.LoadToX64(inst.RA, true, update);
#ifdef _M_X64
			MOV(32, R(EAX), MComplex(RBX, gpr.R(inst.RA).GetSimpleReg(), 1, offset));
#else
			LEA(32, EAX, MDisp(gpr.R(inst.RA).GetSimpleReg(), offset));
			AND(32, R(EAX), Imm32(Memory::MEMVIEW32_MASK));
			MOV(32, R(EAX), MDisp(EAX, (u32)Memory::base));
#endif
			BSWAP(32, EAX);
			MOV(32, M(&temp64), R(EAX));
			fpr.LoadToX64(inst.RS, false, true);
			X64Reg r = fpr.R(inst.RS).GetSimpleReg();
			MOVD_xmm(XMM0, M(&temp64));
			PUNPCKLWD(XMM0, R(XMM0)); // unpack to higher word in each dword..
			PSRAD(XMM0, 16);          // then use this signed shift to sign extend. clever eh? :P
			CVTDQ2PD(XMM0, R(XMM0));
			MOVDDUP(r, M((void*)&m_dequantizeTableD[ldScale]));
			MULPD(r, R(XMM0));
			SHUFPD(r, R(r), 1);
			if (update && offset != 0)
				ADD(32, gpr.R(inst.RA), Imm32(offset));
			}
			break;

			/*
			Dynamic quantizer. Todo when we have a test set.
			MOVZX(32, 8, EAX, M(((char *)&PowerPC::ppcState.spr[SPR_GQR0 + inst.I]) + 3));  // it's in the high byte.
			AND(32, R(EAX), Imm8(0x3F));
			MOV(32, R(ECX), Imm32((u32)&m_dequantizeTableD));
			MOVDDUP(r, MComplex(RCX, EAX, 8, 0));
			*/
		default:
			// 4 0
			// 6 0 //power tennis
			// 5 0 
			// PanicAlert("ld:%i %i", ldType, (int)inst.W);
			Default(inst);
			return;
	}

	//u32 EA = (m_GPR[_inst.RA] + _inst.SIMM_12) : _inst.SIMM_12;
}
