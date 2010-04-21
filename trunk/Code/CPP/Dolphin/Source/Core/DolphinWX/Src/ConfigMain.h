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

#ifndef __CONFIG_MAIN_h__
#define __CONFIG_MAIN_h__

#include <wx/wx.h>
#include <wx/gbsizer.h>
#include <wx/notebook.h>
#include <wx/filepicker.h>
#include "ConfigManager.h"

class CConfigMain : public wxDialog
{
public:

	CConfigMain(wxWindow* parent,
		wxWindowID id = 1,
#ifndef NO_MOD
		const wxString& title = wxT("Dolphin ����"),
#else
		const wxString& title = wxT("Dolphin (MOD) ����"),
#endif
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE);
	virtual ~CConfigMain();

	void OnClick(wxMouseEvent& event);
	void CloseClick(wxCommandEvent& event);
	void OnSelectionChanged(wxCommandEvent& event);
	void OnConfig(wxCommandEvent& event);

	bool bRefreshList;

private:

	DECLARE_EVENT_TABLE();

	wxBoxSizer* sGeneralPage; // General Settings
	wxCheckBox* ConfirmStop, *UsePanicHandlers;
	wxCheckBox* HideCursor;
	wxChoice* InterfaceLang;
	wxChoice* Framelimit;
	wxRadioBox* Theme;
	wxCheckBox* Fullscreen;
	wxCheckBox* RenderToMain;
	wxButton* HotkeyConfig;

	wxBoxSizer* sCore;
	wxStaticBoxSizer* sbBasic, *sbAdvanced, *sbInterface, *sbDisplay;
	wxCheckBox* AlwaysHLE_BS2;
	wxRadioButton* m_RadioInt;
	wxRadioButton* m_RadioJIT;
	wxRadioButton* m_RadioJITIL;
	wxCheckBox* CPUThread;
	wxCheckBox* DSPThread;
	wxCheckBox* LockThreads;
	wxCheckBox* SkipIdle;
	wxCheckBox* EnableCheats;

	wxBoxSizer* sGamecube; // GC settings
	wxStaticBoxSizer* sbGamecubeIPLSettings;
	wxGridBagSizer* sGamecubeIPLSettings;
	wxStaticText* GCSystemLangText;
	wxChoice* GCSystemLang;
	wxChoice *GCEXIDevice[3];
	wxButton *GCMemcardPath[2];
	wxChoice *GCSIDevice[4];

	wxBoxSizer* sWii; // Wii settings
	wxStaticBoxSizer* sbWiimoteSettings;
	wxGridBagSizer* sWiimoteSettings;
	wxStaticBoxSizer* sbWiiIPLSettings;
	wxGridBagSizer* sWiiIPLSettings;
	wxStaticBoxSizer* sbWiiDeviceSettings;
	wxBoxSizer* sPaths;
	wxStaticBoxSizer* sbISOPaths;		
	wxBoxSizer* sISOButtons;
	wxGridBagSizer* sOtherPaths;
	wxBoxSizer* sPlugins;
	wxStaticBoxSizer* sbGraphicsPlugin;
	wxStaticBoxSizer* sbDSPPlugin;
	wxStaticBoxSizer* sbPadPlugin;
	wxStaticBoxSizer* sbWiimotePlugin;

	wxNotebook *Notebook;
	wxPanel *GeneralPage;
	wxPanel *GamecubePage;
	wxPanel *WiiPage;
	wxPanel *PathsPage;
	wxPanel *PluginPage;

	wxButton* m_Close;

	FILE* pStream;

	wxStaticText* WiiSensBarPosText;
	wxChoice* WiiSensBarPos;

	wxCheckBox* WiiScreenSaver; // IPL settings
	wxCheckBox* WiiProgressiveScan;
	wxCheckBox* WiiEuRGB60;
	wxStaticText* WiiAspectRatioText;
	wxChoice* WiiAspectRatio;
	wxStaticText* WiiSystemLangText;
	wxChoice* WiiSystemLang;
	wxCheckBox* WiiSDCard;
	wxCheckBox* WiiKeyboard;

	wxListBox* ISOPaths;
	wxButton* AddISOPath;
	wxButton* RemoveISOPath;
	wxCheckBox* RecersiveISOPath;
	wxStaticText* DefaultISOText;
	wxFilePickerCtrl* DefaultISO;
	wxStaticText* DVDRootText;
	wxDirPickerCtrl* DVDRoot;
	wxStaticText* ApploaderPathText;
	wxFilePickerCtrl* ApploaderPath;

	wxStaticText* PADText;
	wxButton* PADConfig;
	wxChoice* PADSelection;
	wxButton* DSPConfig;
	wxStaticText* DSPText;
	wxChoice* DSPSelection;
	wxButton* GraphicConfig;
	wxStaticText* GraphicText;
	wxChoice* GraphicSelection;
	wxButton* WiimoteConfig;
	wxStaticText* WiimoteText;
	wxChoice* WiimoteSelection;

	wxArrayString arrayStringFor_InterfaceLang;
	wxArrayString arrayStringFor_Framelimit;
	wxArrayString arrayStringFor_GCSystemLang;
	wxArrayString arrayStringFor_WiiSensBarPos;
	wxArrayString arrayStringFor_WiiAspectRatio;
	wxArrayString arrayStringFor_WiiSystemLang;
	wxArrayString arrayStringFor_ISOPaths;
	wxArrayString arrayStringFor_Themes;

	enum
	{
		ID_NOTEBOOK = 1000,
		ID_GENERALPAGE,
		ID_GAMECUBEPAGE,
		ID_WIIPAGE,
		ID_PATHSPAGE,
		ID_PLUGINPAGE,

		ID_ALWAYS_HLE_BS2,
		ID_RADIOJIT,
		ID_RADIOJITIL,
		ID_RADIOINT,
		ID_CPUTHREAD,
		ID_DSPTHREAD,
		ID_LOCKTHREADS,
		ID_IDLESKIP,
		ID_ENABLECHEATS,

		ID_INTERFACE_CONFIRMSTOP, // Interface settings
		ID_INTERFACE_USEPANICHANDLERS,
		ID_DISPLAY_FULLSCREEN,
		ID_DISPLAY_HIDECURSOR,
		ID_DISPLAY_RENDERTOMAIN,
		ID_HOTKEY_CONFIG,
		ID_INTERFACE_LANG_TEXT, ID_INTERFACE_LANG,
		ID_INTERFACE_THEME,
		ID_FRAMELIMIT_TEXT, ID_FRAMELIMIT,

		ID_GC_SRAM_LNG_TEXT,
		ID_GC_SRAM_LNG,
		ID_GC_EXIDEVICE_SLOTA_TEXT,
		ID_GC_EXIDEVICE_SLOTA,
		ID_GC_EXIDEVICE_SLOTA_PATH,
		ID_GC_EXIDEVICE_SLOTB_TEXT,
		ID_GC_EXIDEVICE_SLOTB,
		ID_GC_EXIDEVICE_SLOTB_PATH,
		ID_GC_EXIDEVICE_SP1_TEXT,
		ID_GC_EXIDEVICE_SP1,
		ID_GC_SIDEVICE_TEXT,
		ID_GC_SIDEVICE0,
		ID_GC_SIDEVICE1,
		ID_GC_SIDEVICE2,
		ID_GC_SIDEVICE3,

		ID_WII_BT_BAR_TEXT,
		ID_WII_BT_BAR,
		ID_WII_IPL_SSV,
		ID_WII_IPL_PGS,
		ID_WII_IPL_E60,
		ID_WII_IPL_AR_TEXT,
		ID_WII_IPL_AR,
		ID_WII_IPL_LNG_TEXT,
		ID_WII_IPL_LNG,
		ID_WII_SD_CARD,
		ID_WII_KEYBOARD,

		ID_ISOPATHS,
		ID_ADDISOPATH,
		ID_REMOVEISOPATH,
		ID_RECERSIVEISOPATH,
		ID_DEFAULTISO_TEXT,
		ID_DEFAULTISO,
		ID_DVDROOT_TEXT,
		ID_DVDROOT,
		ID_APPLOADERPATH_TEXT,
		ID_APPLOADERPATH,

		ID_WIIMOTE_ABOUT,
		ID_WIIMOTE_CONFIG,
		ID_WIIMOTE_TEXT,
		ID_WIIMOTE_CB,
		ID_PAD_TEXT,
		ID_PAD_ABOUT ,
		ID_PAD_CONFIG,
		ID_PAD_CB,
		ID_DSP_ABOUT,
		ID_DSP_CONFIG,
		ID_DSP_TEXT,
		ID_DSP_CB,
		ID_GRAPHIC_ABOUT,
		ID_GRAPHIC_CONFIG,
		ID_GRAPHIC_TEXT,
		ID_GRAPHIC_CB
	};

	void InitializeGUILists();
	void InitializeGUIValues();
	void InitializeGUITooltips();

	void CreateGUIControls();
	void UpdateGUI();
	void OnClose(wxCloseEvent& event);
	void CoreSettingsChanged(wxCommandEvent& event);
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
};
#endif
