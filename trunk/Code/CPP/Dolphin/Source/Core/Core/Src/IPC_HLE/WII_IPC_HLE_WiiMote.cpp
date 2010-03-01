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

#include "Common.h" // Common
#include "StringUtil.h"

#include "WII_IPC_HLE_WiiMote.h" // Core
#include "WII_IPC_HLE_Device_usb.h"
#include "../PluginManager.h"
#include "../ConfigManager.h"
#include "../Host.h"
#include "../Core.h"

#include "l2cap.h" // Local
#include "WiiMote_HID_Attr.h"

static CWII_IPC_HLE_Device_usb_oh1_57e_305* s_Usb;

CWII_IPC_HLE_Device_usb_oh1_57e_305* GetUsbPointer()
{
	return s_Usb;
}

CWII_IPC_HLE_WiiMote::CWII_IPC_HLE_WiiMote(CWII_IPC_HLE_Device_usb_oh1_57e_305* _pHost, int _Number, bool ready)
	: m_HIDControlChannel_Connected(false)
	, m_HIDControlChannel_ConnectedWait(false)
	, m_HIDControlChannel_Config(false)
	, m_HIDControlChannel_ConfigWait(false)
	, m_HIDInterruptChannel_Connected(false)
	, m_HIDInterruptChannel_ConnectedWait(false)
	, m_HIDInterruptChannel_Config(false)
	, m_HIDInterruptChannel_ConfigWait(false)
	, m_Name("Nintendo RVL-CNT-01")
	, m_pHost(_pHost)

{
	INFO_LOG(WII_IPC_WIIMOTE, "Wiimote: #%i Constructed", _Number);

	s_Usb = _pHost;

	m_Connected = (ready) ? 0 : -1;
	m_ConnectionHandle = 0x100 + _Number;
	memset(m_LinkKey, 0xA0 + _Number, 16);

	m_BD.b[0] = 0x11;
	m_BD.b[1] = 0x02;
	m_BD.b[2] = 0x19;
	m_BD.b[3] = 0x79;
	m_BD.b[4] = 0x00;
	m_BD.b[5] = _Number;

	uclass[0]= 0x00;
	uclass[1]= 0x04;
	uclass[2]= 0x48;

	features[0] = 0xBC;
	features[1] = 0x02;
	features[2] = 0x04;
	features[3] = 0x38;
	features[4] = 0x08;
	features[5] = 0x00;
	features[6] = 0x00;
	features[7] = 0x00;

	lmp_version = 0x2;
	lmp_subversion = 0x229;
}

void CWII_IPC_HLE_WiiMote::DoState(PointerWrap &p)
{
	p.Do(m_Connected);
}

//
//
//
//
// ---  Simple and ugly state machine
//
//
//
//
//

bool CWII_IPC_HLE_WiiMote::LinkChannel()
{
	if (m_Connected != 2)
		return false;

	// try to connect HID_CONTROL_CHANNEL
	if (!m_HIDControlChannel_Connected)
	{
		if (m_HIDControlChannel_ConnectedWait)
			return false;

		m_HIDControlChannel_ConnectedWait = true;
		SendConnectionRequest(0x0040, HID_CONTROL_CHANNEL);
		return true;
	}

	// try to config HID_CONTROL_CHANNEL
	if (!m_HIDControlChannel_Config)
	{
		if (m_HIDControlChannel_ConfigWait)
			return false;

		m_HIDControlChannel_ConfigWait = true;
		SendConfigurationRequest(0x0040);
		return true;
	}

	// try to connect HID_INTERRUPT_CHANNEL
	if (!m_HIDInterruptChannel_Connected)
	{
		if (m_HIDInterruptChannel_ConnectedWait)
			return false;

		m_HIDInterruptChannel_ConnectedWait = true;
		SendConnectionRequest(0x0041, HID_INTERRUPT_CHANNEL);
		return true;
	}

	// try to config HID_INTERRUPT_CHANNEL
	if (!m_HIDInterruptChannel_Config)
	{
		if (m_HIDInterruptChannel_ConfigWait)
			return false;

		m_HIDInterruptChannel_ConfigWait = true;
		SendConfigurationRequest(0x0041);
		return true;
	}

	m_Connected = 3;
	UpdateStatus();

	return false;
}

// ===================================================
/* Send a status report to the status bar. */
// ----------------
void CWII_IPC_HLE_WiiMote::ShowStatus(const void* _pData)
{
        // Check if it's enabled
    SCoreStartupParameter& StartUp = SConfig::GetInstance().m_LocalCoreStartupParameter;
        bool LedsOn = StartUp.bWiiLeds;
        bool SpeakersOn = StartUp.bWiiSpeakers;

        const u8* data = (const u8*)_pData;

        // Get the last four bits with LED info
        if (LedsOn)
        {
                if (data[1] == 0x11)
                {
                        int led_bits = (data[2] >> 4);
                        Host_UpdateLeds(led_bits);
                }
        }

        int speaker_bits = 0;

        if (SpeakersOn)
        {
                u8 Bits = 0;
                switch (data[1])
                {
                case 0x14: // Enable and disable speakers
                        if (data[2] == 0x02) // Off
                                Bits = 0;
                        else if (data[2] == 0x06) // On
                                Bits = 1;
                        Host_UpdateSpeakerStatus(0, Bits);
                        break;

                case 0x19: // Mute and unmute
                        // Get the value
                        if (data[2] == 0x02) // Unmute
                                Bits = 1;
                        else if (data[2] == 0x06) // Mute
                                Bits = 0;
                        Host_UpdateSpeakerStatus(1, Bits);
                        break;
                // Write to speaker registry, or write sound
                case 0x16:
                case 0x18:
                                // Turn on the activity light
                                Host_UpdateSpeakerStatus(2, 1);
                        break;
                }
        }
}

// Turn off the activity icon again
void CWII_IPC_HLE_WiiMote::UpdateStatus()
{
        // Check if it's enabled
        if (!SConfig::GetInstance().m_LocalCoreStartupParameter.bWiiSpeakers)
                return;
        Host_UpdateStatus();
}

//
//
//
//
// ---  Events
//
//
//
//
//
void CWII_IPC_HLE_WiiMote::Activate(bool ready)
{
	if (ready && m_Connected == -1)
	{
		m_Connected = 0;
	}
	else if (!ready)
	{
		m_pHost->RemoteDisconnect(m_ConnectionHandle);
		EventDisconnect();
	}
}

void CWII_IPC_HLE_WiiMote::EventConnectionAccepted()
{
	m_Connected = 2;
}

void CWII_IPC_HLE_WiiMote::EventDisconnect()
{
	// Send disconnect message to plugin
	u8 Message = WIIMOTE_DISCONNECT;
	CPluginManager::GetInstance().GetWiimote(0)->Wiimote_ControlChannel(m_ConnectionHandle & 0xFF, 99, &Message, 0);

	m_Connected = -1;
	// Clear channel flags
	ResetChannels();
}

bool CWII_IPC_HLE_WiiMote::EventPagingChanged(u8 _pageMode)
{
	if (m_Connected != 0)
		return false;

	if ((_pageMode & 0x2) == 0)
		return false;

	m_Connected = 1;
	return true;
}

void CWII_IPC_HLE_WiiMote::ResetChannels()
{
	// reset connection process
	m_HIDControlChannel_Connected = false;
	m_HIDControlChannel_Config = false;
	m_HIDInterruptChannel_Connected = false;
	m_HIDInterruptChannel_Config = false;
	m_HIDControlChannel_ConnectedWait = false;
	m_HIDControlChannel_ConfigWait = false;
	m_HIDInterruptChannel_ConnectedWait = false;
	m_HIDInterruptChannel_ConfigWait = false;
}


//
//
//
//
// ---  Input parsing
//
//
//
//
//


// ===================================================
// This function receives L2CAP commands from the CPU
// It's called from SendToDevice() in WII_IPC_HLE_Device_usb.cpp.
// ---------------------------------------------------
void CWII_IPC_HLE_WiiMote::ExecuteL2capCmd(u8* _pData, u32 _Size)
{
	// Debugger::PrintDataBuffer(LogTypes::WIIMOTE, _pData, _Size, "SendACLPacket: ");

	// parse the command
	SL2CAP_Header* pHeader = (SL2CAP_Header*)_pData;
	u8* pData = _pData + sizeof(SL2CAP_Header);
	u32 DataSize = _Size - sizeof(SL2CAP_Header);
	INFO_LOG(WII_IPC_WIIMOTE, "  CID 0x%04x, Len 0x%x, DataSize 0x%x", pHeader->CID, pHeader->Length, DataSize);

	if(pHeader->Length != DataSize)
	{
		INFO_LOG(WII_IPC_WIIMOTE, "Faulty packet. It is dropped.");
		return;
	}

	switch (pHeader->CID)
	{
	case 0x0001:
		SignalChannel(pData, DataSize);
		break;

	default:
		{
			_dbg_assert_msg_(WII_IPC_WIIMOTE, DoesChannelExist(pHeader->CID), "L2CAP: SendACLPacket to unknown channel %i", pHeader->CID);
			CChannelMap::iterator  itr= m_Channel.find(pHeader->CID);

			Common::PluginWiimote* mote = CPluginManager::GetInstance().GetWiimote(0);
			if (itr != m_Channel.end())
			{
				SChannel& rChannel = itr->second;
				switch(rChannel.PSM)
				{
				case SDP_CHANNEL:
					HandleSDP(pHeader->CID, pData, DataSize);
					break;

				case HID_CONTROL_CHANNEL:
					mote->Wiimote_ControlChannel(m_ConnectionHandle & 0xFF, pHeader->CID, pData, DataSize);
					// Call Wiimote Plugin
					break;

				case HID_INTERRUPT_CHANNEL:
					ShowStatus(pData);
					mote->Wiimote_InterruptChannel(m_ConnectionHandle & 0xFF, pHeader->CID, pData, DataSize);
					// Call Wiimote Plugin
					break;

				default:
					ERROR_LOG(WII_IPC_WIIMOTE, "channel 0x04%x has unknown PSM %x", pHeader->CID, rChannel.PSM);
					PanicAlert("WIIMOTE: channel 0x04%x has unknown PSM %x", pHeader->CID, rChannel.PSM);
					break;
				}
			}
		}
		break;
	}
}
// ================

void CWII_IPC_HLE_WiiMote::SignalChannel(u8* _pData, u32 _Size)
{    
	while (_Size >= sizeof(SL2CAP_Command))
	{
		SL2CAP_Command* pCommand = (SL2CAP_Command*)_pData;
		_pData += sizeof(SL2CAP_Command);
		_Size = _Size - sizeof(SL2CAP_Command) - pCommand->len;

		switch(pCommand->code)
		{
		case L2CAP_COMMAND_REJ:
			ERROR_LOG(WII_IPC_WIIMOTE, "SignalChannel - L2CAP_COMMAND_REJ (something went wrong)."
				"Try to replace your SYSCONF file with a new copy."
				,pCommand->code); 
			PanicAlert(
				"SignalChannel - L2CAP_COMMAND_REJ (something went wrong)."
				"Try to replace your SYSCONF file with a new copy."
				,pCommand->code);
			break;

		case L2CAP_CONN_REQ:            
			ReceiveConnectionReq(pCommand->ident,  _pData, pCommand->len);
			break;

		case L2CAP_CONN_RSP:
			ReceiveConnectionResponse(pCommand->ident,  _pData, pCommand->len);
			break;

		case L2CAP_CONF_REQ:            
			ReceiveConfigurationReq(pCommand->ident,  _pData, pCommand->len);
			break;

		case L2CAP_CONF_RSP:
			ReceiveConfigurationResponse(pCommand->ident, _pData, pCommand->len);
			break;

		case L2CAP_DISCONN_REQ:
			ReceiveDisconnectionReq(pCommand->ident,  _pData, pCommand->len);
			break;

		default:
			ERROR_LOG(WII_IPC_WIIMOTE, "  Unknown Command-Code (0x%02x)", pCommand->code);
			PanicAlert("SignalChannel %x",pCommand->code);
			return;
		}

		_pData += pCommand->len;
	}
}

//
//
//
//
// ---  Receive Commands from CPU
//
//
//
//
//

void CWII_IPC_HLE_WiiMote::ReceiveConnectionReq(u8 _Ident, u8* _pData, u32 _Size)
{
	SL2CAP_CommandConnectionReq* pCommandConnectionReq = (SL2CAP_CommandConnectionReq*)_pData;

	// create the channel
	SChannel& rChannel = m_Channel[pCommandConnectionReq->scid];
	rChannel.PSM = pCommandConnectionReq->psm;
	rChannel.SCID = pCommandConnectionReq->scid;
	rChannel.DCID = pCommandConnectionReq->scid;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] ReceiveConnectionRequest");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Ident: 0x%02x", _Ident);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    PSM: 0x%04x", rChannel.PSM);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    SCID: 0x%04x", rChannel.SCID);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    DCID: 0x%04x", rChannel.DCID);

	// response
	SL2CAP_ConnectionResponse Rsp;
	Rsp.scid   = rChannel.SCID;
	Rsp.dcid   = rChannel.DCID;
	Rsp.result = 0x00;
	Rsp.status = 0x00;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] SendConnectionResponse");
	SendCommandToACL(_Ident, L2CAP_CONN_RSP, sizeof(SL2CAP_ConnectionResponse), (u8*)&Rsp);
}

void CWII_IPC_HLE_WiiMote::ReceiveConnectionResponse(u8 _Ident, u8* _pData, u32 _Size)
{
	l2cap_conn_rsp* rsp = (l2cap_conn_rsp*)_pData;

	_dbg_assert_(WII_IPC_WIIMOTE, _Size == sizeof(l2cap_conn_rsp));

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] ReceiveConnectionResponse");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    DCID: 0x%04x", rsp->dcid);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    SCID: 0x%04x", rsp->scid);
 	DEBUG_LOG(WII_IPC_WIIMOTE, "    Result: 0x%04x", rsp->result);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Status: 0x%04x", rsp->status);

	_dbg_assert_(WII_IPC_WIIMOTE, rsp->result == 0);
	_dbg_assert_(WII_IPC_WIIMOTE, rsp->status == 0);
	_dbg_assert_(WII_IPC_WIIMOTE, DoesChannelExist(rsp->scid));

	SChannel& rChannel = m_Channel[rsp->scid];
	rChannel.DCID = rsp->dcid;

	//
	// AyuanX: I'm commenting this out because CPU thinks he is faster than WiiMote
	// and basically CPU will take the initiative to config channel
	// in any case we don't want to race against CPU, or we are doomed
	// so we wait for CPU to request first
	//
	/*
	if (rChannel.PSM == HID_CONTROL_CHANNEL)
		m_HIDControlChannel_Connected = true;

	if (rChannel.PSM == HID_INTERRUPT_CHANNEL)
		m_HIDInterruptChannel_Connected = true;
	*/
}

void CWII_IPC_HLE_WiiMote::ReceiveConfigurationReq(u8 _Ident, u8* _pData, u32 _Size)
{
	u32 Offset = 0;
	SL2CAP_CommandConfigurationReq* pCommandConfigReq = (SL2CAP_CommandConfigurationReq*)_pData;

	_dbg_assert_(WII_IPC_WIIMOTE, pCommandConfigReq->flags == 0x00); // 1 means that the options are send in multi-packets
	_dbg_assert_(WII_IPC_WIIMOTE, DoesChannelExist(pCommandConfigReq->dcid));

	SChannel& rChannel = m_Channel[pCommandConfigReq->dcid];

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] ReceiveConfigurationRequest");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Ident: 0x%02x", _Ident);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    DCID: 0x%04x", pCommandConfigReq->dcid);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Flags: 0x%04x", pCommandConfigReq->flags);

	Offset += sizeof(SL2CAP_CommandConfigurationReq);

	u8 TempBuffer[1024];
	u32 RespLen = 0;

	SL2CAP_CommandConfigurationResponse* Rsp = (SL2CAP_CommandConfigurationResponse*)TempBuffer;
	Rsp->scid   = rChannel.DCID;
	Rsp->flags  = 0x00;
	Rsp->result = 0x00;

	RespLen += sizeof(SL2CAP_CommandConfigurationResponse);

	// read configuration options
	while (Offset < _Size)
	{
		SL2CAP_Options* pOptions = (SL2CAP_Options*)&_pData[Offset];
		Offset += sizeof(SL2CAP_Options);

		switch(pOptions->type)
		{
		case 0x01:
			{
				_dbg_assert_(WII_IPC_WIIMOTE, pOptions->length == 2);
				SL2CAP_OptionsMTU* pMTU = (SL2CAP_OptionsMTU*)&_pData[Offset];
				rChannel.MTU = pMTU->MTU;
				DEBUG_LOG(WII_IPC_WIIMOTE, "    MTU: 0x%04x", pMTU->MTU);
				// AyuanX: My experiment shows that the MTU is always set to 640 bytes
				// This means that we only need temp_frame_size of 640B instead 1024B
				// Actually I've never seen a frame bigger than 64B
				// But... who cares of several KB mem today? Never mind
			}
			break;

		case 0x02:
			{
				_dbg_assert_(WII_IPC_WIIMOTE, pOptions->length == 2);
				SL2CAP_OptionsFlushTimeOut* pFlushTimeOut = (SL2CAP_OptionsFlushTimeOut*)&_pData[Offset];
				rChannel.FlushTimeOut = pFlushTimeOut->TimeOut;
				DEBUG_LOG(WII_IPC_WIIMOTE, "    FlushTimeOut: 0x%04x", pFlushTimeOut->TimeOut);
			}
			break;

		default:
			_dbg_assert_msg_(WII_IPC_WIIMOTE, 0, "Unknown Option: 0x%02x", pOptions->type);
			break;
		}

		Offset += pOptions->length;

		u32 OptionSize = sizeof(SL2CAP_Options) + pOptions->length;
		memcpy(&TempBuffer[RespLen], pOptions, OptionSize);
		RespLen += OptionSize;
	}

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] SendConfigurationResponse");
	SendCommandToACL(_Ident, L2CAP_CONF_RSP, RespLen, TempBuffer);

	// update state machine
	if (rChannel.PSM == HID_CONTROL_CHANNEL)
		m_HIDControlChannel_Connected = true;
	else if (rChannel.PSM == HID_INTERRUPT_CHANNEL)
		m_HIDInterruptChannel_Connected = true;

}

void CWII_IPC_HLE_WiiMote::ReceiveConfigurationResponse(u8 _Ident, u8* _pData, u32 _Size)
{
	l2cap_conf_rsp* rsp = (l2cap_conf_rsp*)_pData;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] ReceiveConfigurationResponse");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    SCID: 0x%04x", rsp->scid);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Flags: 0x%04x", rsp->flags);
 	DEBUG_LOG(WII_IPC_WIIMOTE, "    Result: 0x%04x", rsp->result);

	_dbg_assert_(WII_IPC_WIIMOTE, rsp->result == 0);

	// update state machine
	SChannel& rChannel = m_Channel[rsp->scid];

	if (rChannel.PSM == HID_CONTROL_CHANNEL)
	{
		m_HIDControlChannel_Config = true;
		INFO_LOG(WII_IPC_WIIMOTE, "Building HID_CONTROL_CHANNEL -- OK");
	}
	else if (rChannel.PSM == HID_INTERRUPT_CHANNEL)
	{
		m_HIDInterruptChannel_Config = true;
		INFO_LOG(WII_IPC_WIIMOTE, "Building HID_INTERRUPT_CHANNEL -- OK");
	}
}

void CWII_IPC_HLE_WiiMote::ReceiveDisconnectionReq(u8 _Ident, u8* _pData, u32 _Size)
{
	SL2CAP_CommandDisconnectionReq* pCommandDisconnectionReq = (SL2CAP_CommandDisconnectionReq*)_pData;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] ReceiveDisconnectionReq");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Ident: 0x%02x", _Ident);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    DCID: 0x%04x", pCommandDisconnectionReq->dcid);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    SCID: 0x%04x", pCommandDisconnectionReq->scid);

	// response
	SL2CAP_CommandDisconnectionResponse Rsp;
	Rsp.dcid   = pCommandDisconnectionReq->dcid;
	Rsp.scid   = pCommandDisconnectionReq->scid;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] SendDisconnectionResponse");
	SendCommandToACL(_Ident, L2CAP_DISCONN_RSP, sizeof(SL2CAP_CommandDisconnectionResponse), (u8*)&Rsp);
}

//
//
//
//
// ---  Send Commands To CPU
//
//
//
//
//

// We assume WiiMote is always connected
void CWII_IPC_HLE_WiiMote::SendConnectionRequest(u16 scid, u16 psm)
{
	// create the channel
	SChannel& rChannel = m_Channel[scid];
	rChannel.PSM = psm;
	rChannel.SCID = scid;

	l2cap_conn_req cr;
	cr.psm = psm;
	cr.scid = scid;

	INFO_LOG(WII_IPC_WIIMOTE, "-----------------------------------------");
	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] SendConnectionRequest");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Psm: 0x%04x", cr.psm);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Scid: 0x%04x", cr.scid);

	SendCommandToACL(L2CAP_CONN_REQ, L2CAP_CONN_REQ, sizeof(l2cap_conn_req), (u8*)&cr);
}

// We don't initiatively disconnet Wiimote though ...
void CWII_IPC_HLE_WiiMote::SendDisconnectRequest(u16 scid)
{
	// create the channel
	SChannel& rChannel = m_Channel[scid];

	l2cap_disconn_req cr;
	cr.dcid = rChannel.DCID;
	cr.scid = rChannel.SCID;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] SendDisconnectionRequest");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Dcid: 0x%04x", cr.dcid);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Scid: 0x%04x", cr.scid);

	SendCommandToACL(L2CAP_DISCONN_REQ, L2CAP_DISCONN_REQ, sizeof(l2cap_disconn_req), (u8*)&cr);
}

void CWII_IPC_HLE_WiiMote::SendConfigurationRequest(u16 scid, u16 MTU, u16 FlushTimeOut)
{
	_dbg_assert_(WII_IPC_WIIMOTE, DoesChannelExist(scid));
	SChannel& rChannel = m_Channel[scid];

	u8 Buffer[1024];
	int Offset = 0;

	l2cap_conf_req* cr = (l2cap_conf_req*)&Buffer[Offset];
	cr->dcid = rChannel.DCID;
	cr->flags = 0;
	Offset += sizeof(l2cap_conf_req);

	SL2CAP_Options* pOptions;

	if (MTU == 0) MTU = rChannel.MTU;
	pOptions = (SL2CAP_Options*)&Buffer[Offset];
	Offset += sizeof(SL2CAP_Options);
	pOptions->type = 1;
	pOptions->length = 2;
	*(u16*)&Buffer[Offset] = MTU;
	Offset += 2;

	if (FlushTimeOut == 0) FlushTimeOut = rChannel.FlushTimeOut;
	pOptions = (SL2CAP_Options*)&Buffer[Offset];
	Offset += sizeof(SL2CAP_Options);
	pOptions->type = 2;
	pOptions->length = 2;
	*(u16*)&Buffer[Offset] = FlushTimeOut;
	Offset += 2;

	INFO_LOG(WII_IPC_WIIMOTE, "[L2CAP] SendConfigurationRequest");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Dcid: 0x%04x", cr->dcid);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Flags: 0x%04x", cr->flags);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    MTU: 0x%04x", MTU);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    FlushTimeOut: 0x%04x", FlushTimeOut);

	SendCommandToACL(L2CAP_CONF_REQ, L2CAP_CONF_REQ, Offset, Buffer);
}


//
//
//
//
// ---  SDP
//
//
//
//
//

#define SDP_UINT8  		0x08
#define SDP_UINT16		0x09
#define SDP_UINT32		0x0A
#define SDP_SEQ8		0x35
#define SDP_SEQ16		0x36

void CWII_IPC_HLE_WiiMote::SDPSendServiceSearchResponse(u16 cid, u16 TransactionID, u8* pServiceSearchPattern, u16 MaximumServiceRecordCount)
{
	// verify block... we handle search pattern for HID service only
	{
		CBigEndianBuffer buffer(pServiceSearchPattern);		
		_dbg_assert_(WII_IPC_WIIMOTE, buffer.Read8(0) == SDP_SEQ8);       // data sequence
		_dbg_assert_(WII_IPC_WIIMOTE, buffer.Read8(1) == 0x03);		  // sequence size

		// HIDClassID
		_dbg_assert_(WII_IPC_WIIMOTE, buffer.Read8(2) == 0x19);
		_dbg_assert_(WII_IPC_WIIMOTE, buffer.Read16(3) == 0x1124);
	}

	u8 DataFrame[1000];
	CBigEndianBuffer buffer(DataFrame);

	int Offset = 0;
	SL2CAP_Header* pHeader = (SL2CAP_Header*)&DataFrame[Offset]; Offset += sizeof(SL2CAP_Header);
	pHeader->CID = cid;

	buffer.Write8 (Offset, 0x03);				Offset++;
	buffer.Write16(Offset, TransactionID);		Offset += 2;		// transaction ID
	buffer.Write16(Offset, 0x0009);				Offset += 2;		// param length
	buffer.Write16(Offset, 0x0001);				Offset += 2;		// TotalServiceRecordCount
	buffer.Write16(Offset, 0x0001);				Offset += 2;		// CurrentServiceRecordCount
	buffer.Write32(Offset, 0x10000);			Offset += 4;		// ServiceRecordHandleList[4]
	buffer.Write8(Offset, 0x00);				Offset++;			// no continuation state;


	pHeader->Length = (u16)(Offset - sizeof(SL2CAP_Header));
	m_pHost->SendACLPacket(GetConnectionHandle(), DataFrame, pHeader->Length + sizeof(SL2CAP_Header));
}

u32 ParseCont(u8* pCont)
{
	u32 attribOffset = 0;
	CBigEndianBuffer attribList(pCont);
	u8 typeID      = attribList.Read8(attribOffset);		attribOffset++;

	if (typeID == 0x02)
	{
		return attribList.Read16(attribOffset);
	}
	else if (typeID == 0x00)
	{
		return 0x00;
	}
	ERROR_LOG(WII_IPC_WIIMOTE,"ParseCont: wrong cont: %i", typeID);
	PanicAlert("ParseCont: wrong cont: %i", typeID);
	return 0;
}


int ParseAttribList(u8* pAttribIDList, u16& _startID, u16& _endID)
{
	u32 attribOffset = 0;
	CBigEndianBuffer attribList(pAttribIDList);

	u8 sequence		= attribList.Read8(attribOffset);		attribOffset++;
	u8 seqSize		= attribList.Read8(attribOffset);		attribOffset++;
	u8 typeID      = attribList.Read8(attribOffset);		attribOffset++;

	_dbg_assert_(WII_IPC_WIIMOTE, sequence == SDP_SEQ8);

	if (typeID == SDP_UINT32)
	{
		_startID = attribList.Read16(attribOffset);			attribOffset += 2;
		_endID = attribList.Read16(attribOffset);			attribOffset += 2;
	}
	else
	{
		_startID = attribList.Read16(attribOffset);		attribOffset += 2;
		_endID = _startID;
		DEBUG_LOG(WII_IPC_WIIMOTE, "Read just a single attrib - not tested");
		PanicAlert("Read just a single attrib - not tested");
	}

	return attribOffset;
}


void CWII_IPC_HLE_WiiMote::SDPSendServiceAttributeResponse(u16 cid, u16 TransactionID, u32 ServiceHandle,
														   u16 startAttrID, u16 endAttrID,
														   u16 MaximumAttributeByteCount, u8* pContinuationState)
{
	if (ServiceHandle != 0x10000)
	{
		ERROR_LOG(WII_IPC_WIIMOTE, "unknown service handle %x" , ServiceHandle);
		PanicAlert("unknown service handle %x" , ServiceHandle);
	}


//	_dbg_assert_(WII_IPC_WIIMOTE, ServiceHandle == 0x10000);

	u32 contState = ParseCont(pContinuationState);

	u32 packetSize = 0;
	const u8* pPacket = GetAttribPacket(ServiceHandle, contState, packetSize);

	// generate package
	u8 DataFrame[1000];
	CBigEndianBuffer buffer(DataFrame);

	int Offset = 0;
	SL2CAP_Header* pHeader = (SL2CAP_Header*)&DataFrame[Offset]; Offset += sizeof(SL2CAP_Header);
	pHeader->CID = cid;
		
	buffer.Write8 (Offset, 0x05);										Offset++;
	buffer.Write16(Offset, TransactionID);								Offset += 2;			// transaction ID

	memcpy(buffer.GetPointer(Offset), pPacket, packetSize);				Offset += packetSize;

	pHeader->Length = (u16)(Offset - sizeof(SL2CAP_Header));
	m_pHost->SendACLPacket(GetConnectionHandle(), DataFrame, pHeader->Length + sizeof(SL2CAP_Header));

	//	Debugger::PrintDataBuffer(LogTypes::WIIMOTE, DataFrame, pHeader->Length + sizeof(SL2CAP_Header), "test response: ");
}

void CWII_IPC_HLE_WiiMote::HandleSDP(u16 cid, u8* _pData, u32 _Size)
{
	//	Debugger::PrintDataBuffer(LogTypes::WIIMOTE, _pData, _Size, "HandleSDP: ");

	CBigEndianBuffer buffer(_pData);

	switch(buffer.Read8(0))
	{
	// SDP_ServiceSearchRequest
	case 0x02:
		{
			WARN_LOG(WII_IPC_WIIMOTE, "!!! SDP_ServiceSearchRequest !!!");

			_dbg_assert_(WII_IPC_WIIMOTE, _Size == 13);

			u16 TransactionID = buffer.Read16(1);
			u16 ParameterLength = buffer.Read16(3);
			u8* pServiceSearchPattern = buffer.GetPointer(5);
			u16 MaximumServiceRecordCount = buffer.Read16(10);
			u8 ContinuationState = buffer.Read8(12);

			SDPSendServiceSearchResponse(cid, TransactionID, pServiceSearchPattern, MaximumServiceRecordCount);
		}
		break;

	// SDP_ServiceAttributeRequest
	case 0x04:
		{
			WARN_LOG(WII_IPC_WIIMOTE, "!!! SDP_ServiceAttributeRequest !!!");

			u16 startAttrID, endAttrID;
			u32 offset = 1;

			u16 TransactionID = buffer.Read16(offset);					offset += 2;
			u16 ParameterLength = buffer.Read16(offset);				offset += 2;
			u32 ServiceHandle = buffer.Read32(offset);					offset += 4;
			u16 MaximumAttributeByteCount = buffer.Read16(offset);		offset += 2;
			offset += ParseAttribList(buffer.GetPointer(offset), startAttrID, endAttrID);
			u8* pContinuationState =  buffer.GetPointer(offset);

			SDPSendServiceAttributeResponse(cid, TransactionID, ServiceHandle, startAttrID, endAttrID, MaximumAttributeByteCount, pContinuationState);
		}
		break;

	default:
		ERROR_LOG(WII_IPC_WIIMOTE, "WIIMOTE: Unknown SDP command %x", _pData[0]);
		PanicAlert("WIIMOTE: Unknown SDP command %x", _pData[0]);
		break;
	}
}


//
//
//
//
// ---  Data Send Functions
//
//
//
//
//

void CWII_IPC_HLE_WiiMote::SendCommandToACL(u8 _Ident, u8 _Code, u8 _CommandLength, u8* _pCommandData)
{
	u8 DataFrame[1024];
	u32 Offset = 0;

	SL2CAP_Header* pHeader = (SL2CAP_Header*)&DataFrame[Offset]; Offset += sizeof(SL2CAP_Header);
	pHeader->Length = sizeof(SL2CAP_Command) + _CommandLength;
	pHeader->CID = 0x0001;

	SL2CAP_Command* pCommand = (SL2CAP_Command*)&DataFrame[Offset]; Offset += sizeof(SL2CAP_Command);
	pCommand->code = _Code;
	pCommand->ident = _Ident;
	pCommand->len = _CommandLength;

	memcpy(&DataFrame[Offset], _pCommandData, _CommandLength);

	INFO_LOG(WII_IPC_WIIMOTE, "Send ACL Command to CPU");
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Ident: 0x%02x", _Ident);
	DEBUG_LOG(WII_IPC_WIIMOTE, "    Code: 0x%02x", _Code);

	// send ....
	m_pHost->SendACLPacket(GetConnectionHandle(), DataFrame, pHeader->Length + sizeof(SL2CAP_Header));

	//	Debugger::PrintDataBuffer(LogTypes::WIIMOTE, DataFrame, pHeader->Length + sizeof(SL2CAP_Header), "m_pHost->SendACLPacket: ");
}


// ===================================================
// On a second boot the _dbg_assert_(WII_IPC_WIIMOTE, DoesChannelExist(scid)) makes a report.
// However the game eventually starts and the Wiimote connects, but it takes at least ten seconds.
// ---------------------------------------------------
void CWII_IPC_HLE_WiiMote::ReceiveL2capData(u16 scid, const void* _pData, u32 _Size)
{
	// Allocate DataFrame
	u8 DataFrame[1024];
	u32 Offset = 0;
	SL2CAP_Header* pHeader = (SL2CAP_Header*)DataFrame;
	Offset += sizeof(SL2CAP_Header);

	// Check if we are already reporting on this channel
	_dbg_assert_(WII_IPC_WIIMOTE, DoesChannelExist(scid));
	SChannel& rChannel = m_Channel[scid];

	// Add an additonal 4 byte header to the Wiimote report
	pHeader->CID = rChannel.DCID;
	pHeader->Length = _Size;

	// Copy the Wiimote report to DataFrame
	memcpy(DataFrame + Offset, _pData, _Size);
	// Update Offset to the final size of the report
	Offset += _Size;

	// Update the status bar
	Host_SetWiiMoteConnectionState(2);

	// Send the report
	m_pHost->SendACLPacket(GetConnectionHandle(), DataFrame, Offset);
}



namespace Core
{
	/* This is called continuously from the Wiimote plugin as soon as it has received
	   a reporting mode. _Size is the byte size of the report. */
	void Callback_WiimoteInput(int _number, u16 _channelID, const void* _pData, u32 _Size)
	{
		const u8* pData = (const u8*)_pData;

		INFO_LOG(WIIMOTE, "====================");
		INFO_LOG(WIIMOTE, "Callback_WiimoteInput: (Wiimote: #%i)", _number);
		DEBUG_LOG(WIIMOTE, "   Data: %s", ArrayToString(pData, _Size, 0, 50).c_str());
		DEBUG_LOG(WIIMOTE, "   Channel: %u", _channelID);

		s_Usb->m_WiiMotes[_number].ReceiveL2capData(_channelID, _pData, _Size);
	}
}

