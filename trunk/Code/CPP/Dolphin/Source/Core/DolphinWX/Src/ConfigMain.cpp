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

#include <string> // System
#include <vector>

#include "Core.h" // Core
#include "HW/EXI.h"
#include "HW/SI.h"

#include "Globals.h" // Local
#include "ConfigMain.h"
#include "PluginManager.h"
#include "ConfigManager.h"
#include "SysConf.h"
#include "Frame.h"


extern CFrame* main_frame;

// Strings for Device Selections
#define DEV_NONE_STR		"<Nothing>"
#define DEV_DUMMY_STR		"Dummy"

#define SIDEV_STDCONT_STR	"Standard Controller"
#define SIDEV_GBA_STR		"GBA"
#define SIDEV_AM_BB_STR		"AM-Baseboard"

#define EXIDEV_MEMCARD_STR	"Memory Card"
#define EXIDEV_MIC_STR		"Mic"
#define EXIDEV_BBA_STR		"BBA"
#define EXIDEV_AM_BB_STR	"AM-Baseboard"


BEGIN_EVENT_TABLE(CConfigMain, wxDialog)

EVT_CLOSE(CConfigMain::OnClose)
EVT_BUTTON(wxID_CLOSE, CConfigMain::CloseClick)

EVT_CHECKBOX(ID_INTERFACE_CONFIRMSTOP, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_INTERFACE_USEPANICHANDLERS, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_INTERFACE_HIDECURSOR, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_INTERFACE_AUTOHIDECURSOR, CConfigMain::CoreSettingsChanged)
EVT_RADIOBOX(ID_INTERFACE_THEME, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_INTERFACE_WIIMOTE_LEDS, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_INTERFACE_WIIMOTE_SPEAKERS, CConfigMain::CoreSettingsChanged)
EVT_CHOICE(ID_INTERFACE_LANG, CConfigMain::CoreSettingsChanged)

EVT_CHECKBOX(ID_ALLWAYS_HLE_BS2, CConfigMain::CoreSettingsChanged)
EVT_RADIOBUTTON(ID_RADIOJIT, CConfigMain::CoreSettingsChanged)
EVT_RADIOBUTTON(ID_RADIOINT, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_CPUTHREAD, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_DSPTHREAD, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_LOCKTHREADS, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_OPTIMIZEQUANTIZERS, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_IDLESKIP, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_ENABLECHEATS, CConfigMain::CoreSettingsChanged)
EVT_CHOICE(ID_FRAMELIMIT, CConfigMain::CoreSettingsChanged)

EVT_CHOICE(ID_GC_SRAM_LNG, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_EXIDEVICE_SLOTA, CConfigMain::GCSettingsChanged)
EVT_BUTTON(ID_GC_EXIDEVICE_SLOTA_PATH, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_EXIDEVICE_SLOTB, CConfigMain::GCSettingsChanged)
EVT_BUTTON(ID_GC_EXIDEVICE_SLOTB_PATH, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_EXIDEVICE_SP1, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE0, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE1, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE2, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE3, CConfigMain::GCSettingsChanged)

EVT_CHOICE(ID_WII_BT_BAR, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_IPL_SSV, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_IPL_PGS, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_IPL_E60, CConfigMain::WiiSettingsChanged)
EVT_CHOICE(ID_WII_IPL_AR, CConfigMain::WiiSettingsChanged)
EVT_CHOICE(ID_WII_IPL_LNG, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_SD_CARD, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_KEYBOARD, CConfigMain::WiiSettingsChanged)

EVT_LISTBOX(ID_ISOPATHS, CConfigMain::ISOPathsSelectionChanged)
EVT_BUTTON(ID_ADDISOPATH, CConfigMain::AddRemoveISOPaths)
EVT_BUTTON(ID_REMOVEISOPATH, CConfigMain::AddRemoveISOPaths)
EVT_CHECKBOX(ID_RECERSIVEISOPATH, CConfigMain::RecursiveDirectoryChanged)
EVT_FILEPICKER_CHANGED(ID_DEFAULTISO, CConfigMain::DefaultISOChanged)
EVT_DIRPICKER_CHANGED(ID_DVDROOT, CConfigMain::DVDRootChanged)
EVT_FILEPICKER_CHANGED(ID_APPLOADERPATH, CConfigMain::ApploaderPathChanged)

EVT_CHOICE(ID_GRAPHIC_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_GRAPHIC_CONFIG, CConfigMain::OnConfig)
EVT_CHOICE(ID_DSP_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_DSP_CONFIG, CConfigMain::OnConfig)
EVT_CHOICE(ID_PAD_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_PAD_CONFIG, CConfigMain::OnConfig)
EVT_CHOICE(ID_WIIMOTE_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_WIIMOTE_CONFIG, CConfigMain::OnConfig)

END_EVENT_TABLE()

CConfigMain::CConfigMain(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	// Control refreshing of the ISOs list
	bRefreshList = false;

	CreateGUIControls();

	// Update selected ISO paths
	for(u32 i = 0; i < SConfig::GetInstance().m_ISOFolder.size(); i++)
	{
		ISOPaths->Append(wxString(SConfig::GetInstance().m_ISOFolder[i].c_str(), *wxConvCurrent));
	}
}

CConfigMain::~CConfigMain()
{
}


// Used to restrict changing of some options while emulator is running
void CConfigMain::UpdateGUI()
{
	if(Core::GetState() != Core::CORE_UNINITIALIZED)
	{
		// Disable the Core stuff on GeneralPage
		AlwaysHLE_BS2->Disable();
		m_RadioJIT->Disable();
		m_RadioInt->Disable();
		CPUThread->Disable();
		DSPThread->Disable();
		LockThreads->Disable();
		OptimizeQuantizers->Disable();
		SkipIdle->Disable();
		EnableCheats->Disable();
		
		GCSystemLang->Disable();
		
		WiiSensBarPos->Disable();
		WiiScreenSaver->Disable();
		WiiProgressiveScan->Disable();
		WiiEuRGB60->Disable();
		WiiAspectRatio->Disable();
		WiiSystemLang->Disable();

		PathsPage->Disable();

		GraphicSelection->Disable();
		DSPSelection->Disable();
		PADSelection->Disable();
		WiimoteSelection->Disable();
	}
}

void CConfigMain::CreateGUIControls()
{
	// Deal with all the language arrayStrings here
	// GC
	arrayStringFor_GCSystemLang.Add(wxT("Ӣ��"));
	arrayStringFor_GCSystemLang.Add(wxT("����"));
	arrayStringFor_GCSystemLang.Add(wxT("����"));
	arrayStringFor_GCSystemLang.Add(wxT("��������"));
	arrayStringFor_GCSystemLang.Add(wxT("�������"));
	arrayStringFor_GCSystemLang.Add(wxT("������"));
	// Wii
	arrayStringFor_WiiSystemLang = arrayStringFor_GCSystemLang;
	arrayStringFor_WiiSystemLang.Insert(wxT("����"), 0);
	arrayStringFor_WiiSystemLang.Add(wxT("��������"));
	arrayStringFor_WiiSystemLang.Add(wxT("��������"));
	arrayStringFor_WiiSystemLang.Add(wxT("����"));
	// GUI
	arrayStringFor_InterfaceLang = arrayStringFor_GCSystemLang;
	// Framelimit
	arrayStringFor_Framelimit.Add(wxT("�ر�"));
	arrayStringFor_Framelimit.Add(wxT("�Զ�"));
	for (int i = 20; i <= 120; i+=10)	// from 20 to 120
		arrayStringFor_Framelimit.Add(wxString::Format(wxT("%i"), i));
		
	// Create the notebook and pages
	Notebook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	GeneralPage = new wxPanel(Notebook, ID_GENERALPAGE, wxDefaultPosition, wxDefaultSize);
	GamecubePage = new wxPanel(Notebook, ID_GAMECUBEPAGE, wxDefaultPosition, wxDefaultSize);
	WiiPage = new wxPanel(Notebook, ID_WIIPAGE, wxDefaultPosition, wxDefaultSize);
	PathsPage = new wxPanel(Notebook, ID_PATHSPAGE, wxDefaultPosition, wxDefaultSize);
	PluginPage = new wxPanel(Notebook, ID_PLUGINPAGE, wxDefaultPosition, wxDefaultSize);

	Notebook->AddPage(GeneralPage, wxT("����"));
	Notebook->AddPage(GamecubePage, wxT("Gamecube"));
	Notebook->AddPage(WiiPage, wxT("Wii"));
	Notebook->AddPage(PathsPage, wxT("·��"));
	Notebook->AddPage(PluginPage, wxT("���"));


	
	// General page

	// Core Settings - Basic
	CPUThread = new wxCheckBox(GeneralPage, ID_CPUTHREAD, wxT("���ö�� (����)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	CPUThread->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread);
	SkipIdle = new wxCheckBox(GeneralPage, ID_IDLESKIP, wxT("�����ӳٲ��� (����)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	SkipIdle->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bSkipIdle);
	EnableCheats = new wxCheckBox(GeneralPage, ID_ENABLECHEATS, wxT("��������"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	EnableCheats->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bEnableCheats);
	
	// Framelimit
	wxStaticText *FramelimitText = new wxStaticText(GeneralPage, ID_FRAMELIMIT_TEXT, wxT("֡������ :"), wxDefaultPosition, wxDefaultSize);
	Framelimit = new wxChoice(GeneralPage, ID_FRAMELIMIT, wxDefaultPosition, wxDefaultSize, arrayStringFor_Framelimit, 0, wxDefaultValidator);
	Framelimit->SetToolTip(wxT("If you set Framelimit higher than game full speed (NTSC:60, PAL:50),\nyou also have to disable Audio Throttle in DSP to make it effective."));
	Framelimit->SetSelection(SConfig::GetInstance().m_Framelimit);

	// Core Settings - Advanced
	//
	AlwaysHLE_BS2 = new wxCheckBox(GeneralPage, ID_ALLWAYS_HLE_BS2, wxT("HLE the IPL (�Ƽ�)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	AlwaysHLE_BS2->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bHLE_BS2);
	m_RadioJIT = new wxRadioButton(GeneralPage, ID_RADIOJIT, wxT("JIT Recompiler (�Ƽ�)"));
	m_RadioInt = new wxRadioButton(GeneralPage, ID_RADIOINT, wxT("Interpreter (�ǳ���)"));
	SConfig::GetInstance().m_LocalCoreStartupParameter.bUseJIT ? m_RadioJIT->SetValue(true) : m_RadioInt->SetValue(true);
	LockThreads = new wxCheckBox(GeneralPage, ID_LOCKTHREADS, wxT("�����̵߳�����"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	LockThreads->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bLockThreads);
	OptimizeQuantizers = new wxCheckBox(GeneralPage, ID_OPTIMIZEQUANTIZERS, wxT("�Ż����� (����)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	OptimizeQuantizers->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bOptimizeQuantizers);
	DSPThread = new wxCheckBox(GeneralPage, ID_DSPTHREAD, wxT("DSP �����߳� (�Ƽ�)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	DSPThread->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bDSPThread);

	// Interface settings

	// Confirm on stop
	ConfirmStop = new wxCheckBox(GeneralPage, ID_INTERFACE_CONFIRMSTOP, wxT("ֹͣʱȷ��"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	ConfirmStop->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bConfirmStop);

	// Use Panic Handlers
	UsePanicHandlers = new wxCheckBox(GeneralPage, ID_INTERFACE_USEPANICHANDLERS, wxT("ʹ�� Panic Handlers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	UsePanicHandlers->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bUsePanicHandlers);

	// Hide Cursor
	wxStaticText *HideCursorText = new wxStaticText(GeneralPage, ID_INTERFACE_HIDECURSOR_TEXT, wxT("���ع��:"), wxDefaultPosition, wxDefaultSize);
	AutoHideCursor = new wxCheckBox(GeneralPage, ID_INTERFACE_AUTOHIDECURSOR, wxT("�Զ�"));
	AutoHideCursor->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bAutoHideCursor);
	HideCursor = new wxCheckBox(GeneralPage, ID_INTERFACE_HIDECURSOR, wxT("����"));
	HideCursor->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor);
	// Wiimote status in statusbar
	wxStaticText *WiimoteStatusText = new wxStaticText(GeneralPage, ID_INTERFACE_WIIMOTE_TEXT, wxT("��ʾ wiimote ״̬:"), wxDefaultPosition, wxDefaultSize);
	WiimoteStatusLEDs = new wxCheckBox(GeneralPage, ID_INTERFACE_WIIMOTE_LEDS, wxT("LEDs"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiimoteStatusLEDs->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bWiiLeds);
	WiimoteStatusSpeakers = new wxCheckBox(GeneralPage, ID_INTERFACE_WIIMOTE_SPEAKERS, wxT("������"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiimoteStatusSpeakers->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bWiiSpeakers);

	// Interface Language
	// At the moment this only changes the language displayed in m_gamelistctrl
	// If someone wants to control the whole GUI's language, it should be set here too
	wxStaticText *InterfaceLangText = new wxStaticText(GeneralPage, ID_INTERFACE_LANG_TEXT, wxT("��Ϸ�б�����:"), wxDefaultPosition, wxDefaultSize);
	InterfaceLang = new wxChoice(GeneralPage, ID_INTERFACE_LANG, wxDefaultPosition, wxDefaultSize, arrayStringFor_InterfaceLang, 0, wxDefaultValidator);
	// need redesign
	InterfaceLang->SetSelection(SConfig::GetInstance().m_InterfaceLanguage);

	// Themes - this should really be a wxChoice...
	wxArrayString ThemeChoices;
	ThemeChoices.Add(wxT("Boomy"));
	ThemeChoices.Add(wxT("Vista"));
	ThemeChoices.Add(wxT("X-Plastik"));
	ThemeChoices.Add(wxT("KDE"));
	Theme = new wxRadioBox(GeneralPage, ID_INTERFACE_THEME, wxT("����"),wxDefaultPosition, wxDefaultSize, ThemeChoices, 1, wxRA_SPECIFY_ROWS);
	// Set selected
	Theme->SetSelection(SConfig::GetInstance().m_LocalCoreStartupParameter.iTheme);

	// ToolTips
	ConfirmStop->SetToolTip(wxT("Show a confirmation box before stopping a game."));
	UsePanicHandlers->SetToolTip(wxT("Show a message box when a potentially serious error has occured.")
		wxT(" Disabling this may avoid annoying and non-fatal messages, but it may also mean that Dolphin")
		wxT(" suddenly crashes without any explanation at all."));
	AutoHideCursor->SetToolTip(wxT("This will auto hide the cursor in fullscreen mode."));
	HideCursor->SetToolTip(wxT("This will always hide the cursor when it's over the rendering window.")
		wxT("\nIt can be convenient in a Wii game that already has a cursor."));
	WiimoteStatusLEDs->SetToolTip(wxT("Show which wiimotes are connected in the statusbar."));
	WiimoteStatusSpeakers->SetToolTip(wxT("Show wiimote speaker status in the statusbar."));
	DSPThread->SetToolTip(wxT("Run DSPLLE on a dedicate thread, this has no effects on DSPHLE."));
	CPUThread->SetToolTip(wxT("This splits the Video and CPU threads, so they can be run on separate cores.")
		wxT("\nCauses major speed improvements on PCs with more than one core,")
		wxT("\nbut can also cause occasional crashes/glitches."));

	InterfaceLang->SetToolTip(wxT("For the time being this will only change the text shown in")
		wxT("\nthe game list of PAL GC games."));

	// Copyright notice
	Theme->SetItemToolTip(0, wxT("������ Milosz Wlazlo [miloszwl@miloszwl.com, miloszwl.deviantart.com]"));
	Theme->SetItemToolTip(1, wxT("������ VistaIcons.com"));
	Theme->SetItemToolTip(2, wxT("Created by black_rider and published on ForumW.org > Web Developments"));
	Theme->SetItemToolTip(3, wxT("������ KDE-Look.org"));

	// Populate the settings
	sCore = new wxBoxSizer(wxHORIZONTAL);
	sbBasic = new wxStaticBoxSizer(wxVERTICAL, GeneralPage, wxT("��������"));
	sbBasic->Add(CPUThread, 0, wxALL, 5);
	sbBasic->Add(SkipIdle, 0, wxALL, 5);
	sbBasic->Add(EnableCheats, 0, wxALL, 5);
	wxBoxSizer *sFramelimit = new wxBoxSizer(wxHORIZONTAL);
	sFramelimit->Add(FramelimitText, 0, wxALL | wxALIGN_CENTER, 1);
	sFramelimit->Add(Framelimit, 0, wxALL | wxEXPAND, 5);
	sbBasic->Add(sFramelimit, 0, wxALL | wxEXPAND, 5);

	sbAdvanced = new wxStaticBoxSizer(wxVERTICAL, GeneralPage, wxT("�߼�����"));
	sbAdvanced->Add(AlwaysHLE_BS2, 0, wxALL, 5);
	wxStaticBoxSizer* sizerCoreType = new wxStaticBoxSizer(wxVERTICAL, GeneralPage, wxT("CPU ģ������"));
	sizerCoreType->Add(m_RadioJIT, 0, wxALL | wxEXPAND, 5);
	sizerCoreType->Add(m_RadioInt, 0, wxALL | wxEXPAND, 5);
	sbAdvanced->Add(sizerCoreType, 0, wxALL, 5);
	sbAdvanced->Add(LockThreads, 0, wxALL, 5);
	sbAdvanced->Add(OptimizeQuantizers, 0, wxALL, 5);
	sbAdvanced->Add(DSPThread, 0, wxALL, 5);
	sCore->Add(sbBasic, 0, wxEXPAND);
	sCore->AddStretchSpacer();
	sCore->Add(sbAdvanced, 0, wxEXPAND);

	sbInterface = new wxStaticBoxSizer(wxVERTICAL, GeneralPage, wxT("��������"));
	sbInterface->Add(ConfirmStop, 0, wxALL, 5);
	sbInterface->Add(UsePanicHandlers, 0, wxALL, 5);
	wxBoxSizer *sHideCursor = new wxBoxSizer(wxHORIZONTAL);
	sHideCursor->Add(HideCursorText);
	sHideCursor->Add(AutoHideCursor, 0, wxLEFT, 5);
	sHideCursor->Add(HideCursor, 0, wxLEFT, 5);
	sbInterface->Add(sHideCursor, 0, wxALL, 5);
	wxBoxSizer *sWiimoteStatus = new wxBoxSizer(wxHORIZONTAL);
	sWiimoteStatus->Add(WiimoteStatusText);
	sWiimoteStatus->Add(WiimoteStatusLEDs, 0, wxLEFT, 5);
	sWiimoteStatus->Add(WiimoteStatusSpeakers, 0, wxLEFT, 5);
	sbInterface->Add(sWiimoteStatus, 0, wxALL, 5);
	sbInterface->Add(Theme, 0, wxEXPAND | wxALL, 5);
	wxBoxSizer *sInterfaceLanguage = new wxBoxSizer(wxHORIZONTAL);
	sInterfaceLanguage->Add(InterfaceLangText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	sInterfaceLanguage->Add(InterfaceLang, 0, wxEXPAND | wxALL, 5);
	sbInterface->Add(sInterfaceLanguage, 0, wxEXPAND | wxALL, 5);

	// Populate the entire page
	sGeneralPage = new wxBoxSizer(wxVERTICAL);
	sGeneralPage->Add(sCore, 0, wxEXPAND | wxALL, 5);
	sGeneralPage->Add(sbInterface, 0, wxEXPAND | wxALL, 5);

	GeneralPage->SetSizer(sGeneralPage);
	sGeneralPage->Layout();


	
	// Gamecube page
	// IPL settings
	sbGamecubeIPLSettings = new wxStaticBoxSizer(wxVERTICAL, GamecubePage, wxT("IPL ����"));
	GCSystemLangText = new wxStaticText(GamecubePage, ID_GC_SRAM_LNG_TEXT, wxT("ϵͳ����:"), wxDefaultPosition, wxDefaultSize);
	GCSystemLang = new wxChoice(GamecubePage, ID_GC_SRAM_LNG, wxDefaultPosition, wxDefaultSize, arrayStringFor_GCSystemLang, 0, wxDefaultValidator);
	GCSystemLang->SetSelection(SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage);
	// Devices
	wxStaticBoxSizer *sbGamecubeDeviceSettings = new wxStaticBoxSizer(wxVERTICAL, GamecubePage, wxT("�豸����"));
	// EXI Devices
	wxStaticText *GCEXIDeviceText[3];
	GCEXIDeviceText[0] = new wxStaticText(GamecubePage, ID_GC_EXIDEVICE_SLOTA_TEXT, wxT("��� A"), wxDefaultPosition, wxDefaultSize);
	GCEXIDeviceText[1] = new wxStaticText(GamecubePage, ID_GC_EXIDEVICE_SLOTB_TEXT, wxT("��� B"), wxDefaultPosition, wxDefaultSize);
	GCEXIDeviceText[2] = new wxStaticText(GamecubePage, ID_GC_EXIDEVICE_SP1_TEXT,	wxT("SP1   "), wxDefaultPosition, wxDefaultSize);
	GCEXIDeviceText[2]->SetToolTip(wxT("Serial Port 1 - This is the port which devices such as the net adapter use"));
	const wxString SlotDevices[] = {wxT(DEV_NONE_STR),wxT(DEV_DUMMY_STR),wxT(EXIDEV_MEMCARD_STR), wxT(EXIDEV_MIC_STR)};
	static const int numSlotDevices = sizeof(SlotDevices)/sizeof(wxString);
	const wxString SP1Devices[] = {wxT(DEV_NONE_STR),wxT(DEV_DUMMY_STR),wxT(EXIDEV_BBA_STR),wxT(EXIDEV_AM_BB_STR)};
	static const int numSP1Devices = sizeof(SP1Devices)/sizeof(wxString);
	GCEXIDevice[0] = new wxChoice(GamecubePage, ID_GC_EXIDEVICE_SLOTA, wxDefaultPosition, wxDefaultSize, numSlotDevices, SlotDevices, 0, wxDefaultValidator);
	GCEXIDevice[1] = new wxChoice(GamecubePage, ID_GC_EXIDEVICE_SLOTB, wxDefaultPosition, wxDefaultSize, numSlotDevices, SlotDevices, 0, wxDefaultValidator);
	GCEXIDevice[2] = new wxChoice(GamecubePage, ID_GC_EXIDEVICE_SP1, wxDefaultPosition, wxDefaultSize, numSP1Devices, SP1Devices, 0, wxDefaultValidator);
	GCMemcardPath[0] = new wxButton(GamecubePage, ID_GC_EXIDEVICE_SLOTA_PATH, wxT("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator);
	GCMemcardPath[1] = new wxButton(GamecubePage, ID_GC_EXIDEVICE_SLOTB_PATH, wxT("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator);
	for (int i = 0; i < 3; ++i)
	{
		bool isMemcard = false;
		switch (SConfig::GetInstance().m_EXIDevice[i])
		{
		case EXIDEVICE_NONE:
			GCEXIDevice[i]->SetStringSelection(SlotDevices[0]);
			break;
		case EXIDEVICE_MEMORYCARD_A:
		case EXIDEVICE_MEMORYCARD_B:
			isMemcard = GCEXIDevice[i]->SetStringSelection(SlotDevices[2]);
			break;
		case EXIDEVICE_MIC:
			GCEXIDevice[i]->SetStringSelection(SlotDevices[3]);
			break;
		case EXIDEVICE_ETH:
			GCEXIDevice[i]->SetStringSelection(SP1Devices[2]);
			break;
		case EXIDEVICE_AM_BASEBOARD:
			GCEXIDevice[i]->SetStringSelection(SP1Devices[3]);
			break;
		case EXIDEVICE_DUMMY:
		default:
			GCEXIDevice[i]->SetStringSelection(SlotDevices[1]);
			break;
		}
		if (!isMemcard && i < 2)
			GCMemcardPath[i]->Disable();
	}
	//SI Devices
	wxStaticText *GCSIDeviceText[4];
	GCSIDeviceText[0] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("�˿� 1"), wxDefaultPosition, wxDefaultSize);
	GCSIDeviceText[1] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("�˿� 2"), wxDefaultPosition, wxDefaultSize);
	GCSIDeviceText[2] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("�˿� 3"), wxDefaultPosition, wxDefaultSize);
	GCSIDeviceText[3] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("�˿� 4"), wxDefaultPosition, wxDefaultSize);
	const wxString SIDevices[] = {wxT(DEV_NONE_STR),wxT(SIDEV_STDCONT_STR),wxT(SIDEV_GBA_STR),wxT(SIDEV_AM_BB_STR)};
	static const int numSIDevices = sizeof(SIDevices)/sizeof(wxString);
	GCSIDevice[0] = new wxChoice(GamecubePage, ID_GC_SIDEVICE0, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	GCSIDevice[1] = new wxChoice(GamecubePage, ID_GC_SIDEVICE1, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	GCSIDevice[2] = new wxChoice(GamecubePage, ID_GC_SIDEVICE2, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	GCSIDevice[3] = new wxChoice(GamecubePage, ID_GC_SIDEVICE3, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	for (int i = 0; i < 4; ++i)
	{
		switch (SConfig::GetInstance().m_SIDevice[i])
		{
		case SI_GC_CONTROLLER:
			GCSIDevice[i]->SetStringSelection(SIDevices[1]);
			break;
		case SI_GBA:
			GCSIDevice[i]->SetStringSelection(SIDevices[2]);
			break;
		case SI_AM_BASEBOARD:
			GCSIDevice[i]->SetStringSelection(SIDevices[3]);
			break;
		default:
			GCSIDevice[i]->SetStringSelection(SIDevices[0]);
			break;
		}
	}

	sGamecube = new wxBoxSizer(wxVERTICAL);
	sGamecubeIPLSettings = new wxGridBagSizer(0, 0);
	sGamecubeIPLSettings->Add(GCSystemLangText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sGamecubeIPLSettings->Add(GCSystemLang, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL, 5);
	sbGamecubeIPLSettings->Add(sGamecubeIPLSettings);
	sGamecube->Add(sbGamecubeIPLSettings, 0, wxEXPAND|wxALL, 5);
	wxBoxSizer *sEXIDevices[4];
	wxBoxSizer *sSIDevices[4];
	for (int i = 0; i < 3; ++i)
	{
		sEXIDevices[i] = new wxBoxSizer(wxHORIZONTAL);
		sEXIDevices[i]->Add(GCEXIDeviceText[i], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		sEXIDevices[i]->Add(GCEXIDevice[i], 0, wxALL, 5);
		if (i < 2)
			sEXIDevices[i]->Add(GCMemcardPath[i], 0, wxALL, 5);
		sbGamecubeDeviceSettings->Add(sEXIDevices[i]);
	}
	for (int i = 0; i < 4; ++i)
	{
		sSIDevices[i] = new wxBoxSizer(wxHORIZONTAL);
		sSIDevices[i]->Add(GCSIDeviceText[i], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		sSIDevices[i]->Add(GCSIDevice[i], 0, wxALL, 5);
		sbGamecubeDeviceSettings->Add(sSIDevices[i]);
	}
	sGamecube->Add(sbGamecubeDeviceSettings, 0, wxEXPAND|wxALL, 5);
	GamecubePage->SetSizer(sGamecube);
	sGamecube->Layout();

	// Wii page
	sbWiimoteSettings = new wxStaticBoxSizer(wxVERTICAL, WiiPage, wxT("Wiimote ����"));
	arrayStringFor_WiiSensBarPos.Add(wxT("�ײ�")); arrayStringFor_WiiSensBarPos.Add(wxT("����"));
	WiiSensBarPosText = new wxStaticText(WiiPage, ID_WII_BT_BAR_TEXT, wxT("Sensor Bar Position:"), wxDefaultPosition, wxDefaultSize);
	WiiSensBarPos = new wxChoice(WiiPage, ID_WII_BT_BAR, wxDefaultPosition, wxDefaultSize, arrayStringFor_WiiSensBarPos, 0, wxDefaultValidator);
	WiiSensBarPos->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("BT.BAR"));

	sbWiiIPLSettings = new wxStaticBoxSizer(wxVERTICAL, WiiPage, wxT("��������"));
	WiiScreenSaver = new wxCheckBox(WiiPage, ID_WII_IPL_SSV, wxT("������Ļ���� (burn-in reduction)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiScreenSaver->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.SSV"));
	WiiProgressiveScan = new wxCheckBox(WiiPage, ID_WII_IPL_PGS, wxT("���� Progressive Scan"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiProgressiveScan->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.PGS"));
	WiiEuRGB60 = new wxCheckBox(WiiPage, ID_WII_IPL_E60, wxT("ʹ�� EuRGB60 ģʽ (PAL60)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiEuRGB60->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.E60"));
	arrayStringFor_WiiAspectRatio.Add(wxT("4:3")); arrayStringFor_WiiAspectRatio.Add(wxT("16:9"));
	WiiAspectRatioText = new wxStaticText(WiiPage, ID_WII_IPL_AR_TEXT, wxT("�߿���:"), wxDefaultPosition, wxDefaultSize);
	WiiAspectRatio = new wxChoice(WiiPage, ID_WII_IPL_AR, wxDefaultPosition, wxDefaultSize, arrayStringFor_WiiAspectRatio, 0, wxDefaultValidator);
	WiiAspectRatio->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.AR"));
	WiiSystemLangText = new wxStaticText(WiiPage, ID_WII_IPL_LNG_TEXT, wxT("ϵͳ����:"), wxDefaultPosition, wxDefaultSize);
	WiiSystemLang = new wxChoice(WiiPage, ID_WII_IPL_LNG, wxDefaultPosition, wxDefaultSize, arrayStringFor_WiiSystemLang, 0, wxDefaultValidator);
	WiiSystemLang->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.LNG"));

	// Devices
	sbWiiDeviceSettings = new wxStaticBoxSizer(wxVERTICAL, WiiPage, wxT("Device Settings"));

	WiiSDCard = new wxCheckBox(WiiPage, ID_WII_SD_CARD, wxT("Insert SD Card"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiSDCard->SetValue(SConfig::GetInstance().m_WiiSDCard);
	WiiKeyboard = new wxCheckBox(WiiPage, ID_WII_KEYBOARD, wxT("Connect USB Keyboard"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiKeyboard->SetToolTip(wxT("This could cause slow down in Wii Menu and some games."));
	WiiKeyboard->SetValue(SConfig::GetInstance().m_WiiKeyboard);

	// Populate sbWiimoteSettings
	sWii = new wxBoxSizer(wxVERTICAL);
	sWiimoteSettings = new wxGridBagSizer(0, 0);
	sWiimoteSettings->Add(WiiSensBarPosText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sWiimoteSettings->Add(WiiSensBarPos, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL, 5);
	sbWiimoteSettings->Add(sWiimoteSettings);
	sWii->Add(sbWiimoteSettings, 0, wxEXPAND|wxALL, 5);

	sWiiIPLSettings = new wxGridBagSizer(0, 0);
	sWiiIPLSettings->Add(WiiScreenSaver, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL, 5);
	sWiiIPLSettings->Add(WiiProgressiveScan, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL, 5);
	sWiiIPLSettings->Add(WiiEuRGB60, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL, 5);
	sWiiIPLSettings->Add(WiiAspectRatioText, wxGBPosition(3, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sWiiIPLSettings->Add(WiiAspectRatio, wxGBPosition(3, 1), wxGBSpan(1, 1), wxALL, 5);
	sWiiIPLSettings->Add(WiiSystemLangText, wxGBPosition(4, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sWiiIPLSettings->Add(WiiSystemLang, wxGBPosition(4, 1), wxGBSpan(1, 1), wxALL, 5);
	sbWiiIPLSettings->Add(sWiiIPLSettings);
	sWii->Add(sbWiiIPLSettings, 0, wxEXPAND|wxALL, 5);

	sbWiiDeviceSettings->Add(WiiSDCard, 0, wxALL, 5);
	sbWiiDeviceSettings->Add(WiiKeyboard, 0, wxALL, 5);
	sWii->Add(sbWiiDeviceSettings, 0, wxEXPAND|wxALL, 5);
	WiiPage->SetSizer(sWii);
	sWii->Layout();


	
	// Paths page
	sbISOPaths = new wxStaticBoxSizer(wxVERTICAL, PathsPage, wxT("ISO Ŀ¼"));
	ISOPaths = new wxListBox(PathsPage, ID_ISOPATHS, wxDefaultPosition, wxDefaultSize, arrayStringFor_ISOPaths, wxLB_SINGLE, wxDefaultValidator);
	AddISOPath = new wxButton(PathsPage, ID_ADDISOPATH, wxT("����..."), wxDefaultPosition, wxDefaultSize, 0);
	RemoveISOPath = new wxButton(PathsPage, ID_REMOVEISOPATH, wxT("�Ƴ�"), wxDefaultPosition, wxDefaultSize, 0);
	RemoveISOPath->Enable(false);
	RecersiveISOPath = new wxCheckBox(PathsPage, ID_RECERSIVEISOPATH, wxT("������Ŀ¼"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	RecersiveISOPath->SetValue(SConfig::GetInstance().m_RecursiveISOFolder);
	DefaultISOText = new wxStaticText(PathsPage, ID_DEFAULTISO_TEXT, wxT("Ĭ�� ISO:"), wxDefaultPosition, wxDefaultSize);
	DefaultISO = new wxFilePickerCtrl(PathsPage, ID_DEFAULTISO, wxEmptyString, wxT("ѡ��һ��Ĭ�� ISO:"),
		wxString::Format(wxT("���� GC/Wii ���� (gcm, iso, gcz)|*.gcm;*.iso;*.gcz|�����ļ� (%s)|%s"), wxFileSelectorDefaultWildcardStr, wxFileSelectorDefaultWildcardStr),
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL|wxFLP_OPEN);
	DefaultISO->SetPath(wxString(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDefaultGCM.c_str(), *wxConvCurrent));

	DVDRootText = new wxStaticText(PathsPage, ID_DVDROOT_TEXT, wxT("DVD ��Ŀ¼:"), wxDefaultPosition, wxDefaultSize);
	DVDRoot = new wxDirPickerCtrl(PathsPage, ID_DVDROOT, wxEmptyString, wxT("ѡ��һ�� DVD ��Ŀ¼:"), wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL);
	DVDRoot->SetPath(wxString(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDVDRoot.c_str(), *wxConvCurrent));
	ApploaderPathText = new wxStaticText(PathsPage, ID_APPLOADERPATH_TEXT, wxT("Apploader:"), wxDefaultPosition, wxDefaultSize);
	ApploaderPath = new wxFilePickerCtrl(PathsPage, ID_APPLOADERPATH, wxEmptyString, wxT("Choose file to use as apploader: (applies to discs constructed from directories only)"),
		wxString::Format(wxT("apploader (.img)|*.img|�����ļ� (%s)|%s"), wxFileSelectorDefaultWildcardStr, wxFileSelectorDefaultWildcardStr),
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL|wxFLP_OPEN);
	ApploaderPath->SetPath(wxString(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strApploader.c_str(), *wxConvCurrent));

	sPaths = new wxBoxSizer(wxVERTICAL);

	sbISOPaths->Add(ISOPaths, 1, wxEXPAND|wxALL, 0);

	sISOButtons = new wxBoxSizer(wxHORIZONTAL);
	sISOButtons->Add(RecersiveISOPath, 0, wxALL|wxALIGN_CENTER, 0);
	sISOButtons->AddStretchSpacer(1);
	sISOButtons->Add(AddISOPath, 0, wxALL, 0);
	sISOButtons->Add(RemoveISOPath, 0, wxALL, 0);
	sbISOPaths->Add(sISOButtons, 0, wxEXPAND|wxALL, 5);
	sPaths->Add(sbISOPaths, 1, wxEXPAND|wxALL, 5);

	sOtherPaths = new wxGridBagSizer(0, 0);
	sOtherPaths->AddGrowableCol(1);
	sOtherPaths->Add(DefaultISOText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sOtherPaths->Add(DefaultISO, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sOtherPaths->Add(DVDRootText, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sOtherPaths->Add(DVDRoot, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sOtherPaths->Add(ApploaderPathText, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sOtherPaths->Add(ApploaderPath, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sPaths->Add(sOtherPaths, 0, wxEXPAND|wxALL, 5);
	PathsPage->SetSizer(sPaths);
	sPaths->Layout();

	
	// Plugins page
	sbGraphicsPlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginPage, wxT("ͼ��"));
	GraphicSelection = new wxChoice(PluginPage, ID_GRAPHIC_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	GraphicConfig = new wxButton(PluginPage, ID_GRAPHIC_CONFIG, wxT("����..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	sbDSPPlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginPage, wxT("DSP��Ƶ"));
	DSPSelection = new wxChoice(PluginPage, ID_DSP_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	DSPConfig = new wxButton(PluginPage, ID_DSP_CONFIG, wxT("����..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	sbPadPlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginPage, wxT("�ֱ�"));
	PADSelection = new wxChoice(PluginPage, ID_PAD_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	PADConfig = new wxButton(PluginPage, ID_PAD_CONFIG, wxT("����..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	sbWiimotePlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginPage, wxT("Wiimote"));
	WiimoteSelection = new wxChoice(PluginPage, ID_WIIMOTE_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	WiimoteConfig = new wxButton(PluginPage, ID_WIIMOTE_CONFIG, wxT("����..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	FillChoiceBox(GraphicSelection, PLUGIN_TYPE_VIDEO, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strVideoPlugin);
	FillChoiceBox(DSPSelection, PLUGIN_TYPE_DSP, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDSPPlugin);
	for (int i = 0; i < MAXPADS; i++)
	    FillChoiceBox(PADSelection, PLUGIN_TYPE_PAD, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strPadPlugin[i]);

	for (int i=0; i < MAXWIIMOTES; i++)
	    FillChoiceBox(WiimoteSelection, PLUGIN_TYPE_WIIMOTE, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strWiimotePlugin[i]);

	sPlugins = new wxBoxSizer(wxVERTICAL);
	sbGraphicsPlugin->Add(GraphicSelection, 1, wxEXPAND|wxALL, 5);
	sbGraphicsPlugin->Add(GraphicConfig, 0, wxALL, 5);
	sPlugins->Add(sbGraphicsPlugin, 0, wxEXPAND|wxALL, 5);

	sbDSPPlugin->Add(DSPSelection, 1, wxEXPAND|wxALL, 5);
	sbDSPPlugin->Add(DSPConfig, 0, wxALL, 5);
	sPlugins->Add(sbDSPPlugin, 0, wxEXPAND|wxALL, 5);

	sbPadPlugin->Add(PADSelection, 1, wxEXPAND|wxALL, 5);
	sbPadPlugin->Add(PADConfig, 0, wxALL, 5);
	sPlugins->Add(sbPadPlugin, 0, wxEXPAND|wxALL, 5);

	sbWiimotePlugin->Add(WiimoteSelection, 1, wxEXPAND|wxALL, 5);
	sbWiimotePlugin->Add(WiimoteConfig, 0, wxALL, 5);
	sPlugins->Add(sbWiimotePlugin, 0, wxEXPAND|wxALL, 5);
	PluginPage->SetSizer(sPlugins);
	sPlugins->Layout();



	m_Close = new wxButton(this, wxID_CLOSE);

	wxBoxSizer* sButtons = new wxBoxSizer(wxHORIZONTAL);
	sButtons->Add(0, 0, 1, wxEXPAND, 5);
	sButtons->Add(m_Close, 0, wxALL, 5);

	wxBoxSizer* sMain = new wxBoxSizer(wxVERTICAL);
	sMain->Add(Notebook, 1, wxEXPAND|wxALL, 5);
	sMain->Add(sButtons, 0, wxEXPAND, 5);

	UpdateGUI();

	this->SetSizer(sMain);
	this->Layout();

	Fit();
	Center();
}

void CConfigMain::OnClose(wxCloseEvent& WXUNUSED (event))
{
	EndModal((bRefreshList) ? wxID_OK : wxID_CLOSE);

	// Sysconf saves when it gets deleted
	//delete SConfig::GetInstance().m_SYSCONF;

	// Save the config. Dolphin crashes to often to save the settings on closing only
	SConfig::GetInstance().SaveSettings();
}

void CConfigMain::CloseClick(wxCommandEvent& WXUNUSED (event))
{
	Close();
}


// Core AND Interface settings
void CConfigMain::CoreSettingsChanged(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	case ID_INTERFACE_CONFIRMSTOP: // Interface
		SConfig::GetInstance().m_LocalCoreStartupParameter.bConfirmStop = ConfirmStop->IsChecked();
		break;
	case ID_INTERFACE_USEPANICHANDLERS: // Interface
		SConfig::GetInstance().m_LocalCoreStartupParameter.bUsePanicHandlers = UsePanicHandlers->IsChecked();
		SetEnableAlert(UsePanicHandlers->IsChecked());
		break;
	case ID_INTERFACE_AUTOHIDECURSOR:
		if (AutoHideCursor->IsChecked()) HideCursor->SetValue(!AutoHideCursor->IsChecked()); // Update the other one
		SConfig::GetInstance().m_LocalCoreStartupParameter.bAutoHideCursor = AutoHideCursor->IsChecked();		
		SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor = HideCursor->IsChecked();		
		break;
	case ID_INTERFACE_HIDECURSOR:
		if (HideCursor->IsChecked()) AutoHideCursor->SetValue(!HideCursor->IsChecked()); // Update the other one
		SConfig::GetInstance().m_LocalCoreStartupParameter.bAutoHideCursor = AutoHideCursor->IsChecked();		
		SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor = HideCursor->IsChecked();
		break;
	case ID_INTERFACE_THEME:
		SConfig::GetInstance().m_LocalCoreStartupParameter.iTheme = Theme->GetSelection();
		main_frame->InitBitmaps();
		break;
	case ID_INTERFACE_WIIMOTE_LEDS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bWiiLeds = WiimoteStatusLEDs->IsChecked();
		break;
	case ID_INTERFACE_WIIMOTE_SPEAKERS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bWiiSpeakers = WiimoteStatusSpeakers->IsChecked();
		break;
	case ID_INTERFACE_LANG:
		SConfig::GetInstance().m_InterfaceLanguage = (INTERFACE_LANGUAGE)InterfaceLang->GetSelection();
		bRefreshList = true;
		break;
	case ID_FRAMELIMIT:
		SConfig::GetInstance().m_Framelimit = (u32)Framelimit->GetSelection();
		break;
	case ID_ALLWAYS_HLE_BS2: // Core
		SConfig::GetInstance().m_LocalCoreStartupParameter.bHLE_BS2 = AlwaysHLE_BS2->IsChecked();
		break;
	case ID_RADIOJIT:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bUseJIT = true;
		if (main_frame->g_pCodeWindow) main_frame->g_pCodeWindow->GetMenuBar()->Check(IDM_INTERPRETER, false);
		break;
	case ID_RADIOINT:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bUseJIT = false;
		if (main_frame->g_pCodeWindow) main_frame->g_pCodeWindow->GetMenuBar()->Check(IDM_INTERPRETER, true);
		break;
	case ID_CPUTHREAD:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread = CPUThread->IsChecked();
		break;
	case ID_DSPTHREAD:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bDSPThread = DSPThread->IsChecked();
		break;
	case ID_LOCKTHREADS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bLockThreads = LockThreads->IsChecked();
		break;
	case ID_OPTIMIZEQUANTIZERS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bOptimizeQuantizers = OptimizeQuantizers->IsChecked();
		break;
	case ID_IDLESKIP:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bSkipIdle = SkipIdle->IsChecked();
		break;
	case ID_ENABLECHEATS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bEnableCheats = EnableCheats->IsChecked();
		break;
	}
}


// GC settings
// -----------------------
void CConfigMain::GCSettingsChanged(wxCommandEvent& event)
{
	int sidevice = 0;
	int exidevice = 0;
	switch (event.GetId())
	{
	case ID_GC_SRAM_LNG:
		SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage = GCSystemLang->GetSelection();
		break;

	case ID_GC_EXIDEVICE_SP1:
		exidevice++;
	case ID_GC_EXIDEVICE_SLOTB:
		exidevice++;
 	case ID_GC_EXIDEVICE_SLOTA:
		ChooseEXIDevice(std::string(event.GetString().mb_str()), exidevice);
 		break;
	case ID_GC_EXIDEVICE_SLOTA_PATH:
		ChooseMemcardPath(SConfig::GetInstance().m_strMemoryCardA, true);
		break;
	case ID_GC_EXIDEVICE_SLOTB_PATH:
		ChooseMemcardPath(SConfig::GetInstance().m_strMemoryCardB, false);
		break;

 	case ID_GC_SIDEVICE3:
		sidevice++;
 	case ID_GC_SIDEVICE2:
		sidevice++;
 	case ID_GC_SIDEVICE1:
 		sidevice++;
 	case ID_GC_SIDEVICE0:
		ChooseSIDevice(std::string(event.GetString().mb_str()), sidevice);
 		break;
	}
}

void CConfigMain::ChooseMemcardPath(std::string& strMemcard, bool isSlotA)
{
	std::string filename = std::string(wxFileSelector(
		wxT("ѡ����Ҫ�򿪵��ļ�"),
		T_FULL_GC_USER_DIR,
		isSlotA ? wxT(GC_MEMCARDA) : wxT(GC_MEMCARDB), 
		wxEmptyString,
		wxT("Gamecube �ڴ濨 (*.raw,*.gcp)|*.raw;*.gcp")).mb_str());

	if (!filename.empty())
	{
		// also check that the path isn't used for the other memcard...
		if (filename.compare(isSlotA ? SConfig::GetInstance().m_strMemoryCardB
		: SConfig::GetInstance().m_strMemoryCardA) != 0)
		{
			strMemcard = filename;

			if (Core::GetState() != Core::CORE_UNINITIALIZED)
			{
				// Change memcard to the new file
				ExpansionInterface::ChangeDevice(
					isSlotA ? 0 : 1, // SlotA: channel 0, SlotB channel 1
					isSlotA ? EXIDEVICE_MEMORYCARD_A : EXIDEVICE_MEMORYCARD_B,
					0);	// SP1 is device 2, slots are device 0
			}
		}
		else
		{
			PanicAlert("Cannot use that file as a memory card.\n"
				"Are you trying to use the same file in both slots?");
		}
	}
}

void CConfigMain::ChooseSIDevice(std::string deviceName, int deviceNum)
{
	TSIDevices tempType;
	if (!deviceName.compare(SIDEV_STDCONT_STR))
		tempType = SI_GC_CONTROLLER;
	else if (!deviceName.compare(SIDEV_GBA_STR))
		tempType = SI_GBA;
	else if (!deviceName.compare(SIDEV_AM_BB_STR))
		tempType = SI_AM_BASEBOARD;
	else
		tempType = SI_NONE;

	SConfig::GetInstance().m_SIDevice[deviceNum] = tempType;

	if (Core::GetState() != Core::CORE_UNINITIALIZED)
	{
		// Change plugged device! :D
		SerialInterface::ChangeDevice(tempType, deviceNum);
	}
}

void CConfigMain::ChooseEXIDevice(std::string deviceName, int deviceNum)
{
	TEXIDevices tempType;

	if (!deviceName.compare(EXIDEV_MEMCARD_STR))
		tempType = deviceNum ? EXIDEVICE_MEMORYCARD_B : EXIDEVICE_MEMORYCARD_A;
	else if (!deviceName.compare(EXIDEV_MIC_STR))
		tempType = EXIDEVICE_MIC;
	else if (!deviceName.compare(EXIDEV_BBA_STR))
		tempType = EXIDEVICE_ETH;
	else if (!deviceName.compare(EXIDEV_AM_BB_STR))
		tempType = EXIDEVICE_AM_BASEBOARD;
	else if (!deviceName.compare(DEV_NONE_STR))
		tempType = EXIDEVICE_NONE;
	else
		tempType = EXIDEVICE_DUMMY;

	// Gray out the memcard path button if we're not on a memcard
	if (tempType == EXIDEVICE_MEMORYCARD_A || tempType == EXIDEVICE_MEMORYCARD_B)
		GCMemcardPath[deviceNum]->Enable();
	else if (deviceNum == 0 || deviceNum == 1)
		GCMemcardPath[deviceNum]->Disable();

	SConfig::GetInstance().m_EXIDevice[deviceNum] = tempType;

	if (Core::GetState() != Core::CORE_UNINITIALIZED)
	{
		// Change plugged device! :D
		ExpansionInterface::ChangeDevice(
			(deviceNum == 1) ? 1 : 0,	// SlotB is on channel 1, slotA and SP1 are on 0
			tempType,					// The device enum to change to
			(deviceNum == 2) ? 2 : 0);	// SP1 is device 2, slots are device 0
	}
}




// Wii settings
// -------------------
void CConfigMain::WiiSettingsChanged(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	case ID_WII_BT_BAR: // Wiimote settings
		SConfig::GetInstance().m_SYSCONF->SetData("BT.BAR", WiiSensBarPos->GetSelection());
		break;

	case ID_WII_IPL_AR: // SYSCONF settings
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.AR", WiiAspectRatio->GetSelection());
		break;
	case ID_WII_IPL_SSV:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.SSV", WiiScreenSaver->IsChecked());
		break;
	case ID_WII_IPL_LNG:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.LNG", WiiSystemLang->GetSelection());
		break;
	case ID_WII_IPL_PGS:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.PGS", WiiProgressiveScan->IsChecked());
		break;
	case ID_WII_IPL_E60:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.E60", WiiEuRGB60->IsChecked());
		break;
	case ID_WII_SD_CARD:
		SConfig::GetInstance().m_WiiSDCard = WiiSDCard->IsChecked();
		break;
	case ID_WII_KEYBOARD:
		SConfig::GetInstance().m_WiiKeyboard = WiiKeyboard->IsChecked();
		break;
	}
}





// Paths settings
// -------------------
void CConfigMain::ISOPathsSelectionChanged(wxCommandEvent& WXUNUSED (event))
{
	if (!ISOPaths->GetStringSelection().empty())
	{
		RemoveISOPath->Enable(true);
	}
	else
	{
		RemoveISOPath->Enable(false);
	}
}

void CConfigMain::AddRemoveISOPaths(wxCommandEvent& event)
{
	if (event.GetId() == ID_ADDISOPATH)
	{
		wxString dirHome;
		wxGetHomeDir(&dirHome);

		wxDirDialog dialog(this, _T("ѡ����Ҫ���ӵ�Ŀ¼"), dirHome, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

		if (dialog.ShowModal() == wxID_OK)
		{
			if (ISOPaths->FindString(dialog.GetPath()) != -1)
			{
				wxMessageBox(_("ѡ���Ŀ¼�Ѿ��������б�"), _("Error"), wxOK);
			}
			else
			{
				bRefreshList = true;
				ISOPaths->Append(dialog.GetPath());
			}
		}
	}
	else
	{
		bRefreshList = true;
		ISOPaths->Delete(ISOPaths->GetSelection());
	}

	// Save changes right away
	SConfig::GetInstance().m_ISOFolder.clear();

	for (unsigned int i = 0; i < ISOPaths->GetCount(); i++)
		SConfig::GetInstance().m_ISOFolder.push_back(std::string(ISOPaths->GetStrings()[i].mb_str()));
}

void CConfigMain::RecursiveDirectoryChanged(wxCommandEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_RecursiveISOFolder = RecersiveISOPath->IsChecked();
	bRefreshList = true;
}

void CConfigMain::DefaultISOChanged(wxFileDirPickerEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDefaultGCM = DefaultISO->GetPath().mb_str();
}

void CConfigMain::DVDRootChanged(wxFileDirPickerEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDVDRoot = DVDRoot->GetPath().mb_str();
}

void CConfigMain::ApploaderPathChanged(wxFileDirPickerEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_LocalCoreStartupParameter.m_strApploader = ApploaderPath->GetPath().mb_str();
}


// Plugin settings
void CConfigMain::OnSelectionChanged(wxCommandEvent& WXUNUSED (event))
{
	// Update plugin filenames
	GetFilename(GraphicSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strVideoPlugin);
	GetFilename(DSPSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDSPPlugin);
	for (int i = 0; i < MAXPADS; i++)
		GetFilename(PADSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strPadPlugin[i]);	
	for (int i = 0; i < MAXWIIMOTES; i++)
		GetFilename(WiimoteSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strWiimotePlugin[i]);
}

void CConfigMain::OnConfig(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	    case ID_GRAPHIC_CONFIG:
		    CallConfig(GraphicSelection);
		    break;

		case ID_DSP_CONFIG:
		    CallConfig(DSPSelection);
		    break;

		case ID_PAD_CONFIG:
		    CallConfig(PADSelection);
		    break;

		case ID_WIIMOTE_CONFIG:
		    CallConfig(WiimoteSelection);
		    break;
	}
}

void CConfigMain::CallConfig(wxChoice* _pChoice)
{
	int Index = _pChoice->GetSelection();
	INFO_LOG(CONSOLE, "CallConfig: %i\n", Index);

	if (Index >= 0)
	{
		const CPluginInfo* pInfo = static_cast<CPluginInfo*>(_pChoice->GetClientData(Index));

		if (pInfo != NULL)
			CPluginManager::GetInstance().OpenConfig((HWND) this->GetHandle(), pInfo->GetFilename().c_str(), pInfo->GetPluginInfo().Type);
	}
}

void CConfigMain::FillChoiceBox(wxChoice* _pChoice, int _PluginType, const std::string& _SelectFilename)
{
	_pChoice->Clear();

	int Index = -1;
	const CPluginInfos& rInfos = CPluginManager::GetInstance().GetPluginInfos();

	for (size_t i = 0; i < rInfos.size(); i++)
	{
		const PLUGIN_INFO& rPluginInfo = rInfos[i].GetPluginInfo();

		if (rPluginInfo.Type == _PluginType)
		{
			wxString temp;
			temp = wxString::FromAscii(rInfos[i].GetPluginInfo().Name);
			int NewIndex = _pChoice->Append(temp, (void*)&rInfos[i]);

			if (rInfos[i].GetFilename() == _SelectFilename)
			{
				Index = NewIndex;
			}
		}
	}

	_pChoice->Select(Index);
}

bool CConfigMain::GetFilename(wxChoice* _pChoice, std::string& _rFilename)
{	
	_rFilename.clear();
	int Index = _pChoice->GetSelection();

	if (Index >= 0)
	{
		const CPluginInfo* pInfo = static_cast<CPluginInfo*>(_pChoice->GetClientData(Index));
		_rFilename = pInfo->GetFilename();
		INFO_LOG(CONSOLE, "GetFilename: %i %s\n", Index, _rFilename.c_str());
		return(true);
	}

	return(false);
}