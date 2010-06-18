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

#include "ModalPopups.h"

using namespace pxSizerFlags;

Dialogs::ImportSettingsDialog::ImportSettingsDialog( wxWindow* parent )
	: wxDialogWithHelpers( parent, _("������������?") )
{
	SetMinWidth( 440 );

	pxStaticText& heading = Text( pxE( ".Popup:ImportExistingSettings",
		L"ģ�����������Ѿ����ڵ������ļ�r.  "
		L"�����뵼���������û���ʹ��Ĭ�����ø��ǵ�����?"
		L"\n\n(���ߵ��ȡ��ѡ��һ����ͬ�������ļ���)" )
	);

	wxBoxSizer& s_buttons = *new wxBoxSizer( wxHORIZONTAL );
	wxButton* b_import	= new wxButton( this, wxID_ANY, _("����") );
	wxButton* b_over	= new wxButton( this, wxID_ANY, _("����") );

	// --------------------------------------------------------------------
	// Layout Some Shizat...

	s_buttons += b_import	| StdButton();
	s_buttons += 16;
	s_buttons += b_over		| StdButton();
	s_buttons += 16;
	s_buttons += new wxButton( this, wxID_CANCEL ) | StdButton();

	*this += 4;
	*this += heading		| StdExpand();
	*this += 12;
	*this += &s_buttons		| StdCenter();

	Connect( b_import->GetId(),	wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImportSettingsDialog::OnImport_Click) );
	Connect( b_over->GetId(),	wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImportSettingsDialog::OnOverwrite_Click) );
}

void Dialogs::ImportSettingsDialog::OnImport_Click( wxCommandEvent& /* evt */ )
{
	AppConfig_OnChangedSettingsFolder( false );		// ... and import existing settings
	g_Conf->Folders.Bios.Mkdir();
	EndModal( wxID_OK );
}

void Dialogs::ImportSettingsDialog::OnOverwrite_Click( wxCommandEvent& /* evt */ )
{
	AppConfig_OnChangedSettingsFolder( true );		// ... and overwrite any existing settings
	g_Conf->Folders.Bios.Mkdir();
	EndModal( wxID_OK );
}
