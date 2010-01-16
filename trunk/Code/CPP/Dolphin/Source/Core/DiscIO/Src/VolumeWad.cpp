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
#include <math.h>

#include "VolumeWad.h"
#include "StringUtil.h"
#include "MathUtil.h"

#define ALIGN_40(x) ROUND_UP(Common::swap32(x), 0x40)

namespace DiscIO
{
CVolumeWAD::CVolumeWAD(IBlobReader* _pReader)
	: m_pReader(_pReader), OpeningBnrOffset(0), hdr_size(0), cert_size(0), tick_size(0), tmd_size(0), data_size(0)
{
	Read(0x00, 4, (u8*)&hdr_size);
	Read(0x08, 4, (u8*)&cert_size);
	Read(0x10, 4, (u8*)&tick_size);
	Read(0x14, 4, (u8*)&tmd_size);
	Read(0x18, 4, (u8*)&data_size);

	OpeningBnrOffset = ALIGN_40(hdr_size) + ALIGN_40(cert_size) + ALIGN_40(tick_size) + ALIGN_40(tmd_size) + ALIGN_40(data_size);
}

CVolumeWAD::~CVolumeWAD()
{
	delete m_pReader;
}

bool CVolumeWAD::Read(u64 _Offset, u64 _Length, u8* _pBuffer) const
{
	if (m_pReader == NULL)
		return false;

	return m_pReader->Read(_Offset, _Length, _pBuffer);
}

IVolume::ECountry CVolumeWAD::GetCountry() const
{
	if (!m_pReader)
		return COUNTRY_UNKNOWN;

	u8 CountryCode;
	u32 Offset = ALIGN_40(hdr_size) + ALIGN_40(cert_size);

	// read the last digit of the titleID in the ticket
	Read(Offset + 0x01E3, 1, &CountryCode);

	return CountrySwitch(CountryCode);
}

std::string CVolumeWAD::GetUniqueID() const
{
	std::string temp = GetMakerID();
	u32 Offset = ALIGN_40(hdr_size) + ALIGN_40(cert_size);

	char GameCode[8];
	if(!Read(Offset + 0x01E0, 4, (u8*)GameCode))
		return "0";

	GameCode[4] = temp.at(0);
	GameCode[5] = temp.at(1);
	GameCode[6] = 0;

	return GameCode;
}

std::string CVolumeWAD::GetMakerID() const
{
	u32 Offset = ALIGN_40(hdr_size) + ALIGN_40(cert_size) + ALIGN_40(tick_size);

	char temp[3] = {1};
	// Some weird channels use 0x0000 in place of the MakerID, so we need a check there
	if (!Read(0x198 + Offset, 2, (u8*)temp) || temp[0] == 0 || temp[1] == 0)
		return "00";

	temp[2] = 0;

	return temp;
}

bool CVolumeWAD::GetTitleID(u8* _pBuffer) const
{
	u32 Offset = ALIGN_40(hdr_size) + ALIGN_40(cert_size);

	if(!Read(Offset + 0x01DC, 8, _pBuffer))
		return false;

	return true;
}

std::string CVolumeWAD::GetName() const
{
	u32 footer_size;

	if (!Read(0x1C, 4, (u8*)&footer_size))
		return "";

	// Offset to the english title
	char temp[84];
	if (!Read(0xF1 + OpeningBnrOffset, 84, (u8*)&temp) || Common::swap32(footer_size) < 0xF1)
		return "";

	// Remove the null bytes due to 16bit char length
	std::string out_temp;
	for (int i = 0; i < sizeof(temp); i+=2)
	{
		// Replace null chars with a single space per null section
 		if (temp[i] == '\0' && i > 0)
		{ 		
			if (out_temp.at(out_temp.size()-1) != ' ')
 				out_temp.push_back(' ');
		}
		else
			out_temp.push_back(temp[i]);
	}
	// Make it a null terminated string
	out_temp.replace(out_temp.end()-1, out_temp.end(), 1, '\0');

	return out_temp;
}

u64 CVolumeWAD::GetSize() const
{
	if (m_pReader)
		return (size_t)m_pReader->GetDataSize();
	else
		return 0;
}

} // namespace
