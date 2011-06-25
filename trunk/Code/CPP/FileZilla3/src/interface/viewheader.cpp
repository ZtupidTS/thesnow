#include <filezilla.h>
#include "viewheader.h"
#include "commandqueue.h"

#ifdef __WXMSW__
#include "wx/msw/uxtheme.h"
#endif //__WXMSW__

#include <wx/combobox.h>

#ifdef __WXMSW__
const int border_offset = 0;
#elif defined(__WXMAC__)
const int border_offset = 6;
#else
const int border_offset = 10;
#endif

// wxComboBox derived class which captures WM_CANCELMODE under Windows
class CComboBoxEx : public wxComboBox
{
public:
	CComboBoxEx(CViewHeader* parent)
		: wxComboBox(parent, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, wxArrayString(), wxCB_DROPDOWN | wxTE_PROCESS_ENTER | wxCB_SORT)
	{
		m_parent = parent;
	}
#ifdef __WXMSW__
protected:
	CViewHeader* m_parent;
	virtual WXLRESULT MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
	{
		if (nMsg == WM_CANCELMODE)
		{
			m_parent->m_bLeftMousePressed = false;
			Refresh();
		}
		else if (nMsg == WM_CAPTURECHANGED && !lParam)
		{
			WXLRESULT res = wxComboBox::MSWDefWindowProc(nMsg, wParam, lParam);

			if (!SendMessage((HWND)GetHandle(), CB_GETDROPPEDSTATE, 0, 0))
			{
				m_parent->m_bLeftMousePressed = false;
				Refresh();
			}
			return res;
		}
		return wxComboBox::MSWDefWindowProc(nMsg, wParam, lParam);
	}
#endif //__WXMSW__

	DECLARE_EVENT_TABLE();
	void OnKeyDown(wxKeyEvent& event)
	{
		if (event.GetKeyCode() != WXK_TAB)
		{
			event.Skip();
			return;
		}

		wxNavigationKeyEvent navEvent;
		navEvent.SetEventObject(m_parent);
		navEvent.SetDirection(!event.ShiftDown());
		navEvent.SetFromTab(true);
		navEvent.ResumePropagation(1);
		m_parent->ProcessEvent(navEvent);
	}

	void OnChar(wxKeyEvent& event)
	{
		if (event.GetKeyCode() == WXK_TAB)
			return;

		event.Skip();
	}
};

BEGIN_EVENT_TABLE(CComboBoxEx, wxComboBox)
EVT_KEY_DOWN(CComboBoxEx::OnKeyDown)
EVT_CHAR(CComboBoxEx::OnChar)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CViewHeader, wxWindow)
EVT_SIZE(CViewHeader::OnSize)
EVT_PAINT(CViewHeader::OnPaint)
END_EVENT_TABLE()

CViewHeader::CViewHeader(wxWindow* pParent, const wxString& label)
	: wxWindow(pParent, wxID_ANY)
{
	m_cbOffset = 0;
	m_labelHeight = 0;
	m_alreadyInPaint = false;
	m_pComboBox = new CComboBoxEx(this);
	m_pLabel = new wxStaticText(this, wxID_ANY, label, wxDefaultPosition, wxDefaultSize);
	wxSize size = GetSize();
	size.SetHeight(m_pComboBox->GetBestSize().GetHeight() + border_offset);

	SetLabel(label);

	SetSize(size);

#ifdef __WXMSW__
	m_pComboBox->Connect(wxID_ANY, wxEVT_PAINT, (wxObjectEventFunction)(wxEventFunction)(wxPaintEventFunction)&CViewHeader::OnComboPaint, 0, this);
	m_pComboBox->Connect(wxID_ANY, wxEVT_LEFT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&CViewHeader::OnComboMouseEvent, 0, this);
	m_pComboBox->Connect(wxID_ANY, wxEVT_LEFT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&CViewHeader::OnComboMouseEvent, 0, this);
	m_bLeftMousePressed = false;
#endif //__WXMSW__
}

void CViewHeader::OnSize(wxSizeEvent& event)
{
	const wxRect client_rect = GetClientRect();

	wxRect rect = client_rect;
	rect.SetWidth(rect.GetWidth() - m_cbOffset + 2);
	rect.SetX(m_cbOffset);
	rect.Deflate(0, border_offset / 2);
	rect.SetWidth(rect.GetWidth() - border_offset / 2);
	m_pComboBox->SetSize(rect);

	rect.SetX(5);
	rect.SetWidth(m_cbOffset - 5);
	rect.SetY((client_rect.GetHeight() - m_labelHeight) / 2 - 1);
	rect.SetHeight(m_labelHeight);
	m_pLabel->SetSize(rect);

	Refresh();
}

#ifdef __WXMSW__

void CViewHeader::OnComboPaint(wxPaintEvent& event)
{
	// We do a small trick to let the control handle the event before we can paint
	if (m_alreadyInPaint)
	{
		event.Skip();
		return;
	}

	wxComboBox* box = m_pComboBox;

	m_alreadyInPaint = true;
	box->Refresh();
	box->Update();
	m_alreadyInPaint = false;

	wxClientDC dc(box);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	int thumbWidth = ::GetSystemMetrics(SM_CXHTHUMB);

	if (m_bLeftMousePressed)
	{
		if (!SendMessage((HWND)box->GetHandle(), CB_GETDROPPEDSTATE, 0, 0))
			m_bLeftMousePressed = false;
	}

#if wxUSE_UXTHEME
	wxUxThemeEngine *p = wxUxThemeEngine::Get();
	if (p && p->IsThemeActive())
	{
	}
	else
#endif //wxUSE_UXTHEME
	{
		dc.SetPen(wxPen(wxSystemSettings::GetColour(box->IsEnabled() ? wxSYS_COLOUR_WINDOW : wxSYS_COLOUR_BTNFACE)));
		wxRect rect = box->GetClientRect();
		rect.Deflate(1);
		wxRect rect2 = rect;
		rect2.SetWidth(rect.GetWidth() - thumbWidth);
		dc.DrawRectangle(rect2);

		if (!m_bLeftMousePressed || !box->IsEnabled())
		{
			wxPoint topLeft = rect.GetTopLeft();
			wxPoint bottomRight = rect.GetBottomRight();
			bottomRight.x--;
			topLeft.x = bottomRight.x - thumbWidth + 1;

			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
			dc.DrawLine(topLeft.x, topLeft.y, bottomRight.x + 1, topLeft.y);
			dc.DrawLine(topLeft.x, topLeft.y + 1, topLeft.x, bottomRight.y);
			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW)));
			dc.DrawLine(bottomRight.x, topLeft.y + 1, bottomRight.x, bottomRight.y + 1);
			dc.DrawLine(topLeft.x, bottomRight.y, bottomRight.x, bottomRight.y);

			topLeft.x++;
			topLeft.y++;
			bottomRight.x--;
			bottomRight.y--;
			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT)));
			dc.DrawLine(topLeft.x, topLeft.y, bottomRight.x + 1, topLeft.y);
			dc.DrawLine(topLeft.x, topLeft.y + 1, topLeft.x, bottomRight.y);
			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW)));
			dc.DrawLine(bottomRight.x, topLeft.y + 1, bottomRight.x, bottomRight.y + 1);
			dc.DrawLine(topLeft.x, bottomRight.y, bottomRight.x, bottomRight.y);

			topLeft.x++;
			topLeft.y++;
			bottomRight.x--;
			bottomRight.y--;
			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
			dc.DrawRectangle(wxRect(topLeft, bottomRight));
		}
		else
		{
			wxPoint topLeft = rect.GetTopLeft();
			wxPoint bottomRight = rect.GetBottomRight();
			bottomRight.x--;
			topLeft.x = bottomRight.x - thumbWidth + 1;

			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW)));
			dc.DrawRectangle(wxRect(topLeft, bottomRight));

			topLeft.x++;
			topLeft.y++;
			bottomRight.x--;
			bottomRight.y--;
			dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
			dc.DrawRectangle(wxRect(topLeft, bottomRight));
		}
	}

	// Cover up dark 3D shadow.
	wxRect rect = box->GetClientRect();
	dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW)));
	dc.DrawRectangle(rect);

}

void CViewHeader::OnComboMouseEvent(wxMouseEvent& event)
{
	if (event.GetEventType() == wxEVT_LEFT_UP)
		m_bLeftMousePressed = false;
	else if (event.GetEventType() == wxEVT_LEFT_DOWN)
		m_bLeftMousePressed = true;

	event.Skip();
}

#endif //__WXMSW__

void CViewHeader::OnPaint(wxPaintEvent& event)
{
	wxRect rect = GetClientRect();
	wxPaintDC dc(this);
	dc.SetPen(*wxBLACK_PEN);
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

#ifdef __WXMSW__
	dc.DrawLine(rect.GetLeft(), rect.GetBottom(), m_cbOffset, rect.GetBottom());
#else
	dc.DrawLine(rect.GetLeft(), rect.GetBottom(), rect.GetRight(), rect.GetBottom());
#endif
}

void CViewHeader::SetLabel(const wxString& label)
{
	m_pLabel->SetLabel(label);
	int w;
	GetTextExtent(label, &w, &m_labelHeight);
	m_cbOffset = w + 10;
}

void CViewHeader::Reparent(CViewHeader** pViewHeader, wxWindow* parent)
{
#if defined __WXMSW__ || defined __WXGTK__ || \
	(defined __WXMAC__ && !(defined __WXMAC_CLASSIC__))
	((wxWindow*)*pViewHeader)->Reparent(parent);
#else
#error CViewHeader::Reparent unimplemented
#endif
}

wxString CViewHeader::GetLabel() const
{
	return m_pLabel->GetLabel();
}

void CViewHeader::AddRecentDirectory(const wxString &directory)
{
	const int len = directory.Len();

	// Check if directory is already in the list
	for (std::list<wxString>::const_iterator iter = m_recentDirectories.begin(); iter != m_recentDirectories.end(); iter++)
	{
		if (*iter == directory)
		{
			m_pComboBox->SetStringSelection(directory);
			m_pComboBox->SetSelection(len, len);
			return;
		}
	}

	if (m_recentDirectories.size() == 20)
	{
		wxString dirToRemove = m_recentDirectories.front();
		m_recentDirectories.pop_front();
		if (dirToRemove == directory)
		{
			m_recentDirectories.push_back(directory);
			m_pComboBox->SetStringSelection(directory);
			m_pComboBox->SetSelection(len, len);
			return;
		}

		int item = m_pComboBox->FindString(dirToRemove, true);
		if (item != wxNOT_FOUND)
			m_pComboBox->Delete(item);
	}

	m_recentDirectories.push_back(directory);
	int item = m_pComboBox->Append(directory);
	m_pComboBox->SetSelection(item);
	m_pComboBox->SetSelection(len, len);
	return;
}

void CViewHeader::SetFocus()
{
	m_pComboBox->SetFocus();
}

bool CViewHeader::IsEnabled() const
{
	return m_pComboBox->IsEnabled();
}

#ifdef __WXGTK__
DECLARE_EVENT_TYPE(fzEVT_LOCALVIEWHEADERSETTEXTSEL, -1)
DEFINE_EVENT_TYPE(fzEVT_LOCALVIEWHEADERSETTEXTSEL)
#endif

BEGIN_EVENT_TABLE(CLocalViewHeader, CViewHeader)
EVT_TEXT(wxID_ANY, CLocalViewHeader::OnTextChanged)
EVT_TEXT_ENTER(wxID_ANY, CLocalViewHeader::OnTextEnter)
EVT_COMBOBOX(wxID_ANY, CLocalViewHeader::OnSelectionChanged)
#ifdef __WXGTK__
EVT_COMMAND(wxID_ANY, fzEVT_LOCALVIEWHEADERSETTEXTSEL, CLocalViewHeader::OnSelectTextEvent)
#endif
END_EVENT_TABLE()

CLocalViewHeader::CLocalViewHeader(wxWindow* pParent, CState* pState)
	: CViewHeader(pParent, _("Local site:")), CStateEventHandler(pState)
{
	pState->RegisterHandler(this, STATECHANGE_LOCAL_DIR);
}

void CLocalViewHeader::OnTextChanged(wxCommandEvent& event)
{
	// This function handles auto-completion

#ifdef __WXGTK__
	m_autoCompletionText = _T("");
#endif

	wxString str = m_pComboBox->GetValue();
	if (str == _T("") || str.Right(1) == _T("/"))
	{
		m_oldValue = str;
		return;
	}
#ifdef __WXMSW__
	if (str.Right(1) == _T("\\"))
	{
		m_oldValue = str;
		return;
	}
#endif

	if (str == m_oldValue)
		return;

	if (str.Left(m_oldValue.Length()) != m_oldValue)
	{
		m_oldValue = str;
		return;
	}

#ifdef __WXMSW__
	if (str.Left(2) == _T("\\\\"))
	{
		int pos = str.Mid(2).Find('\\');
		if (pos == -1)
		{
			// Partial UNC path, no full server yet, skip further processing
			return;
		}

		pos = str.Mid(pos + 3).Find('\\');
		if (pos == -1)
		{
			// Partial UNC path, no full share yet, skip further processing
			return;
		}
	}
#endif

	wxFileName fn(str);
	if (!fn.IsOk())
	{
		m_oldValue = str;
		return;
	}

	wxString name = fn.GetFullName();
	const wxString path = fn.GetPath();

	wxDir dir;
	if (name == _T("") || path == _T(""))
	{
		m_oldValue = str;
		return;
	}
	else
	{
		wxLogNull log;
		if (!dir.Open(path) || !dir.IsOpened())
		{
			m_oldValue = str;
			return;
		}
	}
	wxString found;

	{
		wxLogNull noLog;
		if (!dir.GetFirst(&found, name + _T("*"), wxDIR_DIRS))
		{
			m_oldValue = str;
			return;
		}
	}

	wxString tmp;
	if (dir.GetNext(&tmp))
	{
		m_oldValue = str;
		return;
	}

#ifdef __WXMSW__
	if (found.Left(name.Length()).CmpNoCase(name))
#else
	if (found.Left(name.Length()) != name)
#endif
	{
		m_oldValue = str;
		return;
	}

#ifdef __WXGTK__
	m_autoCompletionText = found.Mid(name.Length()) + wxFileName::GetPathSeparator();

	wxCommandEvent evt(fzEVT_LOCALVIEWHEADERSETTEXTSEL);
	wxPostEvent(this, evt);
#else
	m_pComboBox->SetValue(str + found.Mid(name.Length()) + wxFileName::GetPathSeparator());
	m_pComboBox->SetSelection(str.Length(), m_pComboBox->GetValue().Length() + 1);
#endif

	m_oldValue = str;
}

#ifdef __WXGTK__
void CLocalViewHeader::OnSelectTextEvent(wxCommandEvent& event)
{
	if (m_autoCompletionText == _T(""))
		return;

	const wxString& oldValue = m_pComboBox->GetValue();
	const wxString completionText = m_autoCompletionText;
	m_autoCompletionText = _T("");

	if (m_pComboBox->GetInsertionPoint() != (int)oldValue.Len())
		return;

	m_pComboBox->SetValue(oldValue + completionText);
	m_pComboBox->SetSelection(oldValue.Len(), oldValue.Len() + completionText.Len());
}
#endif

void CLocalViewHeader::OnSelectionChanged(wxCommandEvent& event)
{
#ifdef __WXGTK__
	m_autoCompletionText = _T("");
#endif

	wxString dir = event.GetString();
	if (dir == _T(""))
		return;

	if (!wxDir::Exists(dir))
	{
		const wxString& current = m_pState->GetLocalDir().GetPath();
		int item = m_pComboBox->FindString(current, true);
		if (item != wxNOT_FOUND)
			m_pComboBox->SetSelection(item);

		wxBell();
		return;
	}

	m_pState->SetLocalDir(dir);
}

void CLocalViewHeader::OnTextEnter(wxCommandEvent& event)
{
#ifdef __WXGTK__
	m_autoCompletionText = _T("");
#endif

	wxString dir = m_pComboBox->GetValue();

	wxString error;
	if (!m_pState->SetLocalDir(dir, &error))
	{
		if (error != _T(""))
			wxMessageBox(error, _("Failed to change directory"), wxICON_INFORMATION);
		else
			wxBell();
		m_pComboBox->SetValue(m_pState->GetLocalDir().GetPath());
	}
}

void CLocalViewHeader::OnStateChange(CState* pState, enum t_statechange_notifications notification, const wxString& data, const void* data2)
{
	wxASSERT(notification == STATECHANGE_LOCAL_DIR);

#ifdef __WXGTK__
	m_autoCompletionText = _T("");
#endif

	wxString dir = pState->GetLocalDir().GetPath();
	AddRecentDirectory(dir);
}

BEGIN_EVENT_TABLE(CRemoteViewHeader, CViewHeader)
EVT_TEXT_ENTER(wxID_ANY, CRemoteViewHeader::OnTextEnter)
EVT_COMBOBOX(wxID_ANY, CRemoteViewHeader::OnSelectionChanged)
END_EVENT_TABLE()

CRemoteViewHeader::CRemoteViewHeader(wxWindow* pParent, CState* pState)
	: CViewHeader(pParent, _("Remote site:")), CStateEventHandler(pState)
{
	pState->RegisterHandler(this, STATECHANGE_REMOTE_DIR);
	m_pComboBox->Disable();
}

void CRemoteViewHeader::OnStateChange(CState* pState, enum t_statechange_notifications notification, const wxString& data, const void* data2)
{
	wxASSERT(notification == STATECHANGE_REMOTE_DIR);

	m_path = pState->GetRemotePath();
	if (m_path.IsEmpty())
	{
		m_pComboBox->SetValue(_T(""));
		m_pComboBox->Disable();
	}
	else
	{
		const CServer* const pServer = pState->GetServer();
		if (pServer && *pServer != m_lastServer)
		{
			m_pComboBox->Clear();
			m_recentDirectories.clear();
			m_lastServer = *pServer;
		}
		m_pComboBox->Enable();
		AddRecentDirectory(m_path.GetPath());
	}
}

void CRemoteViewHeader::OnTextEnter(wxCommandEvent& event)
{
	CServerPath path = m_path;
	wxString value = m_pComboBox->GetValue();
	if (value == _T("") || !path.ChangePath(value))
	{
		wxBell();
		return;
	}

	if (!m_pState->m_pCommandQueue->Idle())
	{
		wxBell();
		return;
	}

	m_pState->ChangeRemoteDir(path);
}

void CRemoteViewHeader::OnSelectionChanged(wxCommandEvent& event)
{
	const wxString& dir = event.GetString();
	if (dir == _T(""))
		return;

	CServerPath path = m_path;
	if (!path.SetPath(dir))
	{
		wxBell();
		return;
	}

	if (!m_pState->m_pCommandQueue->Idle())
	{
		wxBell();
		return;
	}

	m_pState->ChangeRemoteDir(path);
}
