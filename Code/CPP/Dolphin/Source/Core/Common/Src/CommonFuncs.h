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

#ifndef _COMMONFUNCS_H_
#define _COMMONFUNCS_H_

#ifdef _WIN32
#define SLEEP(x) Sleep(x)
#else
#define SLEEP(x) usleep(x*1000)
#endif

template <bool> struct CompileTimeAssert;
template<> struct CompileTimeAssert<true> {};

#if defined __GNUC__ && !defined __SSSE3__
#include <emmintrin.h>
static __inline __m128i __attribute__((__always_inline__))
_mm_shuffle_epi8(__m128i a, __m128i mask)
{
	__m128i result;
	__asm__("pshufb %1, %0"
		: "=x" (result)
		: "xm" (mask), "0" (a));
	return result;
}
#endif

#ifndef _WIN32

#include <errno.h>
#ifdef __linux__
#include <byteswap.h>
#else
char * strndup(char const *s, size_t n);
size_t strnlen(const char *s, size_t n);
#endif

// go to debugger mode
	#ifdef GEKKO
		#define Crash()
	#else
		#define Crash() {asm ("int $3");}
	#endif
	#define ARRAYSIZE(A) (sizeof(A)/sizeof((A)[0]))

inline u32 _rotl(u32 x, int shift) {
    shift &= 31;
    if (!shift) return x;
    return (x << shift) | (x >> (32 - shift));
}

inline u64 _rotl64(u64 x, unsigned int shift){
	unsigned int n = shift % 64;
	return (x << n) | (x >> (64 - n));
}

inline u32 _rotr(u32 x, int shift) {
    shift &= 31;
    if (!shift) return x;
    return (x >> shift) | (x << (32 - shift));
}

inline u64 _rotr64(u64 x, unsigned int shift){
	unsigned int n = shift % 64;
	return (x >> n) | (x << (64 - n));
}

#else // WIN32
// Function Cross-Compatibility
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
	#define unlink _unlink
	#define snprintf _snprintf
	#define vscprintf _vscprintf
char* strndup (char const *s, size_t n);	

// 64 bit offsets for windows
	#define fseeko _fseeki64
	#define ftello _ftelli64
	#define atoll _atoi64
	#define stat64 _stat64
	#define fstat64 _fstat64
	#define fileno _fileno

	#if _M_IX86
		#define Crash() {__asm int 3}
	#else
extern "C" {
	__declspec(dllimport) void __stdcall DebugBreak(void);
}
		#define Crash() {DebugBreak();}
	#endif // M_IX86
#endif // WIN32 ndef

// Dolphin's min and max functions
#undef min
#undef max

template<class T>
inline T min(const T& a, const T& b) {return a > b ? b : a;}
template<class T>
inline T max(const T& a, const T& b) {return a > b ? a : b;}

// Generic function to get last error message.
// Call directly after the command or use the error num.
// This function might change the error code.
// Defined in Misc.cpp.
const char* GetLastErrorMsg();

namespace Common
{
inline u8 swap8(u8 _data) {return _data;}

#ifdef _WIN32
inline u16 swap16(u16 _data) {return _byteswap_ushort(_data);}
inline u32 swap32(u32 _data) {return _byteswap_ulong (_data);}
inline u64 swap64(u64 _data) {return _byteswap_uint64(_data);}
#elif __linux__
inline u16 swap16(u16 _data) {return bswap_16(_data);}
inline u32 swap32(u32 _data) {return bswap_32(_data);}
inline u64 swap64(u64 _data) {return bswap_64(_data);}
#elif __APPLE__
inline __attribute__((always_inline)) u16 swap16(u16 _data)
	{return (_data >> 8) | (_data << 8);}
inline __attribute__((always_inline)) u32 swap32(u32 _data)
	{return __builtin_bswap32(_data);}
inline __attribute__((always_inline)) u64 swap64(u64 _data)
	{return __builtin_bswap64(_data);}
#else
// Slow generic implementation.
inline u16 swap16(u16 data) {return (data >> 8) | (data << 8);}
inline u32 swap32(u32 data) {return (swap16(data) << 16) | swap16(data >> 16);}
inline u64 swap64(u64 data) {return ((u64)swap32(data) << 32) | swap32(data >> 32);}
#endif

inline u16 swap16(const u8* _pData) {return swap16(*(const u16*)_pData);}
inline u32 swap32(const u8* _pData) {return swap32(*(const u32*)_pData);}
inline u64 swap64(const u8* _pData) {return swap64(*(const u64*)_pData);}

}  // Namespace Common

#endif // _COMMONFUNCS_H_
