#include <filezilla.h>
#include "filter.h"
#include "listingcomparison.h"
#include "Mainfrm.h"
#include "Options.h"
#include "QueueView.h"
#include "themeprovider.h"
#include "toolbar.h"
#include "xh_toolb_ex.h"

IMPLEMENT_DYNAMIC_CLASS(CToolBar, wxToolBar)

CToolBar::CToolBar()
	: CStateEventHandler(0)
	, m_pMainFrame()
{
}

CToolBar::~CToolBar()
{
	for (std::map<int, wxToolBarToolBase*>::iterator iter = m_hidden_tools.begin(); iter != m_hidden_tools.end(); ++iter)
		delete iter->second;
}

CToolBar* CToolBar::Load(CMainFrame* pMainFrame)
{
	{
		wxSize iconSize;
		wxString str = COptions::Get()->GetOption(OPTION_THEME_ICONSIZE);
		if (str == _T("32x32"))
			iconSize = CThemeProvider::GetIconSize(iconSizeNormal);
		else if (str == _T("48x48"))
			iconSize = CThemeProvider::GetIconSize(iconSizeLarge);
		else
			iconSize = CThemeProvider::GetIconSize(iconSizeSmall);

		wxToolBarXmlHandlerEx::SetIconSize(iconSize);
	}

	CToolBar* toolbar = wxDynamicCast(wxXmlResource::Get()->LoadToolBar(pMainFrame, _T("ID_TOOLBAR")), CToolBar);
	if (!toolbar)
		return 0;

	toolbar->m_pMainFrame = pMainFrame;

	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_REMOTE_IDLE, true, true);
	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_SERVER, true, true);
	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_SYNC_BROWSE, true, true);
	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_COMPARISON, true, true);
	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_APPLYFILTER, true, false);

	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_QUEUEPROCESSING, false, false);
	CContextManager::Get()->RegisterHandler(toolbar, STATECHANGE_CHANGEDCONTEXT, false, false);

	toolbar->RegisterOption(OPTION_SHOW_MESSAGELOG);
	toolbar->RegisterOption(OPTION_SHOW_QUEUE);
	toolbar->RegisterOption(OPTION_SHOW_TREE_LOCAL);
	toolbar->RegisterOption(OPTION_SHOW_TREE_REMOTE);
	toolbar->RegisterOption(OPTION_MESSAGELOG_POSITION);

#if defined(EVT_TOOL_DROPDOWN) && defined(__WXMSW__)
	toolbar->MakeDropdownTool(XRCID("ID_TOOLBAR_SITEMANAGER"));
#endif

#ifdef __WXMSW__
	int majorVersion, minorVersion;
	wxGetOsVersion(& majorVersion, & minorVersion);
	if (majorVersion < 6)
		toolbar->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif

	toolbar->ToggleTool(XRCID("ID_TOOLBAR_FILTER"), CFilterManager::HasActiveFilters());
	toolbar->ToggleTool(XRCID("ID_TOOLBAR_LOGVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_MESSAGELOG) != 0);
	toolbar->ToggleTool(XRCID("ID_TOOLBAR_QUEUEVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_QUEUE) != 0);
	toolbar->ToggleTool(XRCID("ID_TOOLBAR_LOCALTREEVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_TREE_LOCAL) != 0);
	toolbar->ToggleTool(XRCID("ID_TOOLBAR_REMOTETREEVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_TREE_REMOTE) != 0);

	if (COptions::Get()->GetOptionVal(OPTION_MESSAGELOG_POSITION) == 2)
		toolbar->HideTool(XRCID("ID_TOOLBAR_LOGVIEW"));

#ifdef __WXMAC__
	// Hide then re-show fixes some odd sizing
	toolbar->Hide();
	if (COptions::Get()->GetOptionVal(OPTION_TOOLBAR_HIDDEN) == 0)
		toolbar->Show();
#endif
	return toolbar;
}

void CToolBar::OnStateChange(CState* pState, enum t_statechange_notifications notification, const wxString& data, const void* data2)
{
	switch (notification)
	{
	case STATECHANGE_CHANGEDCONTEXT:
	case STATECHANGE_SERVER:
	case STATECHANGE_REMOTE_IDLE:
		UpdateToolbarState();
		break;
	case STATECHANGE_QUEUEPROCESSING:
		{
			const bool check = m_pMainFrame->GetQueue() && m_pMainFrame->GetQueue()->IsActive() != 0;
			ToggleTool(XRCID("ID_TOOLBAR_PROCESSQUEUE"), check);
		}
		break;
	case STATECHANGE_SYNC_BROWSE:
		{
			bool is_sync_browse = pState && pState->GetSyncBrowse();
			ToggleTool(XRCID("ID_TOOLBAR_SYNCHRONIZED_BROWSING"), is_sync_browse);
		}
		break;
	case STATECHANGE_COMPARISON:
		{
			bool is_comparing = pState && pState->GetComparisonManager()->IsComparing();
			ToggleTool(XRCID("ID_TOOLBAR_COMPARISON"), is_comparing);
		}
		break;
	case STATECHANGE_APPLYFILTER:
		ToggleTool(XRCID("ID_TOOLBAR_FILTER"), CFilterManager::HasActiveFilters());
		break;
	default:
		break;
	}
}

void CToolBar::UpdateToolbarState()
{
	CState* pState = CContextManager::Get()->GetCurrentContext();
	if (!pState)
		return;

	const CServer* pServer = pState->GetServer();
	const bool idle = pState->IsRemoteIdle();

	EnableTool(XRCID("ID_TOOLBAR_DISCONNECT"), pServer && idle);
	EnableTool(XRCID("ID_TOOLBAR_CANCEL"), pServer && !idle);
	EnableTool(XRCID("ID_TOOLBAR_COMPARISON"), pServer != 0);
	EnableTool(XRCID("ID_TOOLBAR_SYNCHRONIZED_BROWSING"), pServer != 0);
	EnableTool(XRCID("ID_TOOLBAR_FIND"), pServer && idle);

	ToggleTool(XRCID("ID_TOOLBAR_COMPARISON"), pState->GetComparisonManager()->IsComparing());
	ToggleTool(XRCID("ID_TOOLBAR_SYNCHRONIZED_BROWSING"), pState->GetSyncBrowse());

	bool canReconnect;
	if (pServer || !idle)
		canReconnect = false;
	else
	{
		CServer tmp;
		canReconnect = pState->GetLastServer().GetHost() != _T("");
	}
	EnableTool(XRCID("ID_TOOLBAR_RECONNECT"), canReconnect);
}

void CToolBar::OnOptionChanged(int option)
{
	switch (option)
	{
	case OPTION_SHOW_MESSAGELOG:
		ToggleTool(XRCID("ID_TOOLBAR_LOGVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_MESSAGELOG) != 0);
		break;
	case OPTION_SHOW_QUEUE:
		ToggleTool(XRCID("ID_TOOLBAR_QUEUEVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_QUEUE) != 0);
		break;
	case OPTION_SHOW_TREE_LOCAL:
		ToggleTool(XRCID("ID_TOOLBAR_LOCALTREEVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_TREE_LOCAL) != 0);
		break;
	case OPTION_SHOW_TREE_REMOTE:
		ToggleTool(XRCID("ID_TOOLBAR_REMOTETREEVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_TREE_REMOTE) != 0);
		break;
	case OPTION_MESSAGELOG_POSITION:
		if (COptions::Get()->GetOptionVal(OPTION_MESSAGELOG_POSITION) == 2)
			HideTool(XRCID("ID_TOOLBAR_LOGVIEW"));
		else
		{
			ShowTool(XRCID("ID_TOOLBAR_LOGVIEW"));
			ToggleTool(XRCID("ID_TOOLBAR_LOGVIEW"), COptions::Get()->GetOptionVal(OPTION_SHOW_MESSAGELOG) != 0);
		}
		break;
	default:
		break;
	}
}

#if defined(EVT_TOOL_DROPDOWN) && defined(__WXMSW__)
void CToolBar::MakeDropdownTool(int id)
{
	wxToolBarToolBase* pOldTool = FindById(id);
	if (!pOldTool)
		return;

	wxToolBarToolBase* pTool = new wxToolBarToolBase(0, id,
		pOldTool->GetLabel(), pOldTool->GetNormalBitmap(), pOldTool->GetDisabledBitmap(),
		wxITEM_DROPDOWN, NULL, pOldTool->GetShortHelp(), pOldTool->GetLongHelp());

	int pos = GetToolPos(id);
	wxASSERT(pos != wxNOT_FOUND);

	DeleteToolByPos(pos);
	InsertTool(pos, pTool);
	Realize();
}
#endif

bool CToolBar::ShowTool(int id)
{
	int offset = 0;

	std::map<int, wxToolBarToolBase*>::iterator iter;
	for (iter = m_hidden_tools.begin(); iter != m_hidden_tools.end(); ++iter)
	{
		if (iter->second->GetId() != id)
		{
			offset++;
			continue;
		}

		InsertTool(iter->first - offset, iter->second);
		Realize();
		m_hidden_tools.erase(iter);

		return true;
	}

	return false;
}

bool CToolBar::HideTool(int id)
{
	int pos = GetToolPos(id);
	if (pos == -1)
		return false;

	wxToolBarToolBase* tool = RemoveTool(id);
	if (!tool)
		return false;

	for (std::map<int, wxToolBarToolBase*>::iterator iter = m_hidden_tools.begin(); iter != m_hidden_tools.end(); ++iter)
	{
		if (iter->first > pos)
			break;

		pos++;
	}

	m_hidden_tools[pos] = tool;

	return true;
}
