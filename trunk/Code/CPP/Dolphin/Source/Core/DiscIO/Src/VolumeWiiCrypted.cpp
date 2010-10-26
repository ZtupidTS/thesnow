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

#include "VolumeWiiCrypted.h"
#include "StringUtil.h"

namespace DiscIO
{

CVolumeWiiCrypted::CVolumeWiiCrypted(IBlobReader* _pReader, u64 _VolumeOffset,
									 const unsigned char* _pVolumeKey)
	: m_pReader(_pReader),
	m_pBuffer(0),
	m_VolumeOffset(_VolumeOffset),
	dataOffset(0x20000),
	m_LastDecryptedBlockOffset(-1)
{
	AES_set_decrypt_key(_pVolumeKey, 128, &m_AES_KEY);
	m_pBuffer = new u8[0x8000];
}


CVolumeWiiCrypted::~CVolumeWiiCrypted()
{
	delete m_pReader; // is this really our responsibility?
	m_pReader = NULL;
	delete[] m_pBuffer;
	m_pBuffer = NULL;
}

bool CVolumeWiiCrypted::RAWRead( u64 _Offset, u64 _Length, u8* _pBuffer ) const
{
	// HyperIris: hack for DVDLowUnencryptedRead
	// Medal Of Honor Heroes 2 read this DVD offset for PartitionsInfo
	// and, PartitionsInfo is not encrypted, let's read it directly.
	if (!m_pReader->Read(_Offset, _Length, _pBuffer))
	{
		return(false);
	}
	return true;
}

bool CVolumeWiiCrypted::Read(u64 _ReadOffset, u64 _Length, u8* _pBuffer) const
{
	if (m_pReader == NULL)
	{
		return(false);
	}

	while (_Length > 0)
	{
		static unsigned char IV[16];

		// math block offset
		u64 Block  = _ReadOffset / 0x7C00;
		u64 Offset = _ReadOffset % 0x7C00;

		// read current block
		if (!m_pReader->Read(m_VolumeOffset + dataOffset + Block * 0x8000, 0x8000, m_pBuffer))
		{
			return(false);
		}

		if (m_LastDecryptedBlockOffset != Block)
		{
			memcpy(IV, m_pBuffer + 0x3d0, 16);
			AES_cbc_encrypt(m_pBuffer + 0x400, m_LastDecryptedBlock, 0x7C00, &m_AES_KEY, IV, AES_DECRYPT);

			m_LastDecryptedBlockOffset = Block;
		}

		// copy the encrypted data
		u64 MaxSizeToCopy = 0x7C00 - Offset;
		u64 CopySize = (_Length > MaxSizeToCopy) ? MaxSizeToCopy : _Length;
		memcpy(_pBuffer, &m_LastDecryptedBlock[Offset], (size_t)CopySize);

		// increase buffers
		_Length -= CopySize;
		_pBuffer	+= CopySize;
		_ReadOffset += CopySize;
	}

	return(true);
}

bool CVolumeWiiCrypted::GetTitleID(u8* _pBuffer) const
{
	// Tik is at m_VolumeOffset size 0x2A4
	// TitleID offset in tik is 0x1DC
	return RAWRead(m_VolumeOffset + 0x1DC, 8, _pBuffer);
}
void CVolumeWiiCrypted::GetTMD(u8* _pBuffer, u32 * _sz) const
{
	*_sz = 0;
	u32 tmdSz,
		tmdAddr;

	RAWRead(m_VolumeOffset + 0x2a4, sizeof(u32), (u8*)&tmdSz);
	RAWRead(m_VolumeOffset + 0x2a8, sizeof(u32), (u8*)&tmdAddr);
	tmdSz = Common::swap32(tmdSz);
	tmdAddr = Common::swap32(tmdAddr) << 2;
	RAWRead(m_VolumeOffset + tmdAddr, tmdSz, _pBuffer);
	*_sz = tmdSz;
}
	
std::string CVolumeWiiCrypted::GetUniqueID() const
{
	if (m_pReader == NULL)
	{
		return std::string();
	}

	char ID[7];

	if (!Read(0, 6, (u8*)ID))
	{
		return std::string();
	}

	ID[6] = '\0';

	return ID;
}


IVolume::ECountry CVolumeWiiCrypted::GetCountry() const
{
	if (!m_pReader)
		return COUNTRY_UNKNOWN;

	u8 CountryCode;
	m_pReader->Read(3, 1, &CountryCode);

	return CountrySwitch(CountryCode);
}

std::string CVolumeWiiCrypted::GetMakerID() const
{
	if (m_pReader == NULL)
	{
		return std::string();
	}

	char makerID[3];

	if (!Read(0x4, 0x2, (u8*)&makerID))
	{
		return std::string();
	}
	
	makerID[2] = '\0';

	return makerID;
}

std::string CVolumeWiiCrypted::GetName() const
{
	if (m_pReader == NULL)
	{
		return std::string();
	}

	char name[0xFF];

	if (!Read(0x20, 0x60, (u8*)&name))
	{
		return std::string();
	}

	return name;
}

u32 CVolumeWiiCrypted::GetFSTSize() const
{
	if (m_pReader == NULL)
	{
		return 0;
	}

	u32 size;

	if (!Read(0x428, 0x4, (u8*)&size))
	{
		return 0;
	}

	return size;
}

std::string CVolumeWiiCrypted::GetApploaderDate() const
{
	if (m_pReader == NULL)
	{
		return std::string();
	}

	char date[16];

	if (!Read(0x2440, 0x10, (u8*)&date))
	{
		return std::string();
	}
	
	date[10] = '\0';

	return date;
}

u64 CVolumeWiiCrypted::GetSize() const
{
	if (m_pReader)
	{
		return m_pReader->GetDataSize();
	}
	else
	{
		return 0;
	}
}

} // namespace
