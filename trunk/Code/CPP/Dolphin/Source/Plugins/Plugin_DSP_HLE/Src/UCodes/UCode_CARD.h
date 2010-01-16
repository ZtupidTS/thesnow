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

#ifndef _UCODE_CARD_H
#define _UCODE_CARD_H

#include "UCodes.h"

class CUCode_CARD : public IUCode
{
private:
	enum EDSP_Codes
	{
		DSP_INIT   = 0xDCD10000,
		DSP_RESUME = 0xDCD10001,
		DSP_YIELD  = 0xDCD10002,
		DSP_DONE   = 0xDCD10003,
		DSP_SYNC   = 0xDCD10004,
		DSP_UNKN   = 0xDCD10005,
	};

public:
	CUCode_CARD(CMailHandler& _rMailHandler);
	virtual ~CUCode_CARD();

	void HandleMail(u32 _uMail);
	void Update(int cycles);
};

#endif

