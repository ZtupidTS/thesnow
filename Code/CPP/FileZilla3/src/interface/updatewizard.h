#ifndef __UPDATEWIZARD_H__
#define __UPDATEWIZARD_H__

#if FZ_MANUALUPDATECHECK

#include <wx/wizard.h>
#include "wrapengine.h"

class COptionsBase;
class CUpdateWizard : public wxWizard, protected CWrapEngine
{
public:
	CUpdateWizard(wxWindow* pParent);
	virtual ~CUpdateWizard();

	bool Load();
	bool Run();

	void InitAutoUpdateCheck();
	void DisplayUpdateAvailability(bool showDialog);

protected:
	void FailedTransfer();
	void ParseData();

	void PrepareUpdateAvailablePage(const wxString &newVersion, wxString newUrl, const wxString& newChecksum);

	void RewrapPage(int page);

	wxString m_data;

	wxWindow* m_parent;

	std::vector<wxWizardPageSimple*> m_pages;

	CFileZillaEngine* m_pEngine;

	void SetTransferStatus(const CTransferStatus* pStatus);

	void PrepareUpdateCheckPage();
	void StartUpdateCheck();

	int SendTransferCommand();

	CLocalPath GetDownloadDir() const;
	bool SetLocalFile();

	bool VerifyChecksum();
	void FailedChecksum();

	DECLARE_EVENT_TABLE()
	void OnCheck(wxCommandEvent& event);
	void OnPageChanging(wxWizardEvent& event);
	void OnPageChanged(wxWizardEvent& event);
	void OnFinish(wxWizardEvent& event);
	void OnEngineEvent(wxEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnCancel(wxWizardEvent& event);

	bool m_inTransfer;
	bool m_skipPageChanging;

	int m_currentPage;

	enum ServerProtocol m_urlProtocol;
	wxString m_urlServer;
	wxString m_urlFile;
	wxString m_localFile;
	wxString m_update_checksum;

	wxString m_update_log;

	wxTimer m_statusTimer;
	
	// If there's already another dialog shown, wait
	// with displaying update dialog until the other
	// dialog gets closed. Check once every second.
	wxTimer m_busy_timer;

	bool m_loaded;

	// Hold changelog of what's new
	wxString m_news;

	// Auto check related functions and variables
	// ------------------------------------------

	bool CanAutoCheckForUpdateNow();

	wxTimer m_autoCheckTimer;

	bool m_autoUpdateCheckRunning;
	bool m_updateShown;
	bool m_menuUpdated;
	bool m_start_check;

	COptionsBase* m_update_options;

	bool m_successfully_downloaded;
};

#endif //FZ_MANUALUPDATECHECK

#endif //__UPDATEWIZARD_H__
