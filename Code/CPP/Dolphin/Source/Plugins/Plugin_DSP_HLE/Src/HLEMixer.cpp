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

#include "Config.h" // Local
#include "Globals.h"
#include "DSPHandler.h"
#include "HLEMixer.h"

void HLEMixer::Premix(short *samples, unsigned int numSamples)
{
	// if this was called directly from the HLE
	if (g_Config.m_EnableHLEAudio && IsHLEReady())
	{
		IUCode *pUCode = CDSPHandler::GetInstance().GetUCode();
		if (pUCode && samples)
			pUCode->MixAdd(samples, numSamples);
	}
}

