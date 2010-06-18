/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "System.h"
#include "App.h"

#include "ConfigurationDialog.h"
#include "BaseConfigurationDialog.inl"
#include "ModalPopups.h"
#include "Panels/ConfigurationPanels.h"

using namespace Panels;

Dialogs::SysConfigDialog::SysConfigDialog(wxWindow* parent)
	: BaseConfigurationDialog( parent, _("PS2 设置 - PCSX2"), 580 )
{
	ScopedBusyCursor busy( Cursor_ReallyBusy );

	CreateListbook( wxGetApp().GetImgList_Config() );
	const AppImageIds::ConfigIds& cfgid( wxGetApp().GetImgId().Config );

	AddPage<CpuPanelEE>				( wxT("EE/IOP"),		cfgid.Cpu );
	AddPage<CpuPanelVU>				( wxT("VUs"),			cfgid.Cpu );
	AddPage<VideoPanel>				( wxT("视频图像"),		cfgid.Cpu );
	AddPage<GSWindowSettingsPanel>	( wxT("视频窗口"),			cfgid.Video );
	AddPage<SpeedHacksPanel>		( wxT("游戏加速"),		cfgid.Speedhacks );
	AddPage<GameFixesPanel>			( wxT("游戏修正"),		cfgid.Gamefixes );
	AddPage<GameDatabasePanel>		( wxT("游戏数据库"),cfgid.Plugins );

	AddListbook();
	AddOkCancel();
}

Dialogs::ComponentsConfigDialog::ComponentsConfigDialog(wxWindow* parent)
	: BaseConfigurationDialog( parent, _("应用程序设置 - PCSX2"),  600 )
{
	ScopedBusyCursor busy( Cursor_ReallyBusy );

	CreateListbook( wxGetApp().GetImgList_Config() );
	const AppImageIds::ConfigIds& cfgid( wxGetApp().GetImgId().Config );

	AddPage<PluginSelectorPanel>	( wxT("插件"),		cfgid.Plugins );
	AddPage<BiosSelectorPanel>		( wxT("BIOS"),			cfgid.Cpu );
	AddPage<StandardPathsPanel>		( wxT("文件夹"),		cfgid.Paths );

	AddListbook();
	AddOkCancel();
}
