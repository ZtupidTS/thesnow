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

#include "DSPMemoryMap.h"
#include "DSPIntUtil.h"

namespace DSPInterpreter {

// SRS @M, $(0x18+S)
// 0010 1sss mmmm mmmm
// Move value from register $(0x18+D) to data memory pointed by address 
// CR[0-7] | M. That is, the upper 8 bits of the address are the 
// bottom 8 bits from CR, and the lower 8 bits are from the 8-bit immediate.  
// Note: pc+=2 in duddie's doc seems wrong
void srs(const UDSPInstruction& opc)
{
	u8 reg   = ((opc.hex >> 8) & 0x7) + 0x18;
	u16 addr = (g_dsp.r[DSP_REG_CR] << 8) | (opc.hex & 0xFF);
	dsp_dmem_write(addr, g_dsp.r[reg]);
}
  
// LRS $(0x18+D), @M
// 0010 0ddd mmmm mmmm
// Move value from data memory pointed by address CR[0-7] | M to register
// $(0x18+D).  That is, the upper 8 bits of the address are the bottom 8 bits
// from CR, and the lower 8 bits are from the 8-bit immediate.
void lrs(const UDSPInstruction& opc)
{
	u8 reg   = ((opc.hex >> 8) & 0x7) + 0x18;
	u16 addr = (g_dsp.r[DSP_REG_CR] << 8) | (opc.hex & 0xFF);
	g_dsp.r[reg] = dsp_dmem_read(addr);
	dsp_conditional_extend_accum(reg);
}

// LR $D, @M
// 0000 0000 110d dddd
// mmmm mmmm mmmm mmmm
// Move value from data memory pointed by address M to register $D.
// FIXME: Perform additional operation depending on destination register.
void lr(const UDSPInstruction& opc)
{
	u8 reg   = opc.hex & DSP_REG_MASK;
	u16 addr = dsp_fetch_code();
	u16 val  = dsp_dmem_read(addr);
	dsp_op_write_reg(reg, val);
	dsp_conditional_extend_accum(reg);
}

// SR @M, $S
// 0000 0000 111s ssss
// mmmm mmmm mmmm mmmm
// Store value from register $S to a memory pointed by address M.
// FIXME: Perform additional operation depending on destination register.
void sr(const UDSPInstruction& opc)
{
	u8 reg   = opc.hex & DSP_REG_MASK;
	u16 addr = dsp_fetch_code();
	u16 val  = dsp_op_read_reg(reg);
	dsp_dmem_write(addr, val);
}

// SI @M, #I
// 0001 0110 mmmm mmmm
// iiii iiii iiii iiii
// Store 16-bit immediate value I to a memory location pointed by address
// M (M is 8-bit value sign extended).
void si(const UDSPInstruction& opc)
{
	u16 addr = (s8)opc.hex;
	u16 imm = dsp_fetch_code();
	dsp_dmem_write(addr, imm);
}

// LRR $D, @$S
// 0001 1000 0ssd dddd
// Move value from data memory pointed by addressing register $S to register $D.
// FIXME: Perform additional operation depending on destination register.
void lrr(const UDSPInstruction& opc)
{
	u8 sreg = (opc.hex >> 5) & 0x3;
	u8 dreg = opc.hex & 0x1f;

	u16 val = dsp_dmem_read(dsp_op_read_reg(sreg));
	dsp_op_write_reg(dreg, val);
	dsp_conditional_extend_accum(dreg);
}

// LRRD $D, @$S
// 0001 1000 1ssd dddd
// Move value from data memory pointed by addressing register $S toregister $D.
// Decrement register $S. 
// FIXME: Perform additional operation depending on destination register.
void lrrd(const UDSPInstruction& opc)
{
	u8 sreg = (opc.hex >> 5) & 0x3;
	u8 dreg = opc.hex & 0x1f;

	u16 val = dsp_dmem_read(dsp_op_read_reg(sreg));
	dsp_op_write_reg(dreg, val);
	dsp_conditional_extend_accum(dreg);
	g_dsp.r[sreg] = dsp_decrement_addr_reg(sreg);
}

// LRRI $D, @$S
// 0001 1001 0ssd dddd
// Move value from data memory pointed by addressing register $S to register $D.
// Increment register $S. 
// FIXME: Perform additional operation depending on destination register.
void lrri(const UDSPInstruction& opc)
{
	u8 sreg = (opc.hex >> 5) & 0x3;
	u8 dreg = opc.hex & 0x1f;

	u16 val = dsp_dmem_read(dsp_op_read_reg(sreg));
	dsp_op_write_reg(dreg, val);
	dsp_conditional_extend_accum(dreg);
	g_dsp.r[sreg] = dsp_increment_addr_reg(sreg);
}

// LRRN $D, @$S
// 0001 1001 1ssd dddd
// Move value from data memory pointed by addressing register $S to register $D.
// Add indexing register $(0x4+S) to register $S. 
// FIXME: Perform additional operation depending on destination register.
void lrrn(const UDSPInstruction& opc)
{
	u8 sreg = (opc.hex >> 5) & 0x3;
	u8 dreg = opc.hex & 0x1f;

	u16 val = dsp_dmem_read(dsp_op_read_reg(sreg));
	dsp_op_write_reg(dreg, val);
	dsp_conditional_extend_accum(dreg);
	g_dsp.r[sreg] = dsp_increase_addr_reg(sreg, (s16)g_dsp.r[DSP_REG_IX0 + sreg]);
}

// SRR @$D, $S
// 0001 1010 0dds ssss
// Store value from source register $S to a memory location pointed by 
// addressing register $D. 
// FIXME: Perform additional operation depending on source register.
void srr(const UDSPInstruction& opc)
{
	u8 dreg = (opc.hex >> 5) & 0x3;
	u8 sreg = opc.hex & 0x1f;

	u16 val = dsp_op_read_reg(sreg);
	dsp_dmem_write(g_dsp.r[dreg], val);
}

// SRRD @$D, $S
// 0001 1010 1dds ssss
// Store value from source register $S to a memory location pointed by
// addressing register $D. Decrement register $D. 
// FIXME: Perform additional operation depending on source register.
void srrd(const UDSPInstruction& opc)
{
	u8 dreg = (opc.hex >> 5) & 0x3;
	u8 sreg = opc.hex & 0x1f;

	u16 val = dsp_op_read_reg(sreg);
	dsp_dmem_write(g_dsp.r[dreg], val);
	g_dsp.r[dreg] = dsp_decrement_addr_reg(dreg);
}

// SRRI @$D, $S
// 0001 1011 0dds ssss
// Store value from source register $S to a memory location pointed by
// addressing register $D. Increment register $D. 
// FIXME: Perform additional operation depending on source register.
void srri(const UDSPInstruction& opc)
{
	u8 dreg = (opc.hex >> 5) & 0x3;
	u8 sreg = opc.hex & 0x1f;

	u16 val = dsp_op_read_reg(sreg);
	dsp_dmem_write(g_dsp.r[dreg], val);
	g_dsp.r[dreg] = dsp_increment_addr_reg(dreg);
}

// SRRN @$D, $S
// 0001 1011 1dds ssss
// Store value from source register $S to a memory location pointed by
// addressing register $D. Add DSP_REG_IX0 register to register $D.
// FIXME: Perform additional operation depending on source register.
void srrn(const UDSPInstruction& opc)
{
	u8 dreg = (opc.hex >> 5) & 0x3;
	u8 sreg = opc.hex & 0x1f;

	u16 val = dsp_op_read_reg(sreg);
	dsp_dmem_write(g_dsp.r[dreg], val);
	g_dsp.r[dreg] = dsp_increase_addr_reg(dreg, (s16)g_dsp.r[DSP_REG_IX0 + dreg]);
}

// ILRR $acD.m, @$arS
// 0000 001d 0001 00ss
// Move value from instruction memory pointed by addressing register
// $arS to mid accumulator register $acD.m.
void ilrr(const UDSPInstruction& opc)
{
	u16 reg  = opc.hex & 0x3;
	u16 dreg = DSP_REG_ACM0 + ((opc.hex >> 8) & 1);

	g_dsp.r[dreg] = dsp_imem_read(g_dsp.r[reg]);
	dsp_conditional_extend_accum(dreg);
}

// ILRRD $acD.m, @$arS
// 0000 001d 0001 01ss
// Move value from instruction memory pointed by addressing register
// $arS to mid accumulator register $acD.m. Decrement addressing register $arS.
void ilrrd(const UDSPInstruction& opc)
{
	u16 reg  = opc.hex & 0x3;
	u16 dreg = DSP_REG_ACM0 + ((opc.hex >> 8) & 1);

	g_dsp.r[dreg] = dsp_imem_read(g_dsp.r[reg]);
	dsp_conditional_extend_accum(dreg);
	g_dsp.r[reg] = dsp_decrement_addr_reg(reg);
}

// ILRRI $acD.m, @$S
// 0000 001d 0001 10ss
// Move value from instruction memory pointed by addressing register
// $arS to mid accumulator register $acD.m. Increment addressing register $arS.
void ilrri(const UDSPInstruction& opc)
{
	u16 reg  = opc.hex & 0x3;
	u16 dreg = DSP_REG_ACM0 + ((opc.hex >> 8) & 1);

	g_dsp.r[dreg] = dsp_imem_read(g_dsp.r[reg]);
	dsp_conditional_extend_accum(dreg);
	g_dsp.r[reg] = dsp_increment_addr_reg(reg);
}

// ILRRN $acD.m, @$arS
// 0000 001d 0001 11ss
// Move value from instruction memory pointed by addressing register
// $arS to mid accumulator register $acD.m. Add corresponding indexing
// register $ixS to addressing register $arS.
void ilrrn(const UDSPInstruction& opc)
{
	u16 reg  = opc.hex & 0x3;
	u16 dreg = DSP_REG_ACM0 + ((opc.hex >> 8) & 1);

	g_dsp.r[dreg] = dsp_imem_read(g_dsp.r[reg]);
	dsp_conditional_extend_accum(dreg);
	g_dsp.r[reg] = dsp_increase_addr_reg(reg, (s16)g_dsp.r[DSP_REG_IX0 + reg]);
}

}  // namespace
