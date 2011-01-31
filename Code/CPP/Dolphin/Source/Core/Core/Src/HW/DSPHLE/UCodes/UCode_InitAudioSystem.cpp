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

#include "UCodes.h"
#include "UCode_InitAudioSystem.h"

CUCode_InitAudioSystem::CUCode_InitAudioSystem(DSPHLE *dsp_hle)
	: IUCode(dsp_hle)
{
	DEBUG_LOG(DSPHLE, "CUCode_InitAudioSystem - initialized");
}


CUCode_InitAudioSystem::~CUCode_InitAudioSystem()
{}


void CUCode_InitAudioSystem::Init()
{}


void CUCode_InitAudioSystem::Update(int cycles)
{
	if (m_rMailHandler.IsEmpty())
	{
		m_rMailHandler.PushMail(0x80544348);
		// HALT
	}
}

void CUCode_InitAudioSystem::HandleMail(u32 _uMail)
{}


