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

#ifndef _CONFIGMANAGER_H
#define _CONFIGMANAGER_H

#include <string>
#include <vector>

#include "Boot/Boot.h"
#include "HW/EXI_Device.h"
#include "HW/SI_Device.h"
#include "SysConf.h"

// HyperIris: not sure but a temporary implement
enum INTERFACE_LANGUAGE
{
	INTERFACE_ENGLISH = 0,
	INTERFACE_GERMAN,
	INTERFACE_FRENCH,
	INTERFACE_SPANISH,
	INTERFACE_ITALIAN,
	INTERFACE_DUTCH,

	INTERFACE_OTHER,
};

struct SConfig
{
	// Wii Devices
	bool m_WiiSDCard;
	bool m_WiiKeyboard;
	bool m_WiiAutoReconnect[4];

	// hard coded default plugins ...
	std::string m_DefaultGFXPlugin;
	std::string m_DefaultDSPPlugin;
	std::string m_DefaultWiiMotePlugin;

	// name of the last used filename
	std::string m_LastFilename;

	// gcm folder
	std::vector<std::string> m_ISOFolder;
	bool m_RecursiveISOFolder;

	SCoreStartupParameter m_LocalCoreStartupParameter;

	std::string m_strMemoryCardA;
	std::string m_strMemoryCardB;
	TEXIDevices m_EXIDevice[3];
	TSIDevices m_SIDevice[4];

	// interface language
	INTERFACE_LANGUAGE m_InterfaceLanguage;
	// framelimit choose
	int m_Framelimit;
	bool b_UseFPS;
	// other interface settings
	bool m_InterfaceToolbar;
	bool m_InterfaceStatusbar;
	bool m_InterfaceLogWindow;
	bool m_InterfaceConsole;

	bool m_ListDrives;
	bool m_ListWad;
	bool m_ListWii;
	bool m_ListGC;
	bool m_ListPal;
	bool m_ListUsa;
	bool m_ListJap;
	bool m_ListFrance;
	bool m_ListItaly;
	bool m_ListKorea;
	bool m_ListTaiwan;
	bool m_ListUnknown;

	SysConf* m_SYSCONF;

	// save settings
	void SaveSettings();

	// load settings
	void LoadSettings();

	//Special load settings
	void LoadSettingsWii();

	// Return the permanent and somewhat globally used instance of this struct
	static SConfig& GetInstance() {return(*m_Instance);}

	static void Init();
	static void Shutdown();

private:
	SConfig();
	~SConfig();

	static SConfig* m_Instance;

	DISALLOW_COPY_AND_ASSIGN(SConfig);
};

#endif // endif config manager

