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

#ifndef _FILESYSTEM_GC_WII_H
#define _FILESYSTEM_GC_WII_H

#include <vector>

#include "Filesystem.h"

namespace DiscIO
{

class CFileSystemGCWii : public IFileSystem
{
public:
	CFileSystemGCWii(const IVolume *_rVolume);
	virtual ~CFileSystemGCWii();
	virtual bool IsValid() const { return m_Valid; }
	virtual u64 GetFileSize(const char* _rFullPath);
	virtual size_t GetFileList(std::vector<const SFileInfo *> &_rFilenames);
	virtual const char* GetFileName(u64 _Address);
	virtual u64 ReadFile(const char* _rFullPath, u8* _pBuffer, size_t _MaxBufferSize);
	virtual bool ExportFile(const char* _rFullPath, const char* _rExportFilename);
	virtual bool ExportApploader(const char* _rExportFolder) const;
	virtual bool ExportDOL(const char* _rExportFolder) const;

private:
	bool m_Initialized;
	bool m_Valid;

	u32 m_OffsetShift; // WII offsets are all shifted
	std::vector <SFileInfo> m_FileInfoVector;
	u32 Read32(u64 _Offset) const;
	void GetStringFromOffset(u64 _Offset, char* Filename) const;
	const SFileInfo* FindFileInfo(const char* _rFullPath);
	bool DetectFileSystem();
	void InitFileSystem();
	size_t BuildFilenames(const size_t _FirstIndex, const size_t _LastIndex, const char* _szDirectory, u64 _NameTableOffset);
};

} // namespace

#endif

