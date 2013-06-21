#include <filezilla.h>
#include "settingsdialog.h"
#include "../Options.h"
#include "optionspage.h"
#include "optionspage_connection.h"
#include "optionspage_connection_ftp.h"
#include "optionspage_connection_active.h"
#include "optionspage_connection_passive.h"
#include "optionspage_ftpproxy.h"
#include "optionspage_connection_sftp.h"
#include "optionspage_filetype.h"
#include "optionspage_fileexists.h"
#include "optionspage_themes.h"
#include "optionspage_language.h"
#include "optionspage_transfer.h"
#include "optionspage_updatecheck.h"
#include "optionspage_logging.h"
#include "optionspage_debug.h"
#include "optionspage_interface.h"
#include "optionspage_dateformatting.h"
#include "optionspage_sizeformatting.h"
#include "optionspage_edit.h"
#include "optionspage_edit_associations.h"
#include "optionspage_proxy.h"
#include "optionspage_filelists.h"
#include "../filezillaapp.h"
#include "../Mainfrm.h"

enum pagenames
{
	page_none = -1,
	page_connection = 0,
	page_connection_ftp,
	page_connection_active,
	page_connection_passive,
	page_connection_ftp_proxy,
	page_connection_sftp,
	page_connection_proxy,
	page_transfer,
	page_filetype,
	page_fileexists,
	page_interface,
	page_themes,
	page_dateformatting,
	page_sizeformatting,
	page_filelists,
	page_language,
	page_edit,
	page_edit_associations,
#if FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK
	page_updatecheck,
#endif
	page_logging,
	page_debug
};

// Helper macro to add pages in the most simplistic way
#define ADD_PAGE(name, classname, parent)										\
	wxASSERT(parent < (int)m_pages.size());										\
	page.page = new classname;													\
	if (parent == page_none)													\
		page.id = treeCtrl->AppendItem(root, name);								\
	else																		\
	{																			\
		page.id = treeCtrl->AppendItem(m_pages[(unsigned int)parent].id, name);	\
		treeCtrl->Expand(m_pages[(unsigned int)parent].id);						\
	}																			\
	m_pages.push_back(page);

BEGIN_EVENT_TABLE(CSettingsDialog, wxDialogEx)
EVT_TREE_SEL_CHANGING(XRCID("ID_TREE"), CSettingsDialog::OnPageChanging)
EVT_TREE_SEL_CHANGED(XRCID("ID_TREE"), CSettingsDialog::OnPageChanged)
EVT_BUTTON(XRCID("wxID_OK"), CSettingsDialog::OnOK)
EVT_BUTTON(XRCID("wxID_CANCEL"), CSettingsDialog::OnCancel)
END_EVENT_TABLE()

CSettingsDialog::CSettingsDialog()
{
	m_pMainFrame = 0;
	m_pOptions = COptions::Get();
	m_activePanel = 0;
}

CSettingsDialog::~CSettingsDialog()
{
}

bool CSettingsDialog::Create(CMainFrame* pMainFrame)
{
	m_pMainFrame = pMainFrame;

	SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
	SetParent(pMainFrame);
	if (!wxXmlResource::Get()->LoadDialog(this, GetParent(), _T("ID_SETTINGS")))
		return false;

	if (!LoadPages())
		return false;

	return true;
}

bool CSettingsDialog::LoadPages()
{
	// Get the tree control.

	wxTreeCtrl* treeCtrl = XRCCTRL(*this, "ID_TREE", wxTreeCtrl);
	wxASSERT(treeCtrl);
	if (!treeCtrl)
		return false;

	wxTreeItemId root = treeCtrl->AddRoot(_T(""));

	// Create the instances of the page classes and fill the tree.
	t_page page;
	ADD_PAGE(_("Connection"), COptionsPageConnection, page_none);
	ADD_PAGE(_("FTP"), COptionsPageConnectionFTP, page_connection);
	ADD_PAGE(_("Active mode"), COptionsPageConnectionActive, page_connection_ftp);
	ADD_PAGE(_("Passive mode"), COptionsPageConnectionPassive, page_connection_ftp);
	ADD_PAGE(_("FTP Proxy"), COptionsPageFtpProxy, page_connection_ftp);
	ADD_PAGE(_("SFTP"), COptionsPageConnectionSFTP, page_connection);
	ADD_PAGE(_("Generic proxy"), COptionsPageProxy, page_connection);
	ADD_PAGE(_("Transfers"), COptionsPageTransfer, page_none);
	ADD_PAGE(_("File Types"), COptionsPageFiletype, page_transfer);
	ADD_PAGE(_("File exists action"), COptionsPageFileExists, page_transfer);
	ADD_PAGE(_("Interface"), COptionsPageInterface, page_none);
	ADD_PAGE(_("Themes"), COptionsPageThemes, page_interface);
	ADD_PAGE(_("Date/time format"), COptionsPageDateFormatting, page_interface);
	ADD_PAGE(_("Filesize format"), COptionsPageSizeFormatting, page_interface);
	ADD_PAGE(_("File lists"), COptionsPageFilelists, page_interface);
	ADD_PAGE(_("Language"), COptionsPageLanguage, page_none);
	ADD_PAGE(_("File editing"), COptionsPageEdit, page_none);
	ADD_PAGE(_("Filetype associations"), COptionsPageEditAssociations, page_edit);
#if FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK
	if (!COptions::Get()->GetOptionVal(OPTION_DEFAULT_DISABLEUPDATECHECK))
	{
		ADD_PAGE(_("Updates"), COptionsPageUpdateCheck, page_none);
	}
#endif //FZ_MANUALUPDATECHECK && FZ_AUTOUPDATECHECK
	ADD_PAGE(_("Logging"), COptionsPageLogging, page_none);
	ADD_PAGE(_("Debug"), COptionsPageDebug, page_none);

	treeCtrl->SetQuickBestSize(false);
	treeCtrl->InvalidateBestSize();
	treeCtrl->SetInitialSize();

	// Compensate for scrollbar
	wxSize size = treeCtrl->GetBestSize();
	int scrollWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, treeCtrl);
	size.x += scrollWidth;
	size.y = 0;
	treeCtrl->SetInitialSize(size);
	Layout();

	// Before we can initialize the pages, get the target panel in the settings
	// dialog.
	wxPanel* parentPanel = XRCCTRL(*this, "ID_PAGEPANEL", wxPanel);
	wxASSERT(parentPanel);
	if (!parentPanel)
		return false;

	// Keep track of maximum page size
	size = wxSize();

	for (std::vector<t_page>::iterator iter = m_pages.begin(); iter != m_pages.end(); ++iter)
	{
		if (!iter->page->CreatePage(m_pOptions, this, parentPanel, size))
			return false;
	}

	if (!LoadSettings())
	{
		wxMessageBox(_("Failed to load panels, invalid resource files?"));
		return false;
	}

	wxSize canvas;
	canvas.x = GetSize().x - parentPanel->GetSize().x;
	canvas.y = GetSize().y - parentPanel->GetSize().y;

	// Wrap pages nicely
	std::vector<wxWindow*> pages;
	for (unsigned int i = 0; i < m_pages.size(); i++)
	{
		pages.push_back(m_pages[i].page);
	}
	wxGetApp().GetWrapEngine()->WrapRecursive(pages, 1.33, "Settings", canvas);

	// Keep track of maximum page size
	size = wxSize(0, 0);
	for (std::vector<t_page>::iterator iter = m_pages.begin(); iter != m_pages.end(); ++iter)
		size.IncTo(iter->page->GetSizer()->GetMinSize());

#ifdef __WXGTK__
	size.x += 1;
#endif
	parentPanel->SetInitialSize(size);

	// Adjust pages sizes according to maximum size
	for (std::vector<t_page>::iterator iter = m_pages.begin(); iter != m_pages.end(); ++iter)
	{
		iter->page->GetSizer()->SetMinSize(size);
		iter->page->GetSizer()->Fit(iter->page);
		iter->page->GetSizer()->SetSizeHints(iter->page);
	}

	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);

#ifdef __WXGTK__
	// Pre-show dialog under GTK, else panels won't get initialized properly
	Show();
#endif

	for (std::vector<t_page>::iterator iter = m_pages.begin(); iter != m_pages.end(); ++iter)
		iter->page->Hide();

	// Select first page
	treeCtrl->SelectItem(m_pages[0].id);
	if (!m_activePanel)
	{
		m_activePanel = m_pages[0].page;
		m_activePanel->Display();
	}

	return true;
}

bool CSettingsDialog::LoadSettings()
{
	for (std::vector<t_page>::iterator iter = m_pages.begin(); iter != m_pages.end(); ++iter)
	{
		if (!iter->page->LoadPage())
			return false;
	}

	return true;
}

void CSettingsDialog::OnPageChanged(wxTreeEvent& event)
{
	if (m_activePanel)
		m_activePanel->Hide();

	wxTreeItemId item = event.GetItem();

	unsigned int size = m_pages.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if (m_pages[i].id == item)
		{
			m_activePanel = m_pages[i].page;
			m_activePanel->Display();
			break;
		}
	}
}

void CSettingsDialog::OnOK(wxCommandEvent& event)
{
	unsigned int size = m_pages.size();
	for (unsigned int i = 0; i < size; i++)
	{
		if (!m_pages[i].page->Validate())
		{
			if (m_activePanel != m_pages[i].page)
			{
				wxTreeCtrl* treeCtrl = XRCCTRL(*this, "ID_TREE", wxTreeCtrl);
				treeCtrl->SelectItem(m_pages[i].id);
			}
			return;
		}
	}

	for (unsigned int i = 0; i < size; i++)
		m_pages[i].page->SavePage();

	EndModal(wxID_OK);
}

void CSettingsDialog::OnCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

void CSettingsDialog::OnPageChanging(wxTreeEvent& event)
{
	if (!m_activePanel)
		return;

	if (!m_activePanel->Validate())
		event.Veto();
}
