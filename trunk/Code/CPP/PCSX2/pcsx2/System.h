/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "SysForwardDefs.h"

#include "Utilities/SafeArray.h"
#include "Utilities/Threading.h"		// to use threading stuff, include the Threading namespace in your file.
#include "CDVD/CDVDaccess.h"

typedef SafeArray<u8> VmStateBuffer;
class BaseVUmicroCPU;

// --------------------------------------------------------------------------------------
//  SysAllocVM
// --------------------------------------------------------------------------------------
class SysAllocVM
{
public:
	SysAllocVM();
	virtual ~SysAllocVM() throw();

protected:
	void CleanupMess() throw();
};

// --------------------------------------------------------------------------------------
//  SysCpuProviderPack
// --------------------------------------------------------------------------------------
class SysCpuProviderPack
{
protected:
	ScopedExcept m_RecExceptionEE;
	ScopedExcept m_RecExceptionIOP;

public:
	ScopedPtr<CpuInitializerSet> CpuProviders;

	SysCpuProviderPack();
	virtual ~SysCpuProviderPack() throw();

	void ApplyConfig() const;
	BaseVUmicroCPU* getVUprovider(int whichProvider, int vuIndex) const;

	bool HadSomeFailures( const Pcsx2Config::RecompilerOptions& recOpts ) const;

	bool IsRecAvailable_EE() const		{ return !m_RecExceptionEE; }
	bool IsRecAvailable_IOP() const		{ return !m_RecExceptionIOP; }

	BaseException* GetException_EE() const	{ return m_RecExceptionEE; }
	BaseException* GetException_IOP() const	{ return m_RecExceptionIOP; }

	bool IsRecAvailable_MicroVU0() const;
	bool IsRecAvailable_MicroVU1() const;
	BaseException* GetException_MicroVU0() const;
	BaseException* GetException_MicroVU1() const;

	bool IsRecAvailable_SuperVU0() const;
	bool IsRecAvailable_SuperVU1() const;
	BaseException* GetException_SuperVU0() const;
	BaseException* GetException_SuperVU1() const;

protected:
	void CleanupMess() throw();
};

// GetCpuProviders - this function is not implemented by PCSX2 core -- it must be
// implemented by the provisioning interface.
extern SysCpuProviderPack& GetCpuProviders();

extern void SysLogMachineCaps();				// Detects cpu type and fills cpuInfo structs.
extern void SysClearExecutionCache();	// clears recompiled execution caches!

extern u8 *SysMmapEx(uptr base, u32 size, uptr bounds, const char *caller="Unnamed");
extern void vSyncDebugStuff( uint frame );
extern void NTFS_CompressFile( const wxString& file, bool compressStatus=true );

extern wxString SysGetDiscID();

// --------------------------------------------------------------------------------------
//  PCSX2_SEH - Defines existence of "built in" Structured Exception Handling support.
// --------------------------------------------------------------------------------------
// This should be available on Windows, via Microsoft or Intel compilers (I'm pretty sure Intel
// supports native SEH model).  GNUC in Windows, or any compiler in a non-windows platform, will
// need to use setjmp/longjmp instead to exit recompiled code.
//

//#define PCSX2_SEH		0		// use this to force disable SEH on win32, to test setjmp functionality.

#ifndef PCSX2_SEH
#	if defined(_WIN32) && !defined(__GNUC__)
#		define PCSX2_SEH	1
#	else
#		define PCSX2_SEH	0
#	endif
#endif

// special macro which disables inlining on functions that require their own function stackframe.
// This is due to how Win32 handles structured exception handling.  Linux uses signals instead
// of SEH, and so these functions can be inlined.
#ifdef _WIN32
#	define __unique_stackframe __noinline
#else
#	define __unique_stackframe
#endif


//////////////////////////////////////////////////////////////////////////////////////////
// Different types of message boxes that the emulator can employ from the friendly confines
// of it's blissful unawareness of whatever GUI it runs under. :)  All message boxes exhibit
// blocking behavior -- they prompt the user for action and only return after the user has
// responded to the prompt.
//

namespace Msgbox
{
	extern bool	Alert( const wxString& text, const wxString& caption=L"PCSX2 Message", int icon=wxICON_EXCLAMATION );
	extern bool	OkCancel( const wxString& text, const wxString& caption=L"PCSX2 Message", int icon=0 );
	extern bool	YesNo( const wxString& text, const wxString& caption=L"PCSX2 Message", int icon=wxICON_QUESTION );

	extern int	Assertion( const wxString& text, const wxString& stacktrace );
}

