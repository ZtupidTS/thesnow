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

#ifndef __PLUGIN_MANAGER_H_
#define __PLUGIN_MANAGER_H_

#include "Plugin.h"
#include "PluginDSP.h"
#include "PluginVideo.h"
#include "PluginWiimote.h"
#include "CoreParameter.h"

class CPluginInfo
{
public:
	CPluginInfo(const char *_rFileName);
	bool IsValid() const {return(m_Valid);}
	const PLUGIN_INFO& GetPluginInfo() const {return(m_PluginInfo);}
	const std::string& GetFilename() const {return(m_Filename);}

private:
	PLUGIN_INFO m_PluginInfo;
	std::string m_Filename;
	bool m_Valid;
};

typedef std::vector<CPluginInfo>CPluginInfos;


class CPluginManager
{
public:
	static CPluginManager& GetInstance() {return(*m_Instance);}

	static void Init();
	static void Shutdown();

	Common::PluginVideo *GetVideo();
	Common::PluginDSP *GetDSP();
	Common::PluginWiimote *GetWiimote();

	void FreeVideo();
	void FreeDSP();
	void FreeWiimote();

	void EmuStateChange(PLUGIN_EMUSTATE newState);

	bool InitPlugins();
	void ShutdownPlugins();
	void ShutdownVideoPlugin();
	void ScanForPlugins();
	void OpenConfig(void* _Parent, const char *_rFilename, PLUGIN_TYPE Type);
	void OpenDebug(void* _Parent, const char *_rFilename, PLUGIN_TYPE Type, bool Show);
	const CPluginInfos& GetPluginInfos() {return(m_PluginInfos);}
	PLUGIN_GLOBALS* GetGlobals();

private:
	static CPluginManager* m_Instance;

	CPluginInfos m_PluginInfos;
	PLUGIN_GLOBALS *m_PluginGlobals;
	Common::PluginVideo *m_video;
	Common::PluginDSP *m_dsp;
	Common::PluginWiimote *m_wiimote;

	SCoreStartupParameter * m_params;
	CPluginManager();
	~CPluginManager();
	void GetPluginInfo(CPluginInfo *&info, std::string Filename);
	void *LoadPlugin(const char *_rFilename);

	DISALLOW_COPY_AND_ASSIGN(CPluginManager);
};


#endif
