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

#ifndef _DSPINTERFACE_H
#define _DSPINTERFACE_H

#include "Common.h"
class PointerWrap;

namespace DSP
{

enum DSPInterruptType
{
	INT_DSP		= 0,
	INT_ARAM	= 1,
	INT_AID		= 2
};

// aram size and mask
enum
{
	ARAM_SIZE	= 0x01000000,	// 16 MB
	ARAM_MASK	= 0x00FFFFFF,
};

void Init();
void Shutdown();
void DoState(PointerWrap &p);

void GenerateDSPInterrupt(DSPInterruptType _DSPInterruptType, bool _bSet = true);
void GenerateDSPInterruptFromPlugin(DSPInterruptType _DSPInterruptType, bool _bSet = true);

// Read32
void Read16(u16& _uReturnValue, const u32 _uAddress);
void Read32(u32& _uReturnValue, const u32 _uAddress);	

// Write
void Write16(const u16 _uValue, const u32 _uAddress);
void Write32(const u32 _uValue, const u32 _uAddress);

// Audio/DSP Plugin Helper
u8 ReadARAM(const u32 _uAddress);
void WriteARAM(u8 value, u32 _uAddress);

// Debugger Helper
u8* GetARAMPtr();

void UpdateAudioDMA();
void UpdateDSPSlice(int cycles);

}// end of namespace DSP

#endif


