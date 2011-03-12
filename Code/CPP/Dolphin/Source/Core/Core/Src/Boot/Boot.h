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

#ifndef _BOOT_H
#define _BOOT_H

#include <string>

#include "Common.h"
#include "../CoreParameter.h"

class CBoot
{
public:

	static bool BootUp();
	static bool IsElfWii(const char *filename);
	static bool IsWiiWAD(const char *filename);	

	static std::string GenerateMapFilename();
	
	static u64 Install_WiiWAD(const char *filename);

private:
	static void RunFunction(u32 _iAddr);

	static void UpdateDebugger_MapLoaded(const char* _gameID = NULL);

	static bool LoadMapFromFilename(const std::string& _rFilename, const char* _gameID = NULL);
	static bool Boot_ELF(const char *filename);
	static bool Boot_WiiWAD(const char *filename);

	static bool EmulatedBS2_GC();
	static bool EmulatedBS2_Wii();
	static bool EmulatedBS2(bool _bIsWii);
    static bool Load_BS2(const std::string& _rBootROMFilename);
	static void Load_FST(bool _bIsWii);

    static bool SetupWiiMemory(unsigned int _CountryCode);
};

#endif
