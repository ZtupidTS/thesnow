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

#ifndef _STRINGUTIL_H_
#define _STRINGUTIL_H_

#include <stdarg.h>

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "Common.h"

std::string StringFromFormat(const char* format, ...);
// Cheap!
bool CharArrayFromFormatV(char* out, int outsize, const char* format, va_list args);

template<size_t Count>
inline void CharArrayFromFormat(char (& out)[Count], const char* format, ...)
{
	va_list args;
	va_start(args, format);
	CharArrayFromFormatV(out, Count, format, args);
	va_end(args);
}

// Good
std::string ArrayToString(const u8 *data, u32 size, int line_len = 20, bool spaces = true);

std::string StripSpaces(const std::string &s);
std::string StripQuotes(const std::string &s);
std::string StripNewline(const std::string &s);

// Thousand separator. Turns 12345678 into 12,345,678
template <typename I>
std::string ThousandSeparate(I value, int spaces = 0)
{
	std::ostringstream oss;
#ifdef __APPLE__
	oss.imbue(std::locale());
#else
	oss.imbue(std::locale(""));
#endif
	oss << std::setw(spaces) << value;

	return oss.str();
}

std::string StringFromInt(int value);
std::string StringFromBool(bool value);

bool TryParse(const std::string &str, bool *output);
bool TryParse(const std::string &str, u32 *output);

template <typename N>
bool TryParse(const std::string &str, N *const output)
{
	std::istringstream iss(str);
	
	N tmp = 0;
	if (iss >> tmp)
	{
		*output = tmp;
		return true;
	}
	else
		return false;
}

// TODO: kill this
bool AsciiToHex(const char* _szValue, u32& result);

std::string TabsToSpaces(int tab_size, const std::string &in);

void SplitString(const std::string& str, char delim, std::vector<std::string>& output);

// "C:/Windows/winhelp.exe" to "C:/Windows/", "winhelp", ".exe"
bool SplitPath(const std::string& full_path, std::string* _pPath, std::string* _pFilename, std::string* _pExtension);
// "C:/Windows/winhelp.exe" to "winhelp.exe"
std::string PathToFilename(const std::string &Path);

void BuildCompleteFilename(std::string& _CompleteFilename, const std::string& _Path, const std::string& _Filename);

#endif // _STRINGUTIL_H_
