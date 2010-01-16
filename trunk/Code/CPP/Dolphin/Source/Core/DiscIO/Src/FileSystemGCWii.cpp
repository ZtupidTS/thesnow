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
#include "Common.h"
#include "FileUtil.h"

#include <string>

#include "FileSystemGCWii.h"
#include "StringUtil.h"

namespace DiscIO
{
CFileSystemGCWii::CFileSystemGCWii(const IVolume *_rVolume)
	: IFileSystem(_rVolume),
	m_Initialized(false),
	m_OffsetShift(0)
{
	m_Initialized = InitFileSystem();
}


CFileSystemGCWii::~CFileSystemGCWii()
{
}

bool CFileSystemGCWii::IsInitialized() const
{
	return m_Initialized;
}

u64 CFileSystemGCWii::GetFileSize(const char* _rFullPath) const
{
	if (!m_Initialized)
		return 0;

	const SFileInfo* pFileInfo = FindFileInfo(_rFullPath);

	if (pFileInfo != NULL && !pFileInfo->IsDirectory())
		return pFileInfo->m_FileSize;

	return 0;
}

const char* CFileSystemGCWii::GetFileName(u64 _Address) const
{
	for (size_t i = 0; i < m_FileInfoVector.size(); i++)
	{
		if ((m_FileInfoVector[i].m_Offset <= _Address) &&
		    ((m_FileInfoVector[i].m_Offset + m_FileInfoVector[i].m_FileSize) > _Address))
		{
			return m_FileInfoVector[i].m_FullPath;
		}
	}

	return 0;
}

u64 CFileSystemGCWii::ReadFile(const char* _rFullPath, u8* _pBuffer, size_t _MaxBufferSize) const
{
	if (!m_Initialized)
		return 0;

	const SFileInfo* pFileInfo = FindFileInfo(_rFullPath);
	if (pFileInfo == NULL)
		return 0;

	if (pFileInfo->m_FileSize > _MaxBufferSize)
		return 0;

	DEBUG_LOG(DISCIO, "Filename: %s. Offset: %0x. Size: %0x",_rFullPath,  pFileInfo->m_Offset, pFileInfo->m_FileSize);

	m_rVolume->Read(pFileInfo->m_Offset, pFileInfo->m_FileSize, _pBuffer);
	return pFileInfo->m_FileSize;
}

bool CFileSystemGCWii::ExportFile(const char* _rFullPath, const char* _rExportFilename) const
{
	size_t filesize = (size_t) GetFileSize(_rFullPath);

	if (filesize == 0)
		return false;

	u8* buffer = new u8[filesize];

	if (!ReadFile(_rFullPath, buffer, filesize))
	{
		delete[] buffer;
		return false;
	}

	FILE* f = fopen(_rExportFilename, "wb");

	if (f)
	{
		fwrite(buffer, filesize, 1, f);
		fclose(f);
		delete[] buffer;
		return true;
	}

	delete[] buffer;
	return false;
}

bool CFileSystemGCWii::ExportApploader(const char* _rExportFolder) const
{
	u32 AppSize = Read32(0x2440 + 0x14);// apploader size
	AppSize += Read32(0x2440 + 0x18);	// + trailer size
	AppSize += 0x20;					// + header size
	DEBUG_LOG(DISCIO,"AppSize -> %x", AppSize);

	u8* buffer = new u8[AppSize];
	if (m_rVolume->Read(0x2440, AppSize, buffer))
	{
		char exportName[512];
		sprintf(exportName, "%s/apploader.img", _rExportFolder);
		FILE* AppFile = fopen(exportName, "wb");
		if (AppFile)
		{
			fwrite(buffer, AppSize, 1, AppFile);
			fclose(AppFile);
			delete[] buffer;
			return true;
		}
	}

	delete[] buffer;
	return false;
}

bool CFileSystemGCWii::ExportDOL(const char* _rExportFolder) const
{
	u32 DolOffset = Read32(0x420) << m_OffsetShift;
	u32 DolSize = 0, offset = 0, size = 0;

	// Iterate through the 7 code segments
	for (u8 i = 0; i < 7; i++)
	{
		offset	= Read32(DolOffset + 0x00 + i * 4);
		size	= Read32(DolOffset + 0x90 + i * 4);
		if (offset + size > DolSize)
			DolSize = offset + size;
	}

	// Iterate through the 11 data segments
	for (u8 i = 0; i < 11; i++)
	{
		offset	= Read32(DolOffset + 0x1c + i * 4);
		size	= Read32(DolOffset + 0xac + i * 4);
		if (offset + size > DolSize)
			DolSize = offset + size;
	}

	u8* buffer = new u8[DolSize];
	if (m_rVolume->Read(DolOffset, DolSize, buffer))
	{
		char exportName[512];
		sprintf(exportName, "%s/boot.dol", _rExportFolder);
		FILE* DolFile = fopen(exportName, "wb");
		if (DolFile)
		{
			fwrite(buffer, DolSize, 1, DolFile);
			fclose(DolFile);
			delete[] buffer;
			return true;
		}
	}
	
	delete[] buffer;
	return false;
}

u32 CFileSystemGCWii::Read32(u64 _Offset) const
{
	u32 Temp = 0;
	m_rVolume->Read(_Offset, 4, (u8*)&Temp);
	return Common::swap32(Temp);
}

void CFileSystemGCWii::GetStringFromOffset(u64 _Offset, char* Filename) const
{
	m_rVolume->Read(_Offset, 255, (u8*)Filename);
}

size_t CFileSystemGCWii::GetFileList(std::vector<const SFileInfo *> &_rFilenames) const
{	
	if (_rFilenames.size())
		PanicAlert("GetFileList : input list has contents?");
	_rFilenames.clear();
	_rFilenames.reserve(m_FileInfoVector.size());
	for (size_t i = 0; i < m_FileInfoVector.size(); i++)
		_rFilenames.push_back(&m_FileInfoVector[i]);
	return m_FileInfoVector.size();
}

const SFileInfo* CFileSystemGCWii::FindFileInfo(const char* _rFullPath) const
{
	for (size_t i = 0; i < m_FileInfoVector.size(); i++)
	{
		if (!strcasecmp(m_FileInfoVector[i].m_FullPath, _rFullPath))
			return &m_FileInfoVector[i];
	}

	return NULL;
}

bool CFileSystemGCWii::InitFileSystem()
{
	if (Read32(0x18) == 0x5D1C9EA3)
	{
		m_OffsetShift = 2; // Wii file system
	}
	else if (Read32(0x1c) == 0xC2339F3D)
	{
		m_OffsetShift = 0; // GC file system
	}
	else
	{
		return false;
	}

	// read the whole FST
	u64 FSTOffset = (u64)Read32(0x424) << m_OffsetShift;
	// u32 FSTSize     = Read32(0x428);
	// u32 FSTMaxSize  = Read32(0x42C);


	// read all fileinfos
	SFileInfo Root;
	Root.m_NameOffset = Read32(FSTOffset + 0x0);
	Root.m_Offset     = (u64)Read32(FSTOffset + 0x4) << m_OffsetShift;
	Root.m_FileSize   = Read32(FSTOffset + 0x8);

	if (Root.IsDirectory())
	{
		if (m_FileInfoVector.size())
			PanicAlert("Wtf?");
		u64 NameTableOffset = FSTOffset;

		m_FileInfoVector.reserve((unsigned int)Root.m_FileSize);
		for (u32 i = 0; i < Root.m_FileSize; i++)
		{
			SFileInfo sfi;
			u64 Offset = FSTOffset + (i * 0xC);
			sfi.m_NameOffset = Read32(Offset + 0x0);
			sfi.m_Offset     = (u64)Read32(Offset + 0x4) << m_OffsetShift;
			sfi.m_FileSize   = Read32(Offset + 0x8);

			m_FileInfoVector.push_back(sfi);
			NameTableOffset += 0xC;
		}

		BuildFilenames(1, m_FileInfoVector.size(), NULL, NameTableOffset);
	}

	return true;
}

// Changed this stuff from C++ string to C strings for speed in debug mode. Doesn't matter in release, but
// std::string is SLOW in debug mode.
size_t CFileSystemGCWii::BuildFilenames(const size_t _FirstIndex, const size_t _LastIndex, const char* _szDirectory, u64 _NameTableOffset)
{
	size_t CurrentIndex = _FirstIndex;

	while (CurrentIndex < _LastIndex)
	{
		SFileInfo *rFileInfo = &m_FileInfoVector[CurrentIndex];
		u64 uOffset = _NameTableOffset + (rFileInfo->m_NameOffset & 0xFFFFFF);
		char filename[512];
		memset(filename, 0, sizeof(filename));
		GetStringFromOffset(uOffset, filename);

		// check next index
		if (rFileInfo->IsDirectory())
		{
			// this is a directory, build up the new szDirectory
			if (_szDirectory != NULL)
				CharArrayFromFormat(rFileInfo->m_FullPath, "%s%s/", _szDirectory, filename);
			else
				CharArrayFromFormat(rFileInfo->m_FullPath, "%s/", filename);

			CurrentIndex = BuildFilenames(CurrentIndex + 1, (size_t) rFileInfo->m_FileSize, rFileInfo->m_FullPath, _NameTableOffset);
		}
		else
		{
			// this is a filename
			if (_szDirectory != NULL)
				CharArrayFromFormat(rFileInfo->m_FullPath, "%s%s", _szDirectory, filename);
			else
				CharArrayFromFormat(rFileInfo->m_FullPath, "%s", filename);

			CurrentIndex++;
		}
	}

	return CurrentIndex;
}

}  // namespace
