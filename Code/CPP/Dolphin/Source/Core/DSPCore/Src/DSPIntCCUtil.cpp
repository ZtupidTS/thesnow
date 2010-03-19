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


// HELPER FUNCTIONS

#include "DSPIntCCUtil.h"
#include "DSPCore.h"
#include "DSPInterpreter.h"

namespace DSPInterpreter {

void Update_SR_Register64(s64 _Value, bool carry, bool overflow)
{
	// TODO: recheck 0x1,0x2,even 0x80... implement...
	g_dsp.r[DSP_REG_SR] &= ~SR_CMP_MASK;

	// 0x01
	if (carry)
	{
		g_dsp.r[DSP_REG_SR] |= SR_CARRY;
	}

	// 0x02
	if (overflow)
	{
		g_dsp.r[DSP_REG_SR] |= SR_OVERFLOW;
	}

	// 0x04
	if (_Value == 0)
	{
		g_dsp.r[DSP_REG_SR] |= SR_ARITH_ZERO;
	}

	// 0x08
	if (_Value < 0)
	{
		g_dsp.r[DSP_REG_SR] |= SR_SIGN;
	}

	// 0x10
	if (_Value != (s32)_Value)
	{
		g_dsp.r[DSP_REG_SR] |= SR_OVER_S32;
	}

	// 0x20 - Checks if top bits of m are equal, what is it good for?
	if (((_Value & 0xc0000000) == 0) || ((_Value & 0xc0000000) == 0xc0000000))
	{
		g_dsp.r[DSP_REG_SR] |= SR_TOP2BITS;
	}

	// 0x80
	{
	}
}


void Update_SR_Register16(s16 _Value, bool carry, bool overflow, bool overS32)
{
	// TODO: recheck 0x1,0x2,even 0x80... implement...
	g_dsp.r[DSP_REG_SR] &= ~SR_CMP_MASK;

	// 0x01
	if (carry)
	{
		g_dsp.r[DSP_REG_SR] |= SR_CARRY;
	}

	// 0x02
	if (overflow)
	{
		g_dsp.r[DSP_REG_SR] |= SR_OVERFLOW;
	}
	
	// 0x04
	if (_Value == 0)
	{
		g_dsp.r[DSP_REG_SR] |= SR_ARITH_ZERO;
	}

	// 0x08 
	if (_Value < 0)
	{
		g_dsp.r[DSP_REG_SR] |= SR_SIGN;
	}

	// 0x10
	if (overS32) 
	{
		g_dsp.r[DSP_REG_SR] |= SR_OVER_S32;
	}

	// 0x20 - Checks if top bits of m are equal, what is it good for?
	if ((((u16)_Value >> 14) == 0) || (((u16)_Value >> 14) == 3))
	{
		g_dsp.r[DSP_REG_SR] |= SR_TOP2BITS;
	}

	// 0x80
	{
	}
}

void Update_SR_LZ(bool value) {

	if (value == true) 
		g_dsp.r[DSP_REG_SR] |= SR_LOGIC_ZERO; 
	else
		g_dsp.r[DSP_REG_SR] &= ~SR_LOGIC_ZERO;
}

inline int GetMultiplyModifier()
{
	return (g_dsp.r[DSP_REG_SR] & SR_MUL_MODIFY)?1:2;
}

inline bool isCarry() {
	return (g_dsp.r[DSP_REG_SR] & SR_CARRY) ? true : false;
}

inline bool isLess() {
	return ((g_dsp.r[DSP_REG_SR] & SR_OVERFLOW) != (g_dsp.r[DSP_REG_SR] & SR_SIGN));
}

inline bool isZero() {
	return (g_dsp.r[DSP_REG_SR] & SR_ARITH_ZERO) ? true : false;
}

inline bool isLogicZero() {
	return (g_dsp.r[DSP_REG_SR] & SR_LOGIC_ZERO) ? true : false;
}

//see gdsp_registers.h for flags
bool CheckCondition(u8 _Condition)
{
	switch (_Condition & 0xf)
	{
	case 0x0: // GE - Greater Equal
		return !isLess();
	case 0x1: // L - Less
		return isLess();
	case 0x2: // G - Greater
		return !isLess() && !isZero();
	case 0x3: // LE - Less Equal
		return isLess() || isZero();
	case 0x4: // NZ - Not Zero
		return !isZero();
	case 0x5: // Z - Zero 
		return isZero();
	case 0x6: // NC - Not carry
		return !isCarry();
	case 0x7: // C - Carry 
		return isCarry();
	case 0xc: // LNZ  - Logic Not Zero
		return !isLogicZero();
	case 0xd: // LZ - Logic Zero
		return isLogicZero();

	case 0xf: // Empty - always true.
		return true;
	default:
		ERROR_LOG(DSPLLE, "Unknown condition check: 0x%04x", _Condition & 0xf);
		return false;
	}
}

}  // namespace