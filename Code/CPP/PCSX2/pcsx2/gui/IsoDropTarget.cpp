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
	ScopedCoreThreadSuspend stopped_core;

	if( filenames.GetCount() > 1 )
	{
		wxDialogWithHelpers dialog( m_WindowBound, _("Drag and Drop Error"), wxVERTICAL );
		dialog += dialog.Heading( _("It is an error to drop multiple files onto a PCSX2 window.  One at a time please, thank you.") );
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
		throw Exception::CreateStream( filenames[0] );

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
			wxDialogWithHelpers dialog( m_WindowBound, _("ȷ�� PS2 ����"), wxVERTICAL );
			
			dialog += dialog.Heading(
				_("You have dropped the following ELF binary into PCSX2:\n\n") +
				filenames[0] + L"\n\n" + GetMsg_ConfirmSysReset()
			);

			confirmed = (pxIssueConfirmation( dialog, MsgButtons().Reset().Cancel(), L"DragDrop:BootELF" ) != wxID_CANCEL);
		}

		if( confirmed )
		{
			sApp.SysExecute( g_Conf->CdvdSource, g_Conf->CurrentELF );
		}

		stopped_core.Resume();
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
	memzero( iso );
	iso.handle = _openfile( filenames[0].ToUTF8(), O_RDONLY);

	if( iso.handle == NULL )
		throw Exception::CreateStream( filenames[0] );

	if (isoDetect(&iso))
	{
		Console.WriteLn( L"(Drag&Drop) �ҵ���Ч ISO �ļ�����!" );

		wxWindowID result = wxID_RESET;

		if( SysHasValidState() )
		{
			wxDialogWithHelpers dialog( m_WindowBound, _("ȷ�� PS2 ����"), wxVERTICAL );
						
			dialog += dialog.Heading(_("You have dropped the following ISO image into PCSX2:\n\n") +
				filenames[0] + L"\n\n" +
				_("Do you want to swap discs or boot the new image (via system reset)?")
			);

			result = pxIssueConfirmation( dialog, MsgButtons().Reset().Cancel().Custom(_("Swap Disc")), L"DragDrop:BootIso" );
		}

		if( result != wxID_CANCEL )
		{
			SysUpdateIsoSrcFile( filenames[0] );
			if( result != wxID_RESET )
			{
				CoreThread.ChangeCdvdSource( CDVDsrc_Iso );
			}
			else
			{
				sApp.SysExecute( CDVDsrc_Iso );
			}
		}
	}

	_closefile( iso.handle );
	stopped_core.Resume();
	return true;
}
