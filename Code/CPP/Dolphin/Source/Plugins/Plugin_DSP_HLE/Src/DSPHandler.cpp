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

#include "DSPHandler.h"

CDSPHandler* CDSPHandler::m_pInstance = NULL;

CDSPHandler::CDSPHandler()
	: m_pUCode(NULL),
	m_lastUCode(NULL),
	m_bHalt(false),
	m_bAssertInt(false)
{
	SetUCode(UCODE_ROM);
	m_DSPControl.DSPHalt = 1;
	m_DSPControl.DSPInit = 1;
}

CDSPHandler::~CDSPHandler()
{
	delete m_pUCode;
	m_pUCode = NULL;
}

void CDSPHandler::Update(int cycles)
{
	if (m_pUCode != NULL)
		m_pUCode->Update(cycles);
}

unsigned short CDSPHandler::WriteControlRegister(unsigned short _Value)
{
	UDSPControl Temp(_Value);
	if (Temp.DSPReset)
	{
		SetUCode(UCODE_ROM);
		Temp.DSPReset = 0;
	}
	if (Temp.DSPInit == 0)
	{
		// copy 128 byte from ARAM 0x000000 to IMEM
		SetUCode(UCODE_INIT_AUDIO_SYSTEM);
		Temp.DSPInitCode = 0;
	}

	m_DSPControl.Hex = Temp.Hex;
	return m_DSPControl.Hex;
}

unsigned short CDSPHandler::ReadControlRegister()
{
	return m_DSPControl.Hex;
}

void CDSPHandler::SendMailToDSP(u32 _uMail)
{
	if (m_pUCode != NULL) {
		DEBUG_LOG(DSP_MAIL, "CPU writes 0x%08x", _uMail);
		m_pUCode->HandleMail(_uMail);
	}
}

IUCode* CDSPHandler::GetUCode()
{
	return m_pUCode;
}

void CDSPHandler::SetUCode(u32 _crc)
{
	delete m_pUCode;

	m_pUCode = NULL;
	m_MailHandler.Clear();
	m_pUCode = UCodeFactory(_crc, m_MailHandler);
}

// TODO do it better?
// Assumes that every odd call to this func is by the persistent ucode.
// Even callers are deleted.
void CDSPHandler::SwapUCode(u32 _crc)
{
	m_MailHandler.Clear();

	if (m_lastUCode == NULL)
	{
		m_lastUCode = m_pUCode;
		m_pUCode = UCodeFactory(_crc, m_MailHandler);
	}
	else
	{
		delete m_pUCode;
		m_pUCode = m_lastUCode;
		m_lastUCode = NULL;
	}
}
