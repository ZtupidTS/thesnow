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

#ifndef _BANNER_LOADER_H_
#define _BANNER_LOADER_H_

#include "Filesystem.h"

namespace DiscIO
{
class IBannerLoader
{
	public:

		IBannerLoader()
		{}


		virtual ~IBannerLoader()
		{}


		virtual bool IsValid() = 0;

		virtual bool GetBanner(u32* _pBannerImage) = 0;

		virtual bool GetName(std::string* _rName) = 0;

		virtual bool GetCompany(std::string& _rCompany) = 0;

		virtual bool GetDescription(std::string* _rDescription) = 0;


	protected:

		void CopyToStringAndCheck(std::string& _rDestination, const char* _src);
		
		bool CopyBeUnicodeToString(std::string& _rDestination, const u16* _src, int length);
	private:
		u16 swap16(u16 data)
		{
			return  ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
		}
};

IBannerLoader* CreateBannerLoader(DiscIO::IFileSystem& _rFileSystem, DiscIO::IVolume *pVolume);
} // namespace

#endif

