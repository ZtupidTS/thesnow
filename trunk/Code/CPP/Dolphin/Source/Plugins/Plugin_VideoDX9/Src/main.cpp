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
#include "Atomic.h"
#include "Thread.h"
#include "LogManager.h"

#if defined(HAVE_WX) && HAVE_WX
#include "VideoConfigDiag.h"
#endif // HAVE_WX

#if defined(HAVE_WX) && HAVE_WX
#include "DebuggerPanel.h"
#endif // HAVE_WX

#include "MainBase.h"
#include "main.h"
#include "VideoConfig.h"
#include "Fifo.h"
#include "OpcodeDecoding.h"
#include "TextureCache.h"
#include "BPStructs.h"
#include "VertexManager.h"
#include "FramebufferManager.h"
#include "VertexLoaderManager.h"
#include "VertexShaderManager.h"
#include "PixelShaderManager.h"
#include "VertexShaderCache.h"
#include "PixelShaderCache.h"
#include "CommandProcessor.h"
#include "PixelEngine.h"
#include "OnScreenDisplay.h"
#include "D3DTexture.h"
#include "D3DUtil.h"
#include "EmuWindow.h"
#include "VideoState.h"
#include "XFBConvert.h"
#include "render.h"
#include "DLCache.h"
#include "IniFile.h"

HINSTANCE g_hInstance = NULL;

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


// This is used for the functions right below here which use wxwidgets
WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);

void *DllDebugger(void *_hParent, bool Show)
{
	return new GFXDebuggerPanel((wxWindow*)_hParent);
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

unsigned int Callback_PeekMessages()
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return FALSE;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}


void UpdateFPSDisplay(const char *text)
{
	TCHAR temp[100];
	swprintf_s(temp, sizeof temp, _T("%s | DX9 | %s"), svn_rev_str, text);
	SetWindowText(EmuWindow::GetWnd(), temp);
}

void GetDllInfo(PLUGIN_INFO* _PluginInfo)
{
	_PluginInfo->Version = 0x0100;
	_PluginInfo->Type = PLUGIN_TYPE_VIDEO;
#ifdef DEBUGFAST
	sprintf_s(_PluginInfo->Name, 100, "Dolphin Direct3D9 (DebugFast)");
#elif defined _DEBUG
	sprintf_s(_PluginInfo->Name, 100, "Dolphin Direct3D9 (Debug)");
#else
	sprintf_s(_PluginInfo->Name, 100, _trans("Dolphin Direct3D9"));
#endif
}

void SetDllGlobals(PLUGIN_GLOBALS* _pPluginGlobals)
{
	globals = _pPluginGlobals;
	LogManager::SetInstance((LogManager*)globals->logManager);
}

void DllAbout(HWND _hParent)
{
	//DialogBox(g_hInstance,(LPCTSTR)IDD_ABOUT,_hParent,(DLGPROC)AboutProc);
}

void InitBackendInfo()
{
	g_Config.backend_info.APIType = API_D3D9;
	g_Config.backend_info.bUseRGBATextures = true;
	g_Config.backend_info.bSupportsEFBToRAM = true;
	g_Config.backend_info.bSupportsRealXFB = true;
	g_Config.backend_info.bSupports3DVision = true;
	g_Config.backend_info.bAllowSignedBytes = false;
	g_Config.backend_info.bSupportsDualSourceBlend = false;
	g_Config.backend_info.bSupportsFormatReinterpretation = true;
	int shaderModel = ((D3D::GetCaps().PixelShaderVersion >> 8) & 0xFF);
	int maxConstants = (shaderModel < 3) ? 32 : ((shaderModel < 4) ? 224 : 65536);	
	g_Config.backend_info.bSupportsPixelLighting = C_PLIGHTS + 40 <= maxConstants && C_PMATERIALS + 4 <= maxConstants;
}

void DllConfig(void *_hParent)
{
#if defined(HAVE_WX) && HAVE_WX
	InitBackendInfo();
	D3D::Init();

	// adapters
	g_Config.backend_info.Adapters.clear();
	for (int i = 0; i < D3D::GetNumAdapters(); ++i)
		g_Config.backend_info.Adapters.push_back(D3D::GetAdapter(i).ident.Description);

	// aamodes
	g_Config.backend_info.AAModes.clear();
	if (g_Config.iAdapter < D3D::GetNumAdapters())
	{
		const D3D::Adapter &adapter = D3D::GetAdapter(g_Config.iAdapter);

		for (int i = 0; i < (int)adapter.aa_levels.size(); ++i)
			g_Config.backend_info.AAModes.push_back(adapter.aa_levels[i].name);
	}


	VideoConfigDiag *const diag = new VideoConfigDiag((wxWindow*)_hParent, _trans("Direct3D9"), "gfx_dx9");
	diag->ShowModal();
	diag->Destroy();

	D3D::Shutdown();
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

	g_Config.Load((std::string(File::GetUserPath(D_CONFIG_IDX)) + "gfx_dx9.ini").c_str());
	g_Config.GameIniLoad(globals->game_ini);
	UpdateProjectionHack(g_Config.iPhackvalue);	// DX9 projection hack could be disabled by commenting out this line
	UpdateActiveConfig();

	g_VideoInitialize.pWindowHandle = (void*)EmuWindow::Create((HWND)g_VideoInitialize.pWindowHandle, g_hInstance, _T("Loading - Please wait."));
	if (g_VideoInitialize.pWindowHandle == NULL)
	{
		ERROR_LOG(VIDEO, "An error has occurred while trying to create the window.");
		return;
	}
	else if (FAILED(D3D::Init()))
	{
		MessageBox(GetActiveWindow(), _T("不能初始化 Direct3D. 请确认您已经安装了最新的 DirectX"), _T("Fatal Error"), MB_OK);
		return;
	}

	g_VideoInitialize.pPeekMessages = &Callback_PeekMessages;
	g_VideoInitialize.pUpdateFPSDisplay = &UpdateFPSDisplay;

	_pVideoInitialize->pPeekMessages = g_VideoInitialize.pPeekMessages;
	_pVideoInitialize->pUpdateFPSDisplay = g_VideoInitialize.pUpdateFPSDisplay;

	// Now the window handle is written
	_pVideoInitialize->pWindowHandle = g_VideoInitialize.pWindowHandle;

	OSD::AddMessage("Dolphin Direct3D9 Video Plugin.", 5000);
	s_PluginInitialized = true;
}

void Video_Prepare()
{
	// Better be safe...
	s_efbAccessRequested = FALSE;
	s_FifoShuttingDown = FALSE;
	s_swapRequested = FALSE;

	// internal interfaces
	g_renderer = new DX9::Renderer;
	g_texture_cache = new DX9::TextureCache;
	g_vertex_manager = new DX9::VertexManager;
	// VideoCommon
	BPInit();
	Fifo_Init();
	VertexLoaderManager::Init();
	OpcodeDecoder_Init();
	VertexShaderManager::Init();
	PixelShaderManager::Init();
	CommandProcessor::Init();
	PixelEngine::Init();
	DLCache::Init();

	// Notify the core that the video plugin is ready
	g_VideoInitialize.pCoreMessage(WM_USER_CREATE);
}

void Shutdown()
{
	s_PluginInitialized = false;

	s_efbAccessRequested = FALSE;
	s_FifoShuttingDown = FALSE;
	s_swapRequested = FALSE;

	// VideoCommon
	DLCache::Shutdown();
	Fifo_Shutdown();
	CommandProcessor::Shutdown();
	PixelShaderManager::Shutdown();
	VertexShaderManager::Shutdown();
	OpcodeDecoder_Shutdown();
	VertexLoaderManager::Shutdown();

	// internal interfaces
	PixelShaderCache::Shutdown();
	VertexShaderCache::Shutdown();
	delete g_vertex_manager;
	delete g_texture_cache;
	delete g_renderer;
	D3D::Shutdown();
	EmuWindow::Close();
}
