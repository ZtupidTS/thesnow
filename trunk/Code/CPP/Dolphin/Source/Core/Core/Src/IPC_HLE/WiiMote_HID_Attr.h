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

#ifndef WIIMOTE_HID_ATTR_H_
#define WIIMOTE_HID_ATTR_H_

#if 0
struct SAttrib
{
	u16 ID;
	u8* pData;
	u16 size;

	SAttrib(u16 _ID, u8* _Data, u16 _size) 
		: ID(_ID)
		, pData(_Data)
		, size(_size)
	{ }
};

typedef std::vector<SAttrib> CAttribTable;

const CAttribTable& GetAttribTable();
#endif

const u8* GetAttribPacket(u32 serviceHandle, u32 cont, u32& _size);

#endif

