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



// =======================================================
// File description
// -------------
/*  Here we handle /dev/es requests. We have cases for these functions, the exact
	DevKitPro/libogc name is in parenthesis:

	0x20 GetTitleID (ES_GetTitleID) (Input: none, Output: 8 bytes)
	0x1d GetDataDir (ES_GetDataDir) (Input: 8 bytes, Output: 30 bytes)

	0x1b DiGetTicketView (Input: none, Output: 216 bytes) 
	0x16 GetConsumption (Input: 8 bytes, Output: 0 bytes, 4 bytes) // there are two output buffers

	0x12 GetNumTicketViews (ES_GetNumTicketViews) (Input: 8 bytes, Output: 4 bytes)
	0x14 GetTMDViewSize (ES_GetTMDViewSize) (Input: ?, Output: ?) // I don't get this anymore,
		it used to come after 0x12

	but only the first two are correctly supported. For the other four we ignore any potential
	input and only write zero to the out buffer. However, most games only use first two,
	but some Nintendo developed games use the other ones to:
	
	0x1b: Mario Galaxy, Mario Kart, SSBB
	0x16: Mario Galaxy, Mario Kart, SSBB
	0x12: Mario Kart
	0x14: Mario Kart: But only if we don't return a zeroed out buffer for the 0x12 question,
		and instead answer for example 1 will this question appear.
 
*/
// =============

#include "WII_IPC_HLE_Device_es.h"

#include "../PowerPC/PowerPC.h"
#include "../VolumeHandler.h"
#include "FileUtil.h"
#include "AES/aes.h"

#include "../Boot/Boot_DOL.h"
#include "NandPaths.h"
#include "CommonPaths.h"
CWII_IPC_HLE_Device_es::CWII_IPC_HLE_Device_es(u32 _DeviceID, const std::string& _rDeviceName) 
    : IWII_IPC_HLE_Device(_DeviceID, _rDeviceName)
    , m_pContentLoader(NULL)
    , m_TitleID(-1)
    , AccessIdentID(0x6000000)
	, m_ContentFile()
{}

CWII_IPC_HLE_Device_es::~CWII_IPC_HLE_Device_es()
{}

void CWII_IPC_HLE_Device_es::LoadWAD(const std::string& _rContentFile) 
{
	m_ContentFile = _rContentFile;
}

bool CWII_IPC_HLE_Device_es::Open(u32 _CommandAddress, u32 _Mode)
{
    m_pContentLoader = &DiscIO::CNANDContentManager::Access().GetNANDLoader(m_ContentFile);

    // check for cd ...
    if (m_pContentLoader->IsValid())
    {
        m_TitleID = m_pContentLoader->GetTitleID();

		m_TitleIDs.clear();
		DiscIO::cUIDsys::AccessInstance().GetTitleIDs(m_TitleIDs);
		// uncomment if  ES_GetOwnedTitlesCount / ES_GetOwnedTitles is implemented
		// m_TitleIDsOwned.clear();
		// DiscIO::cUIDsys::AccessInstance().GetTitleIDs(m_TitleIDsOwned, true);
    }
    else if (VolumeHandler::IsValid())
    {
		// blindly grab the titleID from the disc - it's unencrypted at:
		// offset 0x0F8001DC and 0x0F80044C
		VolumeHandler::RAWReadToPtr((u8*)&m_TitleID, (u64)0x0F8001DC, 8);
		m_TitleID = Common::swap64(m_TitleID);
    }
    else
    {
        m_TitleID = ((u64)0x00010000 << 32) | 0xF00DBEEF;
    }   

    INFO_LOG(WII_IPC_ES, "Set default title to %08x/%08x", (u32)(m_TitleID>>32), (u32)m_TitleID);

	Memory::Write_U32(GetDeviceID(), _CommandAddress+4);
	if (m_Active)
		INFO_LOG(WII_IPC_ES, "Device was ReOpened");
	m_Active = true;
    return true;
}

bool CWII_IPC_HLE_Device_es::Close(u32 _CommandAddress, bool _bForce)
{
    // Leave deletion of the INANDContentLoader objects to CNANDContentManager, don't do it here!
    m_NANDContent.clear();
	m_ContentAccessMap.clear();
	m_pContentLoader = NULL;
	m_TitleIDs.clear();
    m_TitleID = -1;
	AccessIdentID = 0x6000000;

    INFO_LOG(WII_IPC_ES, "ES: Close");
    if (!_bForce)
		Memory::Write_U32(0, _CommandAddress + 4);
	m_Active = false;
	return true;
}

bool CWII_IPC_HLE_Device_es::IOCtlV(u32 _CommandAddress) 
{
    SIOCtlVBuffer Buffer(_CommandAddress);

    DEBUG_LOG(WII_IPC_ES, "%s (0x%x)", GetDeviceName().c_str(), Buffer.Parameter);

    // Prepare the out buffer(s) with zeroes as a safety precaution
    // to avoid returning bad values
    for (u32 i = 0; i < Buffer.NumberPayloadBuffer; i++)
    {
        Memory::Memset(Buffer.PayloadBuffer[i].m_Address, 0,
            Buffer.PayloadBuffer[i].m_Size);
    }

	// Uhh just put this here for now
	u8 keyTable[11][16] = {
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // ECC Private Key
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // Console ID
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // NAND AES Key
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // NAND HMAC
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // Common Key
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // PRNG seed
		{0xab, 0x01, 0xb9, 0xd8, 0xe1, 0x62, 0x2b, 0x08, 0xaf, 0xba, 0xd8, 0x4d, 0xbf, 0xc2, 0xa5, 0x5d,}, // SD Key
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // Unknown
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // Unknown
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // Unknown
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, // Unknown
	};

	switch (Buffer.Parameter)
    {
	case IOCTL_ES_GETDEVICEID:
		{
			_dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETDEVICEID no out buffer");

			INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETDEVICEID");
			// Return arbitrary device ID - TODO allow user to set value?
			Memory::Write_U32(0x31337f11, Buffer.PayloadBuffer[0].m_Address);
			Memory::Write_U32(0, _CommandAddress + 0x4);
			return true;
		}
		break;

	case IOCTL_ES_GETTITLECONTENTSCNT:
		{
			_dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 1);
			_dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1);    

			u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);

			const DiscIO::INANDContentLoader& rNANDCOntent = AccessContentDevice(TitleID);
			u16 NumberOfPrivateContent = 0;
			if (rNANDCOntent.IsValid()) // Not sure if dolphin will ever fail this check
			{
				NumberOfPrivateContent = rNANDCOntent.GetNumEntries();

				if ((u32)(TitleID>>32) == 0x00010000)
					Memory::Write_U32(0, Buffer.PayloadBuffer[0].m_Address);
				else
					Memory::Write_U32(NumberOfPrivateContent, Buffer.PayloadBuffer[0].m_Address);

				Memory::Write_U32(0, _CommandAddress + 0x4);
			}
			else
				Memory::Write_U32((u32)rNANDCOntent.GetContentSize(), _CommandAddress + 0x4);

			INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLECONTENTSCNT: TitleID: %08x/%08x  content count %i", 
				(u32)(TitleID>>32), (u32)TitleID, rNANDCOntent.IsValid() ? NumberOfPrivateContent : (u32)rNANDCOntent.GetContentSize());

			return true;
		}
		break;

    case IOCTL_ES_GETTITLECONTENTS:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 2, "IOCTL_ES_GETTITLECONTENTS bad in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETTITLECONTENTS bad out buffer");

			u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);

			const DiscIO::INANDContentLoader& rNANDCOntent = AccessContentDevice(TitleID);
			if (rNANDCOntent.IsValid()) // Not sure if dolphin will ever fail this check
			{
				for (u16 i = 0; i < rNANDCOntent.GetNumEntries(); i++)
				{
					Memory::Write_U32(rNANDCOntent.GetContentByIndex(i)->m_ContentID, Buffer.PayloadBuffer[0].m_Address + i*4);
					INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLECONTENTS: Index %d: %08x", i, rNANDCOntent.GetContentByIndex(i)->m_ContentID);
				}
				Memory::Write_U32(0, _CommandAddress + 0x4);
			}
			else
			{
				Memory::Write_U32((u32)rNANDCOntent.GetContentSize(), _CommandAddress + 0x4);
				INFO_LOG(WII_IPC_ES,
					"IOCTL_ES_GETTITLECONTENTS: "
					"Unable to open content %lu",
					(unsigned long)rNANDCOntent.\
						GetContentSize());
			}

            return true;
        }
        break;


    case IOCTL_ES_OPENTITLECONTENT:
        {
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 3);
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 0);
            
            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            u32 Index = Memory::Read_U32(Buffer.InBuffer[2].m_Address);

            u32 CFD = AccessIdentID++;
            m_ContentAccessMap[CFD].m_Position = 0;
            m_ContentAccessMap[CFD].m_pContent = AccessContentDevice(TitleID).GetContentByIndex(Index);
            _dbg_assert_msg_(WII_IPC_ES, m_ContentAccessMap[CFD].m_pContent != NULL, "No Content for TitleID: %08x/%08x at Index %x", (u32)(TitleID>>32), (u32)TitleID, Index);
			// Fix for DLC by itsnotmailmail
			if (m_ContentAccessMap[CFD].m_pContent == NULL)
				CFD = 0xffffffff; //TODO: what is the correct error value here?
            Memory::Write_U32(CFD, _CommandAddress + 0x4);		

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_OPENTITLECONTENT: TitleID: %08x/%08x  Index %i -> got CFD %x", (u32)(TitleID>>32), (u32)TitleID, Index, CFD);
            return true;
        }
        break;

    case IOCTL_ES_OPENCONTENT:
        {
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 1);
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 0);

            u32 CFD = AccessIdentID++;
            u32 Index = Memory::Read_U32(Buffer.InBuffer[0].m_Address);

            m_ContentAccessMap[CFD].m_Position = 0;
            m_ContentAccessMap[CFD].m_pContent = AccessContentDevice(m_TitleID).GetContentByIndex(Index);
            _dbg_assert_(WII_IPC_ES, m_ContentAccessMap[CFD].m_pContent != NULL);

            Memory::Write_U32(CFD, _CommandAddress + 0x4);		

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_OPENCONTENT: Index %i -> got CFD %x", Index, CFD);
            return true;
        }
        break;

    case IOCTL_ES_READCONTENT:
        {
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 1);
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1);

            u32 CFD = Memory::Read_U32(Buffer.InBuffer[0].m_Address);
            u32 Size = Buffer.PayloadBuffer[0].m_Size;
            u32 Addr = Buffer.PayloadBuffer[0].m_Address;

            _dbg_assert_(WII_IPC_ES, m_ContentAccessMap.find(CFD) != m_ContentAccessMap.end());
            SContentAccess& rContent = m_ContentAccessMap[CFD];

            _dbg_assert_(WII_IPC_ES, rContent.m_pContent->m_pData != NULL);

            u8* pSrc = &rContent.m_pContent->m_pData[rContent.m_Position];
            u8* pDest = Memory::GetPointer(Addr);

            if (rContent.m_Position + Size > rContent.m_pContent->m_Size) 
            {
                Size = rContent.m_pContent->m_Size-rContent.m_Position;
            }

            if (Size > 0)
            {
				if (pDest) {
					memcpy(pDest, pSrc, Size);
					rContent.m_Position += Size;
				} else {
					PanicAlert("IOCTL_ES_READCONTENT - bad destination");
				}
            }

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_READCONTENT: CFD %x, Addr 0x%x, Size %i -> stream pos %i (Index %i)", CFD, Addr, Size, rContent.m_Position, rContent.m_pContent->m_Index);

            Memory::Write_U32(Size, _CommandAddress + 0x4);		
            return true;
        }
        break;

    case IOCTL_ES_CLOSECONTENT:
        {
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 1);
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 0);

            u32 CFD = Memory::Read_U32(Buffer.InBuffer[0].m_Address);

            CContentAccessMap::iterator itr = m_ContentAccessMap.find(CFD);
            m_ContentAccessMap.erase(itr);

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_CLOSECONTENT: CFD %x", CFD);

            Memory::Write_U32(0, _CommandAddress + 0x4);		
            return true;
        }
        break;

    case IOCTL_ES_SEEKCONTENT:
        {	
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 3);
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 0);

            u32 CFD = Memory::Read_U32(Buffer.InBuffer[0].m_Address);
            u32 Addr = Memory::Read_U32(Buffer.InBuffer[1].m_Address);
            u32 Mode = Memory::Read_U32(Buffer.InBuffer[2].m_Address);

            _dbg_assert_(WII_IPC_ES, m_ContentAccessMap.find(CFD) != m_ContentAccessMap.end());
            SContentAccess& rContent = m_ContentAccessMap[CFD];

            switch (Mode)
            {
            case 0:  // SET
                rContent.m_Position = Addr;
                break;

            case 1:  // CUR
                rContent.m_Position += Addr;
                break;

            case 2:  // END
                rContent.m_Position = rContent.m_pContent->m_Size + Addr;
                break;
            }

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_SEEKCONTENT: CFD %x, Addr 0x%x, Mode %i -> Pos %i", CFD, Addr, Mode, rContent.m_Position);

            Memory::Write_U32(rContent.m_Position, _CommandAddress + 0x4);		
            return true;
        }
        break;

    case IOCTL_ES_GETTITLEDIR:
        {          
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 1);
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1);

            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);

            char* Path = (char*)Memory::GetPointer(Buffer.PayloadBuffer[0].m_Address);
            sprintf(Path, "/%08x/%08x/data", (u32)(TitleID >> 32), (u32)TitleID);

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLEDIR: %s", Path);
        }
        break;

    case IOCTL_ES_GETTITLEID:
        {
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 0);
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETTITLEID no out buffer");

            Memory::Write_U64(m_TitleID, Buffer.PayloadBuffer[0].m_Address);
            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLEID: %08x/%08x", (u32)(m_TitleID>>32), (u32)m_TitleID);
        }
        break;

    case IOCTL_ES_SETUID:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 1, "IOCTL_ES_SETUID no in buffer");
            _dbg_assert_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 0);
			// TODO: fs permissions based on this
            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            INFO_LOG(WII_IPC_ES, "IOCTL_ES_SETUID titleID: %08x/%08x", (u32)(TitleID>>32), (u32)TitleID);
        }
        break;

    case IOCTL_ES_GETTITLECNT:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 0, "IOCTL_ES_GETTITLECNT has an in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETTITLECNT has no out buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.PayloadBuffer[0].m_Size == 4, "IOCTL_ES_GETTITLECNT payload[0].size != 4");

            Memory::Write_U32((u32)m_TitleIDs.size(), Buffer.PayloadBuffer[0].m_Address);

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLECNT: Number of Titles %lu",
                (unsigned long)m_TitleIDs.size());

            Memory::Write_U32(0, _CommandAddress + 0x4);		

            return true;
        }
        break;


    case IOCTL_ES_GETTITLES:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 1, "IOCTL_ES_GETTITLES has an in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETTITLES has no out buffer");

            u32 MaxCount = Memory::Read_U32(Buffer.InBuffer[0].m_Address);
            u32 Count = 0;
            for (int i = 0; i < (int)m_TitleIDs.size(); i++)
            {
                Memory::Write_U64(m_TitleIDs[i], Buffer.PayloadBuffer[0].m_Address + i*8);
                INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLES: %08x/%08x", (u32)(m_TitleIDs[i] >> 32), (u32)m_TitleIDs[i]);
                Count++;
                if (Count >= MaxCount)
                    break;
            }

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTITLES: Number of titles returned %i", Count);
            Memory::Write_U32(0, _CommandAddress + 0x4);		
            return true;
        }
        break;


    case IOCTL_ES_GETVIEWCNT:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 1, "IOCTL_ES_GETVIEWCNT no in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETVIEWCNT no out buffer");

            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
			std::string TicketFilename = Common::CreateTicketFileName(TitleID);

            u32 ViewCount = 0;
            if (File::Exists(TicketFilename.c_str()))
            {
                const u32 SIZE_OF_ONE_TICKET = 676;

                u32 FileSize = (u32)(File::GetSize(TicketFilename.c_str()));
                _dbg_assert_msg_(WII_IPC_ES, (FileSize % SIZE_OF_ONE_TICKET) == 0, "IOCTL_ES_GETVIEWCNT ticket file size seems to be wrong");

                ViewCount = FileSize / SIZE_OF_ONE_TICKET;
                _dbg_assert_msg_(WII_IPC_ES, (ViewCount>0) && (ViewCount<=4), "IOCTL_ES_GETVIEWCNT ticket count seems to be wrong");
            }
            else
            {
                if (TitleID == TITLEID_SYSMENU)
                {
                    PanicAlert("There must be a ticket for 00000001/00000002. Your NAND dump is probably incomplete.");
                }
                ViewCount = 0;
            }
            
            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETVIEWCNT for titleID: %08x/%08x (View Count = %i)", (u32)(TitleID>>32), (u32)TitleID, ViewCount);

            Memory::Write_U32(ViewCount, Buffer.PayloadBuffer[0].m_Address);
            Memory::Write_U32(0, _CommandAddress + 0x4);
            return true;
        }
        break;

    case IOCTL_ES_GETVIEWS:
		{
			_dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 2, "IOCTL_ES_GETVIEWS no in buffer");
			_dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETVIEWS no out buffer");

			u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
			u32 maxViews = Memory::Read_U32(Buffer.InBuffer[1].m_Address);

			std::string TicketFilename = Common::CreateTicketFileName(TitleID);
			if (File::Exists(TicketFilename.c_str()))
			{
				const u32 SIZE_OF_ONE_TICKET = 676;
				FILE* pFile = fopen(TicketFilename.c_str(), "rb");
				if (pFile)
				{
					unsigned int View = 0;
					u8 Ticket[SIZE_OF_ONE_TICKET];
					while (View < maxViews && fread(Ticket, SIZE_OF_ONE_TICKET, 1, pFile) == 1)
					{
						Memory::Write_U32(View, Buffer.PayloadBuffer[0].m_Address + View * 0xD8);
						Memory::WriteBigEData(Ticket+0x1D0, Buffer.PayloadBuffer[0].m_Address + 4 + View * 0xD8, 212);
						View++;
					}
					fclose(pFile);
				}
			}
			else
			{
				PanicAlert("IOCTL_ES_GETVIEWS: Tried to get data from an unknown ticket: %08x/%08x", (u32)(TitleID >> 32), (u32)TitleID);
			}

			INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETVIEWS for titleID: %08x/%08x (MaxViews = %i)", (u32)(TitleID >> 32), (u32)TitleID, maxViews);

			Memory::Write_U32(0, _CommandAddress + 0x4);
			return true;
		}
        break;

    case IOCTL_ES_GETTMDVIEWCNT:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 1, "IOCTL_ES_GETTMDVIEWCNT no in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETTMDVIEWCNT no out buffer");

            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            const DiscIO::INANDContentLoader& Loader = DiscIO::CNANDContentManager::Access().GetNANDLoader(TitleID);

            _dbg_assert_msg_(WII_IPC_ES, Loader.IsValid(), "Loader not valid for TitleID %08x/%08x", (u32)(TitleID >> 32), (u32)TitleID);
            u32 TMDViewCnt = 0;
            if (Loader.IsValid())
            {
                TMDViewCnt += DiscIO::INANDContentLoader::TICKET_VIEW_SIZE; 
                TMDViewCnt += 2; // title version
                TMDViewCnt += 2; // num entries
                TMDViewCnt += (u32)Loader.GetContentSize() * (4+2+2+8); // content id, index, type, size
            }
            Memory::Write_U32(TMDViewCnt, Buffer.PayloadBuffer[0].m_Address);

            Memory::Write_U32(0, _CommandAddress + 0x4);		

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTMDVIEWCNT: title: %08x/%08x (view size %i)", (u32)(TitleID >> 32), (u32)TitleID, TMDViewCnt);
            return true;            
        }
        break;

    case IOCTL_ES_GETTMDVIEWS:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 2, "IOCTL_ES_GETTMDVIEWCNT no in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETTMDVIEWCNT no out buffer");

            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            u32 MaxCount = Memory::Read_U32(Buffer.InBuffer[1].m_Address);
            const DiscIO::INANDContentLoader& Loader = DiscIO::CNANDContentManager::Access().GetNANDLoader(TitleID);            


            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTMDVIEWCNT: title: %08x/%08x   buffersize: %i", (u32)(TitleID >> 32), (u32)TitleID, MaxCount);

            if (Loader.IsValid())
            {
                u32 Address = Buffer.PayloadBuffer[0].m_Address;

                Memory::WriteBigEData(Loader.GetTicketView(), Address, DiscIO::INANDContentLoader::TICKET_VIEW_SIZE);
                Address += DiscIO::INANDContentLoader::TICKET_VIEW_SIZE;
                
                Memory::Write_U16(Loader.GetTitleVersion(), Address); Address += 2;
                Memory::Write_U16(Loader.GetNumEntries(), Address); Address += 2;

                const std::vector<DiscIO::SNANDContent>& rContent = Loader.GetContent();
                for (size_t i=0; i<Loader.GetContentSize(); i++)
                {
                    Memory::Write_U32(rContent[i].m_ContentID,  Address); Address += 4;
                    Memory::Write_U16(rContent[i].m_Index,      Address); Address += 2;
                    Memory::Write_U16(rContent[i].m_Type,       Address); Address += 2;
                    Memory::Write_U64(rContent[i].m_Size,       Address); Address += 8;
                }

                _dbg_assert_(WII_IPC_ES, (Address-Buffer.PayloadBuffer[0].m_Address) == Buffer.PayloadBuffer[0].m_Size);
            }
            Memory::Write_U32(0, _CommandAddress + 0x4);	

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETTMDVIEWS: title: %08x/%08x (buffer size: %i)", (u32)(TitleID >> 32), (u32)TitleID, MaxCount);
            return true;            
        }
        break;

	case IOCTL_ES_GETCONSUMPTION: // This is at least what crediar's ES module does
		Memory::Write_U32(0, Buffer.PayloadBuffer[1].m_Address);
		Memory::Write_U32(0, _CommandAddress + 0x4);
		WARN_LOG(WII_IPC_ES, "IOCTL_ES_GETCONSUMPTION:%d", Memory::Read_U32(_CommandAddress+4));
		return true;

    case IOCTL_ES_GETSTOREDTMD:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 2, "IOCTL_ES_GETSTOREDTMD no in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_GETSTOREDTMD no out buffer");

            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            u32 MaxCount = Memory::Read_U32(Buffer.InBuffer[1].m_Address);
            const DiscIO::INANDContentLoader& Loader = DiscIO::CNANDContentManager::Access().GetNANDLoader(TitleID);            


            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETSTOREDTMD: title: %08x/%08x   buffersize: %i", (u32)(TitleID >> 32), (u32)TitleID, MaxCount);

            if (Loader.IsValid())
            {
                u32 Address = Buffer.PayloadBuffer[0].m_Address;

				Memory::WriteBigEData(Loader.GetTmdHeader(), Address, DiscIO::INANDContentLoader::TMD_HEADER_SIZE);
                Address += DiscIO::INANDContentLoader::TMD_HEADER_SIZE;
                
                const std::vector<DiscIO::SNANDContent>& rContent = Loader.GetContent();
                for (size_t i=0; i<Loader.GetContentSize(); i++)
                {
					Memory::WriteBigEData(rContent[i].m_Header, Address, DiscIO::INANDContentLoader::CONTENT_HEADER_SIZE); 
					Address += DiscIO::INANDContentLoader::CONTENT_HEADER_SIZE;
                }

                _dbg_assert_(WII_IPC_ES, (Address-Buffer.PayloadBuffer[0].m_Address) == Buffer.PayloadBuffer[0].m_Size);
            }
            Memory::Write_U32(0, _CommandAddress + 0x4);	

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETSTOREDTMD: title: %08x/%08x (buffer size: %i)", (u32)(TitleID >> 32), (u32)TitleID, MaxCount);
            return true;            
        }
        break;

    case IOCTL_ES_GETSTOREDTMDSIZE:
        {
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberInBuffer == 1, "IOCTL_ES_GETSTOREDTMDSIZE no in buffer");
            _dbg_assert_msg_(WII_IPC_ES, Buffer.NumberPayloadBuffer == 1, "IOCTL_ES_ES_GETSTOREDTMDSIZE no out buffer");

            u64 TitleID = Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            const DiscIO::INANDContentLoader& Loader = DiscIO::CNANDContentManager::Access().GetNANDLoader(TitleID);

            _dbg_assert_(WII_IPC_ES, Loader.IsValid());
            u32 TMDCnt = 0;
            if (Loader.IsValid())
            {
				TMDCnt += DiscIO::INANDContentLoader::TMD_HEADER_SIZE;
                TMDCnt += (u32)Loader.GetContentSize() * DiscIO::INANDContentLoader::CONTENT_HEADER_SIZE; 
            }
            Memory::Write_U32(TMDCnt, Buffer.PayloadBuffer[0].m_Address);

            Memory::Write_U32(0, _CommandAddress + 0x4);

            INFO_LOG(WII_IPC_ES, "IOCTL_ES_GETSTOREDTMDSIZE: title: %08x/%08x (view size %i)", (u32)(TitleID >> 32), (u32)TitleID, TMDCnt);
            return true;
        }
        break;

	case IOCTL_ES_ENCRYPT:
		{
			u32 keyIndex	= Memory::Read_U32(Buffer.InBuffer[0].m_Address);
			u8* IV			= Memory::GetPointer(Buffer.InBuffer[1].m_Address);
			u8* source		= Memory::GetPointer(Buffer.InBuffer[2].m_Address);
			u32 size		= Buffer.InBuffer[2].m_Size;
			u8* destination	= Memory::GetPointer(Buffer.PayloadBuffer[1].m_Address);

			AES_KEY AESKey;
			AES_set_encrypt_key(keyTable[keyIndex], 128, &AESKey);
			AES_cbc_encrypt(source, destination, size, &AESKey, IV, AES_ENCRYPT);

			_dbg_assert_msg_(WII_IPC_ES, keyIndex == 6, "IOCTL_ES_ENCRYPT: Key type is not SD, data will be crap");
		}
		break;

	case IOCTL_ES_DECRYPT:
		{
			u32 keyIndex	= Memory::Read_U32(Buffer.InBuffer[0].m_Address);
			u8* IV			= Memory::GetPointer(Buffer.InBuffer[1].m_Address);
			u8* source		= Memory::GetPointer(Buffer.InBuffer[2].m_Address);
			u32 size		= Buffer.InBuffer[2].m_Size;
			u8* destination	= Memory::GetPointer(Buffer.PayloadBuffer[1].m_Address);

			AES_KEY AESKey;
			AES_set_decrypt_key(keyTable[keyIndex], 128, &AESKey);
			AES_cbc_encrypt(source, destination, size, &AESKey, IV, AES_DECRYPT);

			_dbg_assert_msg_(WII_IPC_ES, keyIndex == 6, "IOCTL_ES_DECRYPT: Key type is not SD, data will be crap");
		}
		break;


	case IOCTL_ES_LAUNCH:
        {
            _dbg_assert_(WII_IPC_ES, Buffer.NumberInBuffer == 2);
			bool bSuccess = false;
			u16 IOSv = 0xffff;

            u64 TitleID		= Memory::Read_U64(Buffer.InBuffer[0].m_Address);
            u32 view		= Memory::Read_U32(Buffer.InBuffer[1].m_Address);
            u64 ticketid	= Memory::Read_U64(Buffer.InBuffer[1].m_Address+4);
            u32 devicetype	= Memory::Read_U32(Buffer.InBuffer[1].m_Address+12);
            u64 titleid		= Memory::Read_U64(Buffer.InBuffer[1].m_Address+16);
            u16 access		= Memory::Read_U16(Buffer.InBuffer[1].m_Address+24);


			if ((u32)(TitleID>>32) != 0x00000001 || TitleID == TITLEID_SYSMENU)
			{
				const DiscIO::INANDContentLoader& ContentLoader = DiscIO::CNANDContentManager::Access().GetNANDLoader(TitleID);
				if (ContentLoader.IsValid())
				{
					u32 bootInd = ContentLoader.GetBootIndex();
					const DiscIO::SNANDContent* pContent = ContentLoader.GetContentByIndex(bootInd);
					if (pContent)
					{
						LoadWAD(Common::CreateTitleContentPath(TitleID));
						CDolLoader DolLoader(pContent->m_pData, pContent->m_Size);
						PC = DolLoader.GetEntryPoint() | 0x80000000;
						IOSv = ContentLoader.GetIosVersion();
						bSuccess = true;
					}
				}
			}
			else // IOS, MIOS, BC etc
			{
				//TODO: fixme
				// The following is obviously a hack
				// Lie to mem about loading a different ios
				// someone with an affected game should test
				IOSv = TitleID & 0xffff;
			}
			if (!bSuccess)
				PanicAlert("IOCTL_ES_LAUNCH: Game tried to reload ios or a title that is not available in your nand dump\n"
					"TitleID %016llx.\n Dolphin will likely hang now", TitleID);
			// Pass the "#002 check"
			// Apploader should write the IOS version and revision to 0x3140, and compare it
			// to 0x3188 to pass the check, but we don't do it, and i don't know where to read the IOS rev...
			// Currently we just write 0xFFFF for the revision, copy manually and it works fine :p
			// TODO : figure it correctly : where should we read the IOS rev that the wad "needs" ?
			Memory::Write_U16(IOSv, 0x00003140);
			Memory::Write_U16(0xFFFF, 0x00003142);
			Memory::Write_U32(Memory::Read_U32(0x00003140), 0x00003188);
			
			//TODO: provide correct return code when bSuccess= false
			Memory::Write_U32(0, _CommandAddress + 0x4);

            ERROR_LOG(WII_IPC_ES, "IOCTL_ES_LAUNCH %016llx %08x %016llx %08x %016llx %04x", TitleID,view,ticketid,devicetype,titleid,access);
			//					   IOCTL_ES_LAUNCH 0001000248414341 00000001 0001c0fef3df2cfa 00000000 0001000248414341 ffff

            return true;
        }
        break;

	case IOCTL_ES_CHECKKOREAREGION: //note by DacoTaco : name is unknown, i just tried to name it SOMETHING
		//IOS70 has this to let system menu 4.2 check if the console is region changed. it returns -1017
		//if the IOS didn't find the korean keys and 0 if it does. 0 leads to a error 003
		WARN_LOG(WII_IPC_ES,"IOCTL_ES_CHECKKOREAREGION: Title Checked for korean Keys");
		Memory::Write_U32(ES_PARAMTER_SIZE_OR_ALIGNMENT , _CommandAddress + 0x4);
		return true;

    // ===============================================================================================
    // unsupported functions 
    // ===============================================================================================
     case IOCTL_ES_DIGETTICKETVIEW: // (Input: none, Output: 216 bytes) bug crediar :D
        WARN_LOG(WII_IPC_ES, "IOCTL_ES_DIGETTICKETVIEW: this looks really wrong...");
        break;

    case IOCTL_ES_GETDEVICECERT: // (Input: none, Output: 384 bytes)
        WARN_LOG(WII_IPC_ES, "IOCTL_ES_GETDEVICECERT: this looks really wrong...");
        break;

    default:
        WARN_LOG(WII_IPC_ES, "CWII_IPC_HLE_Device_es: 0x%x", Buffer.Parameter);

		DumpCommands(_CommandAddress, 8, LogTypes::WII_IPC_ES);
        INFO_LOG(WII_IPC_ES, "command.Parameter: 0x%08x", Buffer.Parameter);
        break;
    }

    // Write return value (0 means OK)
    Memory::Write_U32(0, _CommandAddress + 0x4);		

    return true;
}

const DiscIO::INANDContentLoader& CWII_IPC_HLE_Device_es::AccessContentDevice(u64 _TitleID)
{
    if (m_pContentLoader->IsValid() && m_pContentLoader->GetTitleID() == _TitleID)
        return* m_pContentLoader;
    
    CTitleToContentMap::iterator itr = m_NANDContent.find(_TitleID);
    if (itr != m_NANDContent.end())
        return *itr->second;

    m_NANDContent[_TitleID] = &DiscIO::CNANDContentManager::Access().GetNANDLoader(_TitleID);

    _dbg_assert_msg_(WII_IPC_ES, m_NANDContent[_TitleID]->IsValid(), "NandContent not valid for TitleID %08x/%08x", (u32)(_TitleID >> 32), (u32)_TitleID);
    return *m_NANDContent[_TitleID];
}

bool CWII_IPC_HLE_Device_es::IsValid(u64 _TitleID) const
{
    if (m_pContentLoader->IsValid() && m_pContentLoader->GetTitleID() == _TitleID)
        return true;

    return false;
}


u32 CWII_IPC_HLE_Device_es::ES_DIVerify(u8* _pTMD, u32 _sz)
{
	u64 titleID = 0xDEADBEEFDEADBEEFull;
	u64 tmdTitleID = Common::swap64(*(u64*)(_pTMD+0x18c));
	VolumeHandler::GetVolume()->GetTitleID((u8*)&titleID);
	if (Common::swap64(titleID) != tmdTitleID)
	{
		return -1;
	}
	std::string contentPath,
				dataPath,
				tmdPath;
	contentPath = Common::CreateTitleContentPath(tmdTitleID) + DIR_SEP;
	dataPath	= Common::CreateTitleDataPath(tmdTitleID) + DIR_SEP;
	tmdPath = contentPath + "/title.tmd";

	File::CreateFullPath(contentPath.c_str());
	File::CreateFullPath(dataPath.c_str());
	if(!File::Exists(tmdPath.c_str()))
	{
		FILE* _pTMDFile = fopen(tmdPath.c_str(), "wb");
		if (_pTMDFile)
		{
			if (fwrite(_pTMD, _sz, 1, _pTMDFile) != 1)
				ERROR_LOG(WII_IPC_ES, "DIVerify failed to write disc tmd to nand");
			fclose(_pTMDFile);
		}
	}
	DiscIO::cUIDsys::AccessInstance().AddTitle(tmdTitleID);
	return 0;
}

