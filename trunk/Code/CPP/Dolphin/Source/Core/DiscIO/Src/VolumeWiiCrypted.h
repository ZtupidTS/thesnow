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

#ifndef _VOLUME_WII_CRYPTED
#define _VOLUME_WII_CRYPTED

#include "Volume.h"
#include "Blob.h"
#include "AES/aes.h"

// --- this volume type is used for encrypted Wii images ---

namespace DiscIO
{
class CVolumeWiiCrypted	: public IVolume
{
public:
	CVolumeWiiCrypted(IBlobReader* _pReader, u64 _VolumeOffset, const unsigned char* _pVolumeKey);
	~CVolumeWiiCrypted();
	bool Read(u64 _Offset, u64 _Length, u8* _pBuffer) const;
	bool RAWRead(u64 _Offset, u64 _Length, u8* _pBuffer) const;
	bool GetTitleID(u8* _pBuffer) const;
	void GetTMD(u8* _pBuffer, u32* _sz) const;
	std::string GetUniqueID() const;
	std::string GetMakerID() const;
	std::string GetName() const;
	u32 GetFSTSize() const;
	std::string GetApploaderDate() const;
	ECountry GetCountry() const;
	u64 GetSize() const;

private:
	IBlobReader* m_pReader;

	u8* m_pBuffer;
	AES_KEY m_AES_KEY;

	u64 m_VolumeOffset;
	u64 dataOffset;

	mutable u64 m_LastDecryptedBlockOffset;
	mutable unsigned char m_LastDecryptedBlock[0x8000];
};

} // namespace

#endif

