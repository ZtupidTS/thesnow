/*  GSnull
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string>

#include <stdio.h>
#include <assert.h>

using namespace std;

#include "GS.h"
#include "GifTransfer.h"
#include "null/GSnull.h"

const unsigned char version  = PS2E_GS_VERSION;
const unsigned char revision = 0;
const unsigned char build    = 1;    // increase that with each version

static char *libraryName = "GSnull Driver";
FILE *gsLog;
Config conf;
u32 GSKeyEvent = 0;
bool GSShift = false, GSAlt = false;

string s_strIniPath="inis";
std::string s_strLogPath("logs/");
const char* s_iniFilename = "GSnull.ini";
GSVars gs;

// Because I haven't bothered to get GSOpen2 working in Windows yet in GSNull.
#ifdef __LINUX__
#define USE_GSOPEN2
#endif

void (*GSirq)();
extern void ResetRegs();
extern void SetMultithreaded();
extern void SetFrameSkip(bool skip);
extern void InitPath();

EXPORT_C_(u32) PS2EgetLibType()
{
	return PS2E_LT_GS;
}

EXPORT_C_(char*) PS2EgetLibName()
{
	return libraryName;
}

EXPORT_C_(u32) PS2EgetLibVersion2(u32 type)
{
	return (version<<16) | (revision<<8) | build;
}

bool OpenLog() {
    bool result = true;
#ifdef GS_LOG
    const std::string LogFile(s_strLogPath + "GSnull.log");

    gsLog = fopen(LogFile.c_str(), "w");
    if (gsLog != NULL)
        setvbuf(gsLog, NULL,  _IONBF, 0);
    else {
        SysMessage("Can't create log file %s\n", LogFile.c_str());
        result = false;
    }
	GS_LOG("GSnull plugin version %d,%d\n",revision,build);
	GS_LOG("GS init\n");
#endif
    return result;
}

void __Log(char *fmt, ...)
{
	va_list list;

	if (!conf.Log || gsLog == NULL) return;

	va_start(list, fmt);
	vfprintf(gsLog, fmt, list);
	va_end(list);
}

EXPORT_C_(void) GSprintf(int timeout, char *fmt, ...)
{
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	GS_LOG("GSprintf:%s", msg);
	printf("GSprintf:%s", msg);
}

void SysPrintf(const char *fmt, ...)
{
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	GS_LOG(msg);
	printf("GSnull:%s", msg);
}

// basic funcs
EXPORT_C_(void) GSsetSettingsDir(const char* dir)
{
	s_strIniPath = (dir == NULL) ? "inis/" : dir;
}

EXPORT_C_(void) GSsetLogDir(const char* dir)
{
	// Get the path to the log directory.
	s_strLogPath = (dir==NULL) ? "logs/" : dir;

	// Reload the log file after updated the path
	if (gsLog) {
        fclose(gsLog);
        gsLog = NULL;
    }
    OpenLog();
}

EXPORT_C_(s32) GSinit()
{
	LoadConfig();

    OpenLog();

	SysPrintf("Initializing GSnull\n");
	return 0;
}

EXPORT_C_(void) GSshutdown()
{
	SysPrintf("Shutting down GSnull\n");
	GSCloseWindow();

#ifdef GS_LOG
	if (gsLog) {
        fclose(gsLog);
        gsLog = NULL;
    }
#endif
}

EXPORT_C_(s32) GSopen(void *pDsp, char *Title, int multithread)
{
	int err = 0;
	GS_LOG("GS open\n");
	//assert( GSirq != NULL );

	err = GSOpenWindow(pDsp, Title);
	gs.MultiThreaded = multithread;

	ResetRegs();
	SetMultithreaded();
	InitPath();
	SysPrintf("Opening GSnull\n");
	return err;
}

#ifdef USE_GSOPEN2
EXPORT_C_(s32) GSopen2( void *pDsp, u32 flags )
{
	GS_LOG("GS open2\n");

    GSOpenWindow2(pDsp, flags);

	gs.MultiThreaded = true;

	ResetRegs();
	SetMultithreaded();
	InitPath();
	SysPrintf("Opening GSnull (2)\n");
	return 0;
}
#endif

EXPORT_C_(void) GSclose()
{
	SysPrintf("Closing GSnull\n");

	// Better to only close the window on Shutdown.  All the other plugins
	// pretty much worked that way, and all old PCSX2 versions expect it as well.
	//GSCloseWindow();
}

EXPORT_C_(void) GSirqCallback(void (*callback)())
{
        GSirq = callback;
}

EXPORT_C_(s32) GSfreeze(int mode, freezeData *data)
{
	return 0;
}

EXPORT_C_(s32) GStest()
{
	SysPrintf("Testing GSnull\n");
	return 0;
}

EXPORT_C_(void) GSvsync(int field)
{
	GSProcessMessages();
}

 // returns the last tag processed (64 bits)
EXPORT_C_(void) GSgetLastTag(u64* ptag)
{
	*(u32*)ptag = gs.nPath3Hack;
	gs.nPath3Hack = 0;
}

EXPORT_C_(void) GSgifSoftReset(u32 mask)
{
	SysPrintf("Doing a soft reset of the GS plugin.\n");
}

EXPORT_C_(void) GSreadFIFO(u64 *mem)
{
}

EXPORT_C_(void) GSreadFIFO2(u64 *mem, int qwc)
{
}

// extended funcs

// GSkeyEvent gets called when there is a keyEvent from the PAD plugin
EXPORT_C_(void) GSkeyEvent(keyEvent *ev)
{
	HandleKeyEvent(ev);
}

EXPORT_C_(void) GSchangeSaveState(int, const char* filename)
{
}

EXPORT_C_(void) GSmakeSnapshot(char *path)
{

	SysPrintf("Taking a snapshot.\n");
}

EXPORT_C_(void) GSmakeSnapshot2(char *pathname, int* snapdone, int savejpg)
{
	SysPrintf("Taking a snapshot to %s.\n", pathname);
}

EXPORT_C_(void) GSsetBaseMem(void*)
{
}

EXPORT_C_(void) GSsetGameCRC(int crc, int gameoptions)
{
	SysPrintf("Setting the crc to '%x' with 0x%x for options.\n", crc, gameoptions);
}

// controls frame skipping in the GS, if this routine isn't present, frame skipping won't be done
EXPORT_C_(void) GSsetFrameSkip(int frameskip)
{
	SetFrameSkip(frameskip != 0);
	SysPrintf("Frameskip set to %d.\n", frameskip);
}

// if start is 1, starts recording spu2 data, else stops
// returns a non zero value if successful
// for now, pData is not used
EXPORT_C_(int) GSsetupRecording(int start, void* pData)
{
	if (start)
		SysPrintf("Pretending to record.\n");
	else
		SysPrintf("Pretending to stop recording.\n");

	return 1;
}

EXPORT_C_(void) GSreset()
{
	SysPrintf("Doing a reset of the GS plugin.");
}

EXPORT_C_(void) GSwriteCSR(u32 value)
{
}

EXPORT_C_(void) GSgetDriverInfo(GSdriverInfo *info)
{
}
