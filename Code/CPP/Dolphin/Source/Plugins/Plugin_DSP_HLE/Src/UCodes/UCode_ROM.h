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

#ifndef _UCODE_ROM
#define _UCODE_ROM

#include "UCodes.h"

class CUCode_Rom : public IUCode
{
public:
	CUCode_Rom(CMailHandler& _rMailHandler);
	virtual ~CUCode_Rom();

	void HandleMail(u32 _uMail);
	void Update(int cycles);

private:
	struct SUCode
	{
		u32 m_RAMAddress;
		u32 m_Length;
		u32 m_IMEMAddress;
		u32 m_Unk;
		u32 m_StartPC;
	};

	SUCode m_CurrentUCode;
	int m_BootTask_numSteps;

	u32 m_NextParameter;

	void BootUCode();
};

#endif

