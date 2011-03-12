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

#include <string>
#include <vector>
#include <wx/mstream.h>

#include "Common.h"
#include "CommonPaths.h"

#include "Globals.h"
#include "FileUtil.h"
#include "ISOFile.h"
#include "StringUtil.h"
#include "Hash.h"

#include "Filesystem.h"
#include "BannerLoader.h"
#include "FileSearch.h"
#include "CompressedBlob.h"
#include "ChunkFile.h"
#include "../resources/no_banner.cpp"

#define CACHE_REVISION 0x10B

#define DVD_BANNER_WIDTH 96
#define DVD_BANNER_HEIGHT 32

static u32 g_ImageTemp[DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT];

GameListItem::GameListItem(const std::string& _rFileName)
	: m_FileName(_rFileName)
	, m_FileSize(0)
	, m_Valid(false)
	, m_BlobCompressed(false)
	, m_pImage(NULL)
	, m_ImageSize(0)
{
	if (LoadFromCache())
	{
		m_Valid = true;
	}
	else
	{
		DiscIO::IVolume* pVolume = DiscIO::CreateVolumeFromFilename(_rFileName);

		if (pVolume != NULL)
		{
			if (!DiscIO::IsVolumeWadFile(pVolume))
				m_Platform = DiscIO::IsVolumeWiiDisc(pVolume) ? WII_DISC : GAMECUBE_DISC;
			else
				m_Platform = WII_WAD;

			m_Company = "N/A";
			for (int i = 0; i < 6; i++)
			{
				m_Name[i] = pVolume->GetName();
				if(m_Name[i] == "") // Couldn't find the name in the WAD...
				{
					std::string FileName;
					SplitPath(_rFileName, NULL, &FileName, NULL);
					m_Name[i] = FileName; // Then just display the filename... Better than something like "No Name"
				}
				m_Description[i] = "No Description";
			}
			m_Country  = pVolume->GetCountry();
			m_FileSize = File::GetSize(_rFileName);
			m_VolumeSize = pVolume->GetSize();

			m_UniqueID = pVolume->GetUniqueID();
			m_BlobCompressed = DiscIO::IsCompressedBlob(_rFileName.c_str());

			// check if we can get some infos from the banner file too
			DiscIO::IFileSystem* pFileSystem = DiscIO::CreateFileSystem(pVolume);

			if (pFileSystem != NULL || m_Platform == WII_WAD)
			{
				DiscIO::IBannerLoader* pBannerLoader = DiscIO::CreateBannerLoader(*pFileSystem, pVolume);

				if (pBannerLoader != NULL)
				{
					if (pBannerLoader->IsValid())
					{
						pBannerLoader->GetName(m_Name); //m_Country == DiscIO::IVolume::COUNTRY_JAP ? 1 : 0);
						pBannerLoader->GetCompany(m_Company);
						pBannerLoader->GetDescription(m_Description);
						if (pBannerLoader->GetBanner(g_ImageTemp))
						{
							m_ImageSize = DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT * 3;
							//use malloc(), since wxImage::Create below calls free() afterwards.
							m_pImage = (u8*)malloc(m_ImageSize);

							for (size_t i = 0; i < DVD_BANNER_WIDTH * DVD_BANNER_HEIGHT; i++)
							{
								m_pImage[i * 3 + 0] = (g_ImageTemp[i] & 0xFF0000) >> 16;
								m_pImage[i * 3 + 1] = (g_ImageTemp[i] & 0x00FF00) >>  8;
								m_pImage[i * 3 + 2] = (g_ImageTemp[i] & 0x0000FF) >>  0;
							}
						}
					}
					delete pBannerLoader;
				}

				delete pFileSystem;
			}

			delete pVolume;

			m_Valid = true;

			// If not Gamecube, create a cache file only if we have an image.
			// Wii isos create their images after you have generated the first savegame
			if (m_Platform == GAMECUBE_DISC || m_pImage)
				SaveToCache();
		}
	}

	if (m_pImage)
	{
		m_Image.Create(DVD_BANNER_WIDTH, DVD_BANNER_HEIGHT, m_pImage);
	}
	else
	{
		// default banner
		wxMemoryInputStream istream(no_banner_png, sizeof no_banner_png);
		wxImage iNoBanner(istream, wxBITMAP_TYPE_PNG);
		m_Image = iNoBanner;
	}
}

GameListItem::~GameListItem()
{
}

bool GameListItem::LoadFromCache()
{
	return CChunkFileReader::Load<GameListItem>(CreateCacheFilename(), CACHE_REVISION, *this);
}

void GameListItem::SaveToCache()
{
	if (!File::IsDirectory(File::GetUserPath(D_CACHE_IDX)))
	{
		File::CreateDir(File::GetUserPath(D_CACHE_IDX));
	}

	CChunkFileReader::Save<GameListItem>(CreateCacheFilename(), CACHE_REVISION, *this);
}

void GameListItem::DoState(PointerWrap &p)
{
	p.Do(m_Name[0]);	p.Do(m_Name[1]);	p.Do(m_Name[2]);
	p.Do(m_Name[3]);	p.Do(m_Name[4]);	p.Do(m_Name[5]);
	p.Do(m_Company);
	p.Do(m_Description[0]);	p.Do(m_Description[1]);	p.Do(m_Description[2]);
	p.Do(m_Description[3]);	p.Do(m_Description[4]);	p.Do(m_Description[5]);
	p.Do(m_UniqueID);
	p.Do(m_FileSize);
	p.Do(m_VolumeSize);
	p.Do(m_Country);
	p.Do(m_BlobCompressed);
	p.DoBuffer(&m_pImage, m_ImageSize);
	p.Do(m_Platform);
}

std::string GameListItem::CreateCacheFilename()
{
	std::string Filename, LegalPathname, extension;
	SplitPath(m_FileName, &LegalPathname, &Filename, &extension);

	if (Filename.empty()) return Filename; // Disc Drive

	// Filename.extension_HashOfFolderPath_Size.cache
	// Append hash to prevent ISO name-clashing in different folders.
	Filename.append(StringFromFormat("%s_%x_%llx.cache",
		extension.c_str(), HashFletcher((const u8 *)LegalPathname.c_str(), LegalPathname.size()),
		File::GetSize(m_FileName)));

	std::string fullname(File::GetUserPath(D_CACHE_IDX));
	fullname += Filename;
	return fullname;
}

const std::string& GameListItem::GetDescription(int index) const
{
	if ((index >=0) && (index < 6))
	{
		return m_Description[index];
	} 
	return m_Description[0];
}

const std::string& GameListItem::GetName(int index) const
{
	if ((index >=0) && (index < 6))
	{
		return m_Name[index];
	} 
	return m_Name[0];
}

const std::string GameListItem::GetWiiFSPath() const
{
	DiscIO::IVolume *Iso = DiscIO::CreateVolumeFromFilename(m_FileName);
	std::string ret;

	if (Iso == NULL)
		return ret;

	if (DiscIO::IsVolumeWiiDisc(Iso) || DiscIO::IsVolumeWadFile(Iso))
	{
		char Path[250];
		u64 Title;

		Iso->GetTitleID((u8*)&Title);
		Title = Common::swap64(Title);

		sprintf(Path, "%stitle/%08x/%08x/data/",
				File::GetUserPath(D_WIIUSER_IDX).c_str(), (u32)(Title>>32), (u32)Title);

		if (!File::Exists(Path))
			File::CreateFullPath(Path);

		if (Path[0] == '.')
			ret = std::string(wxGetCwd().mb_str()) + std::string(Path).substr(strlen(ROOT_DIR));
		else
			ret = std::string(Path);
	}
	delete Iso;

	return ret;
}

