#include <filezilla.h>
#include "fileexistsdlg.h"
#include "Options.h"
#include "sizeformatting.h"
#include "themeprovider.h"

#include <wx/display.h>

BEGIN_EVENT_TABLE(CFileExistsDlg, wxDialogEx)
EVT_BUTTON(XRCID("wxID_OK"), CFileExistsDlg::OnOK)
EVT_BUTTON(XRCID("wxID_CANCEL"), CFileExistsDlg::OnCancel)
EVT_CHECKBOX(wxID_ANY, CFileExistsDlg::OnCheck)
END_EVENT_TABLE()

CFileExistsDlg::CFileExistsDlg(CFileExistsNotification *pNotification)
{
	m_pNotification = pNotification;
	m_pAction1 = m_pAction2 = m_pAction3 = m_pAction4 = m_pAction5 = m_pAction6 = m_pAction7 = 0;
	m_action = CFileExistsNotification::overwrite;
	m_always = false;
	m_queueOnly = false;
	m_directionOnly = false;
}

bool CFileExistsDlg::Create(wxWindow* parent)
{
	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
	SetParent(parent);
	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);

	return true;
}

void CFileExistsDlg::CreateControls()
{
	wxXmlResource::Get()->LoadDialog(this, GetParent(), _T("ID_FILEEXISTSDLG"));
	m_pAction1 = wxDynamicCast(FindWindow(XRCID("ID_ACTION1")), wxRadioButton);
	m_pAction2 = wxDynamicCast(FindWindow(XRCID("ID_ACTION2")), wxRadioButton);
	m_pAction3 = wxDynamicCast(FindWindow(XRCID("ID_ACTION3")), wxRadioButton);
	m_pAction4 = wxDynamicCast(FindWindow(XRCID("ID_ACTION4")), wxRadioButton);
	m_pAction5 = wxDynamicCast(FindWindow(XRCID("ID_ACTION5")), wxRadioButton);
	m_pAction6 = wxDynamicCast(FindWindow(XRCID("ID_ACTION6")), wxRadioButton);
	m_pAction7 = wxDynamicCast(FindWindow(XRCID("ID_ACTION7")), wxRadioButton);

	wxString localFile = m_pNotification->localFile;

	wxString remoteFile = m_pNotification->remotePath.FormatFilename(m_pNotification->remoteFile);
    localFile = GetPathEllipsis(localFile, FindWindow(XRCID("ID_FILE1_NAME")));
    remoteFile = GetPathEllipsis(remoteFile, FindWindow(XRCID("ID_FILE2_NAME")));

	localFile.Replace(_T("&"), _T("&&"));
	remoteFile.Replace(_T("&"), _T("&&"));

	const bool thousands_separator = COptions::Get()->GetOptionVal(OPTION_SIZE_USETHOUSANDSEP) != 0;

	wxString localSize;
	if (m_pNotification->localSize != -1)
		localSize = CSizeFormat::Format(m_pNotification->localSize, true, CSizeFormat::bytes, thousands_separator, 0);
	else
		localSize = _("Size unknown");

	wxString remoteSize;
	if (m_pNotification->remoteSize != -1)
		remoteSize = CSizeFormat::Format(m_pNotification->remoteSize, true, CSizeFormat::bytes, thousands_separator, 0);
	else
		remoteSize = _("Size unknown");

	if (m_pNotification->download)
	{
		wxStaticText *pStatText;

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE1_NAME")));
		if (pStatText)
			pStatText->SetLabel(localFile);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE1_SIZE")));
		if (pStatText)
			pStatText->SetLabel(localSize);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE1_TIME")));
		if (pStatText)
		{
			if (m_pNotification->localTime.IsValid())
				pStatText->SetLabel(m_pNotification->localTime.Format());
			else
				pStatText->SetLabel(_("Date/time unknown"));
		}

		LoadIcon(XRCID("ID_FILE1_ICON"), m_pNotification->localFile);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE2_NAME")));
		if (pStatText)
			pStatText->SetLabel(remoteFile);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE2_SIZE")));
		if (pStatText)
			pStatText->SetLabel(remoteSize);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE2_TIME")));
		if (pStatText)
		{
			if (m_pNotification->remoteTime.IsValid())
				pStatText->SetLabel(m_pNotification->remoteTime.Format());
			else
				pStatText->SetLabel(_("Date/time unknown"));
		}

		LoadIcon(XRCID("ID_FILE2_ICON"), m_pNotification->remoteFile);

		wxCheckBox *pCheckBox = reinterpret_cast<wxCheckBox *>(FindWindow(XRCID("ID_UPDOWNONLY")));
		if (pCheckBox)
			pCheckBox->SetLabel(_("A&pply only to downloads"));
	}
	else
	{
		wxWindow *pStatText;

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE1_NAME")));
		if (pStatText)
			pStatText->SetLabel(remoteFile);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE1_SIZE")));
		if (pStatText)
			pStatText->SetLabel(remoteSize);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE1_TIME")));
		if (pStatText)
		{
			if (m_pNotification->remoteTime.IsValid())
				pStatText->SetLabel(m_pNotification->remoteTime.Format());
			else
				pStatText->SetLabel(_("Date/time unknown"));
		}

		LoadIcon(XRCID("ID_FILE1_ICON"), m_pNotification->remoteFile);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE2_NAME")));
		if (pStatText)
			pStatText->SetLabel(localFile);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE2_SIZE")));
		if (pStatText)
			pStatText->SetLabel(localSize);

		pStatText = reinterpret_cast<wxStaticText *>(FindWindow(XRCID("ID_FILE2_TIME")));
		if (pStatText)
		{
			if (m_pNotification->localTime.IsValid())
				pStatText->SetLabel(m_pNotification->localTime.Format());
			else
				pStatText->SetLabel(_("Date/time unknown"));
		}

		LoadIcon(XRCID("ID_FILE2_ICON"), m_pNotification->localFile);

		wxCheckBox *pCheckBox = reinterpret_cast<wxCheckBox *>(FindWindow(XRCID("ID_UPDOWNONLY")));
		if (pCheckBox)
			pCheckBox->SetLabel(_("A&pply only to uploads"));
	}
}

void CFileExistsDlg::LoadIcon(int id, const wxString &file)
{
	wxStaticBitmap *pStatBmp = reinterpret_cast<wxStaticBitmap *>(FindWindow(id));
	if (!pStatBmp)
		return;

	wxSize size = CThemeProvider::GetIconSize(iconSizeNormal);
	pStatBmp->SetInitialSize(size);
	pStatBmp->InvalidateBestSize();
	pStatBmp->SetBitmap(CThemeProvider::GetBitmap(_T("ART_FILE"), wxART_OTHER, size));

#ifdef __WXMSW__
	SHFILEINFO fileinfo;
	memset(&fileinfo, 0, sizeof(fileinfo));
	if (SHGetFileInfo(file, FILE_ATTRIBUTE_NORMAL, &fileinfo, sizeof(fileinfo), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
	{
		wxBitmap bmp;
		bmp.Create(size.x, size.y);

		wxMemoryDC *dc = new wxMemoryDC;

		wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
		wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

		dc->SelectObject(bmp);

		dc->SetPen(pen);
		dc->SetBrush(brush);
		dc->DrawRectangle(0, 0, size.x, size.y);

		wxIcon icon;
		icon.SetHandle(fileinfo.hIcon);
		icon.SetSize(size.x, size.y);

		dc->DrawIcon(icon, 0, 0);
		delete dc;

		pStatBmp->SetBitmap(bmp);

		return;
	}

#endif //__WXMSW__

	wxFileName fn(file);
	wxString ext = fn.GetExt();
	if (ext == _T(""))
		return;

	wxFileType *pType = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
	if (pType)
	{
		wxIconLocation loc;
		if (pType->GetIcon(&loc) && loc.IsOk())
		{
			wxLogNull *tmp = new wxLogNull;
			wxIcon icon(loc);
			delete tmp;
			if (!icon.Ok())
			{
				delete pType;
				return;
			}

			int width = icon.GetWidth();
			int height = icon.GetHeight();
			if (width && height)
			{
				wxBitmap bmp;
				bmp.Create(icon.GetWidth(), icon.GetHeight());

				wxMemoryDC *dc = new wxMemoryDC;

				wxPen pen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
				wxBrush brush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

				dc->SelectObject(bmp);

				dc->SetPen(pen);
				dc->SetBrush(brush);
				dc->DrawRectangle(0, 0, width, height);

				dc->DrawIcon(icon, 0, 0);
				delete dc;

				pStatBmp->SetBitmap(bmp);

				return;
			}
		}
		delete pType;
	}
}

void CFileExistsDlg::OnOK(wxCommandEvent& event)
{
	if (m_pAction1 && m_pAction1->GetValue())
		m_action = CFileExistsNotification::overwrite;
	else if (m_pAction2 && m_pAction2->GetValue())
		m_action = CFileExistsNotification::overwriteNewer;
	else if (m_pAction3 && m_pAction3->GetValue())
		m_action = CFileExistsNotification::resume;
	else if (m_pAction4 && m_pAction4->GetValue())
		m_action = CFileExistsNotification::rename;
	else if (m_pAction5 && m_pAction5->GetValue())
		m_action = CFileExistsNotification::skip;
	else if (m_pAction6 && m_pAction6->GetValue())
		m_action = CFileExistsNotification::overwriteSizeOrNewer;
	else if (m_pAction7 && m_pAction7->GetValue())
		m_action = CFileExistsNotification::overwriteSize;
	else
		m_action = CFileExistsNotification::overwrite;

	m_always = XRCCTRL(*this, "ID_ALWAYS", wxCheckBox)->GetValue();
	m_directionOnly = XRCCTRL(*this, "ID_UPDOWNONLY", wxCheckBox)->GetValue();
	m_queueOnly = XRCCTRL(*this, "ID_QUEUEONLY", wxCheckBox)->GetValue();
	EndModal(wxID_OK);
}

enum CFileExistsNotification::OverwriteAction CFileExistsDlg::GetAction() const
{
	return m_action;
}

void CFileExistsDlg::OnCancel(wxCommandEvent& event)
{
	m_action = CFileExistsNotification::skip;
	EndModal(wxID_CANCEL);
}

bool CFileExistsDlg::Always(bool &directionOnly, bool &queueOnly) const
{
	directionOnly = m_directionOnly;
	queueOnly = m_queueOnly;
	return m_always;
}

wxString CFileExistsDlg::GetPathEllipsis(wxString path, wxWindow *window)
{
	int string_width; // width of the path string in pixels
	int y;            // dummy variable
	window->GetTextExtent(path, &string_width, &y);

	wxDisplay display(wxDisplay::GetFromWindow(window));
	wxRect rect = display.GetClientArea();
	const int DESKTOP_WIDTH = rect.GetWidth(); // width of the desktop in pixels
	const int maxWidth = (int)(DESKTOP_WIDTH * 0.75);

	// If the path is already short enough, don't change it
	if (string_width <= maxWidth || path.Length() < 20)
		return path;

	wxString fill = _T(" ");
#if wxUSE_UNICODE
	fill += 0x2026; //unicode ellipsis character
#else
	fill += _T("...");
#endif
	fill += _T(" ");

	int fillWidth;
	window->GetTextExtent(fill, &fillWidth, &y);

	// Do initial split roughly in the middle of the string
	int middle = path.Length() / 2;
	wxString left = path.Left(middle);
	wxString right = path.Mid(middle);

	int leftWidth, rightWidth;
	window->GetTextExtent(left, &leftWidth, &y);
	window->GetTextExtent(right, &rightWidth, &y);

	// continue removing one character at a time around the fill until path string is small enough
	while ((leftWidth + fillWidth + rightWidth) > maxWidth)
	{
		if (leftWidth > rightWidth && left.Len() > 10)
		{
			left.RemoveLast();
			window->GetTextExtent(left, &leftWidth, &y);
		}
		else
		{
			if (right.Len() <= 10)
				break;

			right = right.Mid(1);
			window->GetTextExtent(right, &rightWidth, &y);
		}
	}

	return left + fill + right;
}

void CFileExistsDlg::OnCheck(wxCommandEvent& event)
{
	if (event.GetId() != XRCID("ID_UPDOWNONLY") && event.GetId() != XRCID("ID_QUEUEONLY"))
		return;

	if (event.IsChecked())
		XRCCTRL(*this, "ID_ALWAYS", wxCheckBox)->SetValue(true);
}
