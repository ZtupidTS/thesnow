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

// Emulator state saving support.

#ifndef _STATE_H_
#define _STATE_H_

#include <string>

typedef struct
{
	u8 *buffer;
	size_t size;
} saveStruct;

void State_Init();
void State_Shutdown();

// These don't happen instantly - they get scheduled as events.
// ...But only if we're not in the main cpu thread.
//    If we're in the main cpu thread then they run immediately instead
//    because some things (like Lua) need them to run immediately.
// Slots from 0-99.
void State_Save(int slot);
void State_Load(int slot);
void State_Verify(int slot);

void State_SaveAs(const std::string &filename);
void State_LoadAs(const std::string &filename);
void State_VerifyAt(const std::string &filename);

void State_LoadFromBuffer(u8 **buffer);
void State_SaveToBuffer(u8 **buffer);
void State_VerifyBuffer(u8 **buffer);

void State_LoadLastSaved();
void State_UndoSaveState();
void State_UndoLoadState();

size_t State_GetSize();
void State_Flush(); // wait until previously scheduled savestate event (if any) is done


typedef struct  
{
	u8 gameID[6];
	size_t sz;
} state_header;

#endif
