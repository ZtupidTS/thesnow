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

#include "stdafx.h"

#include "VolumeGC.h"
#include "StringUtil.h"
#include "FileMonitor.h"

namespace DiscIO
{
CVolumeGC::CVolumeGC(IBlobReader* _pReader)
	: m_pReader(_pReader)
{}

CVolumeGC::~CVolumeGC()
{
	delete m_pReader;
	m_pReader = NULL; // I don't think this makes any difference, but anyway
}

bool CVolumeGC::Read(u64 _Offset, u64 _Length, u8* _pBuffer) const
{
	if (m_pReader == NULL)
		return false;

	FileMon::FindFilename(_Offset);

	return m_pReader->Read(_Offset, _Length, _pBuffer);
}

bool CVolumeGC::RAWRead( u64 _Offset, u64 _Length, u8* _pBuffer ) const
{
	return false;
}

std::string CVolumeGC::GetUniqueID() const
{
	static const std::string NO_UID("NO_UID");
	if (m_pReader == NULL)
		return NO_UID;

	char id[6];
	if (!Read(0, sizeof(id), reinterpret_cast<u8*>(id)))
	{
		PanicAlert("Failed to read unique ID from disc image");
		return NO_UID;
	}

	return std::string(id, sizeof(id));
}

IVolume::ECountry CVolumeGC::GetCountry() const
{
	if (!m_pReader)
		return COUNTRY_UNKNOWN;

	u8 CountryCode;
	m_pReader->Read(3, 1, &CountryCode);

	return CountrySwitch(CountryCode);
}

std::string CVolumeGC::GetMakerID() const
{
	if (m_pReader == NULL)
		return std::string();

	char makerID[3];
	if (!Read(0x4, 0x2, (u8*)&makerID))
		return std::string();
	makerID[2] = '\0';

	return makerID;
}

std::string CVolumeGC::GetName() const
{
	if (m_pReader == NULL)
		return "";

	char name[128];
	if (!Read(0x20, 0x60, (u8*)&name))
		return "";

	return name;
}

u32 CVolumeGC::GetFSTSize() const
{
	if (m_pReader == NULL)
		return 0;

	u32 size;
	if (!Read(0x428, 0x4, (u8*)&size))
		return 0;

	return Common::swap32(size);
}

std::string CVolumeGC::GetApploaderDate() const
{
	if (m_pReader == NULL)
		return std::string();

	char date[16];
	if (!Read(0x2440, 0x10, (u8*)&date))
		return std::string();
	// Should be 0 already, but just in case
	date[10] = '\0';

	return date;
}

u64 CVolumeGC::GetSize() const
{
	if (m_pReader)
		return m_pReader->GetDataSize();
	else
		return 0;
}

} // namespace
