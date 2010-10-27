
//________________________________________________________________________________________
// File description: Common plugin spec, version #1.0 maintained by F|RES

#ifndef _PLUGINS_H_INCLUDED__
#define _PLUGINS_H_INCLUDED__

// Includes
// ------------
// TODO: See if we can get rid of the windows.h include.
#ifdef _WIN32
	#include <windows.h>
#endif
#include "Common.h"
#include "CommonTypes.h"

// Plugin communication. I place this here rather in Common.h to rebuild less if any of this is changed
// -----------------
enum PLUGIN_COMM
{
	// Begin at 10 in case there is already messages with wParam = 0, 1, 2 and so on
	WM_USER_PAUSE = 10,
	WM_USER_STOP,
	WM_USER_CREATE,
	WM_USER_SETCURSOR,
	WM_USER_KEYDOWN,
	WIIMOTE_DISCONNECT, // Disconnect Wiimote
	INPUT_FRAME_COUNTER // Wind back the frame counter for rerecording
};

// System specific declarations and definitions
// ------------

#ifdef _WIN32
	#define EXPORT	__declspec(dllexport)
	#define CALL __cdecl
#else
	#define EXPORT	__attribute__ ((visibility("default")))
	#define __cdecl
	#define CALL
	#ifndef TRUE
		#define TRUE 1
		#define FALSE 0 
	#endif
#endif

#if defined(__cplusplus)
	extern "C" {
#endif



// Global values
// ------------

// Plugin types
enum PLUGIN_TYPE {
	PLUGIN_TYPE_VIDEO = 1,
	PLUGIN_TYPE_DVD,
	PLUGIN_TYPE_PAD_REMOVED,
	PLUGIN_TYPE_AUDIO,
	PLUGIN_TYPE_COMPILER,
	PLUGIN_TYPE_DSP,
	PLUGIN_TYPE_WIIMOTE_REMOVED,
};

#define STATE_MODE_READ    1
#define STATE_MODE_WRITE   2
#define STATE_MODE_MEASURE 3

// used for notification on emulation state
enum PLUGIN_EMUSTATE {
	PLUGIN_EMUSTATE_PLAY = 1,
	PLUGIN_EMUSTATE_PAUSE,
	PLUGIN_EMUSTATE_STOP,
};

// Export structs
// ------------
typedef struct 
{
	u16 Version;		// Set to 0x0100
	PLUGIN_TYPE Type;	// Set to PLUGIN_TYPE_DVD
	char Name[100];		// Name of the DLL
} PLUGIN_INFO;

// TODO: Remove, or at least remove the void pointers and replace with data.
// This design is just wrong and ugly - the plugins shouldn't have this much access.
typedef struct 
{
	void *was_eventHandler_but_lets_not_break_abi;
	void *logManager;
	char game_ini[MAX_PATH];
	char unique_id[16];
} PLUGIN_GLOBALS;

// GLOBAL I N T E R F A C E 
// ____________________________________________________________________________
// Function: GetDllInfo
// Purpose:  This function allows the emulator to gather information
//           about the DLL by filling in the PluginInfo structure.
// input:    A pointer to a PLUGIN_INFO structure that needs to be
//           filled by the function. (see def above)
// output:   none
//
EXPORT void CALL GetDllInfo(PLUGIN_INFO* _pPluginInfo);

// ___________________________________________________________________________
// Function: DllConfig
// Purpose:  This function is optional function that is provided
//           to allow the user to configure the DLL
// input:    A handle to the window that calls this function
// output:   none
//
EXPORT void CALL DllConfig(void *_hParent);

// ___________________________________________________________________________
// Function: DllDebugger
// Purpose:  Open the debugger
// input:    a handle to the window that calls this function
// output:   none
//
EXPORT void* CALL DllDebugger(void *_hParent, bool Show);

// ___________________________________________________________________________
// Function: DllSetGlobals
// Purpose:  Set the pointer for globals variables
// input:    a pointer to the global struct
// output:   none
//
EXPORT void CALL SetDllGlobals(PLUGIN_GLOBALS* _pPluginGlobals);

// ___________________________________________________________________________
// Function: Initialize
// Purpose: Initialize the plugin
// input:    Init
// output:   none
//
EXPORT void CALL Initialize(void *init);

// ___________________________________________________________________________
// Function: Shutdown
// Purpose:  This function is called when the emulator is shutting down
//           a game allowing the dll to de-initialise.
// input:    none
// output:   none
//
EXPORT void CALL Shutdown(void);

// ___________________________________________________________________________
// Function: DoState
// Purpose:  Saves/load state
// input/output: ptr
// input: mode
//
EXPORT void CALL DoState(unsigned char **ptr, int mode);

// ___________________________________________________________________________
// Function: EmuStateChange
// Purpose: Notifies the plugin of a change in emulation state
// input:    newState
// output:   none
//
EXPORT void CALL EmuStateChange(PLUGIN_EMUSTATE newState);


#if defined(__cplusplus)
	}
#endif

#endif // _PLUGINS_H_INCLUDED__
