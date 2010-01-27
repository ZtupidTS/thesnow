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

// Additional copyrights go to Duddie and Tratax (c) 2004


#include "DSPInterpreter.h"

#include "DSPCore.h"
#include "DSPIntUtil.h"

namespace DSPInterpreter {

void unknown(const UDSPInstruction& opc)
{
	//_assert_msg_(MASTER_LOG, !g_dsp.exception_in_progress_hack, "assert while exception");
	ERROR_LOG(DSPLLE, "LLE: Unrecognized opcode 0x%04x, pc 0x%04x", opc.hex, g_dsp.pc);
}

// MRR $D, $S
// 0001 11dd ddds ssss
// Move value from register $S to register $D.
// FIXME: Perform additional operation depending on destination register.
void mrr(const UDSPInstruction& opc)
{
	u8 sreg = opc.hex & 0x1f;
	u8 dreg = (opc.hex >> 5) & 0x1f;

	u16 val = dsp_op_read_reg(sreg);
	dsp_op_write_reg(dreg, val);
	dsp_conditional_extend_accum(dreg);
}

// LRI $D, #I
// 0000 0000 100d dddd
// iiii iiii iiii iiii
// Load immediate value I to register $D. 
// FIXME: Perform additional operation depending on destination register.

// DSPSpy discovery: This, and possibly other instructions that load a
// register, has a different behaviour in S40 mode if loaded to AC0.M: The
// value gets sign extended to the whole accumulator! This does not happen in
// S16 mode.
void lri(const UDSPInstruction& opc)
{
	u8 reg  = opc.hex & DSP_REG_MASK;
	u16 imm = dsp_fetch_code();
	dsp_op_write_reg(reg, imm);
	dsp_conditional_extend_accum(reg);
}

// LRIS $(0x18+D), #I
// 0000 1ddd iiii iiii
// Load immediate value I (8-bit sign extended) to accumulator register. 
// FIXME: Perform additional operation depending on destination register.
void lris(const UDSPInstruction& opc)
{
	u8 reg  = ((opc.hex >> 8) & 0x7) + DSP_REG_AXL0;
	u16 imm = (s8)opc.hex;
	dsp_op_write_reg(reg, imm);
	dsp_conditional_extend_accum(reg);
}


// TSTAXL $acR
// 1000 r001 xxxx xxxx
// r specifies one of the main accumulators.
// Definitely not a test instruction - it changes the accums.
// Not affected by m0/m2. Not affected by s16/s40.
void tstaxl(const UDSPInstruction& opc)
{
	// This is probably all wrong.
	//u8 reg  = (opc.hex >> 8) & 0x1;
	//s16 val = dsp_get_ax_l(reg);
	//Update_SR_Register16(val);
}

// ADDARN $arD, $ixS
// 0000 0000 0001 ssdd
// Adds indexing register $ixS to an addressing register $arD.
void addarn(const UDSPInstruction& opc)
{
	u8 dreg = opc.hex & 0x3;
	u8 sreg = (opc.hex >> 2) & 0x3;

	g_dsp.r[dreg] = dsp_increase_addr_reg(dreg, (s16)g_dsp.r[DSP_REG_IX0 + sreg]);

	// It is critical for the Zelda ucode that this one wraps correctly.
}

// NX
// 1000 -000 xxxx xxxx
// No operation, but can be extended with extended opcode.
void nx(const UDSPInstruction& opc)
{
	zeroWriteBackLog();
	// This opcode is supposed to do nothing - it's used if you want to use
	// an opcode extension but not do anything. At least according to duddie.
}

//-------------------------------------------------------------
// DAR $arD  ?
// 0000 0000 0000 01dd
// Decrement address register $arD.
void dar(const UDSPInstruction& opc)
{
	g_dsp.r[opc.hex & 0x3] = dsp_decrement_addr_reg(opc.hex & 0x3);
}

// IAR $arD  ?
// 0000 0000 0000 10dd
// Increment address register $arD.
void iar(const UDSPInstruction& opc)
{
	g_dsp.r[opc.hex & 0x3] = dsp_increment_addr_reg(opc.hex & 0x3);
}

// XAR $arD  ?
// 0000 0000 0000 11dd
// $arD result somehow depends on $wrD
// unknown atm
// used in IPL ucode
void xar(const UDSPInstruction& opc)
{
//	u8 dreg = opc.hex & 0x3;
}

// SBCLR #I
// 0001 0011 0000 0iii
// bit of status register $sr. Bit number is calculated by adding 6 to
// immediate value I.
void sbclr(const UDSPInstruction& opc)
{
	u8 bit = (opc.hex & 0xff) + 6;
	g_dsp.r[DSP_REG_SR] &= ~(1 << bit);
}

// SBSET #I
// 0001 0010 0000 0iii
// Set bit of status register $sr. Bit number is calculated by adding 6 to
// immediate value I.
void sbset(const UDSPInstruction& opc)
{
	u8 bit = (opc.hex & 0xff) + 6;
	g_dsp.r[DSP_REG_SR] |= (1 << bit);
}


// This is a bunch of flag setters, flipping bits in SR. So far so good,
// but it's harder to know exactly what effect they have.
void srbith(const UDSPInstruction& opc)
{
	zeroWriteBackLog();
	switch ((opc.hex >> 8) & 0xf)
	{
	// M0/M2 change the multiplier mode (it can multiply by 2 for free).
	case 0xa:  // M2
		g_dsp.r[DSP_REG_SR] &= ~SR_MUL_MODIFY;
		break;
	case 0xb:  // M0
		g_dsp.r[DSP_REG_SR] |= SR_MUL_MODIFY;
		break;

	// If set, treat multiplicands as unsigned.
	// If clear, treat them as signed.
	case 0xc:  // CLR15
		g_dsp.r[DSP_REG_SR] &= ~SR_MUL_UNSIGNED;
		break;
	case 0xd:  // SET15
		g_dsp.r[DSP_REG_SR] |= SR_MUL_UNSIGNED;
		break;

	// Automatic 40-bit sign extension when loading ACx.M.
    // SET40 changes something very important: see the LRI instruction above.
	case 0xe:  // SET16 (CLR40)
		g_dsp.r[DSP_REG_SR] &= ~SR_40_MODE_BIT;
		break;

	case 0xf:  // SET40  
		g_dsp.r[DSP_REG_SR] |= SR_40_MODE_BIT;
		break;

	default:
		break;
	}
}

}  // namespace
