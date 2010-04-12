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

#ifndef _ACTIONREPLAY_H_
#define _ACTIONREPLAY_H_

#include "IniFile.h"

namespace ActionReplay
{

struct AREntry
{
	AREntry() {}
	AREntry(u32 _addr, u32 _value) : cmd_addr(_addr), value(_value) {}
	u32 cmd_addr;
	u32 value;
};

struct ARCode
{
	std::string name;
	std::vector<AREntry> ops;
	bool active;
};

void RunAllActive();
bool RunCode(const ARCode &arcode);
void LoadCodes(IniFile &ini, bool forceLoad);
void LoadCodes(std::vector<ARCode> &_arCodes, IniFile &ini);
size_t GetCodeListSize();
ARCode GetARCode(size_t index);
void SetARCode_IsActive(bool active, size_t index);
void UpdateActiveList();
void EnableSelfLogging(bool enable);
const std::vector<std::string> &GetSelfLog();
bool IsSelfLogging();
}  // namespace

#endif // _ACTIONREPLAY_H_
