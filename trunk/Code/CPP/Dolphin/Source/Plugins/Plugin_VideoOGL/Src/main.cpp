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



// OpenGL Plugin Documentation
/* 

1.1 Display settings

Internal and fullscreen resolution: Since the only internal resolutions allowed
are also fullscreen resolution allowed by the system there is only need for one
resolution setting that applies to both the internal resolution and the
fullscreen resolution.  - Apparently no, someone else doesn't agree

Todo: Make the internal resolution option apply instantly, currently only the
native and 2x option applies instantly. To do this we need to be able to change
the reinitialize FramebufferManager:Init() while a game is running.

1.2 Screenshots


The screenshots should be taken from the internal representation of the picture
regardless of what the current window size is. Since AA and wireframe is
applied together with the picture resizing this rule is not currently applied
to AA or wireframe pictures, they are instead taken from whatever the window
size is.

Todo: Render AA and wireframe to a separate picture used for the screenshot in
addition to the one for display.

1.3 AA

Make AA apply instantly during gameplay if possible

*/

#include "Globals.h"
#include "Atomic.h"
#include "Thread.h"
#include "LogManager.h"

#include <cstdarg>

#ifdef _WIN32
#include "EmuWindow.h"
#include "IniFile.h"
#endif

#if defined(HAVE_WX) && HAVE_WX
#include "VideoConfigDiag.h"
#include "DebuggerPanel.h"
#endif // HAVE_WX

#include "MainBase.h"
#include "VideoConfig.h"
#include "LookUpTables.h"
#include "ImageWrite.h"
#include "Render.h"
#include "GLUtil.h"
#include "Fifo.h"
#include "OpcodeDecoding.h"
#include "TextureCache.h"
#include "BPStructs.h"
#include "VertexLoader.h"
#include "VertexLoaderManager.h"
#include "VertexManager.h"
#include "PixelShaderCache.h"
#include "PixelShaderManager.h"
#include "VertexShaderCache.h"
#include "VertexShaderManager.h"
#include "XFBConvert.h"
#include "CommandProcessor.h"
#include "PixelEngine.h"
#include "TextureConverter.h"
#include "PostProcessing.h"
#include "OnScreenDisplay.h"
#include "Setup.h"
#include "DLCache.h"
#include "FramebufferManager.h"

#include "VideoState.h"

// Logging
int GLScissorX, GLScissorY, GLScissorW, GLScissorH;

#ifdef _WIN32
HINSTANCE g_hInstance;

wxLocale *InitLanguageSupport()
{
	wxLocale *m_locale;
	unsigned int language = 0;

	IniFile ini;
	ini.Load(File::GetUserPath(F_DOLPHINCONFIG_IDX));
	ini.Get("Interface", "Language", &language, wxLANGUAGE_DEFAULT);

	// Load language if possible, fall back to system default otherwise
	if(wxLocale::IsAvailable(language))
	{
		m_locale = new wxLocale(language);

		m_locale->AddCatalogLookupPathPrefix(wxT("Languages"));

		m_locale->AddCatalog(wxT("dolphin-emu"));

		if(!m_locale->IsOk())
		{
			PanicAlertT("Error loading selected language. Falling back to system default.");
			delete m_locale;
			m_locale = new wxLocale(wxLANGUAGE_DEFAULT);
		}
	}
	else
	{
		PanicAlertT("The selected language is not supported by your system. Falling back to system default.");
		m_locale = new wxLocale(wxLANGUAGE_DEFAULT);
	}
	return m_locale;
}

class wxDLLApp : public wxApp
{
	bool OnInit()
	{
		return true;
	}
};
IMPLEMENT_APP_NO_MAIN(wxDLLApp) 
WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);
// ------------------

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	static wxLocale *m_locale;
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			{
				wxSetInstance((HINSTANCE)hinstDLL);
				wxInitialize();
				m_locale = InitLanguageSupport();
			}
			break; 

		case DLL_PROCESS_DETACH:
			wxUninitialize();
			delete m_locale;
			break;
	}

	g_hInstance = hinstDLL;
	return TRUE;
}
#endif // _WIN32

void GetDllInfo(PLUGIN_INFO* _PluginInfo)
{
	_PluginInfo->Version = 0x0100;
	_PluginInfo->Type = PLUGIN_TYPE_VIDEO;
#ifdef DEBUGFAST
	sprintf(_PluginInfo->Name, "Dolphin OpenGL (DebugFast)");
#elif defined _DEBUG
	sprintf(_PluginInfo->Name, "Dolphin OpenGL (Debug)");
#else
	sprintf(_PluginInfo->Name, _trans("Dolphin OpenGL"));
#endif
}

void SetDllGlobals(PLUGIN_GLOBALS* _pPluginGlobals)
{
	globals = _pPluginGlobals;
	LogManager::SetInstance((LogManager*)globals->logManager);
}

void *DllDebugger(void *_hParent, bool Show)
{
#if defined(HAVE_WX) && HAVE_WX
	return new GFXDebuggerPanel((wxWindow *)_hParent);
#else
	return NULL;
#endif
}

void GetShaders(std::vector<std::string> &shaders)
{
        shaders.clear();
        if (File::IsDirectory(File::GetUserPath(D_SHADERS_IDX)))
        {
                File::FSTEntry entry;
                File::ScanDirectoryTree(File::GetUserPath(D_SHADERS_IDX), entry);
                for (u32 i = 0; i < entry.children.size(); i++) 
                {
                        std::string name = entry.children[i].virtualName.c_str();
                        if (!strcasecmp(name.substr(name.size() - 4).c_str(), ".txt"))
                                name = name.substr(0, name.size() - 4);
                        shaders.push_back(name);
                }
        }
        else
        {
                File::CreateDir(File::GetUserPath(D_SHADERS_IDX));
        }
}

void InitBackendInfo()
{
	g_Config.backend_info.APIType = API_OPENGL;
	g_Config.backend_info.bUseRGBATextures = false;
	g_Config.backend_info.bSupportsEFBToRAM = true;
	g_Config.backend_info.bSupportsRealXFB = true;
	g_Config.backend_info.bSupports3DVision = false;
	g_Config.backend_info.bAllowSignedBytes = true;
	g_Config.backend_info.bSupportsDualSourceBlend = false; // supported, but broken
	g_Config.backend_info.bSupportsFormatReinterpretation = false;
	g_Config.backend_info.bSupportsPixelLighting = true;
}

void DllConfig(void *_hParent)
{
#if defined(HAVE_WX) && HAVE_WX
	InitBackendInfo();

	// aamodes
	const char* caamodes[] = {"None", "2x", "4x", "8x", "8x CSAA", "8xQ CSAA", "16x CSAA", "16xQ CSAA"};
	g_Config.backend_info.AAModes.assign(caamodes, caamodes + sizeof(caamodes)/sizeof(*caamodes));

	// pp shaders
	GetShaders(g_Config.backend_info.PPShaders);

	VideoConfigDiag *const diag = new VideoConfigDiag((wxWindow*)_hParent, "OpenGL", "gfx_opengl");
	diag->ShowModal();
	diag->Destroy();
#endif
}

void Initialize(void *init)
{
	InitBackendInfo();

	frameCount = 0;
	SVideoInitialize *_pVideoInitialize = (SVideoInitialize*)init;
	// Create a shortcut to _pVideoInitialize that can also update it
	g_VideoInitialize = *(_pVideoInitialize);
	InitXFBConvTables();

	g_Config.Load((std::string(File::GetUserPath(D_CONFIG_IDX)) + "gfx_opengl.ini").c_str());
	g_Config.GameIniLoad(globals->game_ini);

	g_Config.UpdateProjectionHack();
#if defined _WIN32
	// Enable support for PNG screenshots.
	wxImage::AddHandler( new wxPNGHandler );
#endif
	UpdateActiveConfig();

	if (!OpenGL_Create(g_VideoInitialize, 640, 480))
	{
		g_VideoInitialize.pLog("Renderer::Create failed\n", TRUE);
		return;
	}

	_pVideoInitialize->pPeekMessages = g_VideoInitialize.pPeekMessages;
	_pVideoInitialize->pUpdateFPSDisplay = g_VideoInitialize.pUpdateFPSDisplay;

	// Now the window handle is written
	_pVideoInitialize->pWindowHandle = g_VideoInitialize.pWindowHandle;

	OSD::AddMessage("Dolphin OpenGL Video Plugin.", 5000);
	s_PluginInitialized = true;
}

// This is called after Initialize() from the Core
// Run from the graphics thread
void Video_Prepare()
{
	OpenGL_MakeCurrent();
	//if (!Renderer::Init()) {
	//	g_VideoInitialize.pLog("Renderer::Create failed\n", TRUE);
	//	PanicAlert("Can't create opengl renderer. You might be missing some required opengl extensions, check the logs for more info");
	//	exit(1);
	//}

	g_renderer = new OGL::Renderer;

	s_efbAccessRequested = FALSE;
	s_FifoShuttingDown = FALSE;
	s_swapRequested = FALSE;

	CommandProcessor::Init();
	PixelEngine::Init();

	g_texture_cache = new OGL::TextureCache;

	BPInit();
	g_vertex_manager = new OGL::VertexManager;
	Fifo_Init(); // must be done before OpcodeDecoder_Init()
	OpcodeDecoder_Init();
	VertexShaderCache::Init();
	VertexShaderManager::Init();
	PixelShaderCache::Init();
	PixelShaderManager::Init();
	PostProcessing::Init();
	GL_REPORT_ERRORD();
	VertexLoaderManager::Init();
	TextureConverter::Init();
	DLCache::Init();

	// Notify the core that the video plugin is ready
	g_VideoInitialize.pCoreMessage(WM_USER_CREATE);

	s_PluginInitialized = true;
	INFO_LOG(VIDEO, "Video plugin initialized.");

}

void Shutdown()
{
	s_PluginInitialized = false;

	s_efbAccessRequested = FALSE;
	s_FifoShuttingDown = FALSE;
	s_swapRequested = FALSE;
	DLCache::Shutdown();
	Fifo_Shutdown();
	PostProcessing::Shutdown();

	// The following calls are NOT Thread Safe
	// And need to be called from the video thread
	TextureConverter::Shutdown();
	VertexLoaderManager::Shutdown();
	VertexShaderCache::Shutdown();
	VertexShaderManager::Shutdown();
	PixelShaderManager::Shutdown();
	PixelShaderCache::Shutdown();
	delete g_vertex_manager;
	delete g_texture_cache;
	OpcodeDecoder_Shutdown();
	delete g_renderer;
	OpenGL_Shutdown();
}
