/*  ZZ Open GL graphics plugin
 *  Copyright (c)2009-2010 zeydlitz@gmail.com, arcum42@gmail.com
 *  Based on Zerofrog's ZeroGS KOSMOS (c)2005-2008
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "Util.h"
#include "GS.h"
#include "Profile.h"
#include "GLWin.h"
#include "ZZoglFlushHack.h"


using namespace std;

extern void SaveSnapshot(const char* filename);

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

GLWindow GLWin;
GSinternal gs;
GSconf conf;

int ppf, g_GSMultiThreaded, CurrentSavestate = 0;
int g_LastCRC = 0, g_TransferredToGPU = 0, s_frameskipping = 0;
int g_SkipFlushFrame = 0;
GetSkipCount GetSkipCount_Handler = 0;

int UPDATE_FRAMES = 16, g_nFrame = 0, g_nRealFrame = 0;
float fFPS = 0;

void (*GSirq)();
u8* g_pBasePS2Mem = NULL;
wxString s_strIniPath(L"inis");  	// Air's new ini path (r2361)

bool SaveStateExists = true;		// We could not know save slot status before first change occured
const char* SaveStateFile = NULL;	// Name of SaveFile for access check.

extern const char* s_aa[5];
extern const char* pbilinear[];
// statistics
u32 g_nGenVars = 0, g_nTexVars = 0, g_nAlphaVars = 0, g_nResolve = 0;

#define VER 3
const unsigned char zgsversion	= PS2E_GS_VERSION;
unsigned char zgsrevision = 0; // revision and build gives plugin version
unsigned char zgsbuild	= VER;
unsigned char zgsminor = 0;

#ifdef _DEBUG
char *libraryName	 = "ZZ Ogl PG (Debug) ";
#elif defined(ZEROGS_DEVBUILD)
char *libraryName	 = "ZZ Ogl PG (Dev)";
#else
char *libraryName	 = "ZZ Ogl PG ";
#endif

extern int g_nPixelShaderVer, g_nFrameRender, g_nFramesSkipped;

extern void WriteAA();
extern void WriteBilinear();
extern void ZZDestroy();
extern bool ZZCreate(int width, int height);
extern void ZZGSStateReset();
extern int ZZSave(s8* pbydata);
extern bool ZZLoad(s8* pbydata);

// switches the render target to the real target, flushes the current render targets and renders the real image
extern void RenderCRTC(int interlace);

#if defined(_WIN32) && defined(_DEBUG)
HANDLE g_hCurrentThread = NULL;
#endif

extern int VALIDATE_THRESH;
extern u32 TEXDESTROY_THRESH;

u32 CALLBACK PS2EgetLibType()
{
	return PS2E_LT_GS;
}

char* CALLBACK PS2EgetLibName()
{
	return libraryName;
}

u32 CALLBACK PS2EgetLibVersion2(u32 type)
{
	return (zgsversion << 16) | (zgsrevision << 8) | zgsbuild | (zgsminor << 24);
}

void CALLBACK GSsetBaseMem(void* pmem)
{
	g_pBasePS2Mem = (u8*)pmem;
}

void CALLBACK GSsetSettingsDir(const char* dir)
{
	s_strIniPath = (dir == NULL) ? wxString(L"inis") : wxString(dir, wxConvFile);
}

void CALLBACK GSsetLogDir(const char* dir)
{
	ZZLog::SetDir(dir);
}

void CALLBACK GSsetGameCRC(int crc, int options)
{
    // build a list of function pointer for GetSkipCount (SkipDraw)
	static GetSkipCount GSC_list[NUMBER_OF_TITLES];
	static bool inited = false;
	
	if (!inited)
	{
		inited = true;

		memset(GSC_list, 0, sizeof(GSC_list));
		
		GSC_list[Okami] = GSC_Okami;
		GSC_list[MetalGearSolid3] = GSC_MetalGearSolid3;
		GSC_list[DBZBT2] = GSC_DBZBT2;
		GSC_list[DBZBT3] = GSC_DBZBT3;
		GSC_list[SFEX3] = GSC_SFEX3;
		GSC_list[Bully] = GSC_Bully;
		GSC_list[BullyCC] = GSC_BullyCC;
		GSC_list[SoTC] = GSC_SoTC;
		GSC_list[OnePieceGrandAdventure] = GSC_OnePieceGrandAdventure;
		GSC_list[OnePieceGrandBattle] = GSC_OnePieceGrandBattle;
		GSC_list[ICO] = GSC_ICO;
		GSC_list[GT4] = GSC_GT4;
		GSC_list[WildArms4] = GSC_WildArms4;
		GSC_list[WildArms5] = GSC_WildArms5;
		GSC_list[Manhunt2] = GSC_Manhunt2;
		GSC_list[CrashBandicootWoC] = GSC_CrashBandicootWoC;
		GSC_list[ResidentEvil4] = GSC_ResidentEvil4;
		GSC_list[Spartan] = GSC_Spartan;
		GSC_list[AceCombat4] = GSC_AceCombat4;
		GSC_list[Drakengard2] = GSC_Drakengard2;
		GSC_list[Tekken5] = GSC_Tekken5;
		GSC_list[IkkiTousen] = GSC_IkkiTousen;
		GSC_list[GodOfWar] = GSC_GodOfWar;
		GSC_list[GodOfWar2] = GSC_GodOfWar2;
		GSC_list[GiTS] = GSC_GiTS;
		GSC_list[Onimusha3] = GSC_Onimusha3;
		GSC_list[TalesOfAbyss] = GSC_TalesOfAbyss;
		GSC_list[SonicUnleashed] = GSC_SonicUnleashed;
		GSC_list[Genji] = GSC_Genji;
		GSC_list[StarOcean3] = GSC_StarOcean3;
		GSC_list[ValkyrieProfile2] = GSC_ValkyrieProfile2;
		GSC_list[RadiataStories] = GSC_RadiataStories;
	}

	// TEXDESTROY_THRESH starts out at 16.
	VALIDATE_THRESH = 8;
	conf.mrtdepth = (conf.settings().disable_mrt_depth != 0);

	if (!conf.mrtdepth)
		ZZLog::WriteLn("Disabling MRT depth writing.");
	else
		ZZLog::WriteLn("Enabling MRT depth writing.");

	bool CRCValueChanged = (g_LastCRC != crc);

	g_LastCRC = crc;

	if (crc != 0) ZZLog::WriteLn("Current game CRC is %x.", crc);

	if (CRCValueChanged && (crc != 0))
	{
		for (int i = 0; i < GAME_INFO_INDEX; i++)
		{
			if (crc_game_list[i].crc == crc)
			{
				ZZLog::WriteLn("Found CRC[%x] in crc game list.", crc);
				
				if (crc_game_list[i].v_thresh > 0) 
				{
					VALIDATE_THRESH = crc_game_list[i].v_thresh;
					ZZLog::WriteLn("Setting VALIDATE_THRESH to %d", VALIDATE_THRESH);
				}
				
				if (crc_game_list[i].t_thresh > 0) 
				{
					TEXDESTROY_THRESH = crc_game_list[i].t_thresh;
					ZZLog::WriteLn("Setting TEXDESTROY_THRESH to %d", TEXDESTROY_THRESH);
				}

                // FIXME need to check SkipDraw is positive (enabled by users)
                GetSkipCount_Handler = GSC_list[crc_game_list[i].title];

				if (!conf.disableHacks) 
				{
					conf.def_hacks._u32 |= crc_game_list[i].flags;
					ListHacks();
				}
				return;
			}
		}
	}
	ListHacks();
}

void CALLBACK GSsetFrameSkip(int frameskip)
{
	FUNCLOG
	s_frameskipping |= frameskip;

	if (frameskip && g_nFrameRender > 1)
	{
		SetFrameSkip(true);
	}
	else if (!frameskip && g_nFrameRender <= 0)
	{
		SetFrameSkip(false);
	}
}

void CALLBACK GSreset()
{
	FUNCLOG

	memset(&gs, 0, sizeof(gs));

	ZZGSStateReset();

	gs.prac = 1;
	prim = &gs._prim[0];
	gs.imageTransfer = -1;
	gs.q = 1;
}

void CALLBACK GSgifSoftReset(u32 mask)
{
	FUNCLOG

	if (mask & 1) memset(&gs.path[0], 0, sizeof(gs.path[0]));
	if (mask & 2) memset(&gs.path[1], 0, sizeof(gs.path[1]));
	if (mask & 4) memset(&gs.path[2], 0, sizeof(gs.path[2]));

	gs.imageTransfer = -1;
	gs.q = 1;
}

s32 CALLBACK GSinit()
{
	FUNCLOG

    ZZLog::Open();
	ZZLog::WriteLn("Calling GSinit.");

	WriteTempRegs();
	GSreset();
	
	ZZLog::WriteLn("GSinit finished.");
	return 0;
}

__forceinline void InitMisc()
{
	WriteBilinear();
	WriteAA();
	InitProfile();
	InitPath();
	ResetRegs();
}

s32 CALLBACK GSopen(void *pDsp, char *Title, int multithread)
{
	FUNCLOG

	g_GSMultiThreaded = multithread;

	ZZLog::WriteLn("Calling GSopen.");

#if defined(_WIN32) && defined(_DEBUG)
	g_hCurrentThread = GetCurrentThread();
#endif

	LoadConfig();
	strcpy(GLWin.title, Title);

	ZZLog::GS_Log("Using %s:%d.%d.%d.", libraryName, zgsrevision, zgsbuild, zgsminor);
	
	ZZLog::WriteLn("Creating ZZOgl window.");
	if ((!GLWin.CreateWindow(pDsp)) || (!ZZCreate(conf.width, conf.height))) return -1;

	ZZLog::WriteLn("Initialization successful.");

	InitMisc();
	ZZLog::GS_Log("GSopen finished.");
	return 0;
}

#ifdef USE_GOPEN2
s32 CALLBACK GSopen2( void* pDsp, INT32 flags )
{
	FUNCLOG

	bool err;

	g_GSMultiThreaded = true;

	ZZLog::WriteLn("Calling GSopen2.");

#if defined(_WIN32) && defined(_DEBUG)
	g_hCurrentThread = GetCurrentThread();
#endif

	LoadConfig();
	
	ZZLog::GS_Log("Using %s:%d.%d.%d.", libraryName, zgsrevision, zgsbuild, zgsminor);

	ZZLog::WriteLn("Capturing ZZOgl window.");
	if ((!GLWin.GetWindow(pDsp)) || (!ZZCreate2(conf.width, conf.height))) return -1;// Needs to be added.

	ZZLog::WriteLn("Initialization successful.");

	InitMisc();
	ZZLog::GS_Log("GSopen2 finished.");
	return 0;
	
}
#endif

void CALLBACK GSshutdown()
{
	FUNCLOG

	ZZLog::Close();
}
void CALLBACK GSclose()
{
	FUNCLOG

	ZZDestroy();
	GLWin.CloseWindow();

	SaveStateFile = NULL;
	SaveStateExists = true; // default value
    g_LastCRC = 0;
}

void CALLBACK GSirqCallback(void (*callback)())
{
	FUNCLOG

	GSirq = callback;
}

void CALLBACK GSwriteCSR(u32 write)
{
	FUNCLOG

	gs.CSRw = write;
}

#ifdef _WIN32
#define access _access
#endif

void CALLBACK GSchangeSaveState(int newstate, const char* filename)
{
	FUNCLOG

	char str[255];
	sprintf(str, "save state %d", newstate);
	ZZAddMessage(str);
	CurrentSavestate = newstate;

	SaveStateFile = filename;
	SaveStateExists = (access(SaveStateFile, 0) == 0);
}

void CALLBACK GSmakeSnapshot(char *path)
{
	FUNCLOG

	FILE *bmpfile;
	char filename[256];
	u32 snapshotnr = 0;

	// increment snapshot value & try to get filename

	for (;;)
	{
		snapshotnr++;

		sprintf(filename, "%s/snap%03ld.%s", path, snapshotnr, (conf.zz_options.tga_snap) ? "bmp" : "jpg");

		bmpfile = fopen(filename, "rb");

		if (bmpfile == NULL) break;

		fclose(bmpfile);
	}

	// try opening new snapshot file
	if ((bmpfile = fopen(filename, "wb")) == NULL)
	{
		char strdir[255];
		sprintf(strdir, "%s", path);

#ifdef _WIN32
		CreateDirectory(wxString::FromUTF8(strdir), NULL);
#else
		mkdir(path, 0777);
#endif

		if ((bmpfile = fopen(filename, "wb")) == NULL) return;
	}

	fclose(bmpfile);

	// get the bits
	SaveSnapshot(filename);
}

// I'll probably move this somewhere else later, but it's got a ton of dependencies.
static __forceinline void SetGSTitle()
{
	char strtitle[256];

#if !defined(ZEROGS_DEVBUILD)
	const char* g_pShaders[4] = { "full", "reduced", "accurate", "accurate-reduced" };
	const char* g_pInterlace[3] = { "interlace 0 |", "interlace 1 |", "" };
	const char* g_pBilinear[3] = { "", "bilinear |", "forced bilinear |" };

	if (SaveStateFile != NULL && !SaveStateExists)
		SaveStateExists = (access(SaveStateFile, 0) == 0);
	else
		SaveStateExists = true;

	sprintf(strtitle, "ZZ Open GL 0.%d.%d | %.1f fps | %s%s%s savestate %d%s | shaders %s | (%.1f)", zgsbuild, zgsminor, fFPS,
			g_pInterlace[conf.interlace], g_pBilinear[conf.bilinear], (conf.aa ? s_aa[conf.aa] : ""),
					CurrentSavestate, (SaveStateExists ? "" :  "*"),
					g_pShaders[g_nPixelShaderVer], (ppf&0xfffff) / (float)UPDATE_FRAMES);

#else
	sprintf(strtitle, "%d | %.1f fps (sk:%d%%) | g: %.1f, t: %.1f, a: %.1f, r: %.1f | p: %.1f | tex: %d %d (%d kbpf)", g_nFrame, fFPS,
			100*g_nFramesSkipped / g_nFrame,
			g_nGenVars / (float)UPDATE_FRAMES, g_nTexVars / (float)UPDATE_FRAMES, g_nAlphaVars / (float)UPDATE_FRAMES,
			g_nResolve / (float)UPDATE_FRAMES, (ppf&0xfffff) / (float)UPDATE_FRAMES,
			g_MemTargs.listTargets.size(), g_MemTargs.listClearedTargets.size(), g_TransferredToGPU >> 10);

	//_snprintf(strtitle, 512, "%x %x", *(int*)(g_pbyGSMemory + 256 * 0x3e0c + 4), *(int*)(g_pbyGSMemory + 256 * 0x3e04 + 4));
#endif

//	if( g_nFrame > 100 && fFPS > 60.0f ) {
//		ZZLog::Debug_Log("Set profile.");
//		g_bWriteProfile = 1;
//	}
	GLWin.SetTitle(strtitle);
}

void CALLBACK GSvsync(int interlace)
{
	FUNCLOG

	//ZZLog::GS_Log("Calling GSvsync.");

	static u32 dwTime = timeGetTime();
	static int nToNextUpdate = 1;

	GL_REPORT_ERRORD();

	g_nRealFrame++;

	// !interlace? Hmmm... Fixme.
	RenderCRTC(!interlace);

	GLWin.ProcessEvents();

	if (--nToNextUpdate <= 0)
	{
		u32 d = timeGetTime();
		fFPS = UPDATE_FRAMES * 1000.0f / (float)max(d - dwTime, (u32)1);
		dwTime = d;
		g_nFrame += UPDATE_FRAMES;
		SetGSTitle();

//		if( g_nFrame > 100 && fFPS > 60.0f ) {
//			ZZLog::Debug_Log("Set profile.");
//			g_bWriteProfile = 1;
//		}

		if (fFPS < 16) 
			UPDATE_FRAMES = 4;
		else if (fFPS < 32) 
			UPDATE_FRAMES = 8;
		else 
			UPDATE_FRAMES = 16;

		nToNextUpdate = UPDATE_FRAMES;

		ppf = 0;
		g_TransferredToGPU = 0;
		g_nGenVars = 0;
		g_nTexVars = 0;
		g_nAlphaVars = 0;
		g_nResolve = 0;
		g_nFramesSkipped = 0;
        g_SkipFlushFrame = 0;
	}

#if defined(ZEROGS_DEVBUILD)
	if (g_bWriteProfile)
	{
		//g_bWriteProfile = 0;
		DVProfWrite("prof.txt", UPDATE_FRAMES);
		DVProfClear();
	}

#endif
	GL_REPORT_ERRORD();
}

void CALLBACK GSreadFIFO(u64 *pMem)
{
	FUNCLOG

	//ZZLog::GS_Log("Calling GSreadFIFO.");

	TransferLocalHost((u32*)pMem, 1);
}

void CALLBACK GSreadFIFO2(u64 *pMem, int qwc)
{
	FUNCLOG

	//ZZLog::GS_Log("Calling GSreadFIFO2.");

	TransferLocalHost((u32*)pMem, qwc);
}

int CALLBACK GSsetupRecording(int start, void* pData)
{
	FUNCLOG

	if (start)
		StartCapture();
	else
		StopCapture();

	return 1;
}

s32 CALLBACK GSfreeze(int mode, freezeData *data)
{
	FUNCLOG

	switch (mode)
	{
		case FREEZE_LOAD:
			if (!ZZLoad(data->data)) ZZLog::Error_Log("GS: Bad load format!");
			g_nRealFrame += 100;
			break;

		case FREEZE_SAVE:
			ZZSave(data->data);
			break;

		case FREEZE_SIZE:
			data->size = ZZSave(NULL);
			break;

		default:
			break;
	}

	return 0;
}
