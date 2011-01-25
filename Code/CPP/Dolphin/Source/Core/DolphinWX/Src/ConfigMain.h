﻿// Copyright (C) 2003 Dolphin Project.

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

#ifndef __CONFIG_MAIN_h__
#define __CONFIG_MAIN_h__

#include <wx/wx.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/filepicker.h>
#include "ConfigManager.h"
#if defined(HAVE_XRANDR) && HAVE_XRANDR
#include "X11Utils.h"
#endif

class CConfigMain : public wxDialog
{
public:

	CConfigMain(wxWindow* parent,
		wxWindowID id = 1,
#ifndef NO_MOD
		const wxString& title = _("Dolphin 配置"),
#else
		const wxString& title = _("Dolphin (MOD) 配置"),
#endif
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE);
	virtual ~CConfigMain();

	void OnOk(wxCommandEvent& event);
	void CloseClick(wxCommandEvent& event);
	void OnSelectionChanged(wxCommandEvent& event);
	void OnConfig(wxCommandEvent& event);

	bool bRefreshList;

private:
	wxNotebook* Notebook;
	wxPanel* GeneralPage;
	wxPanel* GamecubePage;
	wxPanel* DisplayPage;
	wxPanel* WiiPage;
	wxPanel* PathsPage;
	wxPanel* PluginsPage;

	wxBoxSizer* sGeneralPage; // General Settings
	wxStaticBoxSizer* sbBasic, *sbAdvanced; // Basic and Advanced sections
	
	// Basic
	wxCheckBox* CPUThread;
	wxCheckBox* SkipIdle;
	wxCheckBox* EnableCheats;
	wxChoice* Framelimit;
	wxCheckBox* UseFPSForLimiting;
	
	// Advanced
	wxCheckBox* AlwaysHLE_BS2;
	wxCheckBox* EnableOpenCL;
	wxRadioBox* CPUEngine;
	wxCheckBox* DSPThread;
	wxCheckBox* LockThreads;


	wxBoxSizer* sDisplayPage; // Display settings
	wxStaticBoxSizer* sbDisplay, *sbInterface; // Display and Interface sections

	// Display
	wxChoice* FullscreenResolution;
	wxSpinCtrl* WindowWidth, *WindowHeight;
	wxCheckBox* WindowAutoSize;
	wxCheckBox* Fullscreen;
	wxCheckBox* HideCursor;
	wxCheckBox* RenderToMain;
	wxCheckBox* ProgressiveScan;
	wxCheckBox* NTSCJ;

	// Interface
	wxCheckBox* ConfirmStop;
	wxCheckBox* UsePanicHandlers;
	wxRadioBox* Theme;
	wxChoice* InterfaceLang;
	wxButton* HotkeyConfig;


	wxBoxSizer* sGamecubePage; // GC settings
	wxStaticBoxSizer* sbGamecubeIPLSettings;
	wxGridBagSizer* sGamecubeIPLSettings;

	// IPL
	wxChoice* GCSystemLang;

	// Device
	wxChoice* GCEXIDevice[3];
	wxButton* GCMemcardPath[2];
	wxChoice* GCSIDevice[4];


	wxBoxSizer* sWiiPage; // Wii settings
	wxStaticBoxSizer* sbWiimoteSettings, *sbWiiIPLSettings, *sbWiiDeviceSettings; // Wiimote, Misc and Device sections
	wxGridBagSizer* sWiimoteSettings, *sWiiIPLSettings;

	// Wiimote
	wxChoice* WiiSensBarPos;
	wxSlider* WiiSensBarSens;
	wxCheckBox* WiimoteMotor;

	// Misc
	wxCheckBox* WiiScreenSaver;
	wxCheckBox* WiiEuRGB60;
	wxChoice* WiiAspectRatio;
	wxChoice* WiiSystemLang;

	// Device
	wxCheckBox* WiiSDCard;
	wxCheckBox* WiiKeyboard;


	wxBoxSizer* sPathsPage; // Paths settings
	wxStaticBoxSizer* sbISOPaths;
	wxGridBagSizer* sOtherPaths;

	// ISO Directories
	wxListBox* ISOPaths;
	wxCheckBox* RecursiveISOPath;
	wxButton* AddISOPath;
	wxButton* RemoveISOPath;

	// DefaultISO, DVD Root, Apploader
	wxFilePickerCtrl* DefaultISO;
	wxDirPickerCtrl* DVDRoot;
	wxFilePickerCtrl* ApploaderPath;


	wxBoxSizer* sPluginsPage; // Plugins settings
	wxStaticBoxSizer* sbGraphicsPlugin, *sbDSPPlugin;  // Graphics, DSP sections

	// Graphics
	wxChoice* GraphicSelection;
	wxButton* GraphicConfig;

	// DSP
	wxChoice* DSPSelection;
	wxButton* DSPConfig;

	wxButton* m_Ok;

	FILE* pStream;

	wxArrayString arrayStringFor_Framelimit;
	wxArrayString arrayStringFor_CPUEngine;
	wxArrayString arrayStringFor_FullscreenResolution;
	wxArrayString arrayStringFor_Themes;
	wxArrayString arrayStringFor_InterfaceLang;
	wxArrayString arrayStringFor_GCSystemLang;
	wxArrayString arrayStringFor_WiiSensBarPos;
	wxArrayString arrayStringFor_WiiAspectRatio;
	wxArrayString arrayStringFor_WiiSystemLang;
	wxArrayString arrayStringFor_ISOPaths;

	enum
	{
		ID_NOTEBOOK = 1000,
		ID_GENERALPAGE,
		ID_DISPLAYPAGE,
		ID_GAMECUBEPAGE,
		ID_WIIPAGE,
		ID_PATHSPAGE,
		ID_PLUGINPAGE,

		ID_CPUTHREAD,
		ID_IDLESKIP,
		ID_ENABLECHEATS,
		ID_FRAMELIMIT,
		ID_FRAMELIMIT_USEFPSFORLIMITING,
		
		ID_ALWAYS_HLE_BS2,
		ID_ENABLE_OPENCL,
		ID_CPUENGINE,
		ID_LOCKTHREADS,
		ID_DSPTHREAD,


		ID_DISPLAY_FULLSCREENRES,
		ID_DISPLAY_WINDOWWIDTH,
		ID_DISPLAY_WINDOWHEIGHT,
		ID_DISPLAY_AUTOSIZE,
		ID_DISPLAY_FULLSCREEN,
		ID_DISPLAY_HIDECURSOR,
		ID_DISPLAY_RENDERTOMAIN,
		ID_DISPLAY_PROGSCAN,
		ID_DISPLAY_NTSCJ,

		// Interface settings
		ID_INTERFACE_CONFIRMSTOP,
		ID_INTERFACE_USEPANICHANDLERS,
		ID_INTERFACE_THEME,
		ID_INTERFACE_LANG,
		ID_HOTKEY_CONFIG,


		ID_GC_SRAM_LNG,

		ID_GC_EXIDEVICE_SLOTA,
		ID_GC_EXIDEVICE_SLOTA_PATH,
		ID_GC_EXIDEVICE_SLOTB,
		ID_GC_EXIDEVICE_SLOTB_PATH,
		ID_GC_EXIDEVICE_SP1,
		ID_GC_SIDEVICE0,
		ID_GC_SIDEVICE1,
		ID_GC_SIDEVICE2,
		ID_GC_SIDEVICE3,


		ID_WII_BT_BAR,
		ID_WII_BT_SENS,
		ID_WII_BT_MOT,

		ID_WII_IPL_SSV,
		ID_WII_IPL_E60,
		ID_WII_IPL_AR,
		ID_WII_IPL_LNG,

		ID_WII_SD_CARD,
		ID_WII_KEYBOARD,


		ID_ISOPATHS,
		ID_RECURSIVEISOPATH,
		ID_ADDISOPATH,
		ID_REMOVEISOPATH,

		ID_DEFAULTISO,
		ID_DVDROOT,
		ID_APPLOADERPATH,


		ID_GRAPHIC_CB,
		ID_GRAPHIC_CONFIG,
		ID_GRAPHIC_ABOUT,

		ID_DSP_CB,
		ID_DSP_CONFIG,
		ID_DSP_ABOUT,
	};

	void InitializeGUILists();
	void InitializeGUIValues();
	void InitializeGUITooltips();

	void CreateGUIControls();
	void UpdateGUI();
	void OnClose(wxCloseEvent& event);

	void CoreSettingsChanged(wxCommandEvent& event);

	void DisplaySettingsChanged(wxCommandEvent& event);
	void AddResolutions();
	void OnSpin(wxSpinEvent& event);

	void GCSettingsChanged(wxCommandEvent& event);
	void ChooseMemcardPath(std::string& strMemcard, bool isSlotA);
	void ChooseSIDevice(std::string deviceName, int deviceNum);
	void ChooseEXIDevice(std::string deviceName, int deviceNum);

	void WiiSettingsChanged(wxCommandEvent& event);

	void ISOPathsSelectionChanged(wxCommandEvent& event);
	void RecursiveDirectoryChanged(wxCommandEvent& event);
	void AddRemoveISOPaths(wxCommandEvent& event);
	void DefaultISOChanged(wxFileDirPickerEvent& event);
	void DVDRootChanged(wxFileDirPickerEvent& event);
	void ApploaderPathChanged(wxFileDirPickerEvent& WXUNUSED (event));

	void FillChoiceBox(wxChoice* _pChoice, int _PluginType, const std::string& _SelectFilename);
	void CallConfig(wxChoice* _pChoice);
	bool GetFilename(wxChoice* _pChoice, std::string& _rFilename);
	DECLARE_EVENT_TABLE();
};
#endif
