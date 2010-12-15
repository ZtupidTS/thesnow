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

#include <stdio.h>
#include <stdlib.h>

#include "SI.h"
#include "SI_Device.h"
#include "SI_DeviceGCController.h"

#include "EXI_Device.h"
#include "EXI_DeviceMic.h"

#include "GCPad.h"

#include "../OnFrame.h"

#include "../CoreTiming.h"
#include "SystemTimers.h"
#include "ProcessorInterface.h"


// --- standard gamecube controller ---
CSIDevice_GCController::CSIDevice_GCController(int _iDeviceNumber)
	: ISIDevice(_iDeviceNumber)
	, m_TButtonComboStart(0)
	, m_TButtonCombo(0)
	, m_LastButtonCombo(COMBO_NONE)
{
	memset(&m_Origin, 0, sizeof(SOrigin));
	m_Origin.uCommand			= CMD_ORIGIN;
	m_Origin.uOriginStickX		= 0x80; // center
	m_Origin.uOriginStickY		= 0x80;
	m_Origin.uSubStickStickX	= 0x80;
	m_Origin.uSubStickStickY	= 0x80;
	m_Origin.uTrigger_L			= 0x1F; // 0-30 is the lower deadzone
	m_Origin.uTrigger_R			= 0x1F;

	// Dunno if we need to do this, game/lib should set it?
	m_Mode						= 0x03;
}

int CSIDevice_GCController::RunBuffer(u8* _pBuffer, int _iLength)
{
	// For debug logging only
	ISIDevice::RunBuffer(_pBuffer, _iLength);

	int iPosition = 0;
	while (iPosition < _iLength)
	{
		// Read the command
		EBufferCommands command = static_cast<EBufferCommands>(_pBuffer[iPosition ^ 3]);
		iPosition++;

		// Handle it
		switch (command)
		{
		case CMD_RESET:
			{
				*(u32*)&_pBuffer[0] = SI_GC_CONTROLLER;
				iPosition = _iLength; // Break the while loop
			}
			break;

		case CMD_ORIGIN:
			{
				INFO_LOG(SERIALINTERFACE, "PAD - Get Origin");
				u8* pCalibration = reinterpret_cast<u8*>(&m_Origin);
				for (int i = 0; i < (int)sizeof(SOrigin); i++)
				{
					_pBuffer[i ^ 3] = *pCalibration++;
				}				
			}
			iPosition = _iLength;
			break;

		// Recalibrate (FiRES: i am not 100 percent sure about this)
		case CMD_RECALIBRATE:
			{
				INFO_LOG(SERIALINTERFACE, "PAD - Recalibrate");
				u8* pCalibration = reinterpret_cast<u8*>(&m_Origin);
				for (int i = 0; i < (int)sizeof(SOrigin); i++)
				{
					_pBuffer[i ^ 3] = *pCalibration++;
				}				
			}
			iPosition = _iLength;
			break;

		// DEFAULT
		default:
			{
				ERROR_LOG(SERIALINTERFACE, "unknown SI command     (0x%x)", command);
				PanicAlert("SI: Unknown command");
				iPosition = _iLength;
			}			
			break;
		}
	}

	return iPosition;
}


// GetData

// Return true on new data (max 7 Bytes and 6 bits ;)
// [00?SYXBA] [1LRZUDRL] [x] [y] [cx] [cy] [l] [r]
//  |\_ ERR_LATCH (error latched - check SISR)
//  |_ ERR_STATUS (error on last GetData or SendCmd?)
bool CSIDevice_GCController::GetData(u32& _Hi, u32& _Low)
{
	SPADStatus PadStatus;
	memset(&PadStatus, 0, sizeof(PadStatus));

	Pad::GetStatus(ISIDevice::m_iDeviceNumber, &PadStatus);

#if defined(HAVE_WX) && HAVE_WX
	u32 netValues[2];
	if (NetPlay_GetInput(ISIDevice::m_iDeviceNumber, PadStatus, netValues))
	{
		_Hi  = netValues[0];	// first 4 bytes
		_Low = netValues[1];	// last  4 bytes
		return true;
	}
#endif

	Frame::SetPolledDevice();

	if(Frame::IsPlayingInput())
		Frame::PlayController(&PadStatus, ISIDevice::m_iDeviceNumber);
	else 
	{
		// Only pad 0 is allowed to autofire right now
		if(Frame::IsAutoFiring() && ISIDevice::m_iDeviceNumber == 0)
			Frame::ModifyController(&PadStatus, 0);

		if(Frame::IsRecordingInput())
			Frame::RecordInput(&PadStatus, ISIDevice::m_iDeviceNumber);
	}

	// Thankfully changing mode does not change the high bits ;)
	_Hi  = (u32)((u8)PadStatus.stickY);
	_Hi |= (u32)((u8)PadStatus.stickX << 8);
	_Hi |= (u32)((u16)PadStatus.button << 16);
	_Hi |= 0x00800000; // F|RES: means that the pad must be "combined" with the origin to match the "final" OSPad-Struct
	//_Hi |= 0x20000000; // ?

	// Low bits are packed differently per mode
	if (m_Mode == 0 || m_Mode == 5 || m_Mode == 6 || m_Mode == 7)
	{
		_Low  = (u8)(PadStatus.analogB >> 4);					// Top 4 bits
		_Low |= (u32)((u8)(PadStatus.analogA >> 4) << 4);		// Top 4 bits
		_Low |= (u32)((u8)(PadStatus.triggerRight >> 4) << 8);	// Top 4 bits
		_Low |= (u32)((u8)(PadStatus.triggerLeft >> 4) << 12);	// Top 4 bits
		_Low |= (u32)((u8)(PadStatus.substickY) << 16);			// All 8 bits
		_Low |= (u32)((u8)(PadStatus.substickX) << 24);			// All 8 bits
	}
	else if (m_Mode == 1)
	{
		_Low  = (u8)(PadStatus.analogB >> 4);					// Top 4 bits
		_Low |= (u32)((u8)(PadStatus.analogA >> 4) << 4);		// Top 4 bits
		_Low |= (u32)((u8)PadStatus.triggerRight << 8);			// All 8 bits
		_Low |= (u32)((u8)PadStatus.triggerLeft << 16);			// All 8 bits
		_Low |= (u32)((u8)PadStatus.substickY << 24);			// Top 4 bits
		_Low |= (u32)((u8)PadStatus.substickX << 28);			// Top 4 bits
	}
	else if (m_Mode == 2)
	{
		_Low  = (u8)(PadStatus.analogB);						// All 8 bits
		_Low |= (u32)((u8)(PadStatus.analogA) << 8);			// All 8 bits
		_Low |= (u32)((u8)(PadStatus.triggerRight >> 4) << 16);	// Top 4 bits
		_Low |= (u32)((u8)(PadStatus.triggerLeft >> 4) << 20);	// Top 4 bits
		_Low |= (u32)((u8)PadStatus.substickY << 24);			// Top 4 bits
		_Low |= (u32)((u8)PadStatus.substickX << 28);			// Top 4 bits
	}
	else if (m_Mode == 3)
	{
		// Analog A/B are always 0
		_Low  = (u8)PadStatus.triggerRight;						// All 8 bits
		_Low |= (u32)((u8)PadStatus.triggerLeft << 8);			// All 8 bits
		_Low |= (u32)((u8)PadStatus.substickY << 16);			// All 8 bits
		_Low |= (u32)((u8)PadStatus.substickX << 24);			// All 8 bits
	}
	else if (m_Mode == 4)
	{
		_Low  = (u8)(PadStatus.analogB);						// All 8 bits
		_Low |= (u32)((u8)(PadStatus.analogA) << 8);			// All 8 bits
		// triggerLeft/Right are always 0
		_Low |= (u32)((u8)PadStatus.substickY << 16);			// All 8 bits
		_Low |= (u32)((u8)PadStatus.substickX << 24);			// All 8 bits
	}

	// Keep track of the special button combos (embedded in controller hardware... :( )
	EButtonCombo tempCombo;
	if ((PadStatus.button & 0xff00) == (PAD_BUTTON_Y|PAD_BUTTON_X|PAD_BUTTON_START))
		tempCombo = COMBO_ORIGIN;
	else if ((PadStatus.button & 0xff00) == (PAD_BUTTON_B|PAD_BUTTON_X|PAD_BUTTON_START))
		tempCombo = COMBO_RESET;
	else
		tempCombo = COMBO_NONE;
	if (tempCombo != m_LastButtonCombo)
	{
		m_LastButtonCombo = tempCombo;
		if (m_LastButtonCombo != COMBO_NONE)
			m_TButtonComboStart = CoreTiming::GetTicks();
	}
	if (m_LastButtonCombo != COMBO_NONE)
	{
		m_TButtonCombo = CoreTiming::GetTicks();
		if ((m_TButtonCombo - m_TButtonComboStart) > SystemTimers::GetTicksPerSecond() * 3)
		{
			if (m_LastButtonCombo == COMBO_RESET)
				ProcessorInterface::ResetButton_Tap();
			else if (m_LastButtonCombo == COMBO_ORIGIN)
			{
				m_Origin.uOriginStickX		= PadStatus.stickX;
				m_Origin.uOriginStickY		= PadStatus.stickY;
				m_Origin.uSubStickStickX	= PadStatus.substickX;
				m_Origin.uSubStickStickY	= PadStatus.substickY;
				m_Origin.uTrigger_L			= PadStatus.triggerLeft;
				m_Origin.uTrigger_R			= PadStatus.triggerRight;
			}
			m_LastButtonCombo = COMBO_NONE;
		}
	}

	SetMic(PadStatus.MicButton); // This is dumb and should not be here

	return true;
}


// SendCommand
void CSIDevice_GCController::SendCommand(u32 _Cmd, u8 _Poll)
{
	UCommand command(_Cmd);

	switch (command.Command)
	{
	// Costis sent it in some demos :)
	case 0x00:
		break;

	case CMD_WRITE:
		{
			unsigned int uType = command.Parameter1;  // 0 = stop, 1 = rumble, 2 = stop hard
			unsigned int uStrength = command.Parameter2;

#if defined(HAVE_WX) && HAVE_WX
			// get the correct pad number that should rumble locally when using netplay
			const u8 numPAD = NetPlay_GetPadNum(ISIDevice::m_iDeviceNumber);
#else
			const u8 numPAD = 0;
#endif

			if (numPAD < 4)
				Pad::Rumble(numPAD, uType, uStrength);

			if (!_Poll)
			{
				m_Mode = command.Parameter2;
				INFO_LOG(SERIALINTERFACE, "PAD %i set to mode %i", ISIDevice::m_iDeviceNumber, m_Mode);
			}
		}
		break;

	default:
		{
			ERROR_LOG(SERIALINTERFACE, "unknown direct command     (0x%x)", _Cmd);
			PanicAlert("SI: Unknown direct command");
		}			
		break;
	}
}
