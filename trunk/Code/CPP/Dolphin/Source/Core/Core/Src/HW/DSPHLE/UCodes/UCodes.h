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

#ifndef _UCODES_H
#define _UCODES_H

#include "Common.h"
#include "ChunkFile.h"
#include "Thread.h"

#include "../DSPHLE.h"
#include "../../Memmap.h"

#define UCODE_ROM                   0x0000000
#define UCODE_INIT_AUDIO_SYSTEM     0x0000001

class CMailHandler;

inline u8 HLEMemory_Read_U8(u32 _uAddress)
{
	_uAddress &= Memory::RAM_MASK;
	return Memory::m_pRAM[_uAddress];
}

inline u16 HLEMemory_Read_U16(u32 _uAddress)
{
	_uAddress &= Memory::RAM_MASK;
	return Common::swap16(*(u16*)&Memory::m_pRAM[_uAddress]);
}

inline u32 HLEMemory_Read_U32(u32 _uAddress)
{
	_uAddress &= Memory::RAM_MASK;
	return Common::swap32(*(u32*)&Memory::m_pRAM[_uAddress]);
}

inline void* HLEMemory_Get_Pointer(u32 _uAddress)
{
	_uAddress &= Memory::RAM_MASK;
	return &Memory::m_pRAM[_uAddress];
}

class IUCode
{
public:
	IUCode(DSPHLE *dsphle)
		: m_rMailHandler(dsphle->AccessMailHandler())
		, m_UploadSetupInProgress(false)
		, m_DSPHLE(dsphle)
		, m_NextUCode()
		, m_NextUCode_steps(0)
		, m_NeedsResumeMail(false)
	{}

	virtual ~IUCode()
	{}

	virtual void HandleMail(u32 _uMail) = 0;

	// Cycles are out of the 81/121mhz the DSP runs at.
	virtual void Update(int cycles) = 0;
	virtual void MixAdd(short* buffer, int size) {}

	virtual void DoState(PointerWrap &p) {}

protected:
	void PrepareBootUCode(u32 mail);

	// Some ucodes (notably zelda) require a resume mail to be
	// sent if they are be started via PrepareBootUCode.
	// The HLE can use this to 
	bool NeedsResumeMail();

	CMailHandler& m_rMailHandler;
	std::mutex m_csMix;

	enum EDSP_Codes
	{
		DSP_INIT        = 0xDCD10000,
		DSP_RESUME      = 0xDCD10001,
		DSP_YIELD       = 0xDCD10002,
		DSP_DONE        = 0xDCD10003,
		DSP_SYNC        = 0xDCD10004,
		DSP_FRAME_END   = 0xDCD10005,
	};

	// UCode is forwarding mails to PrepareBootUCode
	// UCode only needs to set this to true, IUCode will set to false when done!
	bool m_UploadSetupInProgress;

	// Need a pointer back to DSPHLE to switch ucodes.
	DSPHLE *m_DSPHLE;

private:
	struct SUCode
	{
		u32 mram_dest_addr;
		u16 mram_size;
		u16 mram_dram_addr;
		u32 iram_mram_addr;
		u16 iram_size;
		u16 iram_dest;
		u16 iram_startpc;
		u32 dram_mram_addr;
		u16 dram_size;
		u16 dram_dest;
	};
	SUCode	m_NextUCode;
	int	m_NextUCode_steps;

	bool m_NeedsResumeMail;
};

extern IUCode* UCodeFactory(u32 _CRC, DSPHLE *dsp_hle, bool bWii);

#endif
