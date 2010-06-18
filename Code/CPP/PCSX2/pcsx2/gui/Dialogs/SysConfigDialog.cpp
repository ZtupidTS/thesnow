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
	: BaseConfigurationDialog( parent, _("PS2 ���� - PCSX2"), 580 )
{
	ScopedBusyCursor busy( Cursor_ReallyBusy );

	CreateListbook( wxGetApp().GetImgList_Config() );
	const AppImageIds::ConfigIds& cfgid( wxGetApp().GetImgId().Config );

	AddPage<CpuPanelEE>				( wxT("EE/IOP"),		cfgid.Cpu );
	AddPage<CpuPanelVU>				( wxT("VUs"),			cfgid.Cpu );
	AddPage<VideoPanel>				( wxT("��Ƶͼ��"),		cfgid.Cpu );
	AddPage<GSWindowSettingsPanel>	( wxT("��Ƶ����"),			cfgid.Video );
	AddPage<SpeedHacksPanel>		( wxT("��Ϸ����"),		cfgid.Speedhacks );
	AddPage<GameFixesPanel>			( wxT("��Ϸ����"),		cfgid.Gamefixes );
	AddPage<GameDatabasePanel>		( wxT("��Ϸ���ݿ�"),cfgid.Plugins );

	AddListbook();
	AddOkCancel();
}

Dialogs::ComponentsConfigDialog::ComponentsConfigDialog(wxWindow* parent)
	: BaseConfigurationDialog( parent, _("Ӧ�ó������� - PCSX2"),  600 )
{
	ScopedBusyCursor busy( Cursor_ReallyBusy );

	CreateListbook( wxGetApp().GetImgList_Config() );
	const AppImageIds::ConfigIds& cfgid( wxGetApp().GetImgId().Config );

	AddPage<PluginSelectorPanel>	( wxT("���"),		cfgid.Plugins );
	AddPage<BiosSelectorPanel>		( wxT("BIOS"),			cfgid.Cpu );
	AddPage<StandardPathsPanel>		( wxT("�ļ���"),		cfgid.Paths );

	AddListbook();
	AddOkCancel();
}
