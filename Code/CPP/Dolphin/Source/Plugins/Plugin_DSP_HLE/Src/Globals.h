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

#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "Common.h"
#include "pluginspecs_dsp.h"
#include "StringUtil.h"

extern DSPInitialize g_dspInitialize;
extern PLUGIN_GLOBALS* globals;

extern u8* g_pMemory;

// TODO: Wii support? Most likely audio data still must be in the old 24MB TRAM.
#define RAM_MASK 0x1FFFFFF

inline u8 Memory_Read_U8(u32 _uAddress)
{
	_uAddress &= RAM_MASK;
	return g_pMemory[_uAddress];
}

inline u16 Memory_Read_U16(u32 _uAddress)
{
	_uAddress &= RAM_MASK;
	return Common::swap16(*(u16*)&g_pMemory[_uAddress]);
}

inline u32 Memory_Read_U32(u32 _uAddress)
{
	_uAddress &= RAM_MASK;
	return Common::swap32(*(u32*)&g_pMemory[_uAddress]);
}

inline float Memory_Read_Float(u32 _uAddress)
{
	u32 uTemp = Memory_Read_U32(_uAddress);
	return *(float*)&uTemp;
}

inline void* Memory_Get_Pointer(u32 _uAddress)
{
	_uAddress &= RAM_MASK;
	return &g_pMemory[_uAddress];
}

#endif
