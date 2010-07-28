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

static void CheckHacksOverrides()
{
	if( !wxGetApp().Overrides.HasCustomHacks() ) return;
	
	// The user has commandline overrides enabled, so the options they see here and/or apply won't match
	// the commandline overrides.  Let them know!

	wxDialogWithHelpers dialog( wxFindWindowByName( L"Dialog:" + Dialogs::SysConfigDialog::GetNameStatic() ), _("设置覆盖警告") );
	
	dialog += dialog.Text( pxE(".Panel:HasHacksOverrides",
		L"Warning!  You are running PCSX2 with command line options that override your configured settings.  "
		L"These command line options will not be reflected in the Settings dialog, and will be disabled "
		L"if you apply any changes here."
	));

	// [TODO] : List command line option overrides in action?

	pxIssueConfirmation( dialog, MsgButtons().OK(), L"Dialog.SysConfig.Overrides" );
}

static void CheckPluginsOverrides()
{
	if( !wxGetApp().Overrides.HasPluginsOverride() ) return;
	
	// The user has commandline overrides enabled, so the options they see here and/or apply won't match
	// the commandline overrides.  Let them know!

	wxDialogWithHelpers dialog( NULL, _("组件覆盖警告") );
	
	dialog += dialog.Text( pxE(".Panel:HasPluginsOverrides",
		L"Warning!  You are running PCSX2 with command line options that override your configured plugin and/or folder settings.  "
		L"These command line options will not be reflected in the settings dialog, and will be disabled "
		L"when you apply settings changes here."
	));

	// [TODO] : List command line option overrides in action?

	pxIssueConfirmation( dialog, MsgButtons().OK(), L"Dialog.ComponentsConfig.Overrides" );
}

Dialogs::SysConfigDialog::SysConfigDialog(wxWindow* parent)
	: BaseConfigurationDialog( parent, AddAppName(_("模拟设置 - %s")), 580 )
{
	ScopedBusyCursor busy( Cursor_ReallyBusy );

	CreateListbook( wxGetApp().GetImgList_Config() );
	const AppImageIds::ConfigIds& cfgid( wxGetApp().GetImgId().Config );

	AddPage<CpuPanelEE>				( wxT("EE/IOP"),		cfgid.Cpu );
	AddPage<CpuPanelVU>				( wxT("VUs"),			cfgid.Cpu );
	AddPage<VideoPanel>				( wxT("视频图像"),		cfgid.Cpu );
	AddPage<GSWindowSettingsPanel>	( wxT("视频窗口"),		cfgid.Video );
	AddPage<SpeedHacksPanel>		( wxT("游戏加速"),		cfgid.Speedhacks );
	AddPage<GameFixesPanel>			( wxT("游戏修正"),		cfgid.Gamefixes );
//	AddPage<GameDatabasePanel>		( wxT("游戏数据库"),	cfgid.Plugins );

	AddListbook();
	AddOkCancel();

	if( wxGetApp().Overrides.HasCustomHacks() )
		wxGetApp().PostMethod( CheckHacksOverrides );
}

Dialogs::ComponentsConfigDialog::ComponentsConfigDialog(wxWindow* parent)
	: BaseConfigurationDialog( parent, AddAppName(_("组件选择 - %s")),  600 )
{
	ScopedBusyCursor busy( Cursor_ReallyBusy );

	CreateListbook( wxGetApp().GetImgList_Config() );
	const AppImageIds::ConfigIds& cfgid( wxGetApp().GetImgId().Config );

	AddPage<PluginSelectorPanel>	( wxT("插件"),		cfgid.Plugins );
	AddPage<BiosSelectorPanel>		( wxT("BIOS"),			cfgid.Cpu );
	AddPage<StandardPathsPanel>		( wxT("文件夹"),		cfgid.Paths );

	AddListbook();
	AddOkCancel();

	if( wxGetApp().Overrides.HasPluginsOverride() )
		wxGetApp().PostMethod( CheckPluginsOverrides );
}
