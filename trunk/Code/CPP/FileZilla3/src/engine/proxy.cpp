#include <filezilla.h>
#include "proxy.h"
#include <errno.h>
#include "ControlSocket.h"

enum handshake_state
{
	http_wait,

	socks5_method,
	socks5_auth,
	socks5_request,
	socks5_request_addrtype,
	socks5_request_address
};

CProxySocket::CProxySocket(CSocketEventHandler* pEvtHandler, CSocket* pSocket, CControlSocket* pOwner)
	: CBackend(pEvtHandler), m_pOwner(pOwner)
{
	m_pSocket = pSocket;
	m_pSocket->SetEventHandler(this);

	m_proxyState = noconn;

	m_pSendBuffer = 0;
	m_pRecvBuffer = 0;

	m_proxyType = unknown;

	m_can_write = false;
	m_can_read = false;
}

CProxySocket::~CProxySocket()
{
	if (m_pSocket)
		m_pSocket->SetEventHandler(0);
	delete [] m_pSendBuffer;
	delete [] m_pRecvBuffer;
}

static wxString base64encode(const wxString& str)
{
	// Code shamelessly taken from wxWidgets and adopted to encode UTF-8 strings.
	// wxWidget's http class encodes string from arbitrary encoding into base64,
	// could as well encode /dev/random
	static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	wxString buf;

	const wxWX2MBbuf utf8 = str.mb_str(wxConvUTF8);
	const char* from = utf8;

	size_t len = strlen(from);
	while (len >= 3) { // encode full blocks first
		buf << wxString::Format(wxT("%c%c"), base64[(from[0] >> 2) & 0x3f], base64[((from[0] << 4) & 0x30) | ((from[1] >> 4) & 0xf)]);
		buf << wxString::Format(wxT("%c%c"), base64[((from[1] << 2) & 0x3c) | ((from[2] >> 6) & 0x3)], base64[from[2] & 0x3f]);
		from += 3;
		len -= 3;
	}
	if (len > 0) { // pad the remaining characters
		buf << wxString::Format(wxT("%c"), base64[(from[0] >> 2) & 0x3f]);
		if (len == 1) {
			buf << wxString::Format(wxT("%c="), base64[(from[0] << 4) & 0x30]);
		} else {
			buf << wxString::Format(wxT("%c%c"), base64[((from[0] << 4) & 0x30) | ((from[1] >> 4) & 0xf)], base64[(from[1] << 2) & 0x3c]);
		}
		buf << wxString::Format(wxT("="));
	}

	return buf;
}

int CProxySocket::Handshake(enum CProxySocket::ProxyType type, const wxString& host, unsigned int port, const wxString& user, const wxString& pass)
{
	if (type == CProxySocket::unknown || host == _T("") || port < 1 || port > 65535)
		return EINVAL;

	if (m_proxyState != noconn)
		return EALREADY;

	const wxWX2MBbuf host_raw = host.mb_str(wxConvUTF8);

	if (type != HTTP && type != SOCKS5)
		return EPROTONOSUPPORT;

	m_user = user;
	m_pass = pass;
	m_host = host;
	m_port = port;
	m_proxyType = type;

	m_proxyState = handshake;

	if (type == HTTP)
	{
		m_handshakeState = http_wait;

#if wxUSE_UNICODE
		wxWX2MBbuf challenge;
#else
		const wxWX2MBbuf challenge;
#endif
		int challenge_len;
		if (user != _T(""))
		{
			challenge = base64encode(user + _T(":") + pass).mb_str(wxConvUTF8);
			challenge_len = strlen(challenge);
		}
		else
		{
			challenge = (size_t)0;
			challenge_len = 0;
		}

		// Bit oversized, but be on the safe side
		m_pSendBuffer = new char[70 + strlen(host_raw) * 2 + 2*5 + challenge_len + 23];

		if (!challenge)
		{
			m_sendBufferLen = sprintf(m_pSendBuffer, "CONNECT %s:%u HTTP/1.1\r\nHost: %s:%u\r\nUser-Agent: FileZilla\r\n\r\n",
				(const char*)host_raw, port,
				(const char*)host_raw, port);
		}
		else
		{
			m_sendBufferLen = sprintf(m_pSendBuffer, "CONNECT %s:%u HTTP/1.1\r\nHost: %s:%u\r\nProxy-Authorization: Basic %s\r\nUser-Agent: FileZilla\r\n\r\n",
				(const char*)host_raw, port,
				(const char*)host_raw, port,
				(const char*)challenge);
		}

		m_pRecvBuffer = new char[4096];
		m_recvBufferLen = 4096;
		m_recvBufferPos = 0;
	}
	else
	{
		m_pSendBuffer = new char[4];
		m_pSendBuffer[0] = 5; // Protocol version
		if (user != _T(""))
		{
			m_pSendBuffer[1] = 2; // # auth methods supported
			m_pSendBuffer[2] = 0; // Method: No auth
			m_pSendBuffer[3] = 2; // Method: Username and password
			m_sendBufferLen = 4;
		}
		else
		{
			m_pSendBuffer[1] = 1; // # auth methods supported
			m_pSendBuffer[2] = 0; // Method: No auth
			m_sendBufferLen = 3;
		}

		m_pRecvBuffer = new char[1024];
		m_recvBufferLen = 2;
		m_recvBufferPos = 0;

		m_handshakeState = socks5_method;
	}

	return EINPROGRESS;
}

void CProxySocket::OnSocketEvent(CSocketEvent& event)
{
	switch (event.GetType())
	{
	case CSocketEvent::hostaddress:
		{
			const wxString& address = event.GetData();
			m_pOwner->LogMessage(Status, _("Connecting to %s..."), address.c_str());
		}
	case CSocketEvent::connection_next:
		if (event.GetError())
			m_pOwner->LogMessage(Status, _("Connection attempt failed with \"%s\", trying next address."), CSocket::GetErrorDescription(event.GetError()).c_str());
		break;
	case CSocketEvent::connection:
		if (event.GetError())
		{
			if (m_proxyState == handshake)
				m_proxyState = noconn;
			CSocketEvent *forwarded_event = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::connection, event.GetError());
			CSocketEventDispatcher::Get().SendEvent(forwarded_event);
		}
		else
		{
			m_pOwner->LogMessage(Status, _("Connection with proxy established, performing handshake..."));
		}
		break;
	case CSocketEvent::read:
		OnReceive();
		break;
	case CSocketEvent::write:
		OnSend();
		break;
	case CSocketEvent::close:
		OnReceive();
		/*if (m_proxyState == handshake)
			m_proxyState = noconn;
		m_pEvtHandler->AddPendingEvent(event);*/
		break;
	default:
		m_pOwner->LogMessage(Debug_Warning, _T("Unhandled socket event %d"), event.GetType());
		break;
	}
}

void CProxySocket::Detach()
{
	if (!m_pSocket)
		return;

	m_pSocket->SetEventHandler(0);
	m_pSocket = 0;
}

void CProxySocket::OnReceive()
{
	m_can_read = true;

	if (m_proxyState != handshake)
		return;

	switch (m_handshakeState)
	{
	case http_wait:
		for (;;)
		{
			int error;
			int do_read = m_recvBufferLen - m_recvBufferPos - 1;
			char* end = 0;
			for (int i = 0; i < 2; i++)
			{
				int read;
				if (!i)
					read = m_pSocket->Peek(m_pRecvBuffer + m_recvBufferPos, do_read, error);
				else
					read = m_pSocket->Read(m_pRecvBuffer + m_recvBufferPos, do_read, error);
				if (read == -1)
				{
					if (error != EAGAIN)
					{
						m_proxyState = noconn;
						CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, error);
						CSocketEventDispatcher::Get().SendEvent(evt);
					}
					else
						m_can_read = false;
					return;
				}
				if (!read)
				{
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
				if (m_pSendBuffer)
				{
					m_proxyState = noconn;
					m_pOwner->LogMessage(Debug_Warning, _T("Incoming data before requst fully sent"));
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}

				if (!i)
				{
					// Response ends with strstr
					m_pRecvBuffer[m_recvBufferPos + read] = 0;
					end = strstr(m_pRecvBuffer, "\r\n\r\n");
					if (!end)
					{
						if (m_recvBufferPos + read + 1 == m_recvBufferLen)
						{
							m_proxyState = noconn;
							m_pOwner->LogMessage(Debug_Warning, _T("Incoming header too large"));
							CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ENOMEM);
							CSocketEventDispatcher::Get().SendEvent(evt);
							return;
						}
						do_read = read;
					}
					else
						do_read = end - m_pRecvBuffer + 4 - m_recvBufferPos;
				}
				else
				{
					if (read != do_read)
					{
						m_proxyState = noconn;
						m_pOwner->LogMessage(Debug_Warning, _T("Could not read what got peeked"));
						CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
						CSocketEventDispatcher::Get().SendEvent(evt);
						return;
					}
					m_recvBufferPos += read;
				}
			}

			if (!end)
				continue;

			end = strchr(m_pRecvBuffer, '\r');
			wxASSERT(end);
			*end = 0;
			wxString reply(m_pRecvBuffer, wxConvUTF8);
			m_pOwner->LogMessage(Response, _("Proxy reply: %s"), reply.c_str());

			if (reply.Left(10) != _T("HTTP/1.1 2") && reply.Left(10) != _T("HTTP/1.0 2"))
			{
				m_proxyState = noconn;
				CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNRESET);
				CSocketEventDispatcher::Get().SendEvent(evt);
				return;
			}

			m_proxyState = conn;
			CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::connection, 0);
			CSocketEventDispatcher::Get().SendEvent(evt);
			return;
		}
	case socks5_method:
	case socks5_auth:
	case socks5_request:
	case socks5_request_addrtype:
	case socks5_request_address:
		if (m_pSendBuffer)
			return;
		while (m_recvBufferLen && m_can_read && m_proxyState == handshake)
		{
			int error;
			int read = m_pSocket->Read(m_pRecvBuffer + m_recvBufferPos, m_recvBufferLen, error);
			if (read == -1)
			{
				if (error != EAGAIN)
				{
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, error);
					CSocketEventDispatcher::Get().SendEvent(evt);
				}
				else
					m_can_read = false;
				return;
			}
			if (!read)
			{
				m_proxyState = noconn;
				CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
				CSocketEventDispatcher::Get().SendEvent(evt);
				return;
			}
			m_recvBufferPos += read;
			m_recvBufferLen -= read;

			if (m_recvBufferLen)
				continue;

			m_recvBufferPos = 0;

			// All data got read, parse it
			switch (m_handshakeState)
			{
			default:
				if (m_pRecvBuffer[0] != 5)
				{
					m_pOwner->LogMessage(Debug_Warning, _("Unknown SOCKS protocol version: %d"), (int)m_pRecvBuffer[0]);
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
				break;
			case socks5_auth:
				if (m_pRecvBuffer[0] != 1)
				{
					m_pOwner->LogMessage(Debug_Warning, _("Unknown protocol version of SOCKS Username/Password Authentication subnegotiation: %d"), (int)m_pRecvBuffer[0]);
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
				break;
			case socks5_request_address:
			case socks5_request_addrtype:
				// Nothing to do
				break;
			}
			switch (m_handshakeState)
			{
			case socks5_method:
				{
					const char method = m_pRecvBuffer[1];
					switch (method)
					{
					case 0:
						m_handshakeState = socks5_request;
						break;
					case 2:
						m_handshakeState = socks5_auth;
						break;
					default:
						m_pOwner->LogMessage(Debug_Warning, _("No supported SOCKS5 auth method"));
						m_proxyState = noconn;
						CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
						CSocketEventDispatcher::Get().SendEvent(evt);
						return;
					}
				}
				break;
			case socks5_auth:
				if (m_pRecvBuffer[1] != 0)
				{
					m_pOwner->LogMessage(Debug_Warning, _("Proxy authentication failed"));
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
				m_handshakeState = socks5_request;
				break;
			case socks5_request:
				if (m_pRecvBuffer[1])
				{
					wxString error;
					switch (m_pRecvBuffer[1])
					{
					case 1:
						error = _("General SOCKS server failure");
						break;
					case 2:
						error = _("Connection not allowed by ruleset");
						break;
					case 3:
						error = _("Network unreachable");
						break;
					case 4:
						error = _("Host unreachable");
						break;
					case 5:
						error = _("Connection refused");
						break;
					case 6:
						error = _("TTL expired");
						break;
					case 7:
						error = _("Command not supported");
						break;
					case 8:
						error = _("Address type not supported");
						break;
					default:
						error.Printf(_("Unassigned error code %d"), (int)(unsigned char)m_pRecvBuffer[1]);
						break;
					}

					m_pOwner->LogMessage(Debug_Warning, _("Proxy request failed: %s"), error.c_str());
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
				m_handshakeState = socks5_request_addrtype;
				m_recvBufferLen = 3;
				break;
			case socks5_request_addrtype:
				switch (m_pRecvBuffer[1])
				{
				case 1:
					m_recvBufferLen = 5;
					break;
				case 3:
					m_recvBufferLen = m_pRecvBuffer[2] + 2;
					break;
				case 4:
					m_recvBufferLen = 17;
					break;
				default:
					m_pOwner->LogMessage(Debug_Warning, _("Proxy request failed: Unknown address type in CONNECT reply"));
					m_proxyState = noconn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
				m_handshakeState = socks5_request_address;
				break;
			case socks5_request_address:
				{
					// We're done
					m_proxyState = conn;
					CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::connection, 0);
					CSocketEventDispatcher::Get().SendEvent(evt);
					return;
				}
			default:
				wxFAIL;
				break;
			}

			switch (m_handshakeState)
			{
			case socks5_auth:
				{
					const wxWX2MBbuf user = m_user.mb_str(wxConvUTF8);
					const wxWX2MBbuf pass = m_pass.mb_str(wxConvUTF8);

					const int userlen = strlen(user);
					const int passlen = strlen(pass);
					m_sendBufferLen = userlen + passlen + 3;
					m_pSendBuffer = new char[m_sendBufferLen];
					m_pSendBuffer[0] = 1;
					m_pSendBuffer[1] = userlen;
					memcpy(m_pSendBuffer + 2, (const char*)user, userlen);
					m_pSendBuffer[userlen + 2] = passlen;
					memcpy(m_pSendBuffer + userlen + 3, (const char*)pass, passlen);
					m_recvBufferLen = 2;
				}
				break;
			case socks5_request:
				{
					const wxWX2MBbuf host = m_host.mb_str(wxConvUTF8);
					const int hostlen = strlen(host);
					int addrlen = wxMax(hostlen, 16);

					m_pSendBuffer = new char[7 + addrlen];
					m_pSendBuffer[0] = 5;
					m_pSendBuffer[1] = 1; // CONNECT
					m_pSendBuffer[2] = 0; // Reserved

					if (IsIpAddress(m_host))
					{
						// Quite ugly
						wxString ipv6 = GetIPV6LongForm(m_host);
						if (!ipv6.empty())
						{
							ipv6.Replace(_T(":"), _T(""));
							addrlen = 16;
							for (int i = 0; i < 16; i++)
								m_pSendBuffer[4 + i] = (DigitHexToDecNum(ipv6[i * 2]) << 4) + DigitHexToDecNum(ipv6[i * 2 + 1]);

							m_pSendBuffer[3] = 4; // IPv6
						}
						else
						{
							unsigned char *buf = (unsigned char*)m_pSendBuffer + 4;
							int i = 0;
							memset(buf, 0, 4);
							for (const wxChar* p = m_host.c_str(); *p; p++)
							{
								const wxChar& c = *p;
								if (c == '.')
								{
									i++;
									continue;
								}
								buf[i] *= 10;
								buf[i] += c - '0';
							}

							addrlen = 4;

							m_pSendBuffer[3] = 1; // IPv4
						}
					}
					else
					{
						m_pSendBuffer[3] = 3; // Domain name
						m_pSendBuffer[4] = hostlen;
						memcpy(m_pSendBuffer + 5, (const char*)host, hostlen);
						addrlen = hostlen + 1;
					}


					m_pSendBuffer[addrlen + 4] = (m_port >> 8) & 0xFF; // Port in network order
					m_pSendBuffer[addrlen + 5] = m_port & 0xFF;

					m_sendBufferLen = 6 + addrlen;
					m_recvBufferLen = 2;
				}
				break;
			case socks5_request_addrtype:
			case socks5_request_address:
				// Nothing to send, we simply need to wait for more data
				break;
			default:
				wxFAIL;
				break;
			}
			if (m_pSendBuffer && m_can_write)
				OnSend();
		}
		break;
	default:
		m_proxyState = noconn;
		m_pOwner->LogMessage(Debug_Warning, _T("Unhandled handshake state %d"), m_handshakeState);
		CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, ECONNABORTED);
		CSocketEventDispatcher::Get().SendEvent(evt);
		return;
	}
}

void CProxySocket::OnSend()
{
	m_can_write = true;
	if (m_proxyState != handshake || !m_pSendBuffer)
		return;

	for (;;)
	{
		int error;
		int written = m_pSocket->Write(m_pSendBuffer, m_sendBufferLen, error);
		if (written == -1)
		{
			if (error != EAGAIN)
			{
				m_proxyState = noconn;
				CSocketEvent *evt = new CSocketEvent(m_pEvtHandler, this, CSocketEvent::close, error);
				CSocketEventDispatcher::Get().SendEvent(evt);
			}
			else
				m_can_write = false;

			return;
		}

		if (written == m_sendBufferLen)
		{
			delete [] m_pSendBuffer;
			m_pSendBuffer = 0;

			if (m_can_read)
				OnReceive();
			return;
		}
		memmove(m_pSendBuffer, m_pSendBuffer + written, m_sendBufferLen - written);
		m_sendBufferLen -= written;
	}
}

int CProxySocket::Read(void *buffer, unsigned int size, int& error)
{
	error = EAGAIN;
	return -1;
}

int CProxySocket::Peek(void *buffer, unsigned int size, int& error)
{
	error = EAGAIN;
	return -1;
}

int CProxySocket::Write(const void *buffer, unsigned int size, int& error)
{
	error = EAGAIN;
	return -1;
}
