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


// Core

// The external interface to the emulator core. Plus some extras.
// This is another part of the emu that needs cleaning - Core.cpp really has
// too much random junk inside.

#ifndef _CORE_H
#define _CORE_H

#include <vector>
#include <string>

#include "Common.h"
#include "CoreParameter.h"

namespace Core
{

void Callback_VideoLog(const char* _szMessage);
void Callback_VideoCopiedToXFB(bool video_update);
void Callback_VideoGetWindowSize(int& x, int& y, int& width, int& height);
void Callback_VideoRequestWindowSize(int& width, int& height);
void Callback_CoreMessage(int Id);

    enum EState
    {
        CORE_UNINITIALIZED,
        CORE_PAUSE,
        CORE_RUN,
		CORE_STOPPING
    };

    // Init core
    bool Init();
    void Stop();

	std::string StopMessage(bool, std::string);

	bool isRunning();
	bool IsRunningInCurrentThread(); // this tells us whether we are running in the cpu thread.
	bool IsCPUThread(); // this tells us whether we are the cpu thread.
    
    void SetState(EState _State);
    EState GetState();

	void ScreenShot(const std::string& name);
	void ScreenShot();
    
    // Get core parameters kill use SConfig instead
    extern SCoreStartupParameter g_CoreStartupParameter; 

	void Callback_WiimoteInterruptChannel(int _number, u16 _channelID, const void* _pData, u32 _Size);

    void* GetWindowHandle();

    bool GetRealWiimote();

    extern bool bReadTrace;
    extern bool bWriteTrace;
    
    void StartTrace(bool write);
    void DisplayMessage(const std::string &message, int time_in_ms); // This displays messages in a user-visible way.
    void DisplayMessage(const char *message, int time_in_ms); // This displays messages in a user-visible way.
	
	std::string GetStateFileName();
	void SetStateFileName(std::string val);
    
    int SyncTrace();
    void SetBlockStart(u32 addr);
    void StopTrace();

	bool report_slow(int skipped);
	void VideoThrottle();

	// -----------------------------------------
	#ifdef RERECORDING
	// -----------------
		void FrameUpdate();
		void FrameAdvance();
		void FrameStepOnOff();
		void WriteStatus();
		void RerecordingStart();
		void RerecordingStop();
		void WindBack(int Counter);
		extern int g_FrameCounter;
		extern bool g_FrameStep;
	#endif
	// ---------------------------
}  // namespace

#endif

