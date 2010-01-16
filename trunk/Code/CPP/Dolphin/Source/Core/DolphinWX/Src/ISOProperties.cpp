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

#include "Globals.h"

#include "VolumeCreator.h"
#include "Filesystem.h"
#include "ISOProperties.h"
#include "PatchAddEdit.h"
#include "ARCodeAddEdit.h"
#include "ConfigManager.h"
#include "StringUtil.h"

#if USE_XPM_BITMAPS
	#include "../resources/isoprop_file.xpm"
	#include "../resources/isoprop_folder.xpm"
	#include "../resources/isoprop_disc.xpm"
#endif // USE_XPM_BITMAPS

struct WiiPartition
{
	DiscIO::IVolume *Partition;
	DiscIO::IFileSystem *FileSystem;
	std::vector<const DiscIO::SFileInfo *> Files;
};
std::vector<WiiPartition> WiiDisc;

DiscIO::IVolume *OpenISO = NULL;
DiscIO::IFileSystem *pFileSystem = NULL;

std::vector<PatchEngine::Patch> onFrame;
std::vector<ActionReplay::ARCode> arCodes;


BEGIN_EVENT_TABLE(CISOProperties, wxDialog)
	EVT_CLOSE(CISOProperties::OnClose)
	EVT_BUTTON(ID_CLOSE, CISOProperties::OnCloseClick)
	EVT_BUTTON(ID_EDITCONFIG, CISOProperties::OnEditConfig)
	EVT_CHOICE(ID_EMUSTATE, CISOProperties::SetRefresh)
	EVT_CHOICE(ID_EMU_ISSUES, CISOProperties::SetRefresh)
	EVT_LISTBOX(ID_PATCHES_LIST, CISOProperties::ListSelectionChanged)
	EVT_BUTTON(ID_EDITPATCH, CISOProperties::PatchButtonClicked)
	EVT_BUTTON(ID_ADDPATCH, CISOProperties::PatchButtonClicked)
	EVT_BUTTON(ID_REMOVEPATCH, CISOProperties::PatchButtonClicked)
	EVT_LISTBOX(ID_CHEATS_LIST, CISOProperties::ListSelectionChanged)
	EVT_BUTTON(ID_EDITCHEAT, CISOProperties::ActionReplayButtonClicked)
	EVT_BUTTON(ID_ADDCHEAT, CISOProperties::ActionReplayButtonClicked)
	EVT_BUTTON(ID_REMOVECHEAT, CISOProperties::ActionReplayButtonClicked)
	EVT_MENU(IDM_BNRSAVEAS, CISOProperties::OnBannerImageSave)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_TREECTRL, CISOProperties::OnRightClickOnTree)
	EVT_MENU(IDM_EXTRACTFILE, CISOProperties::OnExtractFile)
	EVT_MENU(IDM_EXTRACTDIR, CISOProperties::OnExtractDir)
	EVT_MENU(IDM_EXTRACTALL, CISOProperties::OnExtractDir)
	EVT_MENU(IDM_EXTRACTAPPLOADER, CISOProperties::OnExtractDataFromHeader)
	EVT_MENU(IDM_EXTRACTDOL, CISOProperties::OnExtractDataFromHeader)
	EVT_CHOICE(ID_LANG, CISOProperties::OnChangeBannerLang)
END_EVENT_TABLE()

CISOProperties::CISOProperties(const std::string fileName, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	OpenISO = DiscIO::CreateVolumeFromFilename(fileName);
	if (DiscIO::IsVolumeWiiDisc(OpenISO))
	{
		for (u32 i = 0; i < 0xFFFFFFFF; i++) // yes, technically there can be OVER NINE THOUSAND partitions...
		{
			WiiPartition temp;
			if ((temp.Partition = DiscIO::CreateVolumeFromFilename(fileName, 0, i)) != NULL)
			{
				if ((temp.FileSystem = DiscIO::CreateFileSystem(temp.Partition)) != NULL)
				{
					temp.FileSystem->GetFileList(temp.Files);
					WiiDisc.push_back(temp);
				}
			}
			else
				break;
		}
	}
	else
	{
		// TODO : Should we add a way to browse the wad file ?
		if (!DiscIO::IsVolumeWadFile(OpenISO))
		{
			GCFiles.clear();
			pFileSystem = DiscIO::CreateFileSystem(OpenISO);
			if (pFileSystem)
				pFileSystem->GetFileList(GCFiles);
		}
	}

	OpenGameListItem = new GameListItem(fileName);

	bRefreshList = false;

	CreateGUIControls(DiscIO::IsVolumeWadFile(OpenISO));

	GameIniFile = FULL_GAMECONFIG_DIR + (OpenISO->GetUniqueID()) + ".ini";
	if (GameIni.Load(GameIniFile.c_str()))
		LoadGameConfig();
	else
	{
		FILE *f = fopen(GameIniFile.c_str(), "w");
		fprintf(f, "# %s - %s\n", OpenISO->GetUniqueID().c_str(), OpenISO->GetName().c_str());
		fprintf(f, "[Core] Values set here will override the main dolphin settings.\n");
		fprintf(f, "[EmuState] The Emulation State. 1 is worst, 5 is best, 0 is not set.\n");
		fprintf(f, "[OnFrame] Add memory patches to be applied every frame here.\n");
		fprintf(f, "[ActionReplay] Add action replay cheats here.\n");
		fclose(f);
		if (GameIni.Load(GameIniFile.c_str()))
			LoadGameConfig();
		else
			wxMessageBox(wxString::Format(_("���ܴ��� %s"), wxString::FromAscii(GameIniFile.c_str()).c_str()), _("Error"), wxOK|wxICON_ERROR, this);
	}

	// Disk header and apploader
	m_Name->SetValue(wxString(OpenISO->GetName().c_str(), wxConvUTF8));
	m_GameID->SetValue(wxString(OpenISO->GetUniqueID().c_str(), wxConvUTF8));
	switch (OpenISO->GetCountry())
	{
	case DiscIO::IVolume::COUNTRY_EUROPE:
		m_Country->SetValue(wxT("ŷ��"));
		break;
	case DiscIO::IVolume::COUNTRY_FRANCE:
		m_Country->SetValue(wxT("����"));
		break;
	case DiscIO::IVolume::COUNTRY_ITALY:
		m_Country->SetValue(wxT("�����"));
		break;
	case DiscIO::IVolume::COUNTRY_USA:
		m_Country->SetValue(wxT("����"));
		m_Lang->SetSelection(0);
		m_Lang->Disable(); // For NTSC Games, there's no multi lang
		break;
	case DiscIO::IVolume::COUNTRY_JAPAN:
		m_Country->SetValue(wxT("�ձ�"));
		m_Lang->SetSelection(-1);
		m_Lang->Disable(); // For NTSC Games, there's no multi lang
		break;
	case DiscIO::IVolume::COUNTRY_KOREA:
		m_Country->SetValue(wxT("����"));
		break;
	case DiscIO::IVolume::COUNTRY_TAIWAN:
		m_Country->SetValue(wxT("̨��"));
		m_Lang->SetSelection(-1);
		m_Lang->Disable(); // For NTSC Games, there's no multi lang
		break;
	case DiscIO::IVolume::COUNTRY_SDK:
		m_Country->SetValue(wxT("û�й��� (SDK)"));
		break;
	default:
		m_Country->SetValue(wxT("δ֪"));
		break;
	}
	wxString temp = _T("0x") + wxString::FromAscii(OpenISO->GetMakerID().c_str());
	m_MakerID->SetValue(temp);
	m_Date->SetValue(wxString::FromAscii(OpenISO->GetApploaderDate().c_str()));
	m_FST->SetValue(wxString::Format(_T("%u"), OpenISO->GetFSTSize()));

	// Here we set all the info to be shown (be it SJIS or Ascii) + we set the window title
	ChangeBannerDetails((int)SConfig::GetInstance().m_InterfaceLanguage);
	m_Banner->SetBitmap(OpenGameListItem->GetImage());
	m_Banner->Connect(wxID_ANY, wxEVT_RIGHT_DOWN,
		wxMouseEventHandler(CISOProperties::RightClickOnBanner), (wxObject*)NULL, this);

	// Filesystem browser/dumper
	if (DiscIO::IsVolumeWiiDisc(OpenISO))
	{
		for (u32 i = 0; i < WiiDisc.size(); i++)
		{
			WiiPartition partition = WiiDisc.at(i);
			wxTreeItemId PartitionRoot = m_Treectrl->AppendItem(RootId, wxString::Format(wxT("���� %i"), i), 0, 0, 0);
			CreateDirectoryTree(PartitionRoot, partition.Files, 1, partition.Files.at(0)->m_FileSize);	
			if (i == 1)
				m_Treectrl->Expand(PartitionRoot);
		}
	}
	else
	{
		// TODO : Should we add a way to browse the wad file ?
		if (!DiscIO::IsVolumeWadFile(OpenISO))
		{
			if (!GCFiles.empty())
				CreateDirectoryTree(RootId, GCFiles, 1, GCFiles.at(0)->m_FileSize);	
		}
	}
	m_Treectrl->Expand(RootId);
}

CISOProperties::~CISOProperties()
{
	if (IsVolumeWiiDisc(OpenISO))
	{
		for (std::vector<WiiPartition>::const_iterator PartIter = WiiDisc.begin(); PartIter != WiiDisc.end(); ++PartIter)
		{
			delete PartIter->FileSystem; // Remember the FileList is a member of DiscIO::IFileSystem
			delete PartIter->Partition;
		}
		WiiDisc.clear();
	}
	else
		if (!IsVolumeWadFile(OpenISO))
			if(pFileSystem)
				delete pFileSystem;

	delete OpenISO;
}

size_t CISOProperties::CreateDirectoryTree(wxTreeItemId& parent,
										 std::vector<const DiscIO::SFileInfo*> fileInfos,
										 const size_t _FirstIndex, 
										 const size_t _LastIndex)
{
	size_t CurrentIndex = _FirstIndex;

	while (CurrentIndex < _LastIndex)
	{
		const DiscIO::SFileInfo *rFileInfo = fileInfos[CurrentIndex];
		char *name = (char*)rFileInfo->m_FullPath;

		if (rFileInfo->IsDirectory()) name[strlen(name) - 1] = '\0';
		char *itemName = strrchr(name, DIR_SEP_CHR);

		if(!itemName)
			itemName = name;
		else
			itemName++;

		// check next index
		if (rFileInfo->IsDirectory())
		{
			wxTreeItemId item = m_Treectrl->AppendItem(parent, wxString::FromAscii(itemName), 1, 1);
			CurrentIndex = CreateDirectoryTree(item, fileInfos, CurrentIndex + 1, (size_t)rFileInfo->m_FileSize);
		}
		else
		{
			m_Treectrl->AppendItem(parent, wxString::FromAscii(itemName), 2, 2);
			CurrentIndex++;
		}
	}

	return CurrentIndex;
}

void CISOProperties::CreateGUIControls(bool IsWad)
{
	m_Close = new wxButton(this, ID_CLOSE, _("�ر�"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	EditConfig = new wxButton(this, ID_EDITCONFIG, _("�༭����"), wxDefaultPosition, wxDefaultSize);
	EditConfig->SetToolTip(_("This will let you Manually Edit the INI config file"));

	// Notebook
	m_Notebook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	m_GameConfig = new wxPanel(m_Notebook, ID_GAMECONFIG, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_GameConfig, _("��Ϸ����"));
	m_PatchPage = new wxPanel(m_Notebook, ID_PATCH_PAGE, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_PatchPage, _("����"));
	m_CheatPage = new wxPanel(m_Notebook, ID_ARCODE_PAGE, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_CheatPage, _("AR Codes"));
	m_Information = new wxPanel(m_Notebook, ID_INFORMATION, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Information, _("��Ϣ"));
	m_Filesystem = new wxPanel(m_Notebook, ID_FILESYSTEM, wxDefaultPosition, wxDefaultSize);
	m_Notebook->AddPage(m_Filesystem, _("�ļ�ϵͳ"));

	wxBoxSizer* sButtons;
	sButtons = new wxBoxSizer(wxHORIZONTAL);
	sButtons->Add(EditConfig, 0, wxALL, 5);
	sButtons->Add(0, 0, 1, wxEXPAND, 5);
	sButtons->Add(m_Close, 0, wxALL, 5);


	
	// GameConfig editing - Overrides and emulation state
	sbGameConfig = new wxStaticBoxSizer(wxVERTICAL, m_GameConfig, _("Game-Specific ����"));
	OverrideText = new wxStaticText(m_GameConfig, ID_OVERRIDE_TEXT, _("These settings override core Dolphin settings.\nUndetermined means the game uses Dolphin's setting."), wxDefaultPosition, wxDefaultSize);
	// Core
	sbCoreOverrides = new wxStaticBoxSizer(wxVERTICAL, m_GameConfig, _("����"));
	CPUThread = new wxCheckBox(m_GameConfig, ID_USEDUALCORE, _("���ö�˼���"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	SkipIdle = new wxCheckBox(m_GameConfig, ID_IDLESKIP, _("���ÿ��в���"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	TLBHack = new wxCheckBox(m_GameConfig, ID_TLBHACK, _("TLB �ƽ�"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	// Wii Console
	sbWiiOverrides = new wxStaticBoxSizer(wxVERTICAL, m_GameConfig, _("Wii ����̨"));
	EnableProgressiveScan = new wxCheckBox(m_GameConfig, ID_ENABLEPROGRESSIVESCAN, _("���� Progressive Scan"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	EnableWideScreen = new wxCheckBox(m_GameConfig, ID_ENABLEWIDESCREEN, _("���ÿ���"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	if (!DiscIO::IsVolumeWiiDisc(OpenISO) && !DiscIO::IsVolumeWadFile(OpenISO))
	{
		sbWiiOverrides->ShowItems(false);
 		EnableProgressiveScan->Hide();
 		EnableWideScreen->Hide();
	}
	// Video
	sbVideoOverrides = new wxStaticBoxSizer(wxVERTICAL, m_GameConfig, _("��Ƶ"));
	ForceFiltering = new wxCheckBox(m_GameConfig, ID_FORCEFILTERING, _("ǿ��ɸѡ"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	EFBCopyDisable = new wxCheckBox(m_GameConfig, ID_EFBCOPYDISABLE, _("���ø��Ƶ� EFB"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	EFBToTextureEnable = new wxCheckBox(m_GameConfig, ID_EFBTOTEXTUREENABLE, _("���� EFB �� RAM"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	SafeTextureCache = new wxCheckBox(m_GameConfig, ID_SAFETEXTURECACHE, _("��ȫ���ʻ���"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	DstAlphaPass = new wxCheckBox(m_GameConfig, ID_DSTALPHAPASS, _("Distance Alpha Pass"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	UseXFB = new wxCheckBox(m_GameConfig, ID_USEXFB, _("ʹ�� XFB"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	// Hack
	BPHack = new wxCheckBox(m_GameConfig, ID_BPHACK, _("FIFO BP Hack"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);
	Hacktext = new wxStaticText(m_GameConfig, ID_HACK_TEXT, _("Projection Hack for: "), wxDefaultPosition, wxDefaultSize);
	arrayStringFor_Hack.Add(_("None"));
	arrayStringFor_Hack.Add(_("������ƻ�(����)������¬ķ"));
	arrayStringFor_Hack.Add(_("����������ʿ"));
	arrayStringFor_Hack.Add(_("Bleach Versus Crusade"));
	arrayStringFor_Hack.Add(_("Final Fantasy CC Echo of Time"));
	arrayStringFor_Hack.Add(_("Harvest Moon Magical Melody"));
	arrayStringFor_Hack.Add(_("Baten Kaitos"));
	arrayStringFor_Hack.Add(_("Baten Kaitos Origin"));
	arrayStringFor_Hack.Add(_("Skies of Arcadia"));
	Hack = new wxChoice(m_GameConfig, ID_HACK, wxDefaultPosition, wxDefaultSize, arrayStringFor_Hack, 0, wxDefaultValidator);

	
	//HLE Audio
	sbHLEaudioOverrides = new wxStaticBoxSizer(wxVERTICAL, m_GameConfig, _("HLE ��Ƶ"));
	UseRE0Fix = new wxCheckBox(m_GameConfig, ID_RE0FIX, _("ʹ�� RE0 ����"), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE|wxCHK_ALLOW_3RD_STATE_FOR_USER, wxDefaultValidator);

	// Emulation State
	sEmuState = new wxBoxSizer(wxHORIZONTAL);
	EmuStateText = new wxStaticText(m_GameConfig, ID_EMUSTATE_TEXT, _("ģ��״̬: "), wxDefaultPosition, wxDefaultSize);
	arrayStringFor_EmuState.Add(_("δ����"));
	arrayStringFor_EmuState.Add(_("��"));
	arrayStringFor_EmuState.Add(_("����"));
	arrayStringFor_EmuState.Add(_("���Խ���"));
	arrayStringFor_EmuState.Add(_("��������"));
	arrayStringFor_EmuState.Add(_("����"));
	EmuState = new wxChoice(m_GameConfig, ID_EMUSTATE, wxDefaultPosition, wxDefaultSize, arrayStringFor_EmuState, 0, wxDefaultValidator);
	EmuIssues = new wxTextCtrl(m_GameConfig, ID_EMU_ISSUES, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	wxBoxSizer* sConfigPage;
	sConfigPage = new wxBoxSizer(wxVERTICAL);
	sbGameConfig->Add(OverrideText, 0, wxEXPAND|wxALL, 5);
	sbCoreOverrides->Add(CPUThread, 0, wxEXPAND|wxLEFT, 5);
	sbCoreOverrides->Add(SkipIdle, 0, wxEXPAND|wxLEFT, 5);
	sbCoreOverrides->Add(TLBHack, 0, wxEXPAND|wxLEFT, 5);
	sbWiiOverrides->Add(EnableProgressiveScan, 0, wxEXPAND|wxLEFT, 5);
	sbWiiOverrides->Add(EnableWideScreen, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(ForceFiltering, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(EFBCopyDisable, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(EFBToTextureEnable, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(SafeTextureCache, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(DstAlphaPass, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(UseXFB, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(BPHack, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(Hacktext, 0, wxEXPAND|wxLEFT, 5);
	sbVideoOverrides->Add(Hack, 0, wxEXPAND|wxLEFT, 5);
	sbHLEaudioOverrides->Add(UseRE0Fix, 0, wxEXPAND|wxLEFT, 5);
	sbGameConfig->Add(sbCoreOverrides, 0, wxEXPAND);
	sbGameConfig->Add(sbWiiOverrides, 0, wxEXPAND);
	sbGameConfig->Add(sbVideoOverrides, 0, wxEXPAND);
	sbGameConfig->Add(sbHLEaudioOverrides, 0, wxEXPAND);
	sConfigPage->Add(sbGameConfig, 0, wxEXPAND|wxALL, 5);
	sEmuState->Add(EmuStateText, 0, wxALIGN_CENTER_VERTICAL);
	sEmuState->Add(EmuState, 0, wxEXPAND);
	sEmuState->Add(EmuIssues,1,wxEXPAND);
	sConfigPage->Add(sEmuState, 0, wxEXPAND|wxALL, 5);
	m_GameConfig->SetSizer(sConfigPage);
	sConfigPage->Layout();

	
	// Patches
	sPatches = new wxBoxSizer(wxVERTICAL);
	Patches = new wxCheckListBox(m_PatchPage, ID_PATCHES_LIST, wxDefaultPosition, wxDefaultSize, arrayStringFor_Patches, wxLB_HSCROLL, wxDefaultValidator);
	sPatchButtons = new wxBoxSizer(wxHORIZONTAL);
	EditPatch = new wxButton(m_PatchPage, ID_EDITPATCH, _("�༭..."), wxDefaultPosition, wxDefaultSize, 0);
	AddPatch = new wxButton(m_PatchPage, ID_ADDPATCH, _("���..."), wxDefaultPosition, wxDefaultSize, 0);
	RemovePatch = new wxButton(m_PatchPage, ID_REMOVEPATCH, _("�Ƴ�"), wxDefaultPosition, wxDefaultSize, 0);
	EditPatch->Enable(false);
	RemovePatch->Enable(false);

	wxBoxSizer* sPatchPage;
	sPatchPage = new wxBoxSizer(wxVERTICAL);
	sPatches->Add(Patches, 1, wxEXPAND|wxALL, 0);
	sPatchButtons->Add(EditPatch,  0, wxEXPAND|wxALL, 0);
	sPatchButtons->AddStretchSpacer();
	sPatchButtons->Add(AddPatch,  0, wxEXPAND|wxALL, 0);
	sPatchButtons->Add(RemovePatch,  0, wxEXPAND|wxALL, 0);
	sPatches->Add(sPatchButtons, 0, wxEXPAND|wxALL, 0);
	sPatchPage->Add(sPatches, 1, wxEXPAND|wxALL, 5);
	m_PatchPage->SetSizer(sPatchPage);
	sPatchPage->Layout();

	
	// Action Replay Cheats
	sCheats = new wxBoxSizer(wxVERTICAL);
	Cheats = new wxCheckListBox(m_CheatPage, ID_CHEATS_LIST, wxDefaultPosition, wxDefaultSize, arrayStringFor_Cheats, wxLB_HSCROLL, wxDefaultValidator);
	sCheatButtons = new wxBoxSizer(wxHORIZONTAL);
	EditCheat = new wxButton(m_CheatPage, ID_EDITCHEAT, _("�༭..."), wxDefaultPosition, wxDefaultSize, 0);
	AddCheat = new wxButton(m_CheatPage, ID_ADDCHEAT, _("���..."), wxDefaultPosition, wxDefaultSize, 0);
	RemoveCheat = new wxButton(m_CheatPage, ID_REMOVECHEAT, _("�Ƴ�"), wxDefaultPosition, wxDefaultSize, 0);
	EditCheat->Enable(false);
	RemoveCheat->Enable(false);

	wxBoxSizer* sCheatPage;
	sCheatPage = new wxBoxSizer(wxVERTICAL);
	sCheats->Add(Cheats, 1, wxEXPAND|wxALL, 0);
	sCheatButtons->Add(EditCheat,  0, wxEXPAND|wxALL, 0);
	sCheatButtons->AddStretchSpacer();
	sCheatButtons->Add(AddCheat,  0, wxEXPAND|wxALL, 0);
	sCheatButtons->Add(RemoveCheat,  0, wxEXPAND|wxALL, 0);
	sCheats->Add(sCheatButtons, 0, wxEXPAND|wxALL, 0);
	sCheatPage->Add(sCheats, 1, wxEXPAND|wxALL, 5);
	m_CheatPage->SetSizer(sCheatPage);
	sCheatPage->Layout();

	
	// ISO Details
	sbISODetails = new wxStaticBoxSizer(wxVERTICAL, m_Information, _("ISO ��ϸ��Ϣ"));
	sISODetails = new wxGridBagSizer(0, 0);
	sISODetails->AddGrowableCol(1);
	m_NameText = new wxStaticText(m_Information, ID_NAME_TEXT, _("����:"), wxDefaultPosition, wxDefaultSize);
	m_Name = new wxTextCtrl(m_Information, ID_NAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_GameIDText = new wxStaticText(m_Information, ID_GAMEID_TEXT, _("��Ϸ ID:"), wxDefaultPosition, wxDefaultSize);
	m_GameID = new wxTextCtrl(m_Information, ID_GAMEID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_CountryText = new wxStaticText(m_Information, ID_COUNTRY_TEXT, _("����:"), wxDefaultPosition, wxDefaultSize);
	m_Country = new wxTextCtrl(m_Information, ID_COUNTRY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_MakerIDText = new wxStaticText(m_Information, ID_MAKERID_TEXT, _("���� ID:"), wxDefaultPosition, wxDefaultSize);
	m_MakerID = new wxTextCtrl(m_Information, ID_MAKERID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_DateText = new wxStaticText(m_Information, ID_DATE_TEXT, _("����:"), wxDefaultPosition, wxDefaultSize);
	m_Date = new wxTextCtrl(m_Information, ID_DATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_FSTText = new wxStaticText(m_Information, ID_FST_TEXT, _("FST ��С:"), wxDefaultPosition, wxDefaultSize);	
	m_FST = new wxTextCtrl(m_Information, ID_FST, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

	// Banner Details
	sbBannerDetails = new wxStaticBoxSizer(wxVERTICAL, m_Information, _("��������ϸ��Ϣ"));
	sBannerDetails = new wxGridBagSizer(0, 0);
	sBannerDetails->AddGrowableCol(1); sBannerDetails->AddGrowableCol(2); sBannerDetails->AddGrowableCol(3);
	m_LangText = new wxStaticText(m_Information, ID_LANG_TEXT, _("��ʾ����:"), wxDefaultPosition, wxDefaultSize);
	arrayStringFor_Lang.Add(_("Ӣ��"));
	arrayStringFor_Lang.Add(_("����"));
	arrayStringFor_Lang.Add(_("����"));
	arrayStringFor_Lang.Add(_("��������"));
	arrayStringFor_Lang.Add(_("�������"));
	arrayStringFor_Lang.Add(_("������"));
	m_Lang = new wxChoice(m_Information, ID_LANG, wxDefaultPosition, wxDefaultSize, arrayStringFor_Lang, 0, wxDefaultValidator);
	m_Lang->SetSelection((int)SConfig::GetInstance().m_InterfaceLanguage);
	m_ShortText = new wxStaticText(m_Information, ID_SHORTNAME_TEXT, _("������:"), wxDefaultPosition, wxDefaultSize);
	m_ShortName = new wxTextCtrl(m_Information, ID_SHORTNAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_MakerText = new wxStaticText(m_Information, ID_MAKER_TEXT, _("������:"), wxDefaultPosition, wxDefaultSize);
	m_Maker = new wxTextCtrl(m_Information, ID_MAKER, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	m_CommentText = new wxStaticText(m_Information, ID_COMMENT_TEXT, _("ע��:"), wxDefaultPosition, wxDefaultSize);
	m_Comment = new wxTextCtrl(m_Information, ID_COMMENT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY);
	m_BannerText = new wxStaticText(m_Information, ID_BANNER_TEXT, _("������:"), wxDefaultPosition, wxDefaultSize);
	m_Banner = new wxStaticBitmap(m_Information, ID_BANNER, wxNullBitmap, wxDefaultPosition, wxSize(96, 32), 0);

	wxBoxSizer* sInfoPage;
	sInfoPage = new wxBoxSizer(wxVERTICAL);
	sISODetails->Add(m_NameText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sISODetails->Add(m_Name, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sISODetails->Add(m_GameIDText, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sISODetails->Add(m_GameID, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sISODetails->Add(m_CountryText, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sISODetails->Add(m_Country, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sISODetails->Add(m_MakerIDText, wxGBPosition(3, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sISODetails->Add(m_MakerID, wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sISODetails->Add(m_DateText, wxGBPosition(4, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sISODetails->Add(m_Date, wxGBPosition(4, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sISODetails->Add(m_FSTText, wxGBPosition(5, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sISODetails->Add(m_FST, wxGBPosition(5, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sbISODetails->Add(sISODetails, 0, wxEXPAND, 5);

	sBannerDetails->Add(m_LangText, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sBannerDetails->Add(m_Lang, wxGBPosition(0, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sBannerDetails->Add(m_ShortText, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sBannerDetails->Add(m_ShortName, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sBannerDetails->Add(m_MakerText, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sBannerDetails->Add(m_Maker, wxGBPosition(2, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sBannerDetails->Add(m_CommentText, wxGBPosition(3, 0), wxGBSpan(1, 1), wxALL, 5);
	sBannerDetails->Add(m_Comment, wxGBPosition(3, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sBannerDetails->Add(m_BannerText, wxGBPosition(4, 0), wxGBSpan(1, 1), wxALL, 5);
	sBannerDetails->Add(m_Banner, wxGBPosition(4, 1), wxGBSpan(1, 1), wxEXPAND|wxALL, 5);
	sbBannerDetails->Add(sBannerDetails, 0, wxEXPAND, 0);
	sInfoPage->Add(sbISODetails, 0, wxEXPAND|wxALL, 5);
	sInfoPage->Add(sbBannerDetails, 0, wxEXPAND|wxALL, 5);
	m_Information->SetSizer(sInfoPage);
	sInfoPage->Layout();

	// Filesystem icons
	wxIcon iconTemp;
	m_iconList = new wxImageList(16, 16);

	iconTemp.CopyFromBitmap(wxBitmap(disc_xpm));   m_iconList->Add(iconTemp); // 0
	iconTemp.CopyFromBitmap(wxBitmap(folder_xpm)); m_iconList->Add(iconTemp); // 1
	iconTemp.CopyFromBitmap(wxBitmap(file_xpm));   m_iconList->Add(iconTemp); // 2

	// Filesystem tree
	m_Treectrl = new wxTreeCtrl(m_Filesystem, ID_TREECTRL, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE, wxDefaultValidator);
	m_Treectrl->AssignImageList(m_iconList);
	RootId = m_Treectrl->AddRoot(wxT("Disc"), 0, 0, 0);

	wxBoxSizer* sTreePage;
	sTreePage = new wxBoxSizer(wxVERTICAL);
	sTreePage->Add(m_Treectrl, 1, wxEXPAND|wxALL, 5);
	m_Filesystem->SetSizer(sTreePage);
	sTreePage->Layout();

	// It's a wad file, so we remove the FileSystem page
	if (IsWad)
		m_Notebook->RemovePage(4);

	
	// Add notebook and buttons to the dialog
	wxBoxSizer* sMain;
	sMain = new wxBoxSizer(wxVERTICAL);
	sMain->Add(m_Notebook, 1, wxEXPAND|wxALL, 5);
	sMain->Add(sButtons, 0, wxEXPAND, 5);
	sMain->SetMinSize(wxSize(400,500));
	SetSizerAndFit(sMain);
}

void CISOProperties::OnClose(wxCloseEvent& WXUNUSED (event))
{
	if (!SaveGameConfig())
		PanicYesNo("Could not save %s", GameIniFile.c_str());

	EndModal(bRefreshList ? wxID_OK : wxID_CANCEL);
}

void CISOProperties::OnCloseClick(wxCommandEvent& WXUNUSED (event))
{
	Close();
}

void CISOProperties::RightClickOnBanner(wxMouseEvent& event)
{
	wxMenu* popupMenu = new wxMenu;
	popupMenu->Append(IDM_BNRSAVEAS, _("���Ϊ..."));
	PopupMenu(popupMenu);

	event.Skip();
}

void CISOProperties::OnBannerImageSave(wxCommandEvent& WXUNUSED (event))
{
	wxString dirHome;

	wxFileDialog dialog(this, _("���Ϊ..."), wxGetHomeDir(&dirHome), wxString::Format(_("%s.png"), m_GameID->GetLabel().c_str()),
		_("*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT, wxDefaultPosition, wxDefaultSize);
	if (dialog.ShowModal() == wxID_OK)
	{
		m_Banner->GetBitmap().ConvertToImage().SaveFile(dialog.GetPath());
	}
}

void CISOProperties::OnRightClickOnTree(wxTreeEvent& event)
{
	m_Treectrl->SelectItem(event.GetItem());

	wxMenu* popupMenu = new wxMenu;

	if (m_Treectrl->GetItemImage(m_Treectrl->GetSelection()) == 0
		&& m_Treectrl->GetFirstVisibleItem() != m_Treectrl->GetSelection())
		popupMenu->Append(IDM_EXTRACTDIR, _("��ѹ������..."));
	else if (m_Treectrl->GetItemImage(m_Treectrl->GetSelection()) == 1)
		popupMenu->Append(IDM_EXTRACTDIR, _("��ѹ��Ŀ¼..."));
	else if (m_Treectrl->GetItemImage(m_Treectrl->GetSelection()) == 2)
		popupMenu->Append(IDM_EXTRACTFILE, _("��ѹ���ļ�..."));

	popupMenu->Append(IDM_EXTRACTALL, _("��ѹ�������ļ�..."));
	popupMenu->AppendSeparator();
	popupMenu->Append(IDM_EXTRACTAPPLOADER, _("Extract Apploader..."));
	popupMenu->Append(IDM_EXTRACTDOL, _("Extract DOL..."));

	PopupMenu(popupMenu);

	event.Skip();
}

void CISOProperties::OnExtractFile(wxCommandEvent& WXUNUSED (event))
{
	wxString Path;
	wxString File;

	File = m_Treectrl->GetItemText(m_Treectrl->GetSelection());
	
	Path = wxFileSelector(
		wxT("�����ļ�"),
		wxEmptyString, File, wxEmptyString,
		wxString::Format
		(
			wxT("�����ļ� (%s)|%s"),
			wxFileSelectorDefaultWildcardStr,
			wxFileSelectorDefaultWildcardStr
		),
		wxFD_SAVE,
		this);

	if (!Path || !File)
		return;

	while (m_Treectrl->GetItemParent(m_Treectrl->GetSelection()) != m_Treectrl->GetRootItem())
	{
		wxString temp;
		temp = m_Treectrl->GetItemText(m_Treectrl->GetItemParent(m_Treectrl->GetSelection()));
		File = temp + wxT(DIR_SEP_CHR) + File;

		m_Treectrl->SelectItem(m_Treectrl->GetItemParent(m_Treectrl->GetSelection()));
	}

	if (DiscIO::IsVolumeWiiDisc(OpenISO))
	{
		int partitionNum = wxAtoi(File.SubString(10, 11));
		File.Remove(0, 12); // Remove "Partition x/"
		WiiDisc.at(partitionNum).FileSystem->ExportFile(File.mb_str(), Path.mb_str());
	}
	else
		pFileSystem->ExportFile(File.mb_str(), Path.mb_str());
}

void CISOProperties::ExportDir(const char* _rFullPath, const char* _rExportFolder, const int partitionNum)
{
	char exportName[512];
	u32 index[2], offsetShift = 0;
	std::vector<const DiscIO::SFileInfo *> fst;
	DiscIO::IFileSystem *FS = 0;

	if (DiscIO::IsVolumeWiiDisc(OpenISO))
	{
		FS = WiiDisc.at(partitionNum).FileSystem;
		offsetShift = 2;
	}
	else
		FS = pFileSystem;

	FS->GetFileList(fst);

	if (!_rFullPath) // Extract all
	{
		index[0] = 0;
		index[1] = (u32)fst.size();

		FS->ExportApploader(_rExportFolder);
		if (!DiscIO::IsVolumeWiiDisc(OpenISO))
			FS->ExportDOL(_rExportFolder);
	}
	else // Look for the dir we are going to extract
	{
		for(index[0] = 0; index[0] < fst.size(); index[0]++)
		{
			if (!strcmp(fst.at(index[0])->m_FullPath, _rFullPath))
			{
				DEBUG_LOG(DISCIO, "Found the Dir at %u", index[0]);
				index[1] = (u32)fst.at(index[0])->m_FileSize;
				break;
			}
		}

		DEBUG_LOG(DISCIO,"Dir found from %u to %u\nextracting to:\n%s",index[0],index[1],_rExportFolder);
	}

	wxString dialogTitle = index[0] ? _T("��ѹ��Ŀ¼") : _T("��ѹ�������ļ�");
	wxProgressDialog dialog(dialogTitle,
					_T("��ѹ����..."),
					index[1], // range
					this, // parent
					wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT |
					wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME |
					wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
					);
	dialog.CenterOnParent();

	// Extraction
	for (u32 i = index[0]; i < index[1]; i++)
	{
		dialog.SetTitle(wxString::Format(_T("%s : %d%%"), dialogTitle.c_str(),
			(u32)(((float)(i - index[0]) / (float)(index[1] - index[0])) * 100)));
		if (!dialog.Update(i, wxString::Format(_T("��ѹ�� %s"), wxString(fst[i]->m_FullPath, *wxConvCurrent).c_str())))
			break;

		if (fst[i]->IsDirectory())
		{
			sprintf(exportName, "%s/%s/", _rExportFolder, fst[i]->m_FullPath);
			DEBUG_LOG(DISCIO, "%s", exportName);		

			if (!File::Exists(exportName) && !File::CreateFullPath(exportName))
			{
				ERROR_LOG(DISCIO, "Could not create the path %s", exportName);
			}
			else
			{
				if (!File::IsDirectory(exportName))
					ERROR_LOG(DISCIO, "%s already exists and is not a directory", exportName);

				DEBUG_LOG(DISCIO, "folder %s already exists", exportName);
			}
		}
		else
		{
			sprintf(exportName, "%s/%s", _rExportFolder, fst[i]->m_FullPath);
			DEBUG_LOG(DISCIO, "%s", exportName);

			if (!File::Exists(exportName) && !FS->ExportFile(fst[i]->m_FullPath, exportName))
			{
				ERROR_LOG(DISCIO, "Could not export %s", exportName);
			}
			else
			{
				DEBUG_LOG(DISCIO, "%s already exists", exportName);
			}
		}
	}
}

void CISOProperties::OnExtractDir(wxCommandEvent& event)
{
	wxString Directory = m_Treectrl->GetItemText(m_Treectrl->GetSelection());
	wxString Path = wxDirSelector(wxT("ѡ��Ҫ��ѹ�������ļ���"));

	if (!Path || !Directory)
		return;

	if (event.GetId() == IDM_EXTRACTALL)
	{
		if (DiscIO::IsVolumeWiiDisc(OpenISO))
			for (u32 i = 0; i < WiiDisc.size(); i++)
				ExportDir(NULL, Path.mb_str(), i);
		else
			ExportDir(NULL, Path.mb_str());

		return;
	}

	while (m_Treectrl->GetItemParent(m_Treectrl->GetSelection()) != m_Treectrl->GetRootItem())
	{
		wxString temp;
		temp = m_Treectrl->GetItemText(m_Treectrl->GetItemParent(m_Treectrl->GetSelection()));
		Directory = temp + wxT(DIR_SEP_CHR) + Directory;

		m_Treectrl->SelectItem(m_Treectrl->GetItemParent(m_Treectrl->GetSelection()));
	}

	if (DiscIO::IsVolumeWiiDisc(OpenISO))
	{
		int partitionNum = wxAtoi(Directory.SubString(10, 11));
		Directory.Remove(0, 12); // Remove "Partition x/"
		ExportDir(Directory.mb_str(), Path.mb_str(), partitionNum);
	}
	else
		ExportDir(Directory.mb_str(), Path.mb_str());
}

void CISOProperties::OnExtractDataFromHeader(wxCommandEvent& event)
{
	std::vector<const DiscIO::SFileInfo *> fst;
	DiscIO::IFileSystem *FS = 0;
	wxString Path = wxDirSelector(wxT("Choose the folder to extract to"));

	if (Path.empty())
		return;

	if (DiscIO::IsVolumeWiiDisc(OpenISO))
		FS = WiiDisc.at(1).FileSystem;
	else
		FS = pFileSystem;

	bool ret = false;
	if (event.GetId() == IDM_EXTRACTAPPLOADER)
	{
		ret = FS->ExportApploader(Path.mb_str());
	}
	else if (event.GetId() == IDM_EXTRACTDOL)
	{
		ret = FS->ExportDOL(Path.mb_str());
	}

	if (!ret)
		PanicAlert("Failed to extract to %s!", Path.mb_str());
}

void CISOProperties::SetRefresh(wxCommandEvent& event)
{
	bRefreshList = true;

	if (event.GetId() == ID_EMUSTATE)
		EmuIssues->Enable(event.GetSelection() != 0);
}

void CISOProperties::LoadGameConfig()
{
	bool bTemp;
	int iTemp;
	std::string sTemp;

	if (GameIni.Get("Core", "CPUThread", &bTemp))
		CPUThread->Set3StateValue((wxCheckBoxState)bTemp);
	else
		CPUThread->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Core", "SkipIdle", &bTemp))
		SkipIdle->Set3StateValue((wxCheckBoxState)bTemp);
	else
		SkipIdle->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Core", "TLBHack", &bTemp))
		TLBHack->Set3StateValue((wxCheckBoxState)bTemp);
	else
		TLBHack->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Wii", "ProgressiveScan", &bTemp))
		EnableProgressiveScan->Set3StateValue((wxCheckBoxState)bTemp);
	else
		EnableProgressiveScan->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Wii", "Widescreen", &bTemp))
		EnableWideScreen->Set3StateValue((wxCheckBoxState)bTemp);
	else
		EnableWideScreen->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "ForceFiltering", &bTemp))
		ForceFiltering->Set3StateValue((wxCheckBoxState)bTemp);
	else
		ForceFiltering->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "EFBCopyDisable", &bTemp))
		EFBCopyDisable->Set3StateValue((wxCheckBoxState)bTemp);
	else
		EFBCopyDisable->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "EFBToTextureEnable", &bTemp))
		EFBToTextureEnable->Set3StateValue((wxCheckBoxState)bTemp);
	else
		EFBToTextureEnable->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "SafeTextureCache", &bTemp))
		SafeTextureCache->Set3StateValue((wxCheckBoxState)bTemp);
	else
		SafeTextureCache->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "DstAlphaPass", &bTemp))
		DstAlphaPass->Set3StateValue((wxCheckBoxState)bTemp);
	else
		DstAlphaPass->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "UseXFB", &bTemp))
		UseXFB->Set3StateValue((wxCheckBoxState)bTemp);
	else
		UseXFB->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("HLEaudio", "UseRE0Fix", &bTemp))
		UseRE0Fix->Set3StateValue((wxCheckBoxState)bTemp);
	else
		UseRE0Fix->Set3StateValue(wxCHK_UNDETERMINED);

	if (GameIni.Get("Video", "FIFOBPHack", &bTemp))
		BPHack->Set3StateValue((wxCheckBoxState)bTemp);
	else
		BPHack->Set3StateValue(wxCHK_UNDETERMINED);

	GameIni.Get("Video", "ProjectionHack", &iTemp, -1);
	Hack->SetSelection(iTemp);

	GameIni.Get("EmuState", "EmulationStateId", &iTemp, -1);
	EmuState->SetSelection(iTemp);

	GameIni.Get("EmuState", "EmulationIssues", &sTemp);
	if (!sTemp.empty())
	{
		EmuIssues->SetValue(wxString(sTemp.c_str(), *wxConvCurrent));
		bRefreshList = true;
	}
	EmuIssues->Enable(EmuState->GetSelection() != 0);

	PatchList_Load();
	ActionReplayList_Load();
}

bool CISOProperties::SaveGameConfig()
{
	if (CPUThread->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Core", "CPUThread");
	else
		GameIni.Set("Core", "CPUThread", CPUThread->Get3StateValue());

	if (SkipIdle->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Core", "SkipIdle");
	else
		GameIni.Set("Core", "SkipIdle", SkipIdle->Get3StateValue());

	if (TLBHack->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Core", "TLBHack");
	else
		GameIni.Set("Core", "TLBHack", TLBHack->Get3StateValue());

	if (EnableProgressiveScan->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Wii", "ProgressiveScan");
	else
		GameIni.Set("Wii", "ProgressiveScan", EnableProgressiveScan->Get3StateValue());

	if (EnableWideScreen->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Wii", "Widescreen");
	else
		GameIni.Set("Wii", "Widescreen", EnableWideScreen->Get3StateValue());

	if (ForceFiltering->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "ForceFiltering");
	else
		GameIni.Set("Video", "ForceFiltering", ForceFiltering->Get3StateValue());

	if (EFBCopyDisable->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "EFBCopyDisable");
	else
		GameIni.Set("Video", "EFBCopyDisable", EFBCopyDisable->Get3StateValue());

	if (EFBToTextureEnable->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "EFBToTextureEnable");
	else
		GameIni.Set("Video", "EFBToTextureEnable", EFBToTextureEnable->Get3StateValue());

	if (SafeTextureCache->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "SafeTextureCache");
	else
		GameIni.Set("Video", "SafeTextureCache", SafeTextureCache->Get3StateValue());

	if (DstAlphaPass->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "DstAlphaPass");
	else
		GameIni.Set("Video", "DstAlphaPass", DstAlphaPass->Get3StateValue());

	if (UseXFB->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "UseXFB");
	else
		GameIni.Set("Video", "UseXFB", UseXFB->Get3StateValue());

	if (UseRE0Fix->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("HLEaudio", "UseRE0Fix");
	else
		GameIni.Set("HLEaudio", "UseRE0Fix", UseRE0Fix->Get3StateValue());

	if (BPHack->Get3StateValue() == wxCHK_UNDETERMINED)
		GameIni.DeleteKey("Video", "FIFOBPHack");
	else
		GameIni.Set("Video", "FIFOBPHack", BPHack->Get3StateValue());

	if (EmuState->GetSelection() == -1)
		GameIni.DeleteKey("Video", "ProjectionHack");
	else
		GameIni.Set("Video", "ProjectionHack", Hack->GetSelection());

	if (EmuState->GetSelection() == -1)
		GameIni.DeleteKey("EmuState", "EmulationStateId");
	else
		GameIni.Set("EmuState", "EmulationStateId", EmuState->GetSelection());

	GameIni.Set("EmuState", "EmulationIssues", (const char*)EmuIssues->GetValue().mb_str(*wxConvCurrent));

	PatchList_Save();
	ActionReplayList_Save();

	return GameIni.Save(GameIniFile.c_str());
}

void CISOProperties::OnEditConfig(wxCommandEvent& WXUNUSED (event))
{
	if (wxFileExists(wxString::FromAscii(GameIniFile.c_str())))
	{
		SaveGameConfig();

		wxFileType* filetype = wxTheMimeTypesManager->GetFileTypeFromExtension(_("ini"));
		if(filetype == NULL) // From extension failed, trying with MIME type now
		{
			filetype = wxTheMimeTypesManager->GetFileTypeFromMimeType(_("text/plain"));
			if(filetype == NULL) // MIME type failed, aborting mission
			{
				PanicAlert("Filetype 'ini' is unknown! Will not open!");
				return;
			}
		}
		wxString OpenCommand;
		OpenCommand = filetype->GetOpenCommand(wxString::FromAscii(GameIniFile.c_str()));
		if(OpenCommand.IsEmpty())
			PanicAlert("Couldn't find open command for extension 'ini'!");
		else
			if(wxExecute(OpenCommand, wxEXEC_SYNC) == -1)
				PanicAlert("wxExecute returned -1 on application run!");

		GameIni.Load(GameIniFile.c_str());
		LoadGameConfig();

		bRefreshList = true; // Just in case
	}

	// Once we're done with the ini edit, give the focus back to Dolphin 
	SetFocus();
}

void CISOProperties::ListSelectionChanged(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	case ID_PATCHES_LIST:
		if (Patches->GetSelection() != wxNOT_FOUND)
		{
			EditPatch->Enable();
			RemovePatch->Enable();
		}
		break;
	case ID_CHEATS_LIST:
		if (Cheats->GetSelection() != wxNOT_FOUND)
		{
			EditCheat->Enable();
			RemoveCheat->Enable();
		}
		break;
	}
}

void CISOProperties::PatchList_Load()
{
	onFrame.clear();
	Patches->Clear();
	PatchEngine::LoadPatchSection("OnFrame", onFrame, GameIni);

	u32 index = 0;
	for (std::vector<PatchEngine::Patch>::const_iterator it = onFrame.begin(); it != onFrame.end(); ++it)
	{
		PatchEngine::Patch p = *it;
		Patches->Append(wxString(p.name.c_str(), *wxConvCurrent));
		Patches->Check(index, p.active);
		++index;
	}
}

void CISOProperties::PatchList_Save()
{
	std::vector<std::string> lines;
	u32 index = 0;
	for (std::vector<PatchEngine::Patch>::const_iterator onFrame_it = onFrame.begin(); onFrame_it != onFrame.end(); ++onFrame_it)
	{
		lines.push_back(Patches->IsChecked(index) ? "+$" + onFrame_it->name : "$" + onFrame_it->name);

		for (std::vector<PatchEngine::PatchEntry>::const_iterator iter2 = onFrame_it->entries.begin(); iter2 != onFrame_it->entries.end(); ++iter2)
		{
			std::string temp;
			
			ToStringFromFormat(&temp, "0x%08X:%s:0x%08X", iter2->address, PatchEngine::PatchTypeStrings[iter2->type], iter2->value);			
			lines.push_back(temp);
		}
		++index;
	}
	GameIni.SetLines("OnFrame", lines);
	lines.clear();
}

void CISOProperties::PatchButtonClicked(wxCommandEvent& event)
{
	int selection = Patches->GetSelection();
	
	switch (event.GetId())
	{
	case ID_EDITPATCH:
		{
		CPatchAddEdit dlg(selection, this);
		dlg.ShowModal();
		}
		break;
	case ID_ADDPATCH:
		{
		CPatchAddEdit dlg(-1, this, 1, _("��Ӳ���"));
		if (dlg.ShowModal() == wxID_OK)
		{
			Patches->Append(wxString(onFrame.back().name.c_str(), *wxConvCurrent));
			Patches->Check((unsigned int)(onFrame.size() - 1), onFrame.back().active);
		}
		}
		break;
	case ID_REMOVEPATCH:
		onFrame.erase(onFrame.begin() + Patches->GetSelection());
		Patches->Delete(Cheats->GetSelection());
		break;
	}

	PatchList_Save();
	Patches->Clear();
	PatchList_Load();

	EditPatch->Enable(false);
	RemovePatch->Enable(false);
}

void CISOProperties::ActionReplayList_Load()
{
	arCodes.clear();
	Cheats->Clear();
	ActionReplay::LoadCodes(arCodes, GameIni);

	u32 index = 0;
	for (std::vector<ActionReplay::ARCode>::const_iterator it = arCodes.begin(); it != arCodes.end(); ++it)
	{
		ActionReplay::ARCode arCode = *it;
		Cheats->Append(wxString(arCode.name.c_str(), *wxConvCurrent));
		Cheats->Check(index, arCode.active);
		++index;
	}
}

void CISOProperties::ActionReplayList_Save()
{
	std::vector<std::string> lines;
	u32 index = 0;
	for (std::vector<ActionReplay::ARCode>::const_iterator iter = arCodes.begin(); iter != arCodes.end(); ++iter)
	{
		ActionReplay::ARCode code = *iter;

		lines.push_back(Cheats->IsChecked(index) ? "+$" + code.name : "$" + code.name);

		for (std::vector<ActionReplay::AREntry>::const_iterator iter2 = code.ops.begin(); iter2 != code.ops.end(); ++iter2)
		{
			lines.push_back(std::string(wxString::Format(wxT("%08X %08X"), iter2->cmd_addr, iter2->value).mb_str()));
		}
		++index;
	}
	GameIni.SetLines("ActionReplay", lines);
}

void CISOProperties::ActionReplayButtonClicked(wxCommandEvent& event)
{
	int selection = Cheats->GetSelection();
	
	switch (event.GetId())
	{
	case ID_EDITCHEAT:
		{
		CARCodeAddEdit dlg(selection, this);
		dlg.ShowModal();
		}
		break;
	case ID_ADDCHEAT:
		{
			CARCodeAddEdit dlg(-1, this, 1, _("Add ActionReplay Code"));
			if (dlg.ShowModal() == wxID_OK)
			{
				Cheats->Append(wxString::FromAscii(arCodes.back().name.c_str()));
				Cheats->Check((unsigned int)(arCodes.size() - 1), arCodes.back().active);
			}
		}
		break;
	case ID_REMOVECHEAT:
		arCodes.erase(arCodes.begin() + Cheats->GetSelection());
		Cheats->Delete(Cheats->GetSelection());
		break;
	}

	ActionReplayList_Save();
	Cheats->Clear();
	ActionReplayList_Load();

	EditCheat->Enable(false);
	RemoveCheat->Enable(false);
}

void CISOProperties::OnChangeBannerLang(wxCommandEvent& event)
{
	ChangeBannerDetails(event.GetSelection());
}

void CISOProperties::ChangeBannerDetails(int lang)
{
	if (OpenGameListItem->GetCountry() == DiscIO::IVolume::COUNTRY_JAPAN
		|| OpenGameListItem->GetCountry() == DiscIO::IVolume::COUNTRY_TAIWAN
		|| OpenGameListItem->GetPlatform() == GameListItem::WII_WAD)
	{
		wxCSConv SJISConv(wxFontMapper::GetEncodingName(wxFONTENCODING_SHIFT_JIS));
		wxString name = wxString(OpenGameListItem->GetName(0).c_str(), SJISConv);

		// Updates the informations shown in the window
		m_ShortName->SetValue(name);
		m_Comment->SetValue(wxString(OpenGameListItem->GetDescription(0).c_str(), SJISConv));
		m_Maker->SetValue(wxString(OpenGameListItem->GetCompany().c_str(), SJISConv));//dev too

		std::string filename, extension;
		SplitPath(OpenGameListItem->GetFileName(), 0, &filename, &extension);

		// Also sets the window's title
		SetTitle(wxString::Format(wxT("%s%s"),
			wxString(StringFromFormat("%s%s: %s - ", filename.c_str(), extension.c_str(), OpenGameListItem->GetUniqueID().c_str()).c_str(), *wxConvCurrent).c_str(),
			name.c_str()));
	}
	else // Do the same for PAL/US Games using Ascii
	{
		wxString name = wxString::FromAscii(OpenGameListItem->GetName(lang).c_str());

		m_ShortName->SetValue(name);
		m_Comment->SetValue(wxString::FromAscii(OpenGameListItem->GetDescription(lang).c_str()));
		m_Maker->SetValue(wxString::FromAscii(OpenGameListItem->GetCompany().c_str()));//dev too

		std::string filename, extension;
		SplitPath(OpenGameListItem->GetFileName(), 0, &filename, &extension);

		SetTitle(wxString::Format(wxT("%s%s"),
			wxString::FromAscii(StringFromFormat("%s%s: %s - ", filename.c_str(), extension.c_str(), OpenGameListItem->GetUniqueID().c_str()).c_str()).c_str(),
			name.c_str()));
	}
}
