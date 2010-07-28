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
#include "IsoDropTarget.h"

#include "Dialogs/ModalPopups.h"

#include "CDVD/IsoFileFormats.h"

#include <wx/wfstream.h>


wxString GetMsg_ConfirmSysReset()
{
	return pxE( ".Popup:ConfirmSysReset",
		L"����������������Ѵ��� PS2 �����״̬; "
		L"���е�ǰ���Ƚ��ᶪʧ.�Ƿ�ȷ��?"
	);
}

bool IsoDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	ScopedCoreThreadPopup stopped_core;

	if( filenames.GetCount() > 1 )
	{
		wxDialogWithHelpers dialog( m_WindowBound, _("Drag and Drop Error") );
		dialog += dialog.Heading(AddAppName(_("It is an error to drop multiple files onto a %s window.  One at a time please, thank you.")));
		pxIssueConfirmation( dialog, MsgButtons().Cancel() );
		return false;
	}

	Console.WriteLn( L"(Drag&Drop) �����ļ���: " + filenames[0] );

	// ---------------
	//    ELF CHECK
	// ---------------
	{
	wxFileInputStream filechk( filenames[0] );

	if( !filechk.IsOk() )
		throw Exception::CannotCreateStream( filenames[0] );

	u8 ident[16];
	filechk.Read( ident, 16 );
	static const u8 elfIdent[4] = { 0x7f, 'E', 'L', 'F' };

	if( ((u32&)ident) == ((u32&)elfIdent) )
	{
		Console.WriteLn( L"(Drag&Drop) �ҵ� ELF �ļ�����!" );

		g_Conf->CurrentELF = filenames[0];

		bool confirmed = true;
		if( SysHasValidState() )
		{
			wxDialogWithHelpers dialog( m_WindowBound, _("ȷ�� PS2 ����") );

			dialog += dialog.Heading(AddAppName(_("You have dropped the following ELF binary into %s:\n\n")));
			dialog += dialog.GetCharHeight();
			dialog += dialog.Text( filenames[0] );
			dialog += dialog.GetCharHeight();
			dialog += dialog.Heading(GetMsg_ConfirmSysReset());

			confirmed = (pxIssueConfirmation( dialog, MsgButtons().Reset().Cancel(), L"DragDrop.BootELF" ) != wxID_CANCEL);
		}

		if( confirmed )
		{
			g_Conf->EmuOptions.UseBOOT2Injection = true;
			sApp.SysExecute( g_Conf->CdvdSource, g_Conf->CurrentELF );
		}
		else
			stopped_core.AllowResume();

		return true;
	}
	}

	// ---------------
	//    ISO CHECK
	// ---------------

	// FIXME : The whole IsoFileFormats api (meaning isoOpen / isoDetect / etc) needs to be
	//   converted to C++ and wxInputStream .  Until then this is a nasty little exception unsafe
	//   hack ;)

	isoFile iso;
	memzero(iso);
	iso.handle = _openfile(filenames[0].ToUTF8(), O_RDONLY);

	if( iso.handle == NULL )
		throw Exception::CannotCreateStream( filenames[0] );

	if (isoDetect(&iso))
	{
		Console.WriteLn( L"(Drag&Drop) �ҵ���Ч ISO �ļ�����!" );
		SwapOrReset_Iso(m_WindowBound, stopped_core, filenames[0], AddAppName(_("You have dropped the following ISO image into %s:")));
	}

	_closefile( iso.handle );
	return true;
}
