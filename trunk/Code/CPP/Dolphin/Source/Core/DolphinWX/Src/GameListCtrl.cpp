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

#include "Globals.h"

#include <wx/imaglist.h>
#include <wx/fontmap.h>

#include <algorithm>

#include "FileSearch.h"
#include "StringUtil.h"
#include "ConfigManager.h"
#include "GameListCtrl.h"
#include "Blob.h"
#include "Core.h"
#include "ISOProperties.h"
#include "IniFile.h"
#include "FileUtil.h"
#include "CDUtils.h"
#include "WxUtils.h"
#include "Main.h"

#include "../resources/Flag_Europe.xpm"
#include "../resources/Flag_France.xpm"
#include "../resources/Flag_Italy.xpm"
#include "../resources/Flag_Japan.xpm"
#include "../resources/Flag_USA.xpm"
#include "../resources/Flag_Taiwan.xpm"
#include "../resources/Flag_Korea.xpm"
#include "../resources/Flag_Unknown.xpm"
#include "../resources/Platform_Wad.xpm"
#include "../resources/Platform_Wii.xpm"
#include "../resources/Platform_Gamecube.xpm"
#include "../resources/rating_gamelist.h"

size_t CGameListCtrl::m_currentItem = 0;
size_t CGameListCtrl::m_numberItem = 0;
std::string CGameListCtrl::m_currentFilename;

static int currentColumn = 0;
bool operator < (const GameListItem &one, const GameListItem &other)
{
	int indexOne = 0;
	int indexOther = 0;

	switch (one.GetCountry())
	{
		case DiscIO::IVolume::COUNTRY_JAPAN:
		case DiscIO::IVolume::COUNTRY_USA:
			indexOne = 0;
			break;
		default:
			indexOne = SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage;
	}

	switch (other.GetCountry())
	{
		case DiscIO::IVolume::COUNTRY_JAPAN:
		case DiscIO::IVolume::COUNTRY_USA:
			indexOther = 0;
			break;
		default:
			indexOther = SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage;
	}

	switch (currentColumn)
	{
		case CGameListCtrl::COLUMN_TITLE:
			return strcasecmp(one.GetName(indexOne).c_str(),
					other.GetName(indexOther).c_str()) < 0;
		case CGameListCtrl::COLUMN_NOTES:
			{
				// On Gamecube we show the company string, while it's empty on
				// other platforms, so we show the description instead
				std::string cmp1 =
					(one.GetPlatform() == GameListItem::GAMECUBE_DISC) ?
					one.GetCompany() : one.GetDescription(indexOne);
				std::string cmp2 =
					(other.GetPlatform() == GameListItem::GAMECUBE_DISC) ?
					other.GetCompany() : other.GetDescription(indexOther);
				return strcasecmp(cmp1.c_str(), cmp2.c_str()) < 0;
			}
		case CGameListCtrl::COLUMN_COUNTRY:
			return (one.GetCountry() < other.GetCountry());
		case CGameListCtrl::COLUMN_SIZE:
			return (one.GetFileSize() < other.GetFileSize());
		case CGameListCtrl::COLUMN_PLATFORM:
			return (one.GetPlatform() < other.GetPlatform());
		default:
			return strcasecmp(one.GetName(indexOne).c_str(),
					other.GetName(indexOther).c_str()) < 0;
	}
}


BEGIN_EVENT_TABLE(wxEmuStateTip, wxTipWindow)
	EVT_KEY_DOWN(wxEmuStateTip::OnKeyDown)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CGameListCtrl, wxListCtrl)
#ifdef _WIN32
	EVT_PAINT(CGameListCtrl::OnPaintDrawImages)
#endif
	EVT_SIZE(CGameListCtrl::OnSize)
	EVT_RIGHT_DOWN(CGameListCtrl::OnRightClick)
	EVT_LEFT_DOWN(CGameListCtrl::OnLeftClick)
	EVT_LIST_KEY_DOWN(LIST_CTRL, CGameListCtrl::OnKeyPress)
	EVT_MOTION(CGameListCtrl::OnMouseMotion)
	EVT_LIST_COL_BEGIN_DRAG(LIST_CTRL, CGameListCtrl::OnColBeginDrag)
	EVT_LIST_COL_CLICK(LIST_CTRL, CGameListCtrl::OnColumnClick)
	EVT_MENU(IDM_PROPERTIES, CGameListCtrl::OnProperties)
	EVT_MENU(IDM_OPENCONTAININGFOLDER, CGameListCtrl::OnOpenContainingFolder)
	EVT_MENU(IDM_OPENSAVEFOLDER, CGameListCtrl::OnOpenSaveFolder)
	EVT_MENU(IDM_EXPORTSAVE, CGameListCtrl::OnExportSave)
	EVT_MENU(IDM_SETDEFAULTGCM, CGameListCtrl::OnSetDefaultGCM)
	EVT_MENU(IDM_COMPRESSGCM, CGameListCtrl::OnCompressGCM)
	EVT_MENU(IDM_MULTICOMPRESSGCM, CGameListCtrl::OnMultiCompressGCM)
	EVT_MENU(IDM_MULTIDECOMPRESSGCM, CGameListCtrl::OnMultiDecompressGCM)
	EVT_MENU(IDM_DELETEGCM, CGameListCtrl::OnDeleteGCM)
	EVT_MENU(IDM_INSTALLWAD, CGameListCtrl::OnInstallWAD)
END_EVENT_TABLE()


CGameListCtrl::CGameListCtrl(wxWindow* parent, const wxWindowID id, const
		wxPoint& pos, const wxSize& size, long style)
	: wxListCtrl(parent, id, pos, size, style), toolTip(0)
{
}

CGameListCtrl::~CGameListCtrl()
{
	if (m_imageListSmall)
		delete m_imageListSmall;
}

void CGameListCtrl::InitBitmaps()
{
	m_imageListSmall = new wxImageList(96, 32);
	SetImageList(m_imageListSmall, wxIMAGE_LIST_SMALL);

	m_FlagImageIndex.resize(DiscIO::IVolume::NUMBER_OF_COUNTRIES);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_EUROPE] =
		m_imageListSmall->Add(wxBitmap(Flag_Europe_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_FRANCE] =
		m_imageListSmall->Add(wxBitmap(Flag_France_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_USA] =
		m_imageListSmall->Add(wxBitmap(Flag_USA_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_JAPAN] =
		m_imageListSmall->Add(wxBitmap(Flag_Japan_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_KOREA] =
		m_imageListSmall->Add(wxBitmap(Flag_Korea_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_ITALY] =
		m_imageListSmall->Add(wxBitmap(Flag_Italy_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_TAIWAN] =
		m_imageListSmall->Add(wxBitmap(Flag_Taiwan_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_SDK] =
		m_imageListSmall->Add(wxBitmap(Flag_Unknown_xpm), wxNullBitmap);
	m_FlagImageIndex[DiscIO::IVolume::COUNTRY_UNKNOWN] =
		m_imageListSmall->Add(wxBitmap(Flag_Unknown_xpm), wxNullBitmap);

	m_PlatformImageIndex.resize(3);
	m_PlatformImageIndex[0] =
		m_imageListSmall->Add(wxBitmap(Platform_Gamecube_xpm), wxNullBitmap);
	m_PlatformImageIndex[1] =
		m_imageListSmall->Add(wxBitmap(Platform_Wii_xpm), wxNullBitmap);
	m_PlatformImageIndex[2] =
		m_imageListSmall->Add(wxBitmap(Platform_Wad_xpm), wxNullBitmap);

	m_EmuStateImageIndex.resize(6);
	m_EmuStateImageIndex[0] =
		m_imageListSmall->Add(wxBitmap(rating_0), wxNullBitmap);
	m_EmuStateImageIndex[1] =
		m_imageListSmall->Add(wxBitmap(rating_1), wxNullBitmap);
	m_EmuStateImageIndex[2] =
		m_imageListSmall->Add(wxBitmap(rating_2), wxNullBitmap);
	m_EmuStateImageIndex[3] =
		m_imageListSmall->Add(wxBitmap(rating_3), wxNullBitmap);
	m_EmuStateImageIndex[4] =
		m_imageListSmall->Add(wxBitmap(rating_4), wxNullBitmap);
	m_EmuStateImageIndex[5] =
		m_imageListSmall->Add(wxBitmap(rating_5), wxNullBitmap);
}

void CGameListCtrl::BrowseForDirectory()
{
	wxString dirHome;
	wxGetHomeDir(&dirHome);

	// browse
	wxDirDialog dialog(this, _("浏览要添加的目录"), dirHome,
			wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (dialog.ShowModal() == wxID_OK)
	{
		std::string sPath(dialog.GetPath().mb_str());
		std::vector<std::string>::iterator itResult = std::find(
				SConfig::GetInstance().m_ISOFolder.begin(),
				SConfig::GetInstance().m_ISOFolder.end(), sPath);

		if (itResult == SConfig::GetInstance().m_ISOFolder.end())
		{
			SConfig::GetInstance().m_ISOFolder.push_back(sPath);
			SConfig::GetInstance().SaveSettings();
		}

		Update();
	}
}

void CGameListCtrl::Update()
{
	// Don't let the user refresh it while a game is running
	if (Core::GetState() != Core::CORE_UNINITIALIZED)
		return;

	if (m_imageListSmall)
	{
		delete m_imageListSmall;
		m_imageListSmall = NULL;
	}

	// NetPlay : Set/Reset the GameList string
	m_gameList.clear();
	m_gamePath.clear();

	Hide();

	ScanForISOs();

	ClearAll();

	if (m_ISOFiles.size() != 0)
	{
		// Don't load bitmaps unless there are games to list
		InitBitmaps();

		// add columns
		InsertColumn(COLUMN_PLATFORM, _T(""));
		InsertColumn(COLUMN_BANNER, _("图标"));
		InsertColumn(COLUMN_TITLE, _("标题"));

		// Instead of showing the notes + the company, which is unknown with
		// wii titles We show in the same column : company for GC games and
		// description for wii/wad games
		InsertColumn(COLUMN_NOTES, _("备注"));
		InsertColumn(COLUMN_COUNTRY, _T(""));
		InsertColumn(COLUMN_SIZE, _("大小"));
		InsertColumn(COLUMN_EMULATION_STATE, _("状态"));


		// set initial sizes for columns
		SetColumnWidth(COLUMN_PLATFORM, 35);
		SetColumnWidth(COLUMN_BANNER, 96);
		SetColumnWidth(COLUMN_TITLE, 200);
		SetColumnWidth(COLUMN_NOTES, 200);
		SetColumnWidth(COLUMN_COUNTRY, 32);
		SetColumnWidth(COLUMN_EMULATION_STATE, 50);

		// add all items
		for (int i = 0; i < (int)m_ISOFiles.size(); i++)
		{
			InsertItemInReportView(i);
			if (m_ISOFiles[i].IsCompressed())
				SetItemTextColour(i, wxColour(0xFF0000));
		}

		// Sort items by Title
		wxListEvent event;
		event.m_col = COLUMN_TITLE; last_column = 0;
		OnColumnClick(event);

		SetColumnWidth(COLUMN_SIZE, wxLIST_AUTOSIZE);
	}
	else
	{
		wxString errorString;
		// We just check for one hide setting to be enabled, as we may only
		// have GC games for example, and hide them, so we should show the
		// second message instead
		if ((SConfig::GetInstance().m_ListGC  &&
			SConfig::GetInstance().m_ListWii  &&
			SConfig::GetInstance().m_ListWad) &&
			(SConfig::GetInstance().m_ListJap &&
			SConfig::GetInstance().m_ListUsa  &&
			SConfig::GetInstance().m_ListPal))
		{
			errorString = _("Dolphin 不能找到任何 GC/Wii ISO.  双击这里浏览文件...");
		}
		else
		{
			errorString = _("Dolphin 当前设置隐藏所有游戏.  双击这里显示所有游戏...");
		}
		InsertColumn(0, _("没有找到 ISO 或者 WADS"));
		long index = InsertItem(0, errorString);
		SetItemFont(index, *wxITALIC_FONT);
		SetColumnWidth(0, wxLIST_AUTOSIZE);
	}

	Show();

	AutomaticColumnWidth();
}

wxString NiceSizeFormat(s64 _size)
{
	const char* sizes[] = {"b", "KB", "MB", "GB", "TB", "PB", "EB"};
	int s = 0;
	int frac = 0;

	while (_size > (s64)1024)
	{
		s++;
		frac   = (int)_size & 1023;
		_size /= (s64)1024;
	}

	float f = (float)_size + ((float)frac / 1024.0f);

	wxString NiceString;
	char tempstr[32];
	sprintf(tempstr,"%3.1f %s", f, sizes[s]);
	NiceString = wxString::FromAscii(tempstr);
	return(NiceString);
}

std::string CGameListCtrl::GetGamePaths() const
{
	return m_gamePath;
}
std::string CGameListCtrl::GetGameNames() const
{
	return m_gameList;
}

#ifdef _WIN32
// This draws our icons on top of the gamelist, it's only used on Windows
void CGameListCtrl::OnPaintDrawImages(wxPaintEvent& event)
{
	int i = 0, nState = 0;
	IniFile ini;
	wxRect itemRect;
	wxPaintDC dc(this);

	// Calls the default drawing code
	wxControl::OnPaint(event);

	// Draw the flags, platform icons and emustate icons on top if there's games to show
	if (m_ISOFiles.size() != 0)
	{
		// Retrieve the topmost shown item and get drawing offsets
		long top = GetTopItem();
		int flagOffset = GetColumnWidth(0) + GetColumnWidth(1) +
			GetColumnWidth(2) + GetColumnWidth(3);
		int stateOffset = flagOffset + GetColumnWidth(4) + GetColumnWidth(5);

		// Only redraw shown lines
		for (i = top; i < top + GetCountPerPage() + 2; i++)
		{
			if (GetItemRect(i, itemRect))
			{
				int itemY = itemRect.GetTop();
				const GameListItem& rISOFile = m_ISOFiles[GetItemData(i)];

				m_imageListSmall->Draw(m_PlatformImageIndex[rISOFile.GetPlatform()],
						dc, itemRect.GetX()+3, itemY);
				m_imageListSmall->Draw(m_FlagImageIndex[rISOFile.GetCountry()],
						dc, flagOffset, itemY);

				ini.Load((std::string(File::GetUserPath(D_GAMECONFIG_IDX)) +
							(rISOFile.GetUniqueID()) + ".ini").c_str());
				ini.Get("EmuState", "EmulationStateId", &nState);
				m_imageListSmall->Draw(m_EmuStateImageIndex[nState],
						dc, stateOffset, itemY);
			}
		}
	}
}
#endif

void CGameListCtrl::InsertItemInReportView(long _Index)
{
	// When using wxListCtrl, there is no hope of per-column text colors.
	// But for reference, here are the old colors that were used: (BGR)
	// title: 0xFF0000
	// company: 0x007030
	int ImageIndex = -1;
#ifdef _WIN32
	wxCSConv SJISConv(wxFontMapper::GetEncodingName(wxFONTENCODING_SHIFT_JIS));
#else
	wxCSConv SJISConv(wxFontMapper::GetEncodingName(wxFONTENCODING_EUC_JP));
#endif
	GameListItem& rISOFile = m_ISOFiles[_Index];
	m_gamePath.append(rISOFile.GetFileName() + '\n');

	// Insert a first row with the platform image, that will be used as the Index
#ifndef _WIN32
	long ItemIndex = InsertItem(_Index, wxEmptyString,
			m_PlatformImageIndex[rISOFile.GetPlatform()]);
#else
	long ItemIndex = InsertItem(_Index, wxEmptyString, -1);
#endif

	if (rISOFile.GetImage().IsOk())
		ImageIndex = m_imageListSmall->Add(rISOFile.GetImage());

	// Set the game's banner in the second column
	SetItemColumnImage(_Index, COLUMN_BANNER, ImageIndex);

	if (rISOFile.GetPlatform() != GameListItem::WII_WAD)
	{
		std::string company;

		// We show the company string on Gamecube only
		// On Wii we show the description instead as the company string is empty
		if (rISOFile.GetPlatform() == GameListItem::GAMECUBE_DISC)
			company = rISOFile.GetCompany().c_str();

		switch (rISOFile.GetCountry())
		{
		case DiscIO::IVolume::COUNTRY_TAIWAN:
		case DiscIO::IVolume::COUNTRY_JAPAN:
			{
				wxString name = wxString(rISOFile.GetName(0).c_str(), SJISConv);
				m_gameList.append(StringFromFormat("%s (J)\n", (const char *)name.c_str()));
				SetItem(_Index, COLUMN_TITLE, name, -1);
				SetItem(_Index, COLUMN_NOTES, wxString(company.size() ?
							company.c_str() : rISOFile.GetDescription(0).c_str(),
							SJISConv), -1);
			}
			break;
		case DiscIO::IVolume::COUNTRY_USA:
			m_gameList.append(StringFromFormat("%s (U)\n", rISOFile.GetName(0).c_str()));
			SetItem(_Index, COLUMN_TITLE,
				wxString::From8BitData(rISOFile.GetName(0).c_str()), -1);
			SetItem(_Index, COLUMN_NOTES,
				wxString::From8BitData(company.size() ?
					company.c_str() : rISOFile.GetDescription(0).c_str()), -1);
			break;
		default:
			m_gameList.append(StringFromFormat("%s (E)\n",
				rISOFile.GetName(SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage).c_str()));
			SetItem(_Index, COLUMN_TITLE,
					wxString::From8BitData(
						rISOFile.GetName(SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage).c_str()),
					-1);
			SetItem(_Index, COLUMN_NOTES,
					wxString::From8BitData(company.size() ?
						company.c_str() :
						rISOFile.GetDescription(SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage).c_str()),
					-1);
			break;
		}
	}
	else // It's a Wad file
	{
		m_gameList.append(StringFromFormat("%s (WAD)\n", rISOFile.GetName(0).c_str()));
		SetItem(_Index, COLUMN_TITLE,
				wxString(rISOFile.GetName(0).c_str(), SJISConv), -1);
		SetItem(_Index, COLUMN_NOTES,
				wxString(rISOFile.GetDescription(0).c_str(), SJISConv), -1);
	}

#ifndef _WIN32
	// Load the INI file for columns that read from it
	IniFile ini;
	ini.Load((std::string(File::GetUserPath(D_GAMECONFIG_IDX)) +
				(rISOFile.GetUniqueID()) + ".ini").c_str());

	// Emulation status
	int nState;
	ini.Get("EmuState", "EmulationStateId", &nState);

	// Emulation state
	SetItemColumnImage(_Index, COLUMN_EMULATION_STATE, m_EmuStateImageIndex[nState]);

	// Country
	SetItemColumnImage(_Index, COLUMN_COUNTRY, m_FlagImageIndex[rISOFile.GetCountry()]);
#endif

	// File size
	SetItem(_Index, COLUMN_SIZE, NiceSizeFormat(rISOFile.GetFileSize()), -1);

	// Background color
	SetBackgroundColor();

	// Item data
	SetItemData(_Index, ItemIndex);
}

wxColour blend50(const wxColour& c1, const wxColour& c2)
{
	unsigned char r,g,b,a;
	r = c1.Red()/2   + c2.Red()/2;
	g = c1.Green()/2 + c2.Green()/2;
	b = c1.Blue()/2  + c2.Blue()/2;
	a = c1.Alpha()/2 + c2.Alpha()/2;
	return a << 24 | b << 16 | g << 8 | r;
}

void CGameListCtrl::SetBackgroundColor()
{
	for(long i = 0; i < GetItemCount(); i++)
	{
		wxColour color = (i & 1) ?
			blend50(wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT),
					wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)) :
			wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
		CGameListCtrl::SetItemBackgroundColour(i, color);
	}
}

void CGameListCtrl::ScanForISOs()
{
	m_ISOFiles.clear();
	CFileSearch::XStringVector Directories(SConfig::GetInstance().m_ISOFolder);

	if (SConfig::GetInstance().m_RecursiveISOFolder)
	{
		for (u32 i = 0; i < Directories.size(); i++)
		{
			File::FSTEntry FST_Temp;
			File::ScanDirectoryTree(Directories.at(i).c_str(), FST_Temp);
			for (u32 j = 0; j < FST_Temp.children.size(); j++)
			{
				if (FST_Temp.children.at(j).isDirectory)
				{
					bool duplicate = false;
					for (u32 k = 0; k < Directories.size(); k++)
					{
						if (strcmp(Directories.at(k).c_str(),
									FST_Temp.children.at(j).physicalName.c_str()) == 0)
						{
							duplicate = true;
							break;
						}
					}
					if (!duplicate)
						Directories.push_back(
								FST_Temp.children.at(j).physicalName.c_str());
				}
			}
		}
	}

	CFileSearch::XStringVector Extensions;

	if (SConfig::GetInstance().m_ListGC)
		Extensions.push_back("*.gcm");
	if (SConfig::GetInstance().m_ListWii || SConfig::GetInstance().m_ListGC)
	{
		Extensions.push_back("*.iso");
		Extensions.push_back("*.data");	//added
		Extensions.push_back("*.ciso");
		Extensions.push_back("*.gcz");
	}
	if (SConfig::GetInstance().m_ListWad)
		Extensions.push_back("*.wad");

	CFileSearch FileSearch(Extensions, Directories);
	const CFileSearch::XStringVector& rFilenames = FileSearch.GetFileNames();

	if (rFilenames.size() > 0)
	{
		wxProgressDialog dialog(_("扫描 ISO 中"),
					_("扫描中..."),
					(int)rFilenames.size(), // range
					this, // parent
					wxPD_APP_MODAL |
					wxPD_ELAPSED_TIME |
					wxPD_ESTIMATED_TIME |
					wxPD_REMAINING_TIME |
					wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
					);
		dialog.CenterOnParent();

		for (u32 i = 0; i < rFilenames.size(); i++)
		{
			std::string FileName;
			SplitPath(rFilenames[i], NULL, &FileName, NULL);

			wxString msg;
			char tempstring[128];
			sprintf(tempstring,"Scanning %s", FileName.c_str());
			msg = wxString(tempstring, *wxConvCurrent);

			// Update with the progress (i) and the message (msg)
			bool Cont = dialog.Update(i, msg);
			if (!Cont)
				break;

			GameListItem ISOFile(rFilenames[i]);
			if (ISOFile.IsValid())
			{
				bool list = true;

				switch(ISOFile.GetPlatform())
				{
					case GameListItem::WII_DISC:
						if (!SConfig::GetInstance().m_ListWii)
							list = false;
						break;
					case GameListItem::WII_WAD:
						if (!SConfig::GetInstance().m_ListWad)
							list = false;
						break;
					default:
						if (!SConfig::GetInstance().m_ListGC)
							list = false;
						break;
				}

				switch(ISOFile.GetCountry())
				{
					case DiscIO::IVolume::COUNTRY_TAIWAN:
						if (!SConfig::GetInstance().m_ListTaiwan)
							list = false;
					case DiscIO::IVolume::COUNTRY_KOREA:
						if (!SConfig::GetInstance().m_ListKorea)
							list = false;
						break;
					case DiscIO::IVolume::COUNTRY_JAPAN:
						if (!SConfig::GetInstance().m_ListJap)
							list = false;
						break;
					case DiscIO::IVolume::COUNTRY_USA:
						if (!SConfig::GetInstance().m_ListUsa)
							list = false;
						break;
					case DiscIO::IVolume::COUNTRY_FRANCE:
						if (!SConfig::GetInstance().m_ListFrance)
							list = false;
						break;
					case DiscIO::IVolume::COUNTRY_ITALY:
						if (!SConfig::GetInstance().m_ListItaly)
							list = false;
						break;
					default:
						if (!SConfig::GetInstance().m_ListPal)
							list = false;
						break;
				}

				if (list)
					m_ISOFiles.push_back(ISOFile);
			}
		}
	}

	if (SConfig::GetInstance().m_ListDrives)
	{
		std::vector<std::string> drives = cdio_get_devices();
		GameListItem * Drive[24];
		// Another silly Windows limitation of 24 drive letters
		for (u32 i = 0; i < drives.size() && i < 24; i++)
		{
			Drive[i] = new GameListItem(drives[i].c_str());
			if (Drive[i]->IsValid())
				m_ISOFiles.push_back(*Drive[i]);
		}
	}

	std::sort(m_ISOFiles.begin(), m_ISOFiles.end());
}

void CGameListCtrl::OnColBeginDrag(wxListEvent& event)
{
	if (event.GetColumn() != COLUMN_TITLE && event.GetColumn() != COLUMN_NOTES)
		event.Veto();
}

const GameListItem *CGameListCtrl::GetISO(int index) const
{
	return &m_ISOFiles[index];
}

CGameListCtrl *caller;
int wxCALLBACK wxListCompare(long item1, long item2, long sortData)
{
	// return 1 if item1 > item2
	// return -1 if item1 < item2
	// return 0 for identity
	const GameListItem *iso1 = caller->GetISO(item1);
	const GameListItem *iso2 = caller->GetISO(item2);

	int t = 1;

	if (sortData < 0)
	{
		t = -1;
		sortData = -sortData;
	}

	int indexOne = 0;
	int indexOther = 0;

	switch (iso1->GetCountry())
	{
		case DiscIO::IVolume::COUNTRY_JAPAN:
		case DiscIO::IVolume::COUNTRY_USA:
			indexOne = 0;
			break;
		default:
			indexOne = SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage;
	}

	switch (iso2->GetCountry())
	{
		case DiscIO::IVolume::COUNTRY_JAPAN:
		case DiscIO::IVolume::COUNTRY_USA:
			indexOther = 0;
			break;
		default:
			indexOther = SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage;
	}

	switch(sortData)
	{
		case CGameListCtrl::COLUMN_TITLE:
			return strcasecmp(iso1->GetName(indexOne).c_str(),
					iso2->GetName(indexOther).c_str()) * t;
		case CGameListCtrl::COLUMN_NOTES:
			{
				std::string cmp1 =
					(iso1->GetPlatform() == GameListItem::GAMECUBE_DISC) ?
					iso1->GetCompany() : iso1->GetDescription(indexOne);
				std::string cmp2 =
					(iso2->GetPlatform() == GameListItem::GAMECUBE_DISC) ?
					iso2->GetCompany() : iso2->GetDescription(indexOther);
				return strcasecmp(cmp1.c_str(), cmp2.c_str()) * t;
			}
		case CGameListCtrl::COLUMN_COUNTRY:
			if(iso1->GetCountry() > iso2->GetCountry())
				return  1 * t;
			if(iso1->GetCountry() < iso2->GetCountry())
				return -1 * t;
			return 0;
		case CGameListCtrl::COLUMN_SIZE:
			if (iso1->GetFileSize() > iso2->GetFileSize())
				return  1 * t;
			if (iso1->GetFileSize() < iso2->GetFileSize())
				return -1 * t;
			return 0;
		case CGameListCtrl::COLUMN_PLATFORM:
			if(iso1->GetPlatform() > iso2->GetPlatform())
				return  1 * t;
			if(iso1->GetPlatform() < iso2->GetPlatform())
				return -1 * t;
			return 0;
		case CGameListCtrl::COLUMN_EMULATION_STATE:
			IniFile ini;
			int nState1 = 0, nState2 = 0;
			std::string GameIni1 = std::string(File::GetUserPath(D_GAMECONFIG_IDX)) +
				iso1->GetUniqueID() + ".ini";
			std::string GameIni2 = std::string(File::GetUserPath(D_GAMECONFIG_IDX)) +
				iso2->GetUniqueID() + ".ini";

			ini.Load(GameIni1.c_str());
			ini.Get("EmuState", "EmulationStateId", &nState1);
			ini.Load(GameIni2.c_str());
			ini.Get("EmuState", "EmulationStateId", &nState2);

			if(nState1 > nState2)
				return  1 * t;
			if(nState1 < nState2)
				return -1 * t;
			return 0;
	}

	return 0;
}

void CGameListCtrl::OnColumnClick(wxListEvent& event)
{
	if(event.GetColumn() != COLUMN_BANNER)
	{
		int current_column = event.GetColumn();

		if(last_column == current_column)
		{
			last_sort = -last_sort;
		}
		else
		{
			last_column = current_column;
			last_sort = current_column;
		}

		caller = this;
		SortItems(wxListCompare, last_sort);
	}

	SetBackgroundColor();

	event.Skip();
}

// This is used by keyboard gamelist search
void CGameListCtrl::OnKeyPress(wxListEvent& event)
{
	static int lastKey = 0, sLoop = 0;
	int Loop = 0;

	for (int i = 0; i < (int)m_ISOFiles.size(); i++)
	{
		// Easy way to get game string
		wxListItem bleh;
		bleh.SetId(i);
		bleh.SetColumn(COLUMN_TITLE);
		bleh.SetMask(wxLIST_MASK_TEXT);
		GetItem(bleh);

		wxString text = bleh.GetText();

		if (text.MakeUpper().at(0) == event.GetKeyCode())
		{
			if (lastKey == event.GetKeyCode() && Loop < sLoop)
			{
				Loop++;
				if (i+1 == (int)m_ISOFiles.size())
					i = -1;
				continue;
			}
			else if (lastKey != event.GetKeyCode())
				sLoop = 0;

			lastKey = event.GetKeyCode();
			sLoop++;

			UnselectAll();
			SetItemState(i, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED,
					wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			EnsureVisible(i);
			break;
		}

		// If we get past the last game in the list,
		// we'll have to go back to the first one.
		if (i+1 == (int)m_ISOFiles.size() && sLoop > 0 && Loop > 0)
			i = -1;
	}

	event.Skip();
}

// This shows a little tooltip with the current Game's emulation state
void CGameListCtrl::OnMouseMotion(wxMouseEvent& event)
{
	int flags;
	long subitem = 0;
	long item = HitTest(event.GetPosition(), flags, &subitem);
	static int lastItem = -1;

	if (item != wxNOT_FOUND)
	{
		if (subitem == COLUMN_EMULATION_STATE)
		{
			if (toolTip || lastItem == item || this != FindFocus())
			{
				event.Skip();
				return;
			}

			const GameListItem& rISO = m_ISOFiles[GetItemData(item)];

			IniFile ini;
			ini.Load((std::string(File::GetUserPath(D_GAMECONFIG_IDX)) +
						(rISO.GetUniqueID()) + ".ini").c_str());

			// Emulation status
			std::string emuState[5] = {
				"Broken",
				"Intro",
				"In-Game",
				"Playable",
				"Perfect"
			};
			std::string issues;

			int nState;
			ini.Get("EmuState", "EmulationStateId", &nState);
			ini.Get("EmuState", "EmulationIssues", &issues, "");

			// Get item Coords then convert from wxWindow coord to Screen coord
			wxRect Rect;
			this->GetItemRect(item, Rect);
			int mx = Rect.GetWidth();
			int my = Rect.GetY();
			this->ClientToScreen(&mx, &my);

			// Show a tooltip containing the EmuState and the state description
			if (nState > 0 && nState < 6)
			{
				char temp[2048];
				sprintf(temp, "^ %s%s%s", emuState[nState -1].c_str(),
						issues.size() > 0 ? " :\n" : "", issues.c_str());
				toolTip = new wxEmuStateTip(this, wxString(temp, *wxConvCurrent), &toolTip);
			}
			else
				toolTip = new wxEmuStateTip(this, _("Not Set"), &toolTip);

			toolTip->SetBoundingRect(wxRect(mx - GetColumnWidth(subitem),
						my, GetColumnWidth(subitem), Rect.GetHeight()));
			toolTip->SetPosition(wxPoint(mx - GetColumnWidth(subitem),
						my - 5 + Rect.GetHeight()));

			lastItem = item;
		}
	}

	event.Skip();
}

void CGameListCtrl::OnLeftClick(wxMouseEvent& event)
{
	// Focus the clicked item.
	int flags;
	long item = HitTest(event.GetPosition(), flags);
	if ((item != wxNOT_FOUND) && (GetSelectedItemCount() == 0) &&
			(!event.ControlDown()) && (!event.ShiftDown()))
	{
		SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
		SetItemState(item, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
		wxGetApp().GetCFrame()->UpdateGUI();
	}

	event.Skip();
}

void CGameListCtrl::OnRightClick(wxMouseEvent& event)
{
	// Focus the clicked item.
	int flags;
	long item = HitTest(event.GetPosition(), flags);
	if (item != wxNOT_FOUND)
	{
		if (GetItemState(item, wxLIST_STATE_SELECTED) != wxLIST_STATE_SELECTED)
		{
			UnselectAll();
			SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
		}
		SetItemState(item, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
	}

	if (GetSelectedItemCount() == 1)
	{
		const GameListItem *selected_iso = GetSelectedISO();
		if (selected_iso)
		{
			wxMenu* popupMenu = new wxMenu;
			popupMenu->Append(IDM_PROPERTIES, _("&Properties"));
			popupMenu->AppendSeparator();

			if (selected_iso->GetPlatform() != GameListItem::GAMECUBE_DISC)
			{
				popupMenu->Append(IDM_OPENSAVEFOLDER, _("Open Wii &save folder"));
				popupMenu->Append(IDM_EXPORTSAVE, _("Export Wii save (Experimental)"));
			}
			popupMenu->Append(IDM_OPENCONTAININGFOLDER, _("Open &containing folder"));
			popupMenu->AppendCheckItem(IDM_SETDEFAULTGCM, _("Set as &default ISO"));

			// First we have to decide a starting value when we append it
			if(selected_iso->GetFileName() == SConfig::GetInstance().
				m_LocalCoreStartupParameter.m_strDefaultGCM)
				popupMenu->FindItem(IDM_SETDEFAULTGCM)->Check();

			popupMenu->AppendSeparator();
			popupMenu->Append(IDM_DELETEGCM, _("&Delete ISO..."));

			if (selected_iso->GetPlatform() != GameListItem::WII_WAD)
			{
				if (selected_iso->IsCompressed())
					popupMenu->Append(IDM_COMPRESSGCM, _("Decompress ISO..."));
				else
					popupMenu->Append(IDM_COMPRESSGCM, _("Compress ISO..."));
			} else
				popupMenu->Append(IDM_INSTALLWAD, _("Install to Wii Menu"));

			PopupMenu(popupMenu);
		}
	}
	else if (GetSelectedItemCount() > 1)
	{
		wxMenu* popupMenu = new wxMenu;
		popupMenu->Append(IDM_DELETEGCM, _("&Delete selected ISOs..."));
		popupMenu->AppendSeparator();
		popupMenu->Append(IDM_MULTICOMPRESSGCM, _("Compress selected ISOs..."));
		popupMenu->Append(IDM_MULTIDECOMPRESSGCM, _("Decompress selected ISOs..."));
		PopupMenu(popupMenu);
	}
}

const GameListItem * CGameListCtrl::GetSelectedISO()
{
	if (m_ISOFiles.size() == 0)
		return NULL;
	else if (GetSelectedItemCount() == 0)
		return NULL;
	else
	{
		long item = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == wxNOT_FOUND)
			return new GameListItem("");
		else
		{
			// Here is a little workaround for multiselections:
			// when > 1 item is selected, return info on the first one
			// and deselect it so the next time GetSelectedISO() is called,
			// the next item's info is returned
			if (GetSelectedItemCount() > 1)
				SetItemState(item, 0, wxLIST_STATE_SELECTED);

			return &m_ISOFiles[GetItemData(item)];
		}
	}
}

void CGameListCtrl::OnOpenContainingFolder(wxCommandEvent& WXUNUSED (event))
{
	const GameListItem *iso = GetSelectedISO();
	if (!iso)
		return;
	std::string path;
	SplitPath(iso->GetFileName(), &path, 0, 0);
	WxUtils::Explore(path.c_str());
}

void CGameListCtrl::OnOpenSaveFolder(wxCommandEvent& WXUNUSED (event))
{
	const GameListItem *iso = GetSelectedISO();
	if (!iso)
		return;
	std::string path = iso->GetWiiFSPath();
	if (!path.empty())
		WxUtils::Explore(path.c_str());
}

void CGameListCtrl::OnExportSave(wxCommandEvent& WXUNUSED (event))
{
	const GameListItem *iso =  GetSelectedISO();
	if (!iso)
		return;
	u64 title;
	DiscIO::IVolume *Iso = DiscIO::CreateVolumeFromFilename(iso->GetFileName());
	if (Iso)
	{
		if (Iso->GetTitleID((u8*)&title))
		{
			title = Common::swap64(title);
			CWiiSaveCrypted* exportSave = new CWiiSaveCrypted("", title);
			delete exportSave;
		}
		delete Iso;
	}
}

// Save this file as the default file
void CGameListCtrl::OnSetDefaultGCM(wxCommandEvent& event)
{
	const GameListItem *iso = GetSelectedISO();
	if (!iso) return;

	if (event.IsChecked())
	{
		// Write the new default value and save it the ini file
		SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDefaultGCM =
			iso->GetFileName();
		SConfig::GetInstance().SaveSettings();
	}
	else
	{
		// Othwerise blank the value and save it
		SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDefaultGCM = "";
		SConfig::GetInstance().SaveSettings();
	}
}

void CGameListCtrl::OnDeleteGCM(wxCommandEvent& WXUNUSED (event))
{
	if (GetSelectedItemCount() == 1)
	{
		const GameListItem *iso = GetSelectedISO();
		if (!iso)
			return;
		if (wxMessageBox(_("Are you sure you want to delete this file?  It will be gone forever!"),
					wxMessageBoxCaptionStr, wxYES_NO | wxICON_EXCLAMATION) == wxYES)
		{
			File::Delete(iso->GetFileName().c_str());
			Update();
		}
	}
	else
	{
		if (wxMessageBox(_("Are you sure you want to delete these files?\nThey will be gone forever!"),
					wxMessageBoxCaptionStr, wxYES_NO | wxICON_EXCLAMATION) == wxYES)
		{
			int selected = GetSelectedItemCount();

			for (int i = 0; i < selected; i++)
			{
				const GameListItem *iso = GetSelectedISO();
				File::Delete(iso->GetFileName().c_str());
			}
			Update();
		}
	}
}

void CGameListCtrl::OnProperties(wxCommandEvent& WXUNUSED (event))
{
	const GameListItem *iso = GetSelectedISO();
	if (!iso)
		return;
	CISOProperties ISOProperties(iso->GetFileName(), this);
	ISOProperties.Center();
	if(ISOProperties.ShowModal() == wxID_OK)
		Update();
}

void CGameListCtrl::OnInstallWAD(wxCommandEvent& WXUNUSED (event))
{
	const GameListItem *iso = GetSelectedISO();
	if (!iso)
		return;

	wxProgressDialog dialog(_("Installing WAD to Wii Menu..."),
		_("Working..."),
		1000, // range
		this, // parent
		wxPD_APP_MODAL |
		wxPD_ELAPSED_TIME |
		wxPD_ESTIMATED_TIME |
		wxPD_REMAINING_TIME |
		wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
		);

	dialog.CenterOnParent();

	CBoot::Install_WiiWAD(iso->GetFileName().c_str());
}

void CGameListCtrl::MultiCompressCB(const char* text, float percent, void* arg)
{
	percent = (((float)m_currentItem) + percent) / (float)m_numberItem;
	wxString textString(StringFromFormat("%s (%i/%i) - %s",
				m_currentFilename.c_str(), (int)m_currentItem+1,
				(int)m_numberItem, text).c_str(), *wxConvCurrent);

	((wxProgressDialog*)arg)->Update((int)(percent*1000), textString);
}

void CGameListCtrl::OnMultiCompressGCM(wxCommandEvent& /*event*/)
{
	CompressSelection(true);
}

void CGameListCtrl::OnMultiDecompressGCM(wxCommandEvent& /*event*/)
{
	CompressSelection(false);
}

void CGameListCtrl::CompressSelection(bool _compress)
{
	wxString dirHome;
	wxGetHomeDir(&dirHome);

	wxDirDialog browseDialog(this, _("Browse for output directory"), dirHome,
			wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	if (browseDialog.ShowModal() != wxID_OK)
		return;

	wxProgressDialog progressDialog(_compress ?
			_("Compressing ISO") : _("Decompressing ISO"),
			_("Working..."),
			1000, // range
			this, // parent
			wxPD_APP_MODAL |
			wxPD_ELAPSED_TIME |
			wxPD_ESTIMATED_TIME |
			wxPD_REMAINING_TIME |
			wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
			);

	progressDialog.SetSize(wxSize(340, 180));
	progressDialog.CenterOnParent();

	m_currentItem = 0;
	m_numberItem = GetSelectedItemCount();
	for (u32 i=0; i < m_numberItem; i++)
	{
		const GameListItem *iso = GetSelectedISO();

			if (!iso->IsCompressed() && _compress)
			{
				std::string FileName, FileExt;
				SplitPath(iso->GetFileName(), NULL, &FileName, &FileExt);
				m_currentFilename = FileName;
				FileName.append(".gcz");

				std::string OutputFileName;
				BuildCompleteFilename(OutputFileName,
						(const char *)browseDialog.GetPath().mb_str(wxConvUTF8),
						FileName);

				if (wxFileExists(wxString::FromAscii(OutputFileName.c_str())) &&
						wxMessageBox(
							_("The file ") + wxString::FromAscii(OutputFileName.c_str()) +
							_(" already exists.\nDo you wish to replace it?"), 
							_("Confirm File Overwrite"),
							wxYES_NO) == wxNO)
					continue;

				DiscIO::CompressFileToBlob(iso->GetFileName().c_str(),
						OutputFileName.c_str(),
						(iso->GetPlatform() == GameListItem::WII_DISC) ? 1 : 0,
						16384, &MultiCompressCB, &progressDialog);
			}
			else if (iso->IsCompressed() && !_compress)
			{
				std::string FileName, FileExt;
				SplitPath(iso->GetFileName(), NULL, &FileName, &FileExt);
				m_currentFilename = FileName;
				if (iso->GetPlatform() == GameListItem::WII_DISC)
					FileName.append(".iso");
				else
					FileName.append(".gcm");

				std::string OutputFileName;
				BuildCompleteFilename(OutputFileName,
						(const char *)browseDialog.GetPath().mb_str(wxConvUTF8),
						FileName);

				if (wxFileExists(wxString::FromAscii(OutputFileName.c_str())) &&
						wxMessageBox(
							_("The file ") + wxString::FromAscii(OutputFileName.c_str()) +
							_(" already exists.\nDo you wish to replace it?"), 
							_("Confirm File Overwrite"),
							wxYES_NO) == wxNO)
					continue;

				DiscIO::DecompressBlobToFile(iso->GetFileName().c_str(),
						OutputFileName.c_str(), &MultiCompressCB, &progressDialog);
			}
			m_currentItem++;
	}
	Update();
}

void CGameListCtrl::CompressCB(const char* text, float percent, void* arg)
{
	((wxProgressDialog*)arg)->
		Update((int)(percent*1000), wxString(text, *wxConvCurrent));
}

void CGameListCtrl::OnCompressGCM(wxCommandEvent& WXUNUSED (event))
{
	const GameListItem *iso = GetSelectedISO();
	if (!iso)
		return;

	wxString path;

	std::string FileName, FilePath, FileExtension;
	SplitPath(iso->GetFileName(), &FilePath, &FileName, &FileExtension);

	do
	{
		if (iso->IsCompressed())
		{
			wxString FileType;
			if (iso->GetPlatform() == GameListItem::WII_DISC)
				FileType = _("All Wii ISO files (iso)") + wxString(wxT("|*.iso"));
			else
				FileType = _("All Gamecube GCM files (gcm)") + wxString(wxT("|*.gcm"));

			path = wxFileSelector(
					_("Save decompressed GCM/ISO"),
					wxString(FilePath.c_str(), *wxConvCurrent),
					wxString(FileName.c_str(), *wxConvCurrent) + FileType.After('*'),
					wxEmptyString,
					FileType + wxT("|") + wxGetTranslation(wxALL_FILES),
					wxFD_SAVE,
					this);
		}
		else
		{
			path = wxFileSelector(
					_("Save compressed GCM/ISO"),
					wxString(FilePath.c_str(), *wxConvCurrent),
					wxString(FileName.c_str(), *wxConvCurrent) + _T(".gcz"),
					wxEmptyString,
					_("All compressed GC/Wii ISO files (gcz)") + 
						wxString::Format(wxT("|*.gcz|%s"), wxGetTranslation(wxALL_FILES)),
					wxFD_SAVE,
					this);
		}
		if (!path)
			return;
	} while (wxFileExists(path) &&
			wxMessageBox(
				_("The file ") + path + _(" already exists.\nDo you wish to replace it?"), 
				_("Confirm File Overwrite"),
				wxYES_NO) == wxNO);

	wxProgressDialog dialog(iso->IsCompressed() ?
			_("Decompressing ISO") : _("Compressing ISO"),
			_("Working..."),
			1000, // range
			this, // parent
			wxPD_APP_MODAL |
			wxPD_ELAPSED_TIME |
			wxPD_ESTIMATED_TIME |
			wxPD_REMAINING_TIME |
			wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
			);

	dialog.SetSize(wxSize(340, 180));
	dialog.CenterOnParent();

	if (iso->IsCompressed())
		DiscIO::DecompressBlobToFile(iso->GetFileName().c_str(),
				path.char_str(), &CompressCB, &dialog);
	else
		DiscIO::CompressFileToBlob(iso->GetFileName().c_str(),
				path.char_str(),
				(iso->GetPlatform() == GameListItem::WII_DISC) ? 1 : 0,
				16384, &CompressCB, &dialog);

	Update();
}

void CGameListCtrl::OnSize(wxSizeEvent& event)
{
	if (lastpos == event.GetSize()) return;
	lastpos = event.GetSize();
	AutomaticColumnWidth();

	event.Skip();
}

void CGameListCtrl::AutomaticColumnWidth()
{
	wxRect rc(GetClientRect());

	if (GetColumnCount() == 1)
		SetColumnWidth(0, rc.GetWidth());
	else if (GetColumnCount() > 4)
	{
		int resizable = rc.GetWidth() - (
			GetColumnWidth(COLUMN_BANNER)
			+ GetColumnWidth(COLUMN_COUNTRY)
			+ GetColumnWidth(COLUMN_SIZE)
			+ GetColumnWidth(COLUMN_EMULATION_STATE)
			+ GetColumnWidth(COLUMN_PLATFORM));

		// We hide the Notes column if the window is too small
		if (resizable > 400)
		{
			SetColumnWidth(COLUMN_TITLE, resizable / 2);
			SetColumnWidth(COLUMN_NOTES, resizable / 2);
		}
		else
		{
			SetColumnWidth(COLUMN_TITLE, resizable);
			SetColumnWidth(COLUMN_NOTES, 0);
		}
	}
}

void CGameListCtrl::UnselectAll()
{
	for (int i=0; i<GetItemCount(); i++)
	{
		SetItemState(i, 0, wxLIST_STATE_SELECTED);
	}

}


