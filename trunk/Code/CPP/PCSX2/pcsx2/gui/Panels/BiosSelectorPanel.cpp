/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
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
#include "App.h"
#include "ConfigurationPanels.h"

#include "Utilities/ScopedPtr.h"
#include "ps2/BiosTools.h"

#include <wx/dir.h>
#include <wx/filepicker.h>
#include <wx/listbox.h>

// =====================================================================================================
//  BaseSelectorPanel
// =====================================================================================================
Panels::BaseSelectorPanel::BaseSelectorPanel( wxWindow* parent )
	: BaseApplicableConfigPanel( parent, wxVERTICAL )
{
	Connect( wxEVT_COMMAND_DIRPICKER_CHANGED,	wxFileDirPickerEventHandler	(BaseSelectorPanel::OnFolderChanged) );
	//Connect( wxEVT_ACTIVATE,					wxActivateEventHandler		(BaseSelectorPanel::OnActivate) );
	Connect( wxEVT_SHOW,						wxShowEventHandler			(BaseSelectorPanel::OnShow) );
}

Panels::BaseSelectorPanel::~BaseSelectorPanel() throw()
{
}

void Panels::BaseSelectorPanel::OnActivate(wxActivateEvent& evt)
{
	evt.Skip();
	if( !evt.GetActive() ) return;
	OnShown();
}

void Panels::BaseSelectorPanel::OnShow(wxShowEvent& evt)
{
	evt.Skip();
	if( !evt.GetShow() ) return;
	OnShown();
}

void Panels::BaseSelectorPanel::OnShown()
{
	if( !ValidateEnumerationStatus() )
		DoRefresh();
}

bool Panels::BaseSelectorPanel::Show( bool visible )
{
	if( visible )
		OnShown();

	return BaseApplicableConfigPanel::Show( visible );
}

void Panels::BaseSelectorPanel::OnRefresh( wxCommandEvent& evt )
{
	ValidateEnumerationStatus();
	DoRefresh();
}

void Panels::BaseSelectorPanel::OnFolderChanged( wxFileDirPickerEvent& evt )
{
	evt.Skip();
	OnShown();
}

// =====================================================================================================
//  BiosSelectorPanel
// =====================================================================================================
Panels::BiosSelectorPanel::BiosSelectorPanel( wxWindow* parent, int idealWidth )
	: BaseSelectorPanel( parent )
	, m_ComboBox( *new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_SORT | wxLB_NEEDED_SB ) )
	, m_FolderPicker( *new DirPickerPanel( this, FolderId_Bios,
		_("BIOS 搜索路径:"),						// static box label
		_("选择 PS2 BIOS ROM所在的文件夹")		// dir picker popup label
	) )
{
	if( idealWidth != wxDefaultCoord ) m_idealWidth = idealWidth;

	m_ComboBox.SetFont( wxFont( m_ComboBox.GetFont().GetPointSize()+1, wxFONTFAMILY_MODERN, wxNORMAL, wxNORMAL, false, L"Lucida Console" ) );
	m_ComboBox.SetMinSize( wxSize( wxDefaultCoord, std::max( m_ComboBox.GetMinSize().GetHeight(), 96 ) ) );

	m_FolderPicker.SetStaticDesc( _("点击浏览按钮选择 PS2 BIOS ROM所在的路径.") );

	*this	+= Text(_("选择一个 BIOS ROM:"));
	*this	+= m_ComboBox		| pxSizerFlags::StdExpand();
	*this	+= 6;
	*this	+= m_FolderPicker	| pxSizerFlags::StdExpand();
}

Panels::BiosSelectorPanel::~BiosSelectorPanel() throw ()
{
}

void Panels::BiosSelectorPanel::Apply()
{
	int sel = m_ComboBox.GetSelection();
	if( sel == wxNOT_FOUND )
	{
		throw Exception::CannotApplySettings( this,
			// English Log
			L"用户不能指定一个有效 BIOS 选择.",

			// Translated
			pxE( ".Popup Error:Invalid BIOS Selection",
				L"请选择一个有效 BIOS. 如果您不能选择一个有效的BIOS,"
				L"请按取消关闭这个设置面板."
			)
		);
	}

	g_Conf->BaseFilenames.Bios = (*m_BiosList)[(int)m_ComboBox.GetClientData(sel)];
}

void Panels::BiosSelectorPanel::AppStatusEvent_OnSettingsApplied()
{
}

bool Panels::BiosSelectorPanel::ValidateEnumerationStatus()
{
	bool validated = true;

	// Impl Note: ScopedPtr used so that resources get cleaned up if an exception
	// occurs during file enumeration.
	ScopedPtr<wxArrayString> bioslist( new wxArrayString() );

	if( m_FolderPicker.GetPath().Exists() )
		wxDir::GetAllFiles( m_FolderPicker.GetPath().ToString(), bioslist, L"*.*", wxDIR_FILES );

	if( !m_BiosList || (*bioslist != *m_BiosList) )
		validated = false;

	m_BiosList.SwapPtr( bioslist );

	return validated;
}

void Panels::BiosSelectorPanel::DoRefresh()
{
	if( !m_BiosList ) return;

	m_ComboBox.Clear();

	wxFileName right( g_Conf->FullpathToBios() );
	right.MakeAbsolute();

	for( size_t i=0; i<m_BiosList->GetCount(); ++i )
	{
		wxString description;
		if( !IsBIOS((*m_BiosList)[i], description) ) continue;
		int sel = m_ComboBox.Append( description, (void*)i );

		wxFileName left( (*m_BiosList)[i] );
		left.MakeAbsolute();

		if( left == right )
			m_ComboBox.SetSelection( sel );
	}
}
