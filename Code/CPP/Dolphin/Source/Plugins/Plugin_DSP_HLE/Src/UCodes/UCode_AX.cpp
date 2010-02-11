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

#include "FileUtil.h" // For IsDirectory()
#include "StringUtil.h" // For StringFromFormat()
#if defined(HAVE_WX) && HAVE_WX
#include "../Debugger/Debugger.h"
//#include "../Logging/File.h" // For PrintFile()
extern DSPDebuggerHLE* m_DebuggerFrame;
#endif
#include <sstream>
#include "../Config.h"

#include "../Globals.h"
#include "Mixer.h"
#include "../MailHandler.h"

#include "UCodes.h"
#include "UCode_AXStructs.h"
#include "UCode_AX.h"
#include "UCode_AX_Voice.h"

// ------------------------------------------------------------------
// Externals
// -----------
extern bool gSSBM;
extern bool gSSBMremedy1;
extern bool gSSBMremedy2;
extern bool gSequenced;
extern bool gVolume;
extern bool gReset;

std::vector<std::string> sMailLog, sMailTime;
// -----------


CUCode_AX::CUCode_AX(CMailHandler& _rMailHandler)
	: IUCode(_rMailHandler)
	, m_addressPBs(0xFFFFFFFF)
{
	// we got loaded
	m_rMailHandler.PushMail(0xDCD10000);

	templbuffer = new int[1024 * 1024];
	temprbuffer = new int[1024 * 1024];
}

CUCode_AX::~CUCode_AX()
{
	m_rMailHandler.Clear();
	delete [] templbuffer;
	delete [] temprbuffer;
}


// Save file to harddrive
void CUCode_AX::SaveLogFile(std::string f, int resizeTo, bool type, bool Wii)
{
	//#ifdef DEBUG_LEVEL
	std::ostringstream ci;
	std::ostringstream cType;
	
	ci << (resizeTo - 1); // write ci
	cType << type; // write cType
	
	std::string FileName = std::string(File::GetUserPath(D_MAILLOGS_IDX)) + std::string(globals->unique_id);
	FileName += "_sep"; FileName += ci.str(); FileName += "_sep"; FileName += cType.str();
	FileName += Wii ? "_sepWii_sep" : "_sepGC_sep"; FileName += ".log";
	
	FILE* fhandle = fopen(FileName.c_str(), "w");
	fprintf(fhandle, "%s", f.c_str());
	fflush(fhandle); fhandle = NULL;
	//#endif
}


// ============================================
// Save the logged AX mail
// ----------------
void CUCode_AX::SaveLog_(bool Wii, const char* _fmt, va_list ap)
{
    char Msg[512];
    vsprintf(Msg, _fmt, ap);

#if defined(HAVE_WX) && HAVE_WX
if (m_DebuggerFrame->ScanMails)
{	

	if (strcmp(Msg, "Begin") == 0)
	{
		TmpMailLog = "";
	}
	else if (strcmp(Msg, "End") == 0)
	{
		if (saveNext && saveNext < 100) // limit because saveNext is not initialized
		{		
			// Save the timestamps and comment
            std::ostringstream ci;
            ci << (saveNext - 1);
			TmpMailLog += "\n\n";
			TmpMailLog += "-----------------------------------------------------------------------\n";
//			TmpMailLog += "Current mail: " + std::string(globals->unique_id) + " mail " + ci + "\n";
			if (Wii)
				TmpMailLog += "Current CRC: " + StringFromFormat("0x%08x \n\n", _CRC);			

			for (u32 i = 0; i < sMailTime.size(); i++)
			{
				char tmpbuf[128]; sprintf(tmpbuf, "Mail %i received: %s\n", i, sMailTime.at(i).c_str());
				TmpMailLog += tmpbuf;
			}
			TmpMailLog += "-----------------------------------------------------------------------";

			sMailLog.push_back(TmpMailLog);
			// Save file to disc
			if (m_DebuggerFrame->StoreMails)
			{
				SaveLogFile(TmpMailLog, saveNext, 1, Wii);
			}
			
			m_DebuggerFrame->DoUpdateMail(); // update the view
			saveNext = 0;
		}
	}
	else
	{
#endif
		TmpMailLog += Msg;
		TmpMailLog += "\n";
		DEBUG_LOG(DSPHLE, "%s", Msg); // also write it to the log
#if defined(HAVE_WX) && HAVE_WX
	}
}
#endif
}
// ----------------


// ============================================
// Save the whole AX mail
// ----------------
void CUCode_AX::SaveMail(bool Wii, u32 _uMail)
{
#if defined(HAVE_WX) && HAVE_WX
if (!m_DebuggerFrame) return;
if (m_DebuggerFrame->ScanMails)
{
	int i = 0;
	std::string sTemp;
	std::string sTempEnd;
	std::string * sAct = &sTemp;
	bool doOnce = true; // for the while loop, to avoid getting stuck

	// Go through the mail
	while (i < 250)
	{
		// Make a new row for each AX-Command
		u16 axcomm = Memory_Read_U16(_uMail + i);
		if (axcomm < 15 && axcomm != 0) // we can at most write 8 messages per log
		{	
			*sAct += "\n";
		}

		char szTemp2[128] = "";
		sprintf(szTemp2, "%08x : 0x%04x\n", _uMail + i, axcomm);
		*sAct += szTemp2;

		 // set i to 160 so that we show some things after the end to			
		if ((axcomm == AXLIST_END || axcomm == 0x000e) && doOnce)
		{
			i = 160;
			sAct = &sTempEnd;
			doOnce = false;
		}

		i += 2;
	}

	// Compare this mail to old mails
	u32 addnew = 0;
	for (u32 j = 0; j < m_DebuggerFrame->sMail.size(); j++)
	{
		if (m_DebuggerFrame->sMail.at(j).length() != sTemp.length())
		{
			//wxMessageBox( wxString::Format("%s  \n\n%s", m_DebuggerFrame->sMail.at(i).c_str(),
			//	sTemp.c_str()) );
			addnew++;	
		}
	}


	// In case the mail didn't match any saved mail, save it
	if (addnew == m_DebuggerFrame->sMail.size())
	{		
		//Console::Print("%i  |  %i\n", addnew, m_DebuggerFrame->sMail.size());
		u32 resizeTo = (u32)(m_DebuggerFrame->sMail.size() + 1);		

		// ------------------------------------
		// get timestamp
		wxDateTime datetime = wxDateTime::UNow();	
		char Msg[128];
		sprintf(Msg, "%04i-%02i-%02i  %02i:%02i:%02i:%03i",
			datetime.GetYear(), datetime.GetMonth() + 1, datetime.GetDay(),
			datetime.GetHour(), datetime.GetMinute(), datetime.GetSecond(), datetime.GetMillisecond());
		sMailTime.push_back(Msg);
		// ------------------------------------

		m_DebuggerFrame->sMail.push_back(sTemp); // save the main comparison mail
		std::string lMail = sTemp +  "------------------\n" + sTempEnd;
		m_DebuggerFrame->sFullMail.push_back(lMail);

		// enable the radio button and update view
		if (resizeTo <= m_DebuggerFrame->m_RadioBox[3]->GetCount())
		{
			m_DebuggerFrame->m_RadioBox[3]->Enable(resizeTo - 1, true);
			m_DebuggerFrame->m_RadioBox[3]->Select(resizeTo - 1);
		}

		addnew = 0;
		saveNext = resizeTo; // save the log to

		// ------------------------------------
		// Save as file
		if (m_DebuggerFrame->StoreMails)
		{
			//Console::Print("m_DebuggerFrame->sMail.size(): %i  |  resizeTo:%i\n", m_DebuggerFrame->sMail.size(), resizeTo);
			SaveLogFile(lMail, resizeTo, 0, Wii);
		}
		
	}
	sTemp = "";
	sTempEnd = "";
}
#endif
}
// ----------------

static bool ReadOutPB(u32 pb_address, AXParamBlock &PB)
{
	const u16 *pSrc = (const u16 *)g_dspInitialize.pGetMemoryPointer(pb_address);
	if (!pSrc)
		return false;
	u16 *pDest = (u16 *)&PB;
	// TODO: SSE. Although it won't help much.
	for (int p = 0; p < (int)sizeof(AXParamBlock) / 2; p++)
	{
		pDest[p] = Common::swap16(pSrc[p]);
	}
	return true;
}

static bool WriteBackPB(u32 pb_address, AXParamBlock &PB)
{
	const u16 *pSrc  = (const u16*)&PB;
	u16 *pDest = (u16 *)g_dspInitialize.pGetMemoryPointer(pb_address);
	if (!pDest)
		return false;
	// TODO: SSE. Although it won't help much.
	for (size_t p = 0; p < sizeof(AXParamBlock) / 2; p++)
	{
		pDest[p] = Common::swap16(pSrc[p]);
	}
	return true;
}

static void ProcessUpdates(AXParamBlock &PB)
{
	// ---------------------------------------------------------------------------------------
	/* Make the updates we are told to do. When there are multiple updates for a block they
	   are placed in memory directly following updaddr. They are mostly for initial time
	   delays, sometimes for the FIR filter or channel volumes. We do all of them at once here.
	   If we get both an on and an off update we chose on. Perhaps that makes the RE1 music
	   work better. */
	u16 *pDest = (u16 *)&PB;
	u16 upd0 = pDest[34]; u16 upd1 = pDest[35]; u16 upd2 = pDest[36]; // num_updates
	u16 upd3 = pDest[37]; u16 upd4 = pDest[38];
	u16 upd_hi = pDest[39]; // update addr
	u16	upd_lo = pDest[40];
	int numupd = upd0 + upd1 + upd2 + upd3 + upd4;
	if (numupd > 64) numupd = 64; // prevent crazy values
	const u32 updaddr   = (u32)(upd_hi << 16) | upd_lo;
	int on = false, off = false;
	for (int j = 0; j < numupd; j++)
	{
		int k = g_Config.m_EnableRE0Fix ? 0 : j;

		const u16 updpar   = Memory_Read_U16(updaddr + k);
		const u16 upddata   = Memory_Read_U16(updaddr + k + 2);
		// some safety checks, I hope it's enough
		if (updaddr > 0x80000000 && updaddr < 0x817fffff
			&& updpar < 63 && updpar > 3 && upddata >= 0 // updpar > 3 because we don't want to change
			// 0-3, those are important
			//&& (upd0 || upd1 || upd2 || upd3 || upd4) // We should use these in some way to I think
			// but I don't know how or when
			&& gSequenced) // on and off option
		{
			pDest[updpar] = upddata;
		}
		if (updpar == 7 && upddata == 1) on++;
		if (updpar == 7 && upddata == 1) off++;
	
		// hack: if we get both an on and an off select on rather than off
		if (on > 0 && off > 0) pDest[7] = 1;
	}
}


void CUCode_AX::MixAdd(short* _pBuffer, int _iSize)
{
	if (_iSize > 1024 * 1024)
		_iSize = 1024 * 1024;

	memset(templbuffer, 0, _iSize * sizeof(int));
	memset(temprbuffer, 0, _iSize * sizeof(int));

#if defined(HAVE_WX) && HAVE_WX
	// write logging data to debugger
	if (m_DebuggerFrame && _pBuffer)
	{
		CUCode_AX::Logging(_pBuffer, _iSize, 0, false);
	}
	
#endif

	AXParamBlock PB;

	for (int x = 0; x < numPBaddr; x++) 
	{
		//u32 blockAddr = m_addressPBs;
		u32 blockAddr = PBaddr[x];
		
		if (!blockAddr)
			return;

		// ------------
		for (int i = 0; i < NUMBER_OF_PBS; i++)
		{
			if (!ReadOutPB(blockAddr, PB))
				break;

			ProcessUpdates(PB);
			MixAddVoice(PB, templbuffer, temprbuffer, _iSize, false);
			
			if (!WriteBackPB(blockAddr, PB))
				break;

			blockAddr = (PB.next_pb_hi << 16) | PB.next_pb_lo;

			// Guess we're out of blocks
			if (!blockAddr)
				break;
		}
	}

	if (_pBuffer)
	{
		for (int i = 0; i < _iSize; i++)
		{
			// Clamp into 16-bit. Maybe we should add a volume compressor here.
			int left  = templbuffer[i] + _pBuffer[0];
			int right = temprbuffer[i] + _pBuffer[1];
			if (left < -32767)  left = -32767;
			if (left > 32767)   left = 32767;
			if (right < -32767) right = -32767;
			if (right >  32767) right = 32767;
			*_pBuffer++ = left;
			*_pBuffer++ = right;
		}
	}
#if defined(HAVE_WX) && HAVE_WX
	// write logging data to debugger again after the update
	if (m_DebuggerFrame && _pBuffer)
	{
		CUCode_AX::Logging(_pBuffer, _iSize, 1, false);
	}
#endif
}


// ------------------------------------------------------------------------------
// Handle incoming mail
// -----------
void CUCode_AX::HandleMail(u32 _uMail)
{
	if ((_uMail & 0xFFFF0000) == MAIL_AX_ALIST)
	{
		// a new List
		DEBUG_LOG(DSPHLE, " >>>> u32 MAIL : General Mail (%08x)", _uMail);
	}
	else if (_uMail == 0xCDD10000) // Action 0 - restart
	{
		m_rMailHandler.PushMail(0xDCD10001);
	}
	else if (_uMail == 0xCDD10001) // Action 1 - new ucode upload
	{
		NOTICE_LOG(DSPHLE,"Game wanted to upload new ucode!");
	}
	else if ((_uMail & 0xFFFF0000) == 0xCDD10000) // Action 2/3
	{
	}
	else
	{
		DEBUG_LOG(DSPHLE, " >>>> u32 MAIL : AXTask Mail (%08x)", _uMail);
		AXTask(_uMail);
	}
}


// ------------------------------------------------------------------------------
// Update with DSP Interrupt
// -----------
void CUCode_AX::Update(int cycles)
{
	// check if we have to sent something
	if (!m_rMailHandler.IsEmpty())
	{
		g_dspInitialize.pGenerateDSPInterrupt();
	}
}
// -----------


// Shortcut to avoid having to write SaveLog(false, ...) every time
void CUCode_AX::SaveLog(const char* _fmt, ...)
{
#if defined(HAVE_WX) && HAVE_WX
	va_list ap; va_start(ap, _fmt); 
	if (m_DebuggerFrame) SaveLog_(false, _fmt, ap); 
	va_end(ap);
#endif
}


// ============================================
// AX seems to bootup one task only and waits for resume-callbacks
// everytime the DSP has "spare time" it sends a resume-mail to the CPU
// and the __DSPHandler calls a AX-Callback which generates a new AXFrame
bool CUCode_AX::AXTask(u32& _uMail)
{
	u32 uAddress = _uMail;
	SaveLog("Begin");
	SaveLog("=====================================================================");
	SaveLog("%08x : AXTask - AXCommandList-Addr:", uAddress);

	u32 Addr__AXStudio;
	u32 Addr__AXOutSBuffer;
	u32 Addr__AXOutSBuffer_1;
	u32 Addr__AXOutSBuffer_2;
	u32 Addr__A;
	u32 Addr__12;
	u32 Addr__4_1;
	u32 Addr__4_2;
        //	u32 Addr__4_3;
        //	u32 Addr__4_4;
	u32 Addr__5_1;
	u32 Addr__5_2;
	u32 Addr__6;
	u32 Addr__9;

	bool bExecuteList = true;

	numPBaddr = 0;

#if defined(HAVE_WX) && HAVE_WX
	if (m_DebuggerFrame) SaveMail(false, _uMail); // Save mail for debugging
#endif
	while (bExecuteList)
	{
		static int last_valid_command = 0;
		u16 iCommand = Memory_Read_U16(uAddress);
		uAddress += 2;

		switch (iCommand)
		{
	    case AXLIST_STUDIOADDR: //00
		    Addr__AXStudio = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST studio address: %08x", uAddress, Addr__AXStudio);
		    break;

	    case 0x001: // 2byte x 10
	    {
		    u32 address = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    u16 param1 = Memory_Read_U16(uAddress);
		    uAddress += 2;
		    u16 param2 = Memory_Read_U16(uAddress);
		    uAddress += 2;
		    u16 param3 = Memory_Read_U16(uAddress);
		    uAddress += 2;
		    SaveLog("%08x : AXLIST 1: %08x, %04x, %04x, %04x", uAddress, address, param1, param2, param3);
	    }
		    break;

	    //
	    // Somewhere we should be getting a bitmask of AX_SYNC values
	    // that tells us what has been updated
	    // Dunno if important
	    //
	    case AXLIST_PBADDR: //02
		    {
			PBaddr[numPBaddr] = Memory_Read_U32(uAddress);
			numPBaddr++;

		    m_addressPBs = Memory_Read_U32(uAddress); // left in for now
		    uAddress += 4;
		    soundStream->GetMixer()->SetHLEReady(true);
		    SaveLog("%08x : AXLIST PB address: %08x", uAddress, m_addressPBs);
		    SaveLog("Update the SoundThread to be in sync");
//          soundStream->Update(); //do it in this thread to avoid sync problems
		    }
		    break;

	    case 0x0003:
		    SaveLog("%08x : AXLIST command 0x0003 ????");
		    break;

	    case 0x0004:  // AUX?
		    Addr__4_1 = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    Addr__4_2 = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST 4_1 4_2 addresses: %08x %08x", uAddress, Addr__4_1, Addr__4_2);
		    break;

	    case 0x0005:
		    Addr__5_1 = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    Addr__5_2 = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST 5_1 5_2 addresses: %08x %08x", uAddress, Addr__5_1, Addr__5_2);
		    break;

	    case 0x0006:
		    Addr__6   = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST 6 address: %08x", uAddress, Addr__6);
		    break;

	    case AXLIST_SBUFFER:
		    Addr__AXOutSBuffer = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST OutSBuffer address: %08x", uAddress, Addr__AXOutSBuffer);
		    break;

	    case 0x0009:
		    Addr__9   = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST 6 address: %08x", Addr__9);
		    break;

	    case AXLIST_COMPRESSORTABLE:  // 0xa
		    Addr__A   = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST CompressorTable address: %08x", uAddress, Addr__A);
		    break;

	    case 0x000e:
		    Addr__AXOutSBuffer_1 = Memory_Read_U32(uAddress);
		    uAddress += 4;

			// Addr__AXOutSBuffer_2 is the address in RAM that we are supposed to mix to.
			// Although we don't, currently.
		    Addr__AXOutSBuffer_2 = Memory_Read_U32(uAddress);
		    uAddress += 4;
		    SaveLog("%08x : AXLIST sbuf2 addresses: %08x %08x", uAddress, Addr__AXOutSBuffer_1, Addr__AXOutSBuffer_2);
		    break;

	    case AXLIST_END:
		    bExecuteList = false;
		    SaveLog("%08x : AXLIST end", uAddress);
		    break;

	    case 0x0010:  //Super Monkey Ball 2
		    SaveLog("%08x : AXLIST 0x0010", uAddress);
		    //should probably read/skip stuff here
		    uAddress += 8;
		    break;

	    case 0x0011:
		    uAddress += 4;
		    break;

	    case 0x0012:
		    Addr__12  = Memory_Read_U16(uAddress);
		    uAddress += 2;
		    break;

	    case 0x0013:
		    uAddress += 6 * 4; // 6 Addresses.
		    break;

		default:
			{
		    static bool bFirst = true;
		    if (bFirst == true)
		    {
			    char szTemp[2048];
				sprintf(szTemp, "Unknown AX-Command 0x%x (address: 0x%08x). Last valid: %02x\n",
					    iCommand, uAddress - 2, last_valid_command);
			    int num = -32;
			    while (num < 64+32)
			    {
				    char szTemp2[128] = "";
					sprintf(szTemp2, "%s0x%04x\n", num == 0 ? ">>" : "  ", Memory_Read_U16(uAddress + num));
				    strcat(szTemp, szTemp2);
				    num += 2;
			    }

				// Wii AX will always show this
			    PanicAlert(szTemp);
			   // bFirst = false;
		    }

		    // unknown command so stop the execution of this TaskList
		    bExecuteList = false;
			}
			break;
		}
		if (bExecuteList)
			last_valid_command = iCommand;
	}
	SaveLog("AXTask - done, send resume");
	SaveLog("=====================================================================");
	SaveLog("End");

	// i hope resume is okay AX
	m_rMailHandler.PushMail(0xDCD10002);
	return true;
}
