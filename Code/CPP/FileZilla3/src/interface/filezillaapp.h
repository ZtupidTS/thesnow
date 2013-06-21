#ifndef __FILEZILLAAPP_H__
#define __FILEZILLAAPP_H__

#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION
#include <wx/debugrpt.h>
#endif

#include <list>

class CWrapEngine;
class CCommandLine;
class CFileZillaApp : public wxApp
{
public:
	CFileZillaApp();
	virtual ~CFileZillaApp();

	virtual bool OnInit();
	virtual int OnExit();

	wxString GetResourceDir() const { return m_resourceDir; }
	wxString GetDefaultsDir() const { return m_defaultsDir; }
	wxString GetLocalesDir() const { return m_localesDir; }

	void CheckExistsFzsftp();

	bool SetLocale(int language);
	int GetCurrentLanguage() const;
	wxString GetCurrentLanguageCode() const;

	void DisplayEncodingWarning();

	CWrapEngine* GetWrapEngine();

	const CCommandLine* GetCommandLine() const { return m_pCommandLine; }

#if 0 // Disabled for now due to some wx controls expecting wxYield to process all events.
#if wxMAJOR_VERSION == 2 && wxMINOR_VERSION == 8
	#define USE_CHUNKED_PROCESS_PENDING_EVENTS 1
#endif
#endif
#if USE_CHUNKED_PROCESS_PENDING_EVENTS
	virtual void ProcessPendingEvents();
#endif

	void ShowStartupProfile();
	void AddStartupProfileRecord(const wxString& msg);

protected:
	bool InitDefaultsDir();
	bool LoadResourceFiles();
	bool LoadLocales();
	int ProcessCommandLine();

	wxLocale* m_pLocale;

	wxString m_resourceDir;
	wxString m_defaultsDir;
	wxString m_localesDir;

#if wxUSE_DEBUGREPORT && wxUSE_ON_FATAL_EXCEPTION
	virtual void OnFatalException();
	void GenerateReport(wxDebugReport::Context ctx);
#endif

	wxString GetDataDir(wxString fileToFind) const;

	// FileExists accepts full paths as parameter,
	// with the addition that path segments may be obmitted
	// with a wildcard (*). A matching directory will then be searched.
	// Example: FileExists(_T("/home/*/.filezilla/filezilla.xml"));
	bool FileExists(const wxString& file) const;

	CWrapEngine* m_pWrapEngine;

	CCommandLine* m_pCommandLine;

	bool m_profilingActive;
	std::list<std::pair<wxDateTime, wxString> > m_startupProfile;
};

DECLARE_APP(CFileZillaApp)

#endif //__FILEZILLAAPP_H__
