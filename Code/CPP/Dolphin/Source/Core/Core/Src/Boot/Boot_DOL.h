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

#ifndef _BOOT_DOL_H
#define _BOOT_DOL_H

#include "Common.h"

class CDolLoader
{
public:
	CDolLoader(const char* _szFilename);
	CDolLoader(u8* _pBuffer, u32 _Size);
	~CDolLoader();

	bool IsWii()		{ return m_isWii; }
	u32 GetEntryPoint()	{ return m_dolheader.entryPoint; }

	// Load into emulated memory
	void Load();

private:
	enum
	{
		DOL_NUM_TEXT	= 7,
		DOL_NUM_DATA	= 11
	};

	struct SDolHeader
	{
		u32 textOffset[DOL_NUM_TEXT];
		u32 dataOffset[DOL_NUM_DATA];

		u32 textAddress[DOL_NUM_TEXT];
		u32 dataAddress[DOL_NUM_DATA];

		u32 textSize[DOL_NUM_TEXT];
		u32 dataSize[DOL_NUM_DATA];

		u32 bssAddress;
		u32 bssSize;
		u32 entryPoint;
		u32 padd[7];
	};
	SDolHeader m_dolheader;

	u8 *data_section[DOL_NUM_DATA];
	u8 *text_section[DOL_NUM_TEXT];

	bool m_isWii;

	// Copy sections to internal buffers
	void Initialize(u8* _pBuffer, u32 _Size);
};

#endif
