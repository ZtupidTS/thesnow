#include <filezilla.h>

#include "../Options.h"
#include "settingsdialog.h"
#include "optionspage.h"
#include "optionspage_edit.h"

BEGIN_EVENT_TABLE(COptionsPageEdit, COptionsPage)
EVT_BUTTON(XRCID("ID_BROWSE"), COptionsPageEdit::OnBrowseEditor)
EVT_RADIOBUTTON(wxID_ANY, COptionsPageEdit::OnRadioButton)
END_EVENT_TABLE()

bool COptionsPageEdit::LoadPage()
{
	bool failure = false;

	COptions* pOptions = COptions::Get();

	wxString editor = pOptions->GetOption(OPTION_EDIT_DEFAULTEDITOR);
	if (editor.empty() || editor[0] == '0')
		SetRCheck(XRCID("ID_DEFAULT_NONE"), true, failure);
	else if (editor[0] == '1')
		SetRCheck(XRCID("ID_DEFAULT_TEXT"), true, failure);
	else
	{
		if (editor[0] == '2')
			editor = editor.Mid(1);

		SetRCheck(XRCID("ID_DEFAULT_CUSTOM"), true, failure);
		SetText(XRCID("ID_EDITOR"), editor, failure);
	}

	if (pOptions->GetOptionVal(OPTION_EDIT_ALWAYSDEFAULT))
		SetRCheck(XRCID("ID_USEDEFAULT"), true, failure);
	else
		SetRCheck(XRCID("ID_USEASSOCIATIONS"), true, failure);

	SetCheckFromOption(XRCID("ID_EDIT_TRACK_LOCAL"), OPTION_EDIT_TRACK_LOCAL, failure);

	if (!failure)
		SetCtrlState();

	return !failure;
}

bool COptionsPageEdit::SavePage()
{
	COptions* pOptions = COptions::Get();

	if (GetRCheck(XRCID("ID_DEFAULT_CUSTOM")))
		pOptions->SetOption(OPTION_EDIT_DEFAULTEDITOR, _T("2") + GetText(XRCID("ID_EDITOR")));
	else
		pOptions->SetOption(OPTION_EDIT_DEFAULTEDITOR, GetRCheck(XRCID("ID_DEFAULT_TEXT")) ? _T("1") : _T("0"));

	if (GetRCheck(XRCID("ID_USEDEFAULT")))
		pOptions->SetOption(OPTION_EDIT_ALWAYSDEFAULT, 1);
	else
		pOptions->SetOption(OPTION_EDIT_ALWAYSDEFAULT, 0);

	SetOptionFromCheck(XRCID("ID_EDIT_TRACK_LOCAL"), OPTION_EDIT_TRACK_LOCAL);

	return true;
}

bool UnquoteCommand(wxString& command, wxString& arguments, bool is_dde = false)
{
	arguments = _T("");

	if (command == _T(""))
		return true;

	wxChar inQuotes = 0;
	wxString file;
	for (unsigned int i = 0; i < command.Len(); i++)
	{
		const wxChar& c = command[i];
		if (c == '"' || c == '\'')
		{
			if (!inQuotes)
				inQuotes = c;
			else if (c != inQuotes)
				file += c;
			else if (command[i + 1] == c)
			{
				file += c;
				i++;
			}
			else
				inQuotes = false;
		}
		else if (command[i] == ' ' && !inQuotes)
		{
			arguments = command.Mid(i + 1);
			arguments.Trim(false);
			break;
		}
		else if (is_dde && !inQuotes && (command[i] == ',' || command[i] == '#'))
		{
			arguments = command.Mid(i + 1);
			arguments.Trim(false);
			break;
		}
		else
			file += command[i];
	}
	if (inQuotes)
		return false;

	command = file;

	return true;
}

bool ProgramExists(const wxString& editor)
{
	if (wxFileName::FileExists(editor))
		return true;

#ifdef __WXMAC__
	if (editor.Right(4) == _T(".app") && wxFileName::DirExists(editor))
		return true;
#endif

	return false;
}

bool COptionsPageEdit::Validate()
{
	const bool custom = GetRCheck(XRCID("ID_DEFAULT_CUSTOM"));
	wxString editor;
	if (custom)
	{
		bool failure = false;

		editor = GetText(XRCID("ID_EDITOR"));
		editor.Trim(true);
		editor.Trim(false);
		SetText(XRCID("EDITOR"), editor, failure);

		if (editor != _T(""))
		{
			wxString args;
			if (!UnquoteCommand(editor, args))
				return DisplayError(_T("ID_EDITOR"), _("Default editor not properly quoted."));

			if (editor == _T(""))
				return DisplayError(_T("ID_EDITOR"), _("Empty quoted string."));

			if (!ProgramExists(editor))
				return DisplayError(_T("ID_EDITOR"), _("The file selected as default editor does not exist."));
		}
	}

	if (GetRCheck(XRCID("ID_USEDEFAULT")))
	{
		if (GetRCheck(XRCID("ID_DEFAULT_NONE")) ||
			(custom && editor.empty()))
		{
			return DisplayError(_T("ID_EDITOR"), _("A default editor needs to be set."));
		}
	}

	return true;
}

void COptionsPageEdit::OnBrowseEditor(wxCommandEvent& event)
{
	wxFileDialog dlg(this, _("Select default editor"), _T(""), _T(""),
#ifdef __WXMSW__
		_T("Executable file (*.exe)|*.exe"),
#elif __WXMAC__
		_T("Applications (*.app)|*.app"),
#else
		wxFileSelectorDefaultWildcardStr,
#endif
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dlg.ShowModal() != wxID_OK)
		return;

	wxString editor = dlg.GetPath();
	if (editor == _T(""))
		return;

	if (!ProgramExists(editor))
	{
		XRCCTRL(*this, "ID_EDITOR", wxWindow)->SetFocus();
		wxMessageBox(_("Selected editor does not exist."), _("File not found"), wxICON_EXCLAMATION, this);
		return;
	}

	if (editor.Find(' ') != -1)
		editor = _T("\"") + editor + _T("\"");

	bool tmp;
	SetText(XRCID("ID_EDITOR"), editor, tmp);
}

void COptionsPageEdit::SetCtrlState()
{
	bool custom = GetRCheck(XRCID("ID_DEFAULT_CUSTOM"));

	XRCCTRL(*this, "ID_EDITOR", wxTextCtrl)->Enable(custom);
	XRCCTRL(*this, "ID_BROWSE", wxButton)->Enable(custom);

	XRCCTRL(*this, "ID_USEDEFAULT", wxRadioButton)->Enable(!GetRCheck(XRCID("ID_DEFAULT_NONE")) || GetRCheck(XRCID("ID_USEDEFAULT")));
}

void COptionsPageEdit::OnRadioButton(wxCommandEvent& event)
{
	SetCtrlState();
}
