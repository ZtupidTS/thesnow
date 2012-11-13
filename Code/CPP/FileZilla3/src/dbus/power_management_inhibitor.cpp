#include <wx/wx.h>
#include "power_management_inhibitor.h"
#include "wxdbusconnection.h"
#include "wxdbusmessage.h"
#include <memory>

class CPowerManagementInhibitorImpl : private wxEvtHandler
{
public:
	CPowerManagementInhibitorImpl();
	virtual ~CPowerManagementInhibitorImpl();

	void RequestIdle();
	void RequestBusy();
private:
	wxDBusConnection* m_pConnection;

	DECLARE_EVENT_TABLE();
	void OnSignal(wxDBusConnectionEvent& event);
	void OnAsyncReply(wxDBusConnectionEvent& event);

	enum _state
	{
		error,
		idle,
		request_busy,
		busy,
		request_idle
	};

	enum _state m_state;

	bool m_debug;

	enum _state m_intended_state;
	unsigned int m_cookie;

	bool m_use_gsm;
};

BEGIN_EVENT_TABLE(CPowerManagementInhibitorImpl, wxEvtHandler)
EVT_DBUS_SIGNAL(wxID_ANY, CPowerManagementInhibitorImpl::OnSignal)
EVT_DBUS_ASYNC_RESPONSE(wxID_ANY, CPowerManagementInhibitorImpl::OnAsyncReply)
END_EVENT_TABLE()

CPowerManagementInhibitor::CPowerManagementInhibitor()
{
	impl = new CPowerManagementInhibitorImpl();
}

CPowerManagementInhibitor::~CPowerManagementInhibitor()
{
	delete impl;
}

void CPowerManagementInhibitor::RequestBusy()
{
	impl->RequestBusy();
}

void CPowerManagementInhibitor::RequestIdle()
{
	impl->RequestIdle();
}

CPowerManagementInhibitorImpl::CPowerManagementInhibitorImpl()
{
#ifdef __WXDEBUG__
	m_debug = true;
#else
	m_debug = false;
	wxString v;
	if (wxGetEnv(_T("FZDEBUG"), &v) && v == _T("1"))
		m_debug = true;
#endif
	m_pConnection = new wxDBusConnection(wxID_ANY, this, false);
	if (!m_pConnection->IsConnected())
	{
		if (m_debug)
			printf("wxD-Bus: Could not connect to session bus\n");
		m_state = error;
	}
	else
		m_state = idle;

	m_intended_state = idle;
	m_cookie = 0;
	m_use_gsm = false;
}

CPowerManagementInhibitorImpl::~CPowerManagementInhibitorImpl()
{
	// Closing connection clears the inhibition
	delete m_pConnection;
}

void CPowerManagementInhibitorImpl::RequestIdle()
{
	m_intended_state = idle;
	if (m_state == error || m_state == idle || m_state == request_idle || m_state == request_busy)
		return;

	if (m_debug)
		printf("wxD-Bus: CPowerManagementInhibitor: Requesting idle\n");

	wxDBusMethodCall *call;
	if (!m_use_gsm)
		call = new wxDBusMethodCall(
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement/Inhibit",
			"org.freedesktop.PowerManagement.Inhibit",
			"UnInhibit");
	else
		call = new wxDBusMethodCall(
			"org.gnome.SessionManager",
			"/org/gnome/SessionManager",
			"org.gnome.SessionManager",
			"Uninhibit");

	m_state = request_idle;

	call->AddUnsignedInt(m_cookie);

	if (!call->CallAsync(m_pConnection, 1000))
	{
		m_state = error;
		if (m_debug)
			printf("wxD-Bus: CPowerManagementInhibitor: Request failed\n");
	}

	delete call;
}


void CPowerManagementInhibitorImpl::RequestBusy()
{
	m_intended_state = busy;
	if (m_state == error || m_state == busy || m_state == request_busy || m_state == request_idle)
		return;

	if (m_debug)
		printf("wxD-Bus: CPowerManagementInhibitor: Requesting busy\n");

	wxDBusMethodCall *call;
	if (!m_use_gsm)
		call = new wxDBusMethodCall(
			"org.freedesktop.PowerManagement",
			"/org/freedesktop/PowerManagement/Inhibit",
			"org.freedesktop.PowerManagement.Inhibit",
			"Inhibit");
	else
		call = new wxDBusMethodCall(
			"org.gnome.SessionManager",
			"/org/gnome/SessionManager",
			"org.gnome.SessionManager",
			"Inhibit");

	m_state = request_busy;

	call->AddString("FileZilla");
	if (m_use_gsm)
		call->AddUnsignedInt(0);
	call->AddString("File transfer or remote operation in progress");
	if (m_use_gsm)
		call->AddUnsignedInt(8);

	if (!call->CallAsync(m_pConnection, 1000))
	{
		if (m_debug)
			printf("wxD-Bus: CPowerManagementInhibitor: Request failed\n");
		if (m_use_gsm)
			m_state = error;
		else
		{
			if (m_debug)
				printf("wxD-Bus: Falling back to org.gnome.SessionManager\n");
			m_use_gsm = true;
			RequestBusy();
		}
	}

	delete call;
}

void CPowerManagementInhibitorImpl::OnSignal(wxDBusConnectionEvent& event)
{
	std::auto_ptr<wxDBusMessage> msg(wxDBusMessage::ExtractFromEvent(&event));
}

void CPowerManagementInhibitorImpl::OnAsyncReply(wxDBusConnectionEvent& event)
{
	std::auto_ptr<wxDBusMessage> msg(wxDBusMessage::ExtractFromEvent(&event));

	if (m_state == error)
		return;

	if (msg->GetType() == DBUS_MESSAGE_TYPE_ERROR)
	{
		if (m_debug)
			printf("wxD-Bus: Reply: Error: %s\n", msg->GetString());

		if (m_state == request_busy && !m_use_gsm)
		{
			if (m_debug)
				printf("wxD-Bus: Falling back to org.gnome.SessionManager\n");
			m_use_gsm = true;
			m_state = idle;
			if (m_intended_state == busy)
				RequestBusy();
		}
		else
			m_state = error;
		return;
	}

	if (m_state == request_idle)
	{
		m_state = idle;
		if (m_debug)
			printf("wxD-Bus: CPowerManagementInhibitor: Request successful\n");
		if (m_intended_state == busy)
			RequestBusy();
		return;
	}
	else if (m_state == request_busy)
	{
		m_state = busy;
		msg->GetUInt(m_cookie);
		if (m_debug)
			printf("wxD-Bus: CPowerManagementInhibitor: Request successful, cookie is %u\n", m_cookie);
		if (m_intended_state == idle)
			RequestIdle();
		return;
	}
	else
	{
		if (m_debug)
			printf("wxD-Bus: Unexpected reply in state %d\n", m_state);
		m_state = error;
	}
}
