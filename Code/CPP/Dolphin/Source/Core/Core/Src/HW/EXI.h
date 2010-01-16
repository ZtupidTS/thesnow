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
#ifndef _EXIINTERFACE_H
#define _EXIINTERFACE_H

#include "Common.h"
#include "EXI_Channel.h"
#include "Thread.h"
class PointerWrap;

namespace ExpansionInterface
{

void Init();
void Shutdown();
void DoState(PointerWrap &p);

void Update();
void UpdateInterrupts();

void ChangeDeviceCallback(u64 userdata, int cyclesLate);
void ChangeDevice(u8 channel, TEXIDevices device, u8 slot);

void Read32(u32& _uReturnValue, const u32 _iAddress);
void Write32(const u32 _iValue, const u32 _iAddress);

} // end of namespace ExpansionInterface

#endif
