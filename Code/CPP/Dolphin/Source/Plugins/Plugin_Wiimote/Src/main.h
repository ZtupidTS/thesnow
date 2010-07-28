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

#ifndef MAIN_H
#define MAIN_H

#include <iostream> // System
#include <vector>

#include "CommonTypes.h"
#include "pluginspecs_wiimote.h"
#if defined(HAVE_X11) && HAVE_X11
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
	#include <X11/keysym.h>
	#include <X11/XKBlib.h>
	extern Display* WMdisplay;
#endif
// Definitions and declarations
bool IsFocus();
//void InterruptDebugging(bool Emu, const void* _pData);
//void ReadDebugging(bool Emu, const void* _pData, int Size);

// Movement recording
#define RECORDING_ROWS 15
#define WM_RECORDING_WIIMOTE 0
#define WM_RECORDING_NUNCHUCK 1
#define WM_RECORDING_IR 2
struct SRecording
{
	int x;
	int y;
	int z;
	double Time;
	u8 IR[12];
};
struct SRecordingAll
{
	std::vector<SRecording> Recording;
	int HotKeySwitch, HotKeyWiimote, HotKeyNunchuck, HotKeyIR;
	int PlaybackSpeed;
	int IRBytes;
};

#ifndef EXCLUDEMAIN_H
	// General
	extern bool g_EmulatorRunning;
	extern u32 g_ISOId;
	extern bool g_SearchDeviceDone;
	extern bool g_RealWiiMotePresent;
#ifdef _WIN32
	extern HINSTANCE g_hInstance;
#endif

	// Debugging
	extern bool g_DebugAccelerometer;
	extern bool g_DebugData;
	extern bool g_DebugComm;
	extern bool g_DebugSoundData;
	extern bool g_DebugCustom;

	// Update speed
	extern int g_UpdateCounter;
	extern double g_UpdateTime;
	extern int g_UpdateWriteScreen;
	extern int g_UpdateRate;
	extern std::vector<int> g_UpdateTimeList;
	
	// Movement recording
	extern std::vector<SRecordingAll> VRecording;

	extern PLUGIN_EMUSTATE g_EmulatorState;

#endif


#endif // MAIN_H
