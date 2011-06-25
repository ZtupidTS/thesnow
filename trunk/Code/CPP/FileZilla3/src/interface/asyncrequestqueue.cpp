#include <filezilla.h>

#include "asyncrequestqueue.h"
#include "defaultfileexistsdlg.h"
#include "fileexistsdlg.h"
#include "loginmanager.h"
#include "Mainfrm.h"
#include "Options.h"
#include "queue.h"
#include "verifycertdialog.h"
#include "verifyhostkeydialog.h"

DECLARE_EVENT_TYPE(fzEVT_PROCESSASYNCREQUESTQUEUE, -1)
DEFINE_EVENT_TYPE(fzEVT_PROCESSASYNCREQUESTQUEUE)

BEGIN_EVENT_TABLE(CAsyncRequestQueue, wxEvtHandler)
EVT_COMMAND(wxID_ANY, fzEVT_PROCESSASYNCREQUESTQUEUE, CAsyncRequestQueue::OnProcessQueue)
EVT_TIMER(wxID_ANY, CAsyncRequestQueue::OnTimer)
END_EVENT_TABLE()

CAsyncRequestQueue::CAsyncRequestQueue(CMainFrame *pMainFrame)
{
	m_pMainFrame = pMainFrame;
	m_pQueueView = 0;
	m_pVerifyCertDlg = new CVerifyCertDialog;
	m_inside_request = false;
	m_timer.SetOwner(this);
}

CAsyncRequestQueue::~CAsyncRequestQueue()
{
	delete m_pVerifyCertDlg;
}

bool CAsyncRequestQueue::ProcessDefaults(CFileZillaEngine *pEngine, CAsyncRequestNotification *pNotification)
{
	// Process notifications, see if we have defaults not requirering user interaction.
	switch (pNotification->GetRequestID())
	{
	case reqId_fileexists:
		{
			CFileExistsNotification *pFileExistsNotification = reinterpret_cast<CFileExistsNotification *>(pNotification);

			// Get the action, go up the hierarchy till one is found
			enum CFileExistsNotification::OverwriteAction action = pFileExistsNotification->overwriteAction;
			if (action == CFileExistsNotification::unknown)
				action = CDefaultFileExistsDlg::GetDefault(pFileExistsNotification->download);
			if (action ==CFileExistsNotification::unknown)
			{
				int option = COptions::Get()->GetOptionVal(pFileExistsNotification->download ? OPTION_FILEEXISTS_DOWNLOAD : OPTION_FILEEXISTS_UPLOAD);
				if (option < CFileExistsNotification::unknown || option >= CFileExistsNotification::ACTION_COUNT)
					action = CFileExistsNotification::unknown;
				else
					action = (enum CFileExistsNotification::OverwriteAction)option;
			}

			// Ask and rename options require user interaction
			if (action == CFileExistsNotification::unknown || action == CFileExistsNotification::ask || action == CFileExistsNotification::rename)
				break;

			if (action == CFileExistsNotification::resume && pFileExistsNotification->ascii)
			{
				// Check if resuming ascii files is allowed
				if (!COptions::Get()->GetOptionVal(OPTION_ASCIIRESUME))
					// Overwrite instead
					action = CFileExistsNotification::overwrite;
			}

			pFileExistsNotification->overwriteAction = action;
			
			pEngine->SetAsyncRequestReply(pNotification);
			delete pNotification;

			return true;
		}
	case reqId_hostkey:
	case reqId_hostkeyChanged:
		{
			CHostKeyNotification *pHostKeyNotification = reinterpret_cast<CHostKeyNotification *>(pNotification);

			if (!CVerifyHostkeyDialog::IsTrusted(pHostKeyNotification))
				break;
			
			pHostKeyNotification->m_trust = true;
			pHostKeyNotification->m_alwaysTrust = false;

			pEngine->SetAsyncRequestReply(pNotification);
			delete pNotification;

			return true;
		}
	case reqId_certificate:
		{
			CCertificateNotification* pCertNotification = reinterpret_cast<CCertificateNotification *>(pNotification);

			if (!m_pVerifyCertDlg->IsTrusted(pCertNotification))
				break;

			pCertNotification->m_trusted = true;
			pEngine->SetAsyncRequestReply(pNotification);
			delete pNotification;

			return true;			
		}
		break;
	default:
		break;
	}

	return false;
}

bool CAsyncRequestQueue::AddRequest(CFileZillaEngine *pEngine, CAsyncRequestNotification *pNotification)
{
	ClearPending(pEngine);

	if (ProcessDefaults(pEngine, pNotification))
		return false;

	t_queueEntry entry;
	entry.pEngine = pEngine;
	entry.pNotification = pNotification;

	m_requestList.push_back(entry);

	if (m_requestList.size() == 1)
	{
		wxCommandEvent evt(fzEVT_PROCESSASYNCREQUESTQUEUE);
		wxPostEvent(this, evt);
	}

	return true;
}

bool CAsyncRequestQueue::ProcessNextRequest()
{
	if (m_requestList.empty())
		return true;

	t_queueEntry &entry = m_requestList.front();

	if (!entry.pEngine->IsPendingAsyncRequestReply(entry.pNotification))
	{
		delete entry.pNotification;
		m_requestList.pop_front();
		return true;
	}

	if (entry.pNotification->GetRequestID() == reqId_fileexists)
	{
		CFileExistsNotification *pNotification = reinterpret_cast<CFileExistsNotification *>(entry.pNotification);

		// Get the action, go up the hierarchy till one is found
		enum CFileExistsNotification::OverwriteAction action = pNotification->overwriteAction;
		if (action == CFileExistsNotification::unknown)
			action = CDefaultFileExistsDlg::GetDefault(pNotification->download);
		if (action == CFileExistsNotification::unknown)
		{
			int option = COptions::Get()->GetOptionVal(pNotification->download ? OPTION_FILEEXISTS_DOWNLOAD : OPTION_FILEEXISTS_UPLOAD);
			if (option <= CFileExistsNotification::unknown || option >= CFileExistsNotification::ACTION_COUNT)
				action = CFileExistsNotification::ask;
			else
				action = (enum CFileExistsNotification::OverwriteAction)option;
		}

		if (action == CFileExistsNotification::ask)
		{
			if (!CheckWindowState())
				return false;

			CFileExistsDlg dlg(pNotification);
			dlg.Create(m_pMainFrame);
			int res = dlg.ShowModal();

			if (res == wxID_OK)
			{
				action = dlg.GetAction();

				bool directionOnly, queueOnly;
				if (dlg.Always(directionOnly, queueOnly))
				{
					if (!queueOnly)
					{
						if (pNotification->download || !directionOnly)
							CDefaultFileExistsDlg::SetDefault(true, action);

						if (!pNotification->download || !directionOnly)
							CDefaultFileExistsDlg::SetDefault(false, action);
					}
					else
					{
						// For the notifications already in the request queue, we have to set the queue action directly
						for (std::list<t_queueEntry>::iterator iter = ++m_requestList.begin(); iter != m_requestList.end(); iter++)
						{
							if (pNotification->GetRequestID() != reqId_fileexists)
								continue;

							CFileExistsNotification* p = reinterpret_cast<CFileExistsNotification *>(iter->pNotification);

							if (!directionOnly || pNotification->download == p->download)
								p->overwriteAction = CFileExistsNotification::OverwriteAction(action);
						}

						enum TransferDirection direction;
						if (directionOnly)
						{
							if (pNotification->download)
								direction = download;
							else
								direction = upload;
						}
						else
							direction = both;

						if (m_pQueueView)
							m_pQueueView->SetDefaultFileExistsAction(action, direction);
					}
				}
			}
			else
				action = CFileExistsNotification::skip;
		}

		if (action == CFileExistsNotification::unknown || action == CFileExistsNotification::ask)
			action = CFileExistsNotification::skip;

		if (action == CFileExistsNotification::resume && pNotification->ascii)
		{
			// Check if resuming ascii files is allowed
			if (!COptions::Get()->GetOptionVal(OPTION_ASCIIRESUME))
				// Overwrite instead
				action = CFileExistsNotification::overwrite;
		}

		switch (action)
		{
		case CFileExistsNotification::rename:
			{
				if (!CheckWindowState())
					return false;

				wxString msg;
				wxString defaultName;
				if (pNotification->download)
				{
					msg.Printf(_("The file %s already exists.\nPlease enter a new name:"), pNotification->localFile.c_str());
					wxFileName fn = pNotification->localFile;
					defaultName = fn.GetFullName();
				}
				else
				{
					wxString fullName = pNotification->remotePath.GetPath() + pNotification->remoteFile;
					msg.Printf(_("The file %s already exists.\nPlease enter a new name:"), fullName.c_str());
					defaultName = pNotification->remoteFile;
				}
				wxTextEntryDialog dlg(m_pMainFrame, msg, _("Rename file"), defaultName);

				// Repeat until user cancels or enters a new name
				while (1)
				{
					int res = dlg.ShowModal();
					if (res == wxID_OK)
					{
						if (dlg.GetValue() == _T(""))
							continue; // Disallow empty names
						if (dlg.GetValue() == defaultName)
						{
							wxMessageDialog dlg2(m_pMainFrame, _("You did not enter a new name for the file. Overwrite the file instead?"), _("Filename unchanged"), 
								wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION | wxCANCEL);
							int res = dlg2.ShowModal();

							if (res == wxID_CANCEL)
								pNotification->overwriteAction = CFileExistsNotification::skip;
							else if (res == wxID_NO)
								continue;
							else
								pNotification->overwriteAction = CFileExistsNotification::skip;
						}
						else
						{
							pNotification->overwriteAction = CFileExistsNotification::rename;
							pNotification->newName = dlg.GetValue();

							// If request got processed successfully, notify queue about filename change
							if (entry.pEngine->SetAsyncRequestReply(entry.pNotification) && m_pQueueView)
								m_pQueueView->RenameFileInTransfer(entry.pEngine, dlg.GetValue(), pNotification->download);
							delete pNotification;

							// Jump near end of function
							goto ProcessNextRequest_done;
						}
					}
					else
						pNotification->overwriteAction = CFileExistsNotification::skip;
					break;
				}
			}
			break;
		default:
			pNotification->overwriteAction = action;
			break;
		}

		entry.pEngine->SetAsyncRequestReply(entry.pNotification);
		delete pNotification;
	}
	else if (entry.pNotification->GetRequestID() == reqId_interactiveLogin)
	{
		CInteractiveLoginNotification* pNotification = reinterpret_cast<CInteractiveLoginNotification*>(entry.pNotification);

		if (CLoginManager::Get().GetPassword(pNotification->server, true, _T(""), pNotification->GetChallenge()))
			pNotification->passwordSet = true;
		else
		{
			// Retry with prompt

			if (!CheckWindowState())
				return false;

			if (CLoginManager::Get().GetPassword(pNotification->server, false, _T(""), pNotification->GetChallenge()))
				pNotification->passwordSet = true;
		}

		entry.pEngine->SetAsyncRequestReply(pNotification);
		delete pNotification;
	}
	else if (entry.pNotification->GetRequestID() == reqId_hostkey || entry.pNotification->GetRequestID() == reqId_hostkeyChanged)
	{
		if (!CheckWindowState())
			return false;

		CHostKeyNotification *pNotification = reinterpret_cast<CHostKeyNotification *>(entry.pNotification);

		if (CVerifyHostkeyDialog::IsTrusted(pNotification))
		{
			pNotification->m_trust = true;
			pNotification->m_alwaysTrust = false;
		}
		else
			CVerifyHostkeyDialog::ShowVerificationDialog(m_pMainFrame, pNotification);

		entry.pEngine->SetAsyncRequestReply(pNotification);
		delete pNotification;
	}
	else if (entry.pNotification->GetRequestID() == reqId_certificate)
	{
		if (!CheckWindowState())
			return false;

		CCertificateNotification* pNotification = reinterpret_cast<CCertificateNotification *>(entry.pNotification);

		m_pVerifyCertDlg->ShowVerificationDialog(pNotification);

		entry.pEngine->SetAsyncRequestReply(entry.pNotification);
		delete entry.pNotification;
	}
	else
	{
		entry.pEngine->SetAsyncRequestReply(entry.pNotification);
		delete entry.pNotification;
	}

ProcessNextRequest_done:
	RecheckDefaults();
	m_requestList.pop_front();

	return true;
}

void CAsyncRequestQueue::ClearPending(const CFileZillaEngine *pEngine)
{
	if (m_requestList.empty())
		return;
	
	// Remove older requests coming from the same engine, but never the first
	// entry in the list as that one displays a dialog at this moment.
	for (std::list<t_queueEntry>::iterator iter = ++m_requestList.begin(); iter != m_requestList.end(); iter++)
	{
		if (iter->pEngine == pEngine)
		{
			m_requestList.erase(iter);

			// At most one pending request per engine possible, 
			// so we can stop here
			break;
		}
	}
}

void CAsyncRequestQueue::RecheckDefaults()
{
	if (m_requestList.size() <= 1)
		return;

	std::list<t_queueEntry>::iterator cur, next;
	cur = ++m_requestList.begin();
	while (cur != m_requestList.end())
	{
		next = cur;
		next++;

		if (ProcessDefaults(cur->pEngine, cur->pNotification))
			m_requestList.erase(cur);
		cur = next;
	}
}

void CAsyncRequestQueue::SetQueue(CQueueView *pQueue)
{
	m_pQueueView = pQueue;
}

void CAsyncRequestQueue::OnProcessQueue(wxCommandEvent &event)
{
	if (m_inside_request)
		return;

	m_inside_request = true;
	bool success = ProcessNextRequest();
	m_inside_request = false;

	if (success && !m_requestList.empty())
	{
		wxCommandEvent evt(fzEVT_PROCESSASYNCREQUESTQUEUE);
		wxPostEvent(this, evt);
	}
}

void CAsyncRequestQueue::TriggerProcessing()
{
	if (m_inside_request)
		return;

	wxCommandEvent evt(fzEVT_PROCESSASYNCREQUESTQUEUE);
	wxPostEvent(this, evt);
}

bool CAsyncRequestQueue::CheckWindowState()
{
	m_timer.Stop();
	wxMouseState mouseState = wxGetMouseState();
	if (mouseState.LeftDown() || mouseState.MiddleDown() || mouseState.RightDown())
	{
		m_timer.Start(1000, true);
		return false;
	}

#ifndef __WXMAC__
	if (m_pMainFrame->IsIconized())
	{
#ifndef __WXGTK__
		m_pMainFrame->Show();
		m_pMainFrame->Iconize(true);
		m_pMainFrame->RequestUserAttention();
#endif
		return false;
	}

	wxWindow* pFocus = m_pMainFrame->FindFocus();
	while (pFocus && pFocus != m_pMainFrame)
		pFocus = pFocus->GetParent();
	if (!pFocus)
		m_pMainFrame->RequestUserAttention();
#endif

	return true;
}

void CAsyncRequestQueue::OnTimer(wxTimerEvent& event)
{
	TriggerProcessing();
}

