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

#ifndef _VOLUME_GC
#define _VOLUME_GC

#include "Volume.h"
#include "Blob.h"

// --- this volume type is used for GC disc images ---

namespace DiscIO
{
class CVolumeGC	: public IVolume
{
public:
	CVolumeGC(IBlobReader* _pReader);
	~CVolumeGC();
	bool Read(u64 _Offset, u64 _Length, u8* _pBuffer) const;
	bool RAWRead(u64 _Offset, u64 _Length, u8* _pBuffer) const;
	std::string GetUniqueID() const;
	std::string GetMakerID() const;
	std::string GetName() const;
	u32 GetFSTSize() const;
	std::string GetApploaderDate() const;
	ECountry GetCountry() const;
	u64 GetSize() const;

private:
	IBlobReader* m_pReader;
};

} // namespace

#endif
