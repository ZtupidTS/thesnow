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

#include "../Globals.h"

#include "UCodes.h"

#include "UCode_AX.h"
#include "UCode_AXWii.h"
#include "UCode_Zelda.h"
#include "UCode_ROM.h"
#include "UCode_CARD.h"
#include "UCode_InitAudioSystem.h"
#include "UCode_GBA.h"
#include "Hash.h"
#include "../DSPHandler.h"

IUCode* UCodeFactory(u32 _CRC, CMailHandler& _rMailHandler)
{
	switch (_CRC)
	{
	case UCODE_ROM:
		INFO_LOG(DSPHLE, "Switching to ROM ucode");
		return new CUCode_Rom(_rMailHandler);
      
	case UCODE_INIT_AUDIO_SYSTEM:
		INFO_LOG(DSPHLE, "Switching to INIT ucode");
		return new CUCode_InitAudioSystem(_rMailHandler);

	case 0x65d6cc6f: // CARD
		INFO_LOG(DSPHLE, "Switching to CARD ucode");
		return new CUCode_CARD(_rMailHandler);

	case 0xdd7e72d5:
		INFO_LOG(DSPHLE, "Switching to GBA ucode");
		return new CUCode_GBA(_rMailHandler);

	case 0x3ad3b7ac: // Naruto3, Paper Mario - The Thousand Year Door
	case 0x3daf59b9: // Alien Hominid
	case 0x4e8a8b21: // spdemo, ctaxi, 18 wheeler, disney, monkeyball 1/2,cubivore,puzzlecollection,wario,
					 // capcom vs snk, naruto2, lost kingdoms, star fox, mario party 4, mortal kombat,
					 // smugglers run warzone, smash brothers, sonic mega collection, ZooCube
					 // nddemo, starfox
	case 0x07f88145: // bustamove, ikaruga, fzero, robotech battle cry, star soldier, soul calibur2,
					 // Zelda:OOT, Tony hawk, viewtiful joe
	case 0xe2136399: // billy hatcher, dragonballz, mario party 5, TMNT, ava1080
		INFO_LOG(DSPHLE, "CRC %08x: AX ucode chosen", _CRC);
		return new CUCode_AX(_rMailHandler);

	case 0x6ba3b3ea: // IPL - PAL
	case 0x24b22038: // IPL - NTSC/NTSC-JAP
	case 0x42f64ac4: // Luigi
	case 0x4be6a5cb: // AC, Pikmin
		INFO_LOG(DSPHLE, "CRC %08x: JAC (early Zelda) ucode chosen", _CRC);
		return new CUCode_Zelda(_rMailHandler, _CRC);

	case 0x6CA33A6D: // DK Jungle Beat
	case 0x86840740: // Zelda WW - US
	case 0x56d36052: // Mario Sunshine
	case 0x2fcdf1ec: // Mario Kart, zelda 4 swords
	case 0x267fd05a: // Pikmin PAL
		INFO_LOG(DSPHLE, "CRC %08x: Zelda ucode chosen", _CRC);
		return new CUCode_Zelda(_rMailHandler, _CRC);

      // WII CRCs
	case 0xb7eb9a9c: // Wii Pikmin - PAL
	case 0xeaeb38cc: // Wii Pikmin 2 - PAL
	case 0x6c3f6f94: // Zelda TP - PAL
	case 0xd643001f: // Mario Galaxy - PAL / WII DK Jungle Beat - PAL    
		INFO_LOG(DSPHLE, "CRC %08x: Zelda Wii ucode chosen\n", _CRC);
		return new CUCode_Zelda(_rMailHandler, _CRC);

	case 0x2ea36ce6: // Some Wii demos
	case 0x5ef56da3: // AX demo
	case 0x347112ba: // raving rabbits
	case 0xfa450138: // wii sports - PAL
	case 0xadbc06bd: // Elebits
	case 0x4cc52064: // Bleach: Versus Crusade    
    case 0xd9c4bf34: // WiiMenu
		INFO_LOG(DSPHLE, "CRC %08x: Wii - AXWii chosen", _CRC);
		return new CUCode_AXWii(_rMailHandler, _CRC);

	default:
		if (g_dspInitialize.bWii)
		{
			PanicAlert("DSPHLE: Unknown ucode (CRC = %08x) - forcing AXWii.\n\nTry LLE plugin if this is homebrew.", _CRC);
			return new CUCode_AXWii(_rMailHandler, _CRC);
		}
		else
		{
			PanicAlert("DSPHLE: Unknown ucode (CRC = %08x) - forcing AX.\n\nTry LLE plugin if this is homebrew.", _CRC);
			return new CUCode_AX(_rMailHandler);
		}
	}

	return NULL;
}

bool IUCode::NeedsResumeMail()
{
	if (m_NeedsResumeMail)
	{
		m_NeedsResumeMail = false;
		return true;
	}
	return false;
}

void IUCode::PrepareBootUCode(u32 mail)
{
	switch (m_NextUCode_steps)
	{
	case 0: m_NextUCode.mram_dest_addr	= mail;				break;
	case 1: m_NextUCode.mram_size		= mail & 0xffff;	break;
	case 2: m_NextUCode.mram_dram_addr	= mail & 0xffff;	break;
	case 3: m_NextUCode.iram_mram_addr	= mail;				break;
	case 4: m_NextUCode.iram_size		= mail & 0xffff;	break;
	case 5: m_NextUCode.iram_dest		= mail & 0xffff;	break;
	case 6: m_NextUCode.iram_startpc	= mail & 0xffff;	break;
	case 7: m_NextUCode.dram_mram_addr	= mail;				break;
	case 8: m_NextUCode.dram_size		= mail & 0xffff;	break;
	case 9: m_NextUCode.dram_dest		= mail & 0xffff;	break;
	}
	m_NextUCode_steps++;

	if (m_NextUCode_steps == 10)
	{
		m_NextUCode_steps = 0;
		m_NeedsResumeMail = true;
		m_UploadSetupInProgress = false;

		u32 ector_crc = HashEctor(
			(u8*)Memory_Get_Pointer(m_NextUCode.iram_mram_addr),
			m_NextUCode.iram_size);

#if defined(_DEBUG) || defined(DEBUGFAST)
		char binFile[MAX_PATH];
		sprintf(binFile, "%sDSP_UC_%08X.bin", File::GetUserPath(D_DUMPDSP_IDX), ector_crc);

		FILE* pFile = fopen(binFile, "wb");
		if (pFile)
		{
			fwrite((u8*)Memory_Get_Pointer(m_NextUCode.iram_mram_addr), m_NextUCode.iram_size, 1, pFile);
			fclose(pFile);
		}
#endif

		DEBUG_LOG(DSPHLE, "PrepareBootUCode 0x%08x", ector_crc);
		DEBUG_LOG(DSPHLE, "DRAM -> MRAM: src %04x dst %08x size %04x",
			m_NextUCode.mram_dram_addr, m_NextUCode.mram_dest_addr,
			m_NextUCode.mram_size);
		DEBUG_LOG(DSPHLE, "MRAM -> IRAM: src %08x dst %04x size %04x startpc %04x",
			m_NextUCode.iram_mram_addr, m_NextUCode.iram_dest,
			m_NextUCode.iram_size, m_NextUCode.iram_startpc);
		DEBUG_LOG(DSPHLE, "MRAM -> DRAM: src %08x dst %04x size %04x",
			m_NextUCode.dram_mram_addr, m_NextUCode.dram_dest,
			m_NextUCode.dram_size);

		if (m_NextUCode.mram_size)
		{
			WARN_LOG(DSPHLE,
				"Trying to boot new ucode with dram download - not implemented");
		}
		if (m_NextUCode.dram_size)
		{
			WARN_LOG(DSPHLE,
				"Trying to boot new ucode with dram upload - not implemented");
		}

		CDSPHandler::GetInstance().SwapUCode(ector_crc);
	}
}
