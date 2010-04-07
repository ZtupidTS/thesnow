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
#include "DSPIntUtil.h"
#include "DSPMemoryMap.h"
#include "DSPIntExtOps.h"

// Extended opcodes do not exist on their own. These opcodes can only be
// attached to opcodes that allow extending (8 lower bits of opcode not used by
// opcode). Extended opcodes do not modify program counter $pc register.

// Most of the suffixes increment or decrement one or more addressing registers
// (the first four, ARx). The increment/decrement is either 1, or the
// corresponding "index" register (the second four, IXx). The addressing
// registers will wrap in odd ways, dictated by the corresponding wrapping
// register, WP0-3.

// The following should be applied as a decrement (and is applied by
// dsp_decrement_addr_reg):
// ar[i] = (ar[i] & wp[i]) == 0 ? ar[i] | wp[i] : ar[i] - 1;
// I have not found the corresponding algorithms for increments yet.
// It's gotta be fairly simple though. See R3123, R3125 in Google Code.
// (May have something to do with (ar[i] ^ wp[i]) == 0)


namespace DSPInterpreter
{

namespace Ext 
{

inline bool IsSameMemArea(u16 a, u16 b)
{
//LM: tested on WII
	if ((a>>10)==(b>>10))
		return true;
	else
		return false;
}

// DR $arR
// xxxx xxxx 0000 01rr
// Decrement addressing register $arR.
void dr(const UDSPInstruction opc) {
	writeToBackLog(0, opc & 0x3, dsp_decrement_addr_reg(opc & 0x3));
}

// IR $arR
// xxxx xxxx 0000 10rr
// Increment addressing register $arR.
void ir(const UDSPInstruction opc) {
	writeToBackLog(0, opc & 0x3, dsp_increment_addr_reg(opc & 0x3));
}

// NR $arR
// xxxx xxxx 0000 11rr
// Add corresponding indexing register $ixR to addressing register $arR.
void nr(const UDSPInstruction opc) {
	u8 reg = opc & 0x3;	
	
	writeToBackLog(0, reg, dsp_increase_addr_reg(reg, (s16)g_dsp.r[DSP_REG_IX0 + reg]));
}

// MV $axD.D, $acS.S
// xxxx xxxx 0001 ddss
// Move value of $acS.S to the $axD.D.
void mv(const UDSPInstruction opc)
{
 	u8 sreg = (opc & 0x3) + DSP_REG_ACL0;
	u8 dreg = ((opc >> 2) & 0x3);

#if 0 //more tests 
	if ((sreg >= DSP_REG_ACM0) && (g_dsp.r[DSP_REG_SR] & SR_40_MODE_BIT)) 
		writeToBackLog(0, dreg + DSP_REG_AXL0, ((u16)dsp_get_acc_h(sreg-DSP_REG_ACM0) & 0x0080) ? 0x8000 : 0x7fff);
	else
#endif	
		writeToBackLog(0, dreg + DSP_REG_AXL0, g_dsp.r[sreg]);
}
	
// S @$arD, $acS.S
// xxxx xxxx 001s s0dd
// Store value of $acS.S in the memory pointed by register $arD.
// Post increment register $arD.
void s(const UDSPInstruction opc)
{
	u8 dreg = opc & 0x3;
	u8 sreg = ((opc >> 3) & 0x3) + DSP_REG_ACL0;

	dsp_dmem_write(g_dsp.r[dreg], g_dsp.r[sreg]);
	writeToBackLog(0, dreg,	dsp_increment_addr_reg(dreg));
}

// SN @$arD, $acS.S
// xxxx xxxx 001s s1dd
// Store value of register $acS.S in the memory pointed by register $arD.
// Add indexing register $ixD to register $arD.
void sn(const UDSPInstruction opc)
{
	u8 dreg = opc & 0x3;
	u8 sreg = ((opc >> 3) & 0x3) + DSP_REG_ACL0;

	dsp_dmem_write(g_dsp.r[dreg], g_dsp.r[sreg]);
	writeToBackLog(0, dreg,	dsp_increase_addr_reg(dreg, (s16)g_dsp.r[DSP_REG_IX0 + dreg]));
}

// L $axD.D, @$arS
// xxxx xxxx 01dd d0ss
// Load $axD.D/$acD.D with value from memory pointed by register $arS. 
// Post increment register $arS.
void l(const UDSPInstruction opc)
{
	u8 sreg = opc & 0x3;
	u8 dreg = ((opc >> 3) & 0x7) + DSP_REG_AXL0;
	
	if ((dreg >= DSP_REG_ACM0) && (g_dsp.r[DSP_REG_SR] & SR_40_MODE_BIT)) 
	{
		u16 val = dsp_dmem_read(g_dsp.r[sreg]);
		writeToBackLog(0, dreg - DSP_REG_ACM0 + DSP_REG_ACH0, (val & 0x8000) ? 0xFFFF : 0x0000);
		writeToBackLog(1, dreg,	val);
		writeToBackLog(2, dreg - DSP_REG_ACM0 + DSP_REG_ACL0, 0);
		writeToBackLog(3, sreg, dsp_increment_addr_reg(sreg));
	}
	else
	{
		writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[sreg]));
		writeToBackLog(1, sreg, dsp_increment_addr_reg(sreg));
	}
}

// LN $axD.D, @$arS
// xxxx xxxx 01dd d0ss
// Load $axD.D/$acD.D with value from memory pointed by register $arS. 
// Add indexing register register $ixS to register $arS.
void ln(const UDSPInstruction opc)
{
	u8 sreg = opc & 0x3;
	u8 dreg = ((opc >> 3) & 0x7) + DSP_REG_AXL0;

	if ((dreg >= DSP_REG_ACM0) && (g_dsp.r[DSP_REG_SR] & SR_40_MODE_BIT)) 
	{
		u16 val = dsp_dmem_read(g_dsp.r[sreg]);
		writeToBackLog(0, dreg - DSP_REG_ACM0 + DSP_REG_ACH0, (val & 0x8000) ? 0xFFFF : 0x0000);
		writeToBackLog(1, dreg,	val);
		writeToBackLog(2, dreg - DSP_REG_ACM0 + DSP_REG_ACL0, 0);
		writeToBackLog(3, sreg, dsp_increase_addr_reg(sreg, (s16)g_dsp.r[DSP_REG_IX0 + sreg]));
	}
	else
	{
		writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[sreg]));
		writeToBackLog(1, sreg, dsp_increase_addr_reg(sreg, (s16)g_dsp.r[DSP_REG_IX0 + sreg]));
	}
}

// LS $axD.D, $acS.m
// xxxx xxxx 10dd 000s
// Load register $axD.D with value from memory pointed by register
// $ar0. Store value from register $acS.m to memory location pointed by
// register $ar3. Increment both $ar0 and $ar3.
void ls(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;

	dsp_dmem_write(g_dsp.r[DSP_REG_AR3], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR0]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increment_addr_reg(DSP_REG_AR3));
	writeToBackLog(2, DSP_REG_AR0, dsp_increment_addr_reg(DSP_REG_AR0)); 
}


// LSN $axD.D, $acS.m
// xxxx xxxx 10dd 010s
// Load register $axD.D with value from memory pointed by register
// $ar0. Store value from register $acS.m to memory location pointed by
// register $ar3. Add corresponding indexing register $ix0 to addressing
// register $ar0 and increment $ar3.
void lsn(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;

	dsp_dmem_write(g_dsp.r[DSP_REG_AR3], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR0]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increment_addr_reg(DSP_REG_AR3));
	writeToBackLog(2, DSP_REG_AR0, dsp_increase_addr_reg(DSP_REG_AR0, (s16)g_dsp.r[DSP_REG_IX0]));
}

// LSM $axD.D, $acS.m
// xxxx xxxx 10dd 100s
// Load register $axD.D with value from memory pointed by register
// $ar0. Store value from register $acS.m to memory location pointed by
// register $ar3. Add corresponding indexing register $ix3 to addressing
// register $ar3 and increment $ar0.
void lsm(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;
	
	dsp_dmem_write(g_dsp.r[DSP_REG_AR3], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR0]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increase_addr_reg(DSP_REG_AR3, (s16)g_dsp.r[DSP_REG_IX0 + DSP_REG_AR3]));
	writeToBackLog(2, DSP_REG_AR0, dsp_increment_addr_reg(DSP_REG_AR0));
}

// LSMN $axD.D, $acS.m
// xxxx xxxx 10dd 110s
// Load register $axD.D with value from memory pointed by register
// $ar0. Store value from register $acS.m to memory location pointed by
// register $ar3. Add corresponding indexing register $ix0 to addressing
// register $ar0 and add corresponding indexing register $ix3 to addressing
// register $ar3.
void lsnm(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;
	
	dsp_dmem_write(g_dsp.r[DSP_REG_AR3], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR0]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increase_addr_reg(DSP_REG_AR3, (s16)g_dsp.r[DSP_REG_IX0 + DSP_REG_AR3]));
	writeToBackLog(2, DSP_REG_AR0, dsp_increase_addr_reg(DSP_REG_AR0, (s16)g_dsp.r[DSP_REG_IX0]));
}

// SL $acS.m, $axD.D
// xxxx xxxx 10dd 001s
// Store value from register $acS.m to memory location pointed by register
// $ar0. Load register $axD.D with value from memory pointed by register
// $ar3. Increment both $ar0 and $ar3.
void sl(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;
	
	dsp_dmem_write(g_dsp.r[DSP_REG_AR0], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increment_addr_reg(DSP_REG_AR3));
	writeToBackLog(2, DSP_REG_AR0, dsp_increment_addr_reg(DSP_REG_AR0)); 
}

// SLN $acS.m, $axD.D
// xxxx xxxx 10dd 011s
// Store value from register $acS.m to memory location pointed by register
// $ar0. Load register $axD.D with value from memory pointed by register
// $ar3. Add corresponding indexing register $ix0 to addressing register $ar0
// and increment $ar3.
void sln(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;
	
	dsp_dmem_write(g_dsp.r[DSP_REG_AR0], g_dsp.r[sreg]);
	
	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increment_addr_reg(DSP_REG_AR3));
	writeToBackLog(2, DSP_REG_AR0, dsp_increase_addr_reg(DSP_REG_AR0, (s16)g_dsp.r[DSP_REG_IX0]));
}

// SLM $acS.m, $axD.D
// xxxx xxxx 10dd 101s
// Store value from register $acS.m to memory location pointed by register
// $ar0. Load register $axD.D with value from memory pointed by register
// $ar3. Add corresponding indexing register $ix3 to addressing register $ar3
// and increment $ar0.
void slm(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;
	
	dsp_dmem_write(g_dsp.r[DSP_REG_AR0], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increase_addr_reg(DSP_REG_AR3, (s16)g_dsp.r[DSP_REG_IX0 + DSP_REG_AR3]));
	writeToBackLog(2, DSP_REG_AR0, dsp_increment_addr_reg(DSP_REG_AR0));
}

// SLMN $acS.m, $axD.D
// xxxx xxxx 10dd 111s
// Store value from register $acS.m to memory location pointed by register
// $ar0. Load register $axD.D with value from memory pointed by register
// $ar3. Add corresponding indexing register $ix0 to addressing register $ar0
// and add corresponding indexing register $ix3 to addressing register $ar3.
void slnm(const UDSPInstruction opc)
{
	u8 sreg = (opc & 0x1) + DSP_REG_ACM0;
	u8 dreg = ((opc >> 4) & 0x3) + DSP_REG_AXL0;
	
	dsp_dmem_write(g_dsp.r[DSP_REG_AR0], g_dsp.r[sreg]);

	writeToBackLog(0, dreg,	dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));
	writeToBackLog(1, DSP_REG_AR3, dsp_increase_addr_reg(DSP_REG_AR3, (s16)g_dsp.r[DSP_REG_IX0 + DSP_REG_AR3]));
	writeToBackLog(2, DSP_REG_AR0, dsp_increase_addr_reg(DSP_REG_AR0, (s16)g_dsp.r[DSP_REG_IX0]));
}

// LD $ax0.d, $ax1.r, @$arS
// xxxx xxxx 11dr 00ss
// example for "nx'ld $AX0.L, $AX1.L, @$AR3"
// Loads the word pointed by AR0 to AX0.H, then loads the word pointed by AR3 to AX0.L. 
// Increments AR0 and AR3.
// If AR0 and AR3 point into the same memory page (upper 6 bits of addr are the same -> games are not doing that!)
// then the value pointed by AR0 is loaded to BOTH AX0.H and AX0.L.
// If AR0 points into an invalid memory page (ie 0x2000), then AX0.H keeps its old value. (not implemented yet)
// If AR3 points into an invalid memory page, then AX0.L gets the same value as AX0.H. (not implemented yet)
void ld(const UDSPInstruction opc)
{
	u8 dreg = (opc >> 5) & 0x1;
	u8 rreg = (opc >> 4) & 0x1;
	u8 sreg = opc & 0x3;

	if (sreg != DSP_REG_AR3) {
		writeToBackLog(0, (dreg << 1) + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[sreg]));

		if (IsSameMemArea(g_dsp.r[sreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[sreg]));
		else
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, sreg,	dsp_increment_addr_reg(sreg));
	} else {
		writeToBackLog(0, rreg + DSP_REG_AXH0, dsp_dmem_read(g_dsp.r[dreg]));

		if (IsSameMemArea(g_dsp.r[dreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[dreg]));
		else
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, dreg,	dsp_increment_addr_reg(dreg));
	}

	writeToBackLog(3, DSP_REG_AR3, dsp_increment_addr_reg(DSP_REG_AR3));
}

// LDN $ax0.d, $ax1.r, @$arS
// xxxx xxxx 11dr 01ss
void ldn(const UDSPInstruction opc)
{
	u8 dreg = (opc >> 5) & 0x1;
	u8 rreg = (opc >> 4) & 0x1;
	u8 sreg = opc & 0x3;
	
	if (sreg != DSP_REG_AR3) {
		writeToBackLog(0, (dreg << 1) + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[sreg]));

		if (IsSameMemArea(g_dsp.r[sreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[sreg]));
		else
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, sreg,	dsp_increase_addr_reg(sreg, (s16)g_dsp.r[DSP_REG_IX0 + sreg]));
	} else {
		writeToBackLog(0, rreg + DSP_REG_AXH0, dsp_dmem_read(g_dsp.r[dreg]));

		if (IsSameMemArea(g_dsp.r[dreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[dreg]));
		else
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, dreg,	dsp_increase_addr_reg(dreg, (s16)g_dsp.r[DSP_REG_IX0 + dreg]));
	}
	
	writeToBackLog(3, DSP_REG_AR3, dsp_increment_addr_reg(DSP_REG_AR3));
}

// LDM $ax0.d, $ax1.r, @$arS
// xxxx xxxx 11dr 10ss
void ldm(const UDSPInstruction opc)
{
	u8 dreg = (opc >> 5) & 0x1;
	u8 rreg = (opc >> 4) & 0x1;
	u8 sreg = opc & 0x3;

	if (sreg != DSP_REG_AR3) {
		writeToBackLog(0, (dreg << 1) + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[sreg]));

		if (IsSameMemArea(g_dsp.r[sreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[sreg]));
		else
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, sreg,	dsp_increment_addr_reg(sreg));
	} else {
		writeToBackLog(0, rreg + DSP_REG_AXH0, dsp_dmem_read(g_dsp.r[dreg]));

		if (IsSameMemArea(g_dsp.r[dreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[dreg]));
		else
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, dreg,	dsp_increment_addr_reg(dreg));
	}

	writeToBackLog(3, DSP_REG_AR3,
				   dsp_increase_addr_reg(DSP_REG_AR3, (s16)g_dsp.r[DSP_REG_IX0 + DSP_REG_AR3]));
}

// LDNM $ax0.d, $ax1.r, @$arS
// xxxx xxxx 11dr 11ss
void ldnm(const UDSPInstruction opc)
{
	u8 dreg = (opc >> 5) & 0x1;
	u8 rreg = (opc >> 4) & 0x1;
	u8 sreg = opc & 0x3;

	if (sreg != DSP_REG_AR3) {
		writeToBackLog(0, (dreg << 1) + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[sreg]));

		if (IsSameMemArea(g_dsp.r[sreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[sreg]));
		else
			writeToBackLog(1, (rreg << 1) + DSP_REG_AXL1, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, sreg,	dsp_increase_addr_reg(sreg, (s16)g_dsp.r[DSP_REG_IX0 + sreg]));
	} else {
		writeToBackLog(0, rreg + DSP_REG_AXH0, dsp_dmem_read(g_dsp.r[dreg]));

		if (IsSameMemArea(g_dsp.r[dreg], g_dsp.r[DSP_REG_AR3]))	
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[dreg]));
		else
			writeToBackLog(1, rreg + DSP_REG_AXL0, dsp_dmem_read(g_dsp.r[DSP_REG_AR3]));

		writeToBackLog(2, dreg,	dsp_increase_addr_reg(dreg, (s16)g_dsp.r[DSP_REG_IX0 + dreg]));
	}

	writeToBackLog(3, DSP_REG_AR3,
				   dsp_increase_addr_reg(DSP_REG_AR3, (s16)g_dsp.r[DSP_REG_IX0 + DSP_REG_AR3]));
}


void nop(const UDSPInstruction opc)
{
}

} // end namespace ext
} // end namespace DSPInterpeter


// The ext ops are calculated in parallel with the actual op. That means that
// both the main op and the ext op see the same register state as input. The
// output is simple as long as the main and ext ops don't change the same
// register. If they do the output is the bitwise or of the result of both the
// main and ext ops.

// The ext op are writing their output into the backlog which is
// being applied to the real registers after the main op was executed
void applyWriteBackLog()
{
	// always make sure to have an extra entry at the end w/ -1 to avoid
	// infinitive loops
	for (int i = 0; writeBackLogIdx[i] != -1; i++) {
		dsp_op_write_reg(writeBackLogIdx[i], g_dsp.r[writeBackLogIdx[i]] | writeBackLog[i]);
		// Clear back log
		writeBackLogIdx[i] = -1;
	}
}

// This function is being called in the main op after all input regs were read
// and before it writes into any regs. This way we can always use bitwise or to
// apply the ext command output, because if the main op didn't change the value
// then 0 | ext output = ext output and if it did then bitwise or is still the
// right thing to do
void zeroWriteBackLog() 
{
	// always make sure to have an extra entry at the end w/ -1 to avoid
	// infinitive loops
	for (int i = 0; writeBackLogIdx[i] != -1; i++) {
		dsp_op_write_reg(writeBackLogIdx[i], 0);
	}
}

//needed for 0x3... (at least)..., + clrl
//ex. corner case -> 0x3060: main opcode modifies .m, and extended .l -> .l shoudnt be zeroed because of .m write...
void zeroWriteBackLogPreserveAcc(u8 acc) 
{
	for (int i = 0; writeBackLogIdx[i] != -1; i++) {
		
		// acc0
		if ((acc == 0) &&  
			((writeBackLogIdx[i] == DSP_REG_ACL0) || (writeBackLogIdx[i] == DSP_REG_ACM0) || (writeBackLogIdx[i] == DSP_REG_ACH0)))
			continue;
		
		// acc1
		if ((acc == 1) && 
			((writeBackLogIdx[i] == DSP_REG_ACL1) || (writeBackLogIdx[i] == DSP_REG_ACM1) || (writeBackLogIdx[i] == DSP_REG_ACH1)))
			continue;

		dsp_op_write_reg(writeBackLogIdx[i], 0);
	}
}
