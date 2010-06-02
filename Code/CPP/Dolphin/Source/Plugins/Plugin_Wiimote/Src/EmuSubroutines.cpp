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


/* HID reports access guide. */

/* 0x10 - 0x1a   Output   EmuMain.cpp: HidOutputReport()
       0x10 - 0x14: General
	   0x15: Status report request from the Wii
	   0x16 and 0x17: Write and read memory or registers
       0x19 and 0x1a: General
   0x20 - 0x22   Input    EmuMain.cpp: HidOutputReport() to the destination
       0x15 leads to a 0x20 Input report
       0x17 leads to a 0x21 Input report
	   0x10 - 0x1a leads to a 0x22 Input report
   0x30 - 0x3f   Input    This file: Update() */

#include <vector>
#include <string>

#include "Common.h" // Common
#include "StringUtil.h"
#include "pluginspecs_wiimote.h"

#include "EmuMain.h" // Local
#include "EmuSubroutines.h"
#include "Config.h" // for g_Config


extern SWiimoteInitialize g_WiimoteInitialize;

namespace WiiMoteEmu
{
extern void PAD_Rumble(u8 _numPAD, unsigned int _uType);

/* Here we process the Output Reports that the Wii sends. Our response will be
   an Input Report back to the Wii. Input and Output is from the Wii's
   perspective, Output means data to the Wiimote (from the Wii), Input means
   data from the Wiimote.
   
   The call browser:

   1. Wiimote_InterruptChannel > InterruptChannel > HidOutputReport
   2. Wiimote_ControlChannel > ControlChannel > HidOutputReport

   The IR enable/disable and speaker enable/disable and mute/unmute values are
		bit2: 0 = Disable (0x02), 1 = Enable (0x06)
*/
void HidOutputReport(u16 _channelID, wm_report* sr)
{
	INFO_LOG(WIIMOTE, "HidOutputReport (page: %i, cid: 0x%02x, wm: 0x%02x)", g_ID, _channelID, sr->wm);

	switch(sr->wm)
	{
	case WM_RUMBLE: // 0x10
		PAD_Rumble(g_ID, sr->data[0]);
		break;

	case WM_LEDS: // 0x11
		INFO_LOG(WIIMOTE, "Set LEDs: 0x%02x", sr->data[0]);
		g_Leds[g_ID] = sr->data[0] >> 4;
		break;

	case WM_REPORT_MODE:  // 0x12
		WmReportMode(_channelID, (wm_report_mode*)sr->data);
		break;

	case WM_IR_PIXEL_CLOCK: // 0x13
		INFO_LOG(WIIMOTE, "WM IR Clock: 0x%02x", sr->data[0]);
		//g_IRClock[g_ID] = (sr->data[0] & 0x04) ? 1 : 0;
		break;

	case WM_SPEAKER_ENABLE: // 0x14
		INFO_LOG(WIIMOTE, "WM Speaker Enable: 0x%02x", sr->data[0]);
		g_Speaker[g_ID] = (sr->data[0] & 0x04) ? 1 : 0;
		break;

	case WM_REQUEST_STATUS: // 0x15
		WmRequestStatus(_channelID, (wm_request_status*)sr->data);
		break;

	case WM_WRITE_DATA: // 0x16
		WmWriteData(_channelID, (wm_write_data*)sr->data);
		break;

	case WM_READ_DATA: // 0x17
		WmReadData(_channelID, (wm_read_data*)sr->data);
		break;

	case WM_WRITE_SPEAKER_DATA: // 0x18
		// TODO: Does this need an ack?
		break;

	case WM_SPEAKER_MUTE: // 0x19
		INFO_LOG(WIIMOTE, "WM Speaker Mute: 0x%02x", sr->data[0]);
		//g_SpeakerMute[g_ID] = (sr->data[0] & 0x04) ? 1 : 0;
		break;

	case WM_IR_LOGIC: // 0x1a
		// This enables or disables the IR lights, we update the global variable g_IR
	    // so that WmRequestStatus() knows about it
		INFO_LOG(WIIMOTE, "WM IR Enable: 0x%02x", sr->data[0]);
		g_IR[g_ID] = (sr->data[0] & 0x04) ? 1 : 0;
		break;

	default:
		PanicAlert("HidOutputReport: Unknown channel 0x%02x", sr->wm);
		return;
	}
	
	// Send general feedback except the following types
	// as these ones generate their own feedbacks
	// or don't send feedbacks
	if ((sr->wm != WM_RUMBLE)
			&& (sr->wm != WM_READ_DATA) 
			&& (sr->wm != WM_REQUEST_STATUS)
			&& (sr->wm != WM_WRITE_SPEAKER_DATA)
		)
	{
		WmSendAck(_channelID, sr->wm);
	}
}


/* Generate the right header for wm reports. The returned values is the length
   of the header before the data begins. It's always two for all reports 0x20 -
   0x22, 0x30 - 0x37 */
int WriteWmReportHdr(u8* dst, u8 wm)
{
	// Update the first byte to 0xa1
	u32 Offset = 0;
	hid_packet* pHidHeader = (hid_packet*)dst;
	Offset += sizeof(hid_packet);
	pHidHeader->type = HID_TYPE_DATA;
	pHidHeader->param = HID_PARAM_INPUT;

	// Update the second byte to the current report type 0x20 - 0x22, 0x30 - 0x37
	wm_report* pReport = (wm_report*)(dst + Offset);
	Offset += sizeof(wm_report);
	pReport->wm = wm;
	return Offset;
}

/* This will generate the 0x22 acknowledgement for most Input reports.
   It has the form of "a1 22 00 00 _reportID 00".
   The first two bytes are the core buttons data,
   00 00 means nothing is pressed.
   The last byte is the success code 00. */
void WmSendAck(u16 _channelID, u8 _reportID)
{
	u8 DataFrame[1024];
	// Write DataFrame header
	u32 Offset = WriteWmReportHdr(DataFrame, WM_ACK_DATA);
	wm_acknowledge* pData = (wm_acknowledge*)(DataFrame + Offset);
	memset(pData, 0, sizeof(wm_acknowledge));

#if defined(HAVE_WX) && HAVE_WX
	FillReportInfo(pData->buttons);
#endif
	pData->reportID = _reportID;
	pData->errorID = 0;
	Offset += sizeof(wm_acknowledge);

	DEBUG_LOG(WIIMOTE,  "WMSendAck");
	DEBUG_LOG(WIIMOTE,  "  Report ID: %02x", _reportID);

	g_WiimoteInitialize.pWiimoteInput(g_ID, _channelID, DataFrame, Offset);

	// Debugging
	//ReadDebugging(true, DataFrame, Offset);
}


/* Read data from Wiimote and Extensions registers. */
void WmReadData(u16 _channelID, wm_read_data* rd) 
{
	u32 address = convert24bit(rd->address);
	u16 size = convert16bit(rd->size);
    u8 addressHI = (address >> 16) & 0xFE;
	INFO_LOG(WIIMOTE,  "Read data");
	DEBUG_LOG(WIIMOTE, "  Read data Space: %x", rd->space);
	DEBUG_LOG(WIIMOTE, "  Read data Address: 0x%06x", address);
	DEBUG_LOG(WIIMOTE, "  Read data Size: 0x%04x", size);

	/* Now we determine what address space we are reading from. Space 0 is
	   Eeprom and space 1 and 2 are the registers. */
	if(rd->space == WM_SPACE_EEPROM) 
	{
		if (address + size > WIIMOTE_EEPROM_SIZE) 
		{
			PanicAlert("WmReadData: address + size out of bounds");
			return;
		}
		SendReadDataReply(_channelID, g_Eeprom[g_ID] + address, address, addressHI, (int)size);
		/*DEBUG_LOG(WIIMOTE, "Read RegEeprom: Size: %i, Address: %08x,  Offset: %08x",
				size, address, (address & 0xffff));*/
	} 
	else if(rd->space == WM_SPACE_REGS1 || rd->space == WM_SPACE_REGS2)
	{
		u8* block;
		u32 blockSize;
		switch(addressHI) 
		{
		case 0xA2:
			block = g_RegSpeaker[g_ID];
			blockSize = WIIMOTE_REG_SPEAKER_SIZE;
			DEBUG_LOG(WIIMOTE, "  Case 0xa2: g_RegSpeaker");
			break;

		case 0xA4:
			block = g_RegExt[g_ID];
			blockSize = WIIMOTE_REG_EXT_SIZE;
			DEBUG_LOG(WIIMOTE, "  Case 0xa4: ExtReg");
			break;

		case 0xA6:
			block = g_RegMotionPlus[g_ID];
			blockSize = WIIMOTE_REG_EXT_SIZE;
			DEBUG_LOG(WIIMOTE, "  Case 0xa6: MotionPlusReg [%x]", address);
			break;

		case 0xB0:
			block = g_RegIr[g_ID];
			blockSize = WIIMOTE_REG_IR_SIZE;
			DEBUG_LOG(WIIMOTE,  "  Case 0xb0: g_RegIr");
			break;

		default:
			ERROR_LOG(WIIMOTE, "WmReadData: bad register block!");
			PanicAlert("WmReadData: bad register block!");
			return;
		}

		// Encrypt data that is read from the Wiimote Extension Register
		if(addressHI == 0xa4)
		{
			// Check if encrypted reads is on
			if(g_RegExt[g_ID][0xf0] == 0xaa)
			{
				/* Copy the registry to a temporary space. We don't want to change the unencrypted
				   data in the registry */
				memcpy(g_RegExtTmp, g_RegExt[g_ID], sizeof(g_RegExt[0]));

				// Encrypt g_RegExtTmp at that location
				wiimote_encrypt(&g_ExtKey[g_ID], &g_RegExtTmp[address & 0xffff], (address & 0xffff), (u8)size);

				// Update the block that SendReadDataReply will eventually send to the Wii
				block = g_RegExtTmp;
			}
		}

		address &= 0xFFFF;
		if(address + size > blockSize)
		{
			PanicAlert("WmReadData: address + size out of bounds! [%d %d %d]", address, size, blockSize);
			return;
		}
		
		// Let this function process the message and send it to the Wii
		SendReadDataReply(_channelID, block + address, address, addressHI, (u8)size);
		
	} 
	else
	{
		PanicAlert("WmReadData: unimplemented parameters (size: %i, addr: 0x%x)!", size, rd->space);
	}
}

/* Here we produce the actual 0x21 Input report that we send to the Wii. The
   message is divided into 16 bytes pieces and sent piece by piece. There will
   be five formatting bytes at the begging of all reports. A common format is
   00 00 f0 00 20, the 00 00 means that no buttons are pressed, the f means 16
   bytes in the message, the 0 means no error, the 00 20 means that the message
   is at the 00 20 offest in the registry that was read.
   
   _Base: The data beginning at _Base[0]
   _Address: The starting address inside the registry, this is used to check for out of bounds reading
   _Size: The total size to send
*/
void SendReadDataReply(u16 _channelID, void* _Base, u16 _Address, u8 _AddressHI, int _Size)
{
	int dataOffset = 0;
	const u8* data = (const u8*)_Base;

	while (_Size > 0)
	{
		u8 DataFrame[1024];
		// Write the first two bytes to DataFrame
		u32 Offset = WriteWmReportHdr(DataFrame, WM_READ_DATA_REPLY);
		
		// Limit the size to 16 bytes
		int copySize = (_Size > 16) ? 16 : _Size;
		// AyuanX: the MTU is 640B though... what a waste!

		wm_read_data_reply* pReply = (wm_read_data_reply*)(DataFrame + Offset);
		memset(pReply,0,sizeof(wm_read_data_reply));
		Offset += sizeof(wm_read_data_reply);

#if defined(HAVE_WX) && HAVE_WX
		FillReportInfo(pReply->buttons);
#endif
		pReply->error = 0;
		// 0x1 means two bytes, 0xf means 16 bytes
		pReply->size = copySize - 1;
		pReply->address = Common::swap16(_Address + dataOffset);

		// Clear the mem first
		memset(pReply->data, 0, 16);

		// Write a pice of _Base to DataFrame
		memcpy(pReply->data, data + dataOffset, copySize);

		// Update DataOffset for the next loop
		dataOffset += copySize;

		/* Out of bounds. The real Wiimote generate an error for the first
		   request to 0x1770 if we dont't replicate that the game will never
		   read the capibration data at the beginning of Eeprom. I think this
		   error is supposed to occur when we try to read above the freely
		   usable space that ends at 0x16ff. */
		if (Common::swap16(pReply->address + pReply->size) > WIIMOTE_EEPROM_FREE_SIZE)
		{
			pReply->size = 0x0f;
			pReply->error = 0x08;
		}

		if (WiiMapping[g_ID].bMotionPlusConnected)
		{
			//MP+ will try to read from this Registeraddress, expecting an error if a previous WM+ activation has been succesful
			//It will also return an error if there was no WM+ present at all
			if (((_Address == 0x00FE ) || (_Address == 0x00FF )) && (_AddressHI == 0xA6) && (g_RegExt[g_ID][0xFF] == 0x05))  
			{
					pReply->size = 0x0f;
					pReply->error = 0x07; //error: write-only area when activated/or not present
			}
		}
	

		// Logging
		DEBUG_LOG(WIIMOTE, "SendReadDataReply");
		DEBUG_LOG(WIIMOTE, "  Buttons: 0x%04x", pReply->buttons);
		DEBUG_LOG(WIIMOTE, "  Error: 0x%x", pReply->error);
		DEBUG_LOG(WIIMOTE, "  Size: 0x%x", pReply->size);
		DEBUG_LOG(WIIMOTE, "  Address: 0x%04x", pReply->address);		

#if defined(_DEBUG) || defined(DEBUGFAST)
		std::string Temp = ArrayToString(DataFrame, Offset);
		DEBUG_LOG(WIIMOTE, "Data: %s", Temp.c_str());
#endif

		// Send a piece
		g_WiimoteInitialize.pWiimoteInput(g_ID, _channelID, DataFrame, Offset);

		// Update the size that is left
		_Size -= copySize;

		// Debugging
		//ReadDebugging(true, DataFrame, Offset);
	}
}


/* Write data to Wiimote and Extensions registers. */
void WmWriteData(u16 _channelID, wm_write_data* wd) 
{
	u32 address = convert24bit(wd->address);
	u8	addressHI = (address >> 16) & 0xFE;
	INFO_LOG(WIIMOTE,  "Write data");
	DEBUG_LOG(WIIMOTE, "  Space: %x", wd->space);
	DEBUG_LOG(WIIMOTE, "  Address: 0x%06x", address);
	DEBUG_LOG(WIIMOTE, "  Size: 0x%02x", wd->size);
	// Write to EEPROM
	if(wd->size <= 16 && wd->space == WM_SPACE_EEPROM)
	{
		if(address + wd->size > WIIMOTE_EEPROM_SIZE) {
			ERROR_LOG(WIIMOTE, "WmWriteData: address + size out of bounds!");
			PanicAlert("WmWriteData: address + size out of bounds!");
			return;
		}
		memcpy(g_Eeprom[g_ID] + address, wd->data, wd->size);
	}
	// Write to registers
	else if(wd->size <= 16 && (wd->space == WM_SPACE_REGS1 || wd->space == WM_SPACE_REGS2))
	{
		u8* block;
		u32 blockSize;
		switch(addressHI)
		{
			case 0xA2:
				block = g_RegSpeaker[g_ID];
				blockSize = WIIMOTE_REG_SPEAKER_SIZE;
				DEBUG_LOG(WIIMOTE, "  Case 0xa2: RegSpeaker");
				break;

			case 0xA4:
				block = g_RegExt[g_ID]; // Extension Controller register
				blockSize = WIIMOTE_REG_EXT_SIZE;
				DEBUG_LOG(WIIMOTE, "  Case 0xa4: ExtReg");
				break;

			case 0xA6:
				block = g_RegMotionPlus[g_ID];
				blockSize = WIIMOTE_REG_EXT_SIZE;
				DEBUG_LOG(WIIMOTE, "  Case 0xa6: MotionPlusReg [%x]", address);
				break;

			case 0xB0:
				block = g_RegIr[g_ID];
				blockSize = WIIMOTE_REG_IR_SIZE;
				INFO_LOG(WIIMOTE, "  Case 0xb0: RegIr");
				break;

			default:
				ERROR_LOG(WIIMOTE, "WmWriteData: bad register block!");   
				PanicAlert("WmWriteData: bad register block!");
				return;
		}

		address &= 0xFFFF;

		// Check if the address is within bounds
		if(address + wd->size > blockSize) {
			PanicAlert("WmWriteData: address + size out of bounds!");
			return;
		}
		
		// Finally write the registers to the right structure
		memcpy(block + address, wd->data, wd->size);

		// Generate key for the Wiimote Extension
		if(blockSize == WIIMOTE_REG_EXT_SIZE)
		{
			// Run the key generation on all writes in the key area, it doesn't matter 
			// that we send it parts of a key, only the last full key will have an effect
			if(address >= 0x40 && address <= 0x4c)
				wiimote_gen_key(&g_ExtKey[g_ID], &g_RegExt[g_ID][0x40]);

		}
		if (WiiMapping[g_ID].bMotionPlusConnected) {
			//If the MP+ gets activated, it's important to send one or two status reports depending on the presence of a pass-through extension
			int sendreport = HandlingMotionPlusWrites(wd->data, addressHI, address);
			g_MotionPlusLastWriteReg[g_ID] = address; 

			switch (sendreport)
			{
				//pass-through extension disconnected and wm+ connected
				case 1: 
					WmRequestStatus(_channelID, (wm_request_status*) wd, 0); 
					WmRequestStatus(_channelID, (wm_request_status*) wd, 1); 
					break;

				//wm+ unplugged(on deactivation)
				case 2:
					WmRequestStatus(_channelID, (wm_request_status*) wd, 0); 
					break;

				//wm+ plugged in(on activation)
				case 3:
					WmRequestStatus(_channelID, (wm_request_status*) wd, 1);
					break;

				default:
					break;
			}

		}
		
	}
	else
	{
		PanicAlert("WmWriteData: unimplemented parameters!");
	}

	/* Just added for home brew... Isn't it enough that we call this from
	InterruptChannel()? Or is there a separate route here that don't pass
	though InterruptChannel()? */
}


/* Here we produce a 0x20 status report to send to the Wii. We currently ignore
   the status request rs and all its eventual instructions it may include (for
   example turn off rumble or something else) and just send the status
   report. */
void WmRequestStatus(u16 _channelID, wm_request_status* rs, int Extension)
{
	u8 DataFrame[1024];
	u32 Offset = WriteWmReportHdr(DataFrame, WM_STATUS_REPORT);

	wm_status_report* pStatus = (wm_status_report*)(DataFrame + Offset);
	Offset += sizeof(wm_status_report);
	memset(pStatus, 0, sizeof(wm_status_report)); // fill the status report with zeros

	// Status values
#if defined(HAVE_WX) && HAVE_WX
	FillReportInfo(pStatus->buttons);
#endif
	pStatus->leds = g_Leds[g_ID]; // leds are 4 bit
	pStatus->ir = g_IR[g_ID]; // 1 bit
	pStatus->speaker = g_Speaker[g_ID]; // 1 bit
	pStatus->battery_low = 0; // battery is okay
	pStatus->battery = 0x5f; // fully charged
	/* Battery levels in voltage
		  0x00 - 0x32: level 1
		  0x33 - 0x43: level 2
		  0x33 - 0x54: level 3
		  0x55 - 0xff: level 4 */

	// Check if we have a specific order about the extension status
	if (Extension == -1)
	{
		if (WiiMapping[g_ID].bMotionPlusConnected)
			pStatus->extension = ((g_MotionPlus[g_ID]) || (WiiMapping[g_ID].iExtensionConnected != EXT_NONE)) ? 1 : 0;
		else
			pStatus->extension = (WiiMapping[g_ID].iExtensionConnected == EXT_NONE) ? 0 : 1;
		// Read config value for the first time
	}
	else
	{
		pStatus->extension = (Extension) ? 1 : 0;
	}

	INFO_LOG(WIIMOTE, "Request Status");
	DEBUG_LOG(WIIMOTE, "  Buttons: 0x%04x", pStatus->buttons);
	DEBUG_LOG(WIIMOTE, "  Extension: %x", pStatus->extension);
	DEBUG_LOG(WIIMOTE, "  Speaker: %x", pStatus->speaker);
	DEBUG_LOG(WIIMOTE, "  IR: %x", pStatus->ir);
	DEBUG_LOG(WIIMOTE, "  LEDs: %x", pStatus->leds);


	g_WiimoteInitialize.pWiimoteInput(g_ID, _channelID, DataFrame, Offset);

	// Debugging
	//ReadDebugging(true, DataFrame, Offset);
}

//http://snzgoo.blogspot.com for more details on what this is doing
int HandlingMotionPlusWrites(u8* data, u8 addressHI, u32 address)
{
	bool MPlusActiveExt = (g_RegExt[g_ID][0xFF] == 0x05) ? 1 : 0;

	switch (addressHI)
	{
	case 0xA4:
		switch (address)
		{
		case 0x00FE:
			if (data[0] == 0x00)
			{ 
				if (MPlusActiveExt)
				{
					if (WiiMapping[g_ID].iExtensionConnected)
					{
						DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]:  Disabling WM+ and swapping registers back", data[0], addressHI, address);
						g_RegExt[g_ID][0xFE] = 0x00;
						SwapExtRegisters();
						return 1; // we need to issue a 0x20 report, if there's an extension connected to the MP+!
					}	
				}
				else
				{
					DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]:  WM+ already inactive", data[0], addressHI, address);
				}
				g_MotionPlus[g_ID] = 1;
			}
			break;

		//1. Disables an active wiimote; 0x20 report sent when iExtensionConnected != NONE : ext disconnect.
		//2. Initializing the pass-through extension: writing 0x55 ->0xA400F0 and then 0x00 to 0xA400FB.
		//3. Single write 0x00 to 0x00FB when MP got activated, part of the MP activation.
		case 0x00FB:	
			if ((data[0] == 0x00) && (g_MotionPlusLastWriteReg[g_ID] == 0xF0))
			{
				switch (g_MotionPlusLastWriteReg[g_ID])
				{
				case 0xF0:
					//1. disabling wiimote,
					if (MPlusActiveExt) //mp already deactivated, no register swap needed
					{
						DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]: Disabling WM+ and swapping registers back", data[0], addressHI, address)
						g_MotionPlus[g_ID] = WiiMapping[g_ID].iExtensionConnected ? 1 : 0;
						g_RegExt[g_ID][0xFE] = 0x05;
						SwapExtRegisters();

						if (!WiiMapping[g_ID].iExtensionConnected)
							return 2; 
					} //2. Default extension init, disable mp if actitaved, else do nothing
					else
					{
						DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]: WM+ already disabled [ext:%i] - no swapping", data[0], addressHI, address, WiiMapping[g_ID].iExtensionConnected);
						g_RegMotionPlus[g_ID][0xFE] = 0x05;
						g_RegMotionPlus[g_ID][0xF7] = 0x08;
					}
					break;
				//3. part of wm activation.
				default:
					if (MPlusActiveExt)
					{
						g_RegExt[g_ID][0xF1] = 0x01;
						g_RegExt[g_ID][0xF7] = 0x08; //init/calibration state flag
	
						if (WiiMapping[g_ID].iExtensionConnected)
						{
							//I don't know what these are for: F6h,F8h, F9h. They seem necessary to be set to 0x00 instead of 0xFF(default),
							//when there's an extension connected to the MP
							g_RegExt[g_ID][0xF6] = 0x00;
							g_RegExt[g_ID][0xF8] = 0x00;
							g_RegExt[g_ID][0xF9] = 0x00;
						}
					}
					break;
				}
			}
			break;

		//switch for invalid/valid data calibration (0x00/0x01)
		case 0x00F1: 
			if (MPlusActiveExt)
			{
				g_RegExt[g_ID][0xF7] = 0x1A; //syncing finished
			}
			break;

		//switch for triggering the calibration/syncing between wiimote and MP (corresponding data will be at 50h)			
		case 0x00F2: 
			if(MPlusActiveExt && (g_RegExt[g_ID][0xF7] < 0x0E))
			{
				g_RegExt[g_ID][0xF7] = 0x0E;
			}
			break;

		}
		break;

	//MotionPlus Register
	case 0xA6:
		switch (address)
		{
		//Enabling WM+: swapping extension registers
		case 0x00FE:	
			if ((data[0] == 0x04) || (data[0] == 0x05))
			{ 
				if (!MPlusActiveExt)
				{
					DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]: Enabling WM+ and swapping registers", data[0], addressHI, address);

					//The WII will try to read from the A6 WM+ register directly after activation,
					//and we need to reply with an error each time as long the mp is still activate.
					//In addition, we need to sent 1-2 0x20 statusreports depending on if theres an extension connected to the MP or not.
					g_MotionPlus[g_ID] = 1;
					SwapExtRegisters();

					g_RegExt[g_ID][0xF7] = 0x08; //Reset flag
					g_RegExt[g_ID][0xFE] = data[0];

					if (WiiMapping[g_ID].iExtensionConnected != EXT_NONE)
					{
						g_RegExt[g_ID][0xF1] = 0x01;
						g_RegExt[g_ID][0xF6] = 0x00;
						g_RegExt[g_ID][0xF8] = 0x00;
						g_RegExt[g_ID][0xF9] = 0x00;
						return 1; // we need to issue 2 0x20 reports, if there's an extension connected to the MP
					}
					return 3; // we need to issue 1 0x20 report, if there's no extension connected to the MP
				}
				else
				{
					DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]: WM already enabled no register swapping", data[0], addressHI, address);
				}
			}
			break;
		//Part of the WM+ init()
		case 0x00F0:
			if (data[0] == 0x55) { 
				//If the wiimote is already active, we will init() the WM+ directly in the ExtReg, shouldn't happen usually
				if (MPlusActiveExt)
				{
					g_RegExt[g_ID][0xFE] = 0x05;
					g_RegExt[g_ID][0xF7] = 0x08;
				}
				if (WiiMapping[g_ID].iExtensionConnected == EXT_NONE)
					g_MotionPlus[g_ID] = 0;
			}
			break;

		default:
			DEBUG_LOG(WIIMOTE, "Writing [0x%02x] to [0x%02x:%04x]: unknown reason", data[0], addressHI, address);
			break;
		}
		break;
	}
	return false;
}

//Swapping Ext/WM+-registers
void SwapExtRegisters()
{
	memset(g_RegExtTmp, 0, sizeof(g_RegExtTmp));
	memcpy(g_RegExtTmp, g_RegExt[g_ID], sizeof(g_RegExt[0]));
	memset(g_RegExt[0], 0, sizeof(g_RegExt[0]));
	memcpy(g_RegExt[g_ID], g_RegMotionPlus[g_ID], sizeof(g_RegMotionPlus[0]));
	memset(g_RegMotionPlus[0], 0, sizeof(g_RegMotionPlus[0]));
	memcpy(g_RegMotionPlus[g_ID], g_RegExtTmp, sizeof(g_RegExtTmp));

	if (g_RegMotionPlus[g_ID][0xFC]) {
		g_RegMotionPlus[g_ID][0xFC] = 0xa6;
	}
	if (g_RegExt[g_ID][0xFC]) {
		g_RegExt[g_ID][0xFC] = 0xa4;
	}
}

} // WiiMoteEmu
