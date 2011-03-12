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

#include "Common.h"

#include "State.h"
#include "Core.h"
#include "ConfigManager.h"
#include "StringUtil.h"
#include "Thread.h"
#include "CoreTiming.h"
#include "OnFrame.h"
#include "HW/Wiimote.h"
#include "HW/DSP.h"
#include "HW/HW.h"
#include "HW/CPU.h"
#include "PowerPC/JitCommon/JitBase.h"

#include "VideoBackendBase.h"

#include <string>

#include <lzo/lzo1x.h>

// TODO: Move to namespace

// TODO: Investigate a memory leak on save/load state

#if defined(__LZO_STRICT_16BIT)
#define IN_LEN      (8*1024u)
#elif defined(LZO_ARCH_I086) && !defined(LZO_HAVE_MM_HUGE_ARRAY)
#define IN_LEN      (60*1024u)
#else
#define IN_LEN      (128*1024ul)
#endif
#define OUT_LEN     (IN_LEN + IN_LEN / 16 + 64 + 3)

static unsigned char __LZO_MMODEL out [ OUT_LEN ];

#define HEAP_ALLOC(var,size) \
	lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

static bool state_op_in_progress = false;

static int ev_Save, ev_BufferSave;
static int ev_Load, ev_BufferLoad;
static int ev_Verify, ev_BufferVerify;

static std::string cur_filename, lastFilename;
static u8 **cur_buffer = NULL;

// Temporary undo state buffers
static u8 *undoLoad = NULL;

static bool const bCompressed = true;

static std::thread saveThread;


// Don't forget to increase this after doing changes on the savestate system 
#define STATE_VERSION 4


void DoState(PointerWrap &p)
{
	u32 cookie = 0xBAADBABE + STATE_VERSION;
	p.Do(cookie);
	if (cookie != 0xBAADBABE + STATE_VERSION)
	{
		//PanicAlert("Savestate version mismatch !\nSorry, you can't load states from other revisions.");
		p.SetMode(PointerWrap::MODE_MEASURE);
		return;
	}
	// Begin with video backend, so that it gets a chance to clear it's caches and writeback modified things to RAM
	// Pause the video thread in multi-threaded mode
	g_video_backend->RunLoop(false);
	g_video_backend->DoState(p);

	if (Core::g_CoreStartupParameter.bWii)
		Wiimote::DoState(p.GetPPtr(), p.GetMode());

	PowerPC::DoState(p);
	HW::DoState(p);
	CoreTiming::DoState(p);

	// Resume the video thread
	g_video_backend->RunLoop(true);
}

void LoadBufferStateCallback(u64 userdata, int cyclesLate)
{
	if (!cur_buffer || !*cur_buffer) {
		Core::DisplayMessage("State does not exist", 1000);
		return;
	}

	u8 *ptr = *cur_buffer;
	PointerWrap p(&ptr, PointerWrap::MODE_READ);
	DoState(p);

	Core::DisplayMessage("Loaded state", 2000);
	state_op_in_progress = false;
}

void SaveBufferStateCallback(u64 userdata, int cyclesLate)
{
	if (!cur_buffer) {
		Core::DisplayMessage("Error saving state", 1000);
		return;
	}

	u8 *ptr = NULL;

	PointerWrap p(&ptr, PointerWrap::MODE_MEASURE);

	if (!*cur_buffer)
	{
		// if we got passed an empty buffer,
		// allocate it with new[]
		// (and the caller is responsible for delete[]ing it later)
		DoState(p);
		size_t sz = (size_t)ptr;
		*cur_buffer = new u8[sz];
	}
	else
	{
		// otherwise the caller is telling us that they have already allocated it with enough space
	}

	ptr = *cur_buffer;
	p.SetMode(PointerWrap::MODE_WRITE);
	DoState(p);

	state_op_in_progress = false;
}

void VerifyBufferStateCallback(u64 userdata, int cyclesLate)
{
	if (!cur_buffer || !*cur_buffer) {
		Core::DisplayMessage("State does not exist", 1000);
		return;
	}

	u8 *ptr = *cur_buffer;
	PointerWrap p(&ptr, PointerWrap::MODE_VERIFY);
	DoState(p);

	Core::DisplayMessage("Verified state", 2000);
	state_op_in_progress = false;
}

void CompressAndDumpState(saveStruct* saveArg)
{
	u8 *buffer = saveArg->buffer;
	size_t sz = saveArg->size;
	lzo_uint out_len = 0;
	state_header header;
	std::string filename = cur_filename;

	delete saveArg;

	// For easy debugging
	Common::SetCurrentThreadName("SaveState thread");

	// Moving to last overwritten save-state
	if (File::Exists(cur_filename))
	{
		if (File::Exists(File::GetUserPath(D_STATESAVES_IDX) + "lastState.sav"))
			File::Delete((File::GetUserPath(D_STATESAVES_IDX) + "lastState.sav"));

		if (!File::Rename(cur_filename, File::GetUserPath(D_STATESAVES_IDX) + "lastState.sav"))
			Core::DisplayMessage("Failed to move previous state to state undo backup", 1000);
	}

	File::IOFile f(filename, "wb");
	if (!f)
	{
		Core::DisplayMessage("Could not save state", 2000);
		delete[] buffer;
		return;
	}

	// Setting up the header
	memcpy(header.gameID, SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID().c_str(), 6);
	header.sz = bCompressed ? sz : 0;

	f.WriteArray(&header, 1);
	if (bCompressed)
	{
		lzo_uint cur_len = 0;
		lzo_uint i = 0;

		for (;;)
		{
			if ((i + IN_LEN) >= sz)
				cur_len = sz - i;
			else
				cur_len = IN_LEN;

			if (lzo1x_1_compress((buffer + i), cur_len, out, &out_len, wrkmem) != LZO_E_OK)
				PanicAlertT("Internal LZO Error - compression failed");

			// The size of the data to write is 'out_len'
			f.WriteArray(&out_len, 1);
			f.WriteBytes(out, out_len);

			if (cur_len != IN_LEN)
				break;
			i += cur_len;
		}
	}
	else
	{
		f.WriteBytes(buffer, sz);
	}

	delete[] buffer;

	Core::DisplayMessage(StringFromFormat("Saved State to %s",
		filename.c_str()).c_str(), 2000);

	state_op_in_progress = false;
}

void SaveStateCallback(u64 userdata, int cyclesLate)
{
	// Pause the core while we save the state
	CCPU::EnableStepping(true);

	// Wait for the other threaded sub-systems to stop too
	SLEEP(100);

	State_Flush();

	// Measure the size of the buffer.
	u8 *ptr = 0;
	PointerWrap p(&ptr, PointerWrap::MODE_MEASURE);
	DoState(p);
	size_t sz = (size_t)ptr;

	// Then actually do the write.
	u8 *buffer = new u8[sz];
	ptr = buffer;
	p.SetMode(PointerWrap::MODE_WRITE);
	DoState(p);

	saveStruct *saveData = new saveStruct;
	saveData->buffer = buffer;
	saveData->size = sz;
	
	if ((Frame::IsRecordingInput() || Frame::IsPlayingInput()) && !Frame::IsRecordingInputFromSaveState())
		Frame::SaveRecording(StringFromFormat("%s.dtm", cur_filename.c_str()).c_str());

	Core::DisplayMessage("Saving State...", 1000);

	saveThread = std::thread(CompressAndDumpState, saveData);

	// Resume the core and disable stepping
	CCPU::EnableStepping(false);
}

void LoadStateCallback(u64 userdata, int cyclesLate)
{
	bool bCompressedState;

	// Stop the core while we load the state
	CCPU::EnableStepping(true);

	// Wait for the other threaded sub-systems to stop too
	SLEEP(100);

	State_Flush();

	// Save temp buffer for undo load state
	// TODO: this should be controlled by a user option,
	// because it slows down every savestate load to provide an often-unused feature.
	{
		delete[] undoLoad;
		undoLoad = NULL;
		cur_buffer = &undoLoad;
		SaveBufferStateCallback(userdata, cyclesLate);
	}

	File::IOFile f(cur_filename, "rb");
	if (!f)
	{
		Core::DisplayMessage("State not found", 2000);
		// Resume the clock
		CCPU::EnableStepping(false);
		return;
	}

	u8 *buffer = NULL;
	state_header header;
	size_t sz;

	f.ReadArray(&header, 1);
	
	if (memcmp(SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID().c_str(), header.gameID, 6)) 
	{
		char gameID[7] = {0};
		memcpy(gameID, header.gameID, 6);
		Core::DisplayMessage(StringFromFormat("State belongs to a different game (ID %s)",
			gameID), 2000);

		// Resume the clock
		CCPU::EnableStepping(false);
		return;
	}

	sz = header.sz;
	bCompressedState = (sz != 0);
	if (bCompressedState)
	{
		Core::DisplayMessage("Decompressing State...", 500);

		lzo_uint i = 0;
		buffer = new u8[sz];
		if (!buffer)
		{
			PanicAlertT("Error allocating buffer");
			// Resume the clock
			CCPU::EnableStepping(false);
			return;
		}
		while (true)
		{
			lzo_uint cur_len = 0;  // number of bytes to read
			lzo_uint new_len = 0;  // number of bytes to write

			if (!f.ReadArray(&cur_len, 1))
				break;

			f.ReadBytes(out, cur_len);
			int res = lzo1x_decompress(out, cur_len, (buffer + i), &new_len, NULL);
			if (res != LZO_E_OK)
			{
				// This doesn't seem to happen anymore.
				PanicAlertT("Internal LZO Error - decompression failed (%d) (%li, %li) \n"
					"Try loading the state again", res, i, new_len);
				delete[] buffer;
				// Resume the clock
				CCPU::EnableStepping(false);
				return;
			}
	
			i += new_len;
		}
	}
	else
	{
		sz = (int)(f.GetSize() - sizeof(state_header));
		buffer = new u8[sz];
		if (!f.ReadBytes(buffer, sz))
			PanicAlert("wtf? reading bytes: %lu", (unsigned long)sz);
	}

	f.Close();

	u8 *ptr = buffer;
	PointerWrap p(&ptr, PointerWrap::MODE_READ);
	DoState(p);

	if (p.GetMode() == PointerWrap::MODE_READ)
		Core::DisplayMessage(StringFromFormat("Loaded state from %s", cur_filename.c_str()).c_str(), 2000);
	else
		Core::DisplayMessage("Unable to Load : Can't load state from other revisions !", 4000);

	delete[] buffer;
	
	if (File::Exists(cur_filename + ".dtm"))
		Frame::LoadInput((cur_filename + ".dtm").c_str());
	else if (!Frame::IsRecordingInputFromSaveState())
		Frame::EndPlayInput(false);

	state_op_in_progress = false;

	// Resume the clock
	CCPU::EnableStepping(false);
}

void VerifyStateCallback(u64 userdata, int cyclesLate)
{
	bool bCompressedState;

	State_Flush();

	File::IOFile f(cur_filename, "rb");
	if (!f)
	{
		Core::DisplayMessage("State not found", 2000);
		return;
	}

	u8 *buffer = NULL;
	state_header header;
	size_t sz;

	f.ReadArray(&header, 1);
	
	if (memcmp(SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID().c_str(), header.gameID, 6)) 
	{
		char gameID[7] = {0};
		memcpy(gameID, header.gameID, 6);
		Core::DisplayMessage(StringFromFormat("State belongs to a different game (ID %s)",
			gameID), 2000);

		return;
	}

	sz = header.sz;
	bCompressedState = (sz != 0);
	if (bCompressedState)
	{
		Core::DisplayMessage("Decompressing State...", 500);

		lzo_uint i = 0;
		buffer = new u8[sz];
		if (!buffer) {
			PanicAlertT("Error allocating buffer");
			return;
		}
		while (true)
		{
			lzo_uint cur_len = 0;
			lzo_uint new_len = 0;
			if (!f.ReadArray(&cur_len, 1))
				break;

			f.ReadBytes(out, cur_len);
			int res = lzo1x_decompress(out, cur_len, (buffer + i), &new_len, NULL);
			if (res != LZO_E_OK)
			{
				// This doesn't seem to happen anymore.
				PanicAlertT("Internal LZO Error - decompression failed (%d) (%ld, %ld) \n"
					"Try verifying the state again", res, i, new_len);

				delete [] buffer;
				return;
			}
	
			// The size of the data to read to our buffer is 'new_len'
			i += new_len;
		}
	}
	else
	{
		sz = (int)(f.GetSize() - sizeof(int));
		buffer = new u8[sz];

		if (!f.ReadBytes(buffer, sz))
			PanicAlert("wtf? failed to read bytes: %lu", (unsigned long)sz);
	}

	u8 *ptr = buffer;
	PointerWrap p(&ptr, PointerWrap::MODE_VERIFY);
	DoState(p);

	if (p.GetMode() == PointerWrap::MODE_READ)
		Core::DisplayMessage(StringFromFormat("Verified state at %s", cur_filename.c_str()).c_str(), 2000);
	else
		Core::DisplayMessage("Unable to Verify : Can't verify state from other revisions !", 4000);

	delete [] buffer;
}

void State_Init()
{
	ev_Load = CoreTiming::RegisterEvent("LoadState", &LoadStateCallback);
	ev_Save = CoreTiming::RegisterEvent("SaveState", &SaveStateCallback);
	ev_Verify = CoreTiming::RegisterEvent("VerifyState", &VerifyStateCallback);
	ev_BufferLoad = CoreTiming::RegisterEvent("LoadBufferState", &LoadBufferStateCallback);
	ev_BufferSave = CoreTiming::RegisterEvent("SaveBufferState", &SaveBufferStateCallback);
	ev_BufferVerify = CoreTiming::RegisterEvent("VerifyBufferState", &VerifyBufferStateCallback);

	if (lzo_init() != LZO_E_OK)
		PanicAlertT("Internal LZO Error - lzo_init() failed");
}

void State_Shutdown()
{
	State_Flush();

	if (undoLoad)
	{
		delete[] undoLoad;
		undoLoad = NULL;
	}
}

static std::string MakeStateFilename(int state_number)
{
	return StringFromFormat("%s%s.s%02i", File::GetUserPath(D_STATESAVES_IDX).c_str(), SConfig::GetInstance().m_LocalCoreStartupParameter.GetUniqueID().c_str(), state_number);
}

void State_SaveAs(const std::string &filename)
{
	if (state_op_in_progress)
		return;
	state_op_in_progress = true;
	cur_filename = filename;
	lastFilename = filename;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_Save);
}

void State_Save(int slot)
{
	State_SaveAs(MakeStateFilename(slot));
}

void State_LoadAs(const std::string &filename)
{
	if (state_op_in_progress)
		return;
	state_op_in_progress = true;
	cur_filename = filename;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_Load);
}

void State_Load(int slot)
{
	State_LoadAs(MakeStateFilename(slot));
}

void State_VerifyAt(const std::string &filename)
{
	if (state_op_in_progress)
		return;
	state_op_in_progress = true;
	cur_filename = filename;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_Verify);
}

void State_Verify(int slot)
{
	State_VerifyAt(MakeStateFilename(slot));
}

void State_LoadLastSaved()
{
	if (lastFilename.empty())
		Core::DisplayMessage("There is no last saved state", 2000);
	else
		State_LoadAs(lastFilename);
}

void State_LoadFromBuffer(u8 **buffer)
{
	if (state_op_in_progress)
		return;
	state_op_in_progress = true;
	cur_buffer = buffer;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_BufferLoad);
}

void State_SaveToBuffer(u8 **buffer)
{
	if (state_op_in_progress)
		return;
	state_op_in_progress = true;
	cur_buffer = buffer;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_BufferSave);
}

void State_VerifyBuffer(u8 **buffer)
{
	if (state_op_in_progress)
		return;
	state_op_in_progress = true;
	cur_buffer = buffer;
	CoreTiming::ScheduleEvent_Threadsafe_Immediate(ev_BufferVerify);
}

void State_Flush()
{
	// If already saving state, wait for it to finish
	if (saveThread.joinable())
	{
		saveThread.join();
	}
}

// Load the last state before loading the state
void State_UndoLoadState()
{
	State_LoadFromBuffer(&undoLoad);
}

// Load the state that the last save state overwritten on
void State_UndoSaveState()
{
	State_LoadAs((File::GetUserPath(D_STATESAVES_IDX) + "lastState.sav").c_str());
}

size_t State_GetSize()
{
	// Measure the size of the buffer.
	u8 *ptr = 0;
	PointerWrap p(&ptr, PointerWrap::MODE_MEASURE);
	DoState(p);
	return (size_t)ptr;
}
