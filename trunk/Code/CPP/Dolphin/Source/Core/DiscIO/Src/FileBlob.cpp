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

#include "Blob.h"
#include "FileBlob.h"

#ifdef _WIN32
	#include <windows.h>
#endif

namespace DiscIO
{

#ifdef _WIN32

PlainFileReader::PlainFileReader(HANDLE hFile_)
{
	hFile = hFile_;
	GetFileSizeEx(hFile, (PLARGE_INTEGER)&size);
}

PlainFileReader* PlainFileReader::Create(const char* filename)
{
	HANDLE hFile = CreateFile(
		filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
		return new PlainFileReader(hFile);
	else
		return 0;
}

PlainFileReader::~PlainFileReader()
{
	CloseHandle(hFile);
}

bool PlainFileReader::Read(u64 offset, u64 nbytes, u8* out_ptr)
{
	if (!SetFilePointerEx(hFile, *(LARGE_INTEGER*)&offset, NULL, FILE_BEGIN))
		return false;

	DWORD bytesRead = 0;
	if (!ReadFile(hFile, out_ptr, (DWORD)nbytes, &bytesRead, NULL))
		return false;
	if (bytesRead != nbytes)
		return false;

	return true;
}

#else // POSIX

PlainFileReader::PlainFileReader(FILE* file__)
{
	file_ = file__;
	#if 0
		fseek64(file_, 0, SEEK_END);
	#else
		fseek(file_, 0, SEEK_END); // I don't have fseek64 with gcc 4.3
	#endif
	size = ftell(file_);
	fseek(file_, 0, SEEK_SET);
}

PlainFileReader* PlainFileReader::Create(const char* filename)
{
	FILE* file_ = fopen(filename, "rb");
	if (file_)
		return new PlainFileReader(file_);
	else
		return 0;
}

PlainFileReader::~PlainFileReader()
{
	fclose(file_);
}

bool PlainFileReader::Read(u64 offset, u64 nbytes, u8* out_ptr)
{
	int seekStatus = fseek(file_, offset, SEEK_SET);
	if (seekStatus != 0)
		return false;
	size_t bytesRead = fread(out_ptr, 1, nbytes, file_);
	return bytesRead == nbytes;
}

#endif

}  // namespace
