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

#include <map>
#include "Common.h"

#include "HLE.h"

#include "../PowerPC/PowerPC.h"
#include "../PowerPC/PPCSymbolDB.h"
#include "../HW/Memmap.h"
#include "../Debugger/Debugger_SymbolMap.h"

#include "HLE_OS.h"
#include "HLE_Misc.h"

namespace HLE
{

using namespace PowerPC;

typedef void (*TPatchFunction)();

enum
{
	HLE_RETURNTYPE_BLR = 0,
	HLE_RETURNTYPE_RFI = 1,
};

struct SPatch
{
	char m_szPatchName[128];	
	TPatchFunction PatchFunction;
	int returnType;
};

static const SPatch OSPatches[] = 
{	
	{ "FAKE_TO_SKIP_0",		        HLE_Misc::UnimplementedFunction },

	// speedup
	{ "OSProtectRange",	            HLE_Misc::UnimplementedFunctionFalse },
	//{ "THPPlayerGetState",			HLE_Misc:THPPlayerGetState },


	// debug out is very nice ;)
	{ "OSReport",					HLE_OS::HLE_GeneralDebugPrint	},
	{ "OSPanic",					HLE_OS::HLE_OSPanic				},
	{ "vprintf",					HLE_OS::HLE_GeneralDebugPrint	},
	{ "printf",						HLE_OS::HLE_GeneralDebugPrint	},
	{ "puts",						HLE_OS::HLE_GeneralDebugPrint	}, // gcc-optimized printf?
	{ "___blank(char *,...)",		HLE_OS::HLE_GeneralDebugPrint	}, // used for early init things (normally)
	{ "___blank",					HLE_OS::HLE_GeneralDebugPrint	},
	{ "__write_console",			HLE_OS::HLE_write_console		}, // used by sysmenu (+more?)

	// wii only
	{ "__OSInitAudioSystem",        HLE_Misc::UnimplementedFunction },			

	// Super Monkey Ball - no longer needed.
	//{ ".evil_vec_cosine",           HLE_Misc::SMB_EvilVecCosine },
	//{ ".evil_normalize",            HLE_Misc::SMB_EvilNormalize },
	//{ ".evil_vec_setlength",        HLE_Misc::SMB_evil_vec_setlength },
	//{ ".evil_vec_something",        HLE_Misc::FZero_evil_vec_normalize },
	{ "PanicAlert",			          HLE_Misc::HLEPanicAlert },
	//{ ".sqrt_internal_needs_cr1",   HLE_Misc::SMB_sqrt_internal },
	//{ ".rsqrt_internal_needs_cr1",  HLE_Misc::SMB_rsqrt_internal },
	//{ ".atan2",						HLE_Misc::SMB_atan2},
	//{ ".sqrt_fz",                   HLE_Misc::FZ_sqrt},

	// F-zero still isn't working correctly, but these aren't really helping.

	//{ ".sqrt_internal_fz",   HLE_Misc::FZ_sqrt_internal },
	//{ ".rsqrt_internal_fz",  HLE_Misc::FZ_rsqrt_internal },

	//{ ".kill_infinites",			HLE_Misc::FZero_kill_infinites },
	// special
	//	{ "GXPeekZ",					HLE_Misc::GXPeekZ},
	//	{ "GXPeekARGB",					HLE_Misc::GXPeekARGB},  
};

static const SPatch OSBreakPoints[] =
{
	{ "FAKE_TO_SKIP_0",									HLE_Misc::UnimplementedFunction },
};


static std::map<u32, u32> orig_instruction;

void Patch(u32 address, const char *hle_func_name)
{
	for (u32 i = 0; i < sizeof(OSPatches) / sizeof(SPatch); i++)
	{
		if (!strcmp(OSPatches[i].m_szPatchName, hle_func_name)) {
			u32 HLEPatchValue = (1 & 0x3f) << 26;
			Memory::Write_U32(HLEPatchValue | i, address);
			return;
		}
	}
}

void PatchFunctions()
{
	orig_instruction.clear();
	for (u32 i = 0; i < sizeof(OSPatches) / sizeof(SPatch); i++)
	{
		Symbol *symbol = g_symbolDB.GetSymbolFromName(OSPatches[i].m_szPatchName);
		if (symbol > 0)
		{
			u32 HLEPatchValue = (1 & 0x3f) << 26;
			for (u32 addr = symbol->address; addr < symbol->address + symbol->size; addr += 4) {
				orig_instruction[addr] = Memory::ReadUnchecked_U32(addr);
				Memory::Write_U32(HLEPatchValue | i, addr);
			}
			INFO_LOG(HLE,"Patching %s %08x", OSPatches[i].m_szPatchName, symbol->address);
		}
	}

	for (size_t i = 1; i < sizeof(OSBreakPoints) / sizeof(SPatch); i++)
	{
		Symbol *symbol = g_symbolDB.GetSymbolFromName(OSPatches[i].m_szPatchName);
		if (symbol > 0)
		{
			PowerPC::breakpoints.Add(symbol->address, false);
			INFO_LOG(HLE,"Adding BP to %s %08x", OSBreakPoints[i].m_szPatchName, symbol->address);
		}
	}

	//    CBreakPoints::AddBreakPoint(0x8000D3D0, false);
}

void Execute(u32 _CurrentPC, u32 _Instruction)
{
	unsigned int FunctionIndex = _Instruction & 0xFFFFF;
	if ((FunctionIndex > 0) && (FunctionIndex < (sizeof(OSPatches) / sizeof(SPatch))))
	{
		OSPatches[FunctionIndex].PatchFunction();
	}
	else
	{
		PanicAlert("HLE system tried to call an undefined HLE function %i.", FunctionIndex);
	}

	//	_dbg_assert_msg_(HLE,NPC == LR, "Broken HLE function (doesn't set NPC)", OSPatches[pos].m_szPatchName);
}

u32 GetOrigInstruction(u32 addr)
{
	std::map<u32, u32>::const_iterator iter = orig_instruction.find(addr);
	if (iter != orig_instruction.end())
		return iter->second;
	else
		return 0;
}

}
