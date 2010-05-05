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

#ifndef _WII_IPC_HLE_DEVICE_USB_H_
#define _WII_IPC_HLE_DEVICE_USB_H_

#include "hci.h"
#include <vector>
#include <queue>
#include "WII_IPC_HLE.h"
#include "WII_IPC_HLE_Device.h"
#include "WII_IPC_HLE_WiiMote.h"

#define HCI_MAX_SIZE	128
#define ACL_MAX_SIZE	128

union UACLHeader
{
	struct
	{
		unsigned ConnectionHandle : 12;
		unsigned PBFlag : 2;
		unsigned BCFlag : 2;
		unsigned Size : 16;
	};
	u32 Hex;
};

struct SQueuedEvent
{
	u8 m_buffer[1024];
	u32 m_size;
	u16 m_connectionHandle;

	SQueuedEvent(u32 size, u16 connectionHandle)
		: m_size(size)
		, m_connectionHandle(connectionHandle)
	{
		if (m_size > 1024)
		{
			// i know this code sux...
			PanicAlert("SQueuedEvent: allocate too big buffer!!");
		}
	}
};


// Important to remember that this class is for /dev/usb/oh1/57e/305 ONLY
// /dev/usb/oh1 -> internal usb bus
// 57e/305 -> VendorID/ProductID of device on usb bus
// This device is ONLY the internal bluetooth module (based on BCM2045 chip)
class CWII_IPC_HLE_Device_usb_oh1_57e_305 : public IWII_IPC_HLE_Device
{
public:

	CWII_IPC_HLE_Device_usb_oh1_57e_305(u32 _DeviceID, const std::string& _rDeviceName);

	virtual ~CWII_IPC_HLE_Device_usb_oh1_57e_305();

	virtual bool Open(u32 _CommandAddress, u32 _Mode);
	virtual bool Close(u32 _CommandAddress, bool _bForce);

	virtual bool IOCtlV(u32 _CommandAddress);
	virtual bool IOCtl(u32 _CommandAddress);

	virtual u32 Update();

	void SendACLPacket(u16 _ConnectionHandle, u8* _pData, u32 _Size);
	void PurgeACLPool();
	void PurgeHCIPool();

	bool RemoteDisconnect(u16 _connectionHandle);

	//hack for wiimote plugin

public:	

	std::vector<CWII_IPC_HLE_WiiMote> m_WiiMotes;
	CWII_IPC_HLE_WiiMote* AccessWiiMote(const bdaddr_t& _rAddr);
	CWII_IPC_HLE_WiiMote* AccessWiiMote(u16 _ConnectionHandle);

	void DoState(PointerWrap &p);

	void NetPlay_WiimoteUpdate(int _number);

private:

	enum
	{
		USB_IOCTL_HCI_COMMAND_MESSAGE	= 0,
		USB_IOCTL_BLKMSG				= 1,
		USB_IOCTL_INTRMSG				= 2,
		USB_IOCTL_SUSPENDDEV			= 5,
		USB_IOCTL_RESUMEDEV				= 6,
		USB_IOCTL_GETDEVLIST			= 12,
		USB_IOCTL_DEVREMOVALHOOK		= 26,
		USB_IOCTL_DEVINSERTHOOK			= 27,
	};

	enum
	{
		HCI_EVENT_ENDPOINT				= 0x81,
		ACL_DATA_BLK_OUT				= 0x02,
		ACL_DATA_ENDPOINT				= 0x82,
	};


	struct SHCICommandMessage 
	{
		u8  bRequestType;
		u8  bRequest;
		u16 wValue;
		u16 wIndex;
		u16 wLength;

		u32 m_PayLoadAddr;
		u32 m_PayLoadSize;
		u32 m_Address;
	};

	struct ACLPool 
	{
		u32 m_number;
		u8 m_data[ACL_MAX_SIZE * 16];

		ACLPool(int num)
			: m_number(num)
		{
		}
	};

	struct HCIPool 
	{
		u32 m_number;
		u8 m_data[HCI_MAX_SIZE * 16];
		u8 m_size[16];

		HCIPool(int num)
			: m_number(num)
		{
		}
	};

	struct CtrlBuffer 
	{
		u32 m_address;
		u32 m_buffer;

		CtrlBuffer(u32 _Address)
			: m_address(_Address)
		{
			if(_Address == 0)
			{
				m_buffer = 0;
			}
			else
			{
				u32 _BufferVector = Memory::Read_U32(_Address + 0x18);
				u32 _InBufferNum = Memory::Read_U32(_Address + 0x10);
				m_buffer = Memory::Read_U32(_BufferVector + _InBufferNum * 8);
			}
		}
	};

	bdaddr_t m_ControllerBD;
	u8 m_ClassOfDevice[HCI_CLASS_SIZE];
	char m_LocalName[HCI_UNIT_NAME_SIZE];
	u8 m_PINType;
	u8 m_ScanEnable;

	u8 m_EventFilterType;
	u8 m_EventFilterCondition;

	u16 m_HostMaxACLSize; 
	u8  m_HostMaxSCOSize; 
	u16 m_HostNumACLPackets; 
	u16 m_HostNumSCOPackets;

	// STATE_TO_SAVE
	SHCICommandMessage m_CtrlSetup;
	u32 m_ACLSetup;
	CtrlBuffer m_HCIBuffer;
	HCIPool m_HCIPool;
	CtrlBuffer m_ACLBuffer;
	ACLPool m_ACLPool;
	u32 m_LastCmd;
	u32 m_PacketCount[4];
	u32 m_FreqDividerMote;
	u32 m_FreqDividerSync;

	// Events
	void AddEventToQueue(const SQueuedEvent& _event);
	bool SendEventCommandStatus(u16 _Opcode);
	void SendEventCommandComplete(u16 _OpCode, void* _pData, u32 _DataSize);
	bool SendEventInquiryResponse();
	bool SendEventInquiryComplete();
	bool SendEventRemoteNameReq(const bdaddr_t& _bd);
	bool SendEventRequestConnection(CWII_IPC_HLE_WiiMote& _rWiiMote);
	bool SendEventConnectionComplete(const bdaddr_t& _bd);
	bool SendEventReadClockOffsetComplete(u16 _connectionHandle);
	bool SendEventConPacketTypeChange(u16 _connectionHandle, u16 _packetType);
	bool SendEventReadRemoteVerInfo(u16 _connectionHandle);
	bool SendEventReadRemoteFeatures(u16 _connectionHandle);
	bool SendEventRoleChange(bdaddr_t _bd, bool _master);
	bool SendEventNumberOfCompletedPackets(u16 _connectionHandle, u16 _count);
	bool SendEventAuthenticationCompleted(u16 _connectionHandle);	
	bool SendEventModeChange(u16 _connectionHandle, u8 _mode, u16 _value);
	bool SendEventDisconnect(u16 _connectionHandle, u8 _Reason);
	bool SendEventRequestLinkKey(const bdaddr_t& _bd);
	bool SendEventLinkKeyNotification(const CWII_IPC_HLE_WiiMote& _rWiiMote);

	// Execute HCI Message
	void ExecuteHCICommandMessage(const SHCICommandMessage& _rCtrlMessage);

	// OGF 0x01	Link control commands and return parameters
	void CommandWriteInquiryMode(u8* _Input);
	void CommandWritePageScanType(u8* _Input);
	void CommandHostBufferSize(u8* _Input);
	void CommandInquiryCancel(u8* _Input);
	void CommandRemoteNameReq(u8* _Input);
	void CommandCreateCon(u8* _Input);
	void CommandAcceptCon(u8* _Input);
	void CommandReadClockOffset(u8* _Input);
	void CommandReadRemoteVerInfo(u8* _Input);
	void CommandReadRemoteFeatures(u8* _Input);
	void CommandAuthenticationRequested(u8* _Input);
	void CommandInquiry(u8* _Input);
	void CommandDisconnect(u8* _Input);
	void CommandLinkKeyNegRep(u8* _Input);
	void CommandLinkKeyRep(u8* _Input);
    void CommandDeleteStoredLinkKey(u8* _Input);
	void CommandChangeConPacketType(u8* _Input);

	// OGF 0x02	Link policy commands and return parameters
	void CommandWriteLinkPolicy(u8* _Input);
	void CommandSniffMode(u8* _Input);

	// OGF 0x03	Host Controller and Baseband commands and return parameters
	void CommandReset(u8* _Input);
	void CommandWriteLocalName(u8* _Input);
	void CommandWritePageTimeOut(u8* _Input);
	void CommandWriteScanEnable(u8* _Input);
	void CommandWriteUnitClass(u8* _Input);
	void CommandReadStoredLinkKey(u8* _Input);
	void CommandWritePinType(u8* _Input);
	void CommandSetEventFilter(u8* _Input);
	void CommandWriteInquiryScanType(u8* _Input);
	void CommandWriteLinkSupervisionTimeout(u8* _Input);

	// OGF 0x04	Informational commands and return parameters 
	void CommandReadBufferSize(u8* _Input);
	void CommandReadLocalVer(u8* _Input);
	void CommandReadLocalFeatures(u8* _Input);
	void CommandReadBDAdrr(u8* _Input);

	// OGF 0x3F Vendor specific
	void CommandVendorSpecific_FC4C(u8* _Input, u32 _Size);
	void CommandVendorSpecific_FC4F(u8* _Input, u32 _Size);	

	void SendToDevice(u16 _ConnectionHandle, u8* _pData, u32 _Size);

	void LOG_LinkKey(const u8* _pLinkKey);
};

class CWII_IPC_HLE_Device_usb_oh0 : public IWII_IPC_HLE_Device
{
public:

	CWII_IPC_HLE_Device_usb_oh0(u32 _DeviceID, const std::string& _rDeviceName);

	virtual ~CWII_IPC_HLE_Device_usb_oh0();

	virtual bool Open(u32 _CommandAddress, u32 _Mode);
	virtual bool Close(u32 _CommandAddress, bool _bForce);  // hermes' dsp demo

	virtual bool IOCtlV(u32 _CommandAddress);
	virtual bool IOCtl(u32 _CommandAddress);

//	virtual u32 Update();
};


// Addresses Human Interface Devices via the Wii's USB 2.0 ports.
// Used by Rock Band 1 + 2 instruments.
class CWII_IPC_HLE_Device_usb_hid : public IWII_IPC_HLE_Device
{
public:
	CWII_IPC_HLE_Device_usb_hid(u32 _DeviceID, const std::string& _rDeviceName);
	virtual ~CWII_IPC_HLE_Device_usb_hid();

	virtual bool Open(u32 _CommandAddress, u32 _Mode);
	virtual bool Close(u32 _CommandAddress, bool _bForce);

	virtual bool IOCtlV(u32 _CommandAddress);
	virtual bool IOCtl(u32 _CommandAddress);

	//	virtual u32 Update();
};

#endif

