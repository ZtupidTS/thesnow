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
#include "Debugger/DebuggerPanel.h"
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
#include "render.h"
#include "DLCache.h"
#include "IniFile.h"
#include "Core.h"

#include "ConfigManager.h"
#include "VideoBackend.h"

namespace DX9
{

unsigned int VideoBackend::PeekMessages()
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

void VideoBackend::UpdateFPSDisplay(const char *text)
{
	TCHAR temp[512];
	swprintf_s(temp, sizeof(temp)/sizeof(TCHAR), _T("%hs | DX9 | %hs"), svn_rev_str, text);
	SetWindowText(EmuWindow::GetWnd(), temp);
}

std::string VideoBackend::GetName()
{
	return "Direct3D9";
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
	int shaderModel = ((DX9::D3D::GetCaps().PixelShaderVersion >> 8) & 0xFF);
	int maxConstants = (shaderModel < 3) ? 32 : ((shaderModel < 4) ? 224 : 65536);	
	g_Config.backend_info.bSupportsPixelLighting = C_PLIGHTS + 40 <= maxConstants && C_PMATERIALS + 4 <= maxConstants;
}

void VideoBackend::ShowConfig(void* parent)
{
#if defined(HAVE_WX) && HAVE_WX
	DX9::D3D::Init();
	InitBackendInfo();

	// adapters
	g_Config.backend_info.Adapters.clear();
	for (int i = 0; i < DX9::D3D::GetNumAdapters(); ++i)
		g_Config.backend_info.Adapters.push_back(DX9::D3D::GetAdapter(i).ident.Description);

	// aamodes
	g_Config.backend_info.AAModes.clear();
	if (g_Config.iAdapter < DX9::D3D::GetNumAdapters())
	{
		const DX9::D3D::Adapter &adapter = DX9::D3D::GetAdapter(g_Config.iAdapter);

		for (int i = 0; i < (int)adapter.aa_levels.size(); ++i)
			g_Config.backend_info.AAModes.push_back(adapter.aa_levels[i].name);
	}
	
	// Clear ppshaders string vector
	g_Config.backend_info.PPShaders.clear();
	
	VideoConfigDiag *const diag = new VideoConfigDiag((wxWindow*)parent, _trans("Direct3D9"), "gfx_dx9");
	diag->ShowModal();
	diag->Destroy();

	g_Config.backend_info.Adapters.clear();
	DX9::D3D::Shutdown();
#endif
}

bool VideoBackend::Initialize(void *&window_handle)
{
	InitBackendInfo();

	frameCount = 0;

	g_Config.Load((File::GetUserPath(D_CONFIG_IDX) + "gfx_dx9.ini").c_str());
	g_Config.GameIniLoad(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strGameIni.c_str());
	UpdateProjectionHack(g_Config.iPhackvalue, g_Config.sPhackvalue);	// DX9 projection hack could be disabled by commenting out this line
	UpdateActiveConfig();

	window_handle = (void*)EmuWindow::Create((HWND)window_handle, GetModuleHandle(0), _T("Loading - Please wait."));
	if (window_handle == NULL)
	{
		ERROR_LOG(VIDEO, "An error has occurred while trying to create the window.");
		return false;
	}
	else if (FAILED(DX9::D3D::Init()))
	{
		MessageBox(GetActiveWindow(), _T("不能初始化 Direct3D. 请确认您已经安装了最新的 DirectX"), _T("Fatal Error"), MB_OK);
		return false;
	}

	OSD::AddMessage("Dolphin Direct3D9 Video Backend.", 5000);
	s_BackendInitialized = true;

	return true;
}

void VideoBackend::Video_Prepare()
{
	// Better be safe...
	s_efbAccessRequested = FALSE;
	s_FifoShuttingDown = FALSE;
	s_swapRequested = FALSE;

	// internal interfaces
	g_renderer = new Renderer;
	g_texture_cache = new TextureCache;
	g_vertex_manager = new VertexManager;
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

	// Notify the core that the video backend is ready
	Core::Callback_CoreMessage(WM_USER_CREATE);
}

void VideoBackend::Shutdown()
{
	s_BackendInitialized = false;

	if (g_renderer)
	{
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
		g_renderer = NULL;
	}
	D3D::Shutdown();
	EmuWindow::Close();
}

}
