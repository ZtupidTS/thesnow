#ifndef __DIALOGEX_H__
#define __DIALOGEX_H__

#include "wrapengine.h"

class wxDialogEx : public wxDialog, public CWrapEngine
{
public:
	bool Load(wxWindow *pParent, const wxString& name);

	bool SetLabel(int id, const wxString& label, unsigned long maxLength = 0);
	wxString GetLabel(int id);

	virtual int ShowModal();

	bool ReplaceControl(wxWindow* old, wxWindow* wnd);

	static int ShownDialogs() { return m_shown_dialogs; }

protected:

	DECLARE_EVENT_TABLE();
	virtual void OnChar(wxKeyEvent& event);

	static int m_shown_dialogs;
};

#endif //__DIALOGEX_H__
