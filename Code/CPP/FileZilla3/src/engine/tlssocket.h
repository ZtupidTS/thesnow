#ifndef __TLSSOCKET_H__
#define __TLSSOCKET_H__

#include <gnutls/gnutls.h>
#include "backend.h"
#include "socket.h"

class CControlSocket;
class CTlsSocket : protected CSocketEventHandler, public CBackend, public CSocketEventSource
{
public:
	enum TlsState
	{
		noconn,
		handshake,
		verifycert,
		conn,
		closing,
		closed
	};

	CTlsSocket(CSocketEventHandler* pEvtHandler, CSocket* pSocket, CControlSocket* pOwner);
	virtual ~CTlsSocket();

	bool Init();
	void Uninit();

	int Handshake(const CTlsSocket* pPrimarySocket = 0, bool try_resume = false);

	virtual int Read(void *buffer, unsigned int size, int& error);
	virtual int Peek(void *buffer, unsigned int size, int& error);
	virtual int Write(const void *buffer, unsigned int size, int& error);

	int Shutdown();

	void TrustCurrentCert(bool trusted);

	enum TlsState GetState() const { return m_tlsState; }

	wxString GetCipherName();
	wxString GetMacName();

	bool ResumedSession() const;

	// PEM formatted
	bool AddTrustedRootCertificate(const wxString& cert);

	static wxString ListTlsCiphers(wxString priority);

protected:

	bool InitSession();
	void UninitSession();
	bool CopySessionData(const CTlsSocket* pPrimarySocket);

	virtual void OnRateAvailable(enum CRateLimiter::rate_direction direction);

	int ContinueHandshake();
	void ContinueShutdown();

	int VerifyCertificate();

	enum TlsState m_tlsState;

	CControlSocket* m_pOwner;

	bool m_initialized;
	gnutls_session_t m_session;

	gnutls_certificate_credentials_t m_certCredentials;

	void LogError(int code);
	void PrintAlert();

	// Failure logs the error, uninits the session and sends a close event
	void Failure(int code, int socket_error);

	static ssize_t PushFunction(gnutls_transport_ptr_t ptr, const void* data, size_t len);
	static ssize_t PullFunction(gnutls_transport_ptr_t ptr, void* data, size_t len);
	ssize_t PushFunction(const void* data, size_t len);
	ssize_t PullFunction(void* data, size_t len);

	void TriggerEvents();

	void OnSocketEvent(CSocketEvent& event);
	void OnRead();
	void OnSend();

	bool m_canReadFromSocket;
	bool m_canWriteToSocket;
	bool m_canCheckCloseSocket;

	bool m_canTriggerRead;
	bool m_canTriggerWrite;

	bool m_socketClosed;

	CSocketBackend* m_pSocketBackend;
	CSocket* m_pSocket;

	bool m_shutdown_requested;

	// Due to the strange gnutls_record_send semantics, call it again
	// with 0 data and 0 length after GNUTLS_E_AGAIN and store the number
	// of bytes written. These bytes get skipped on next write from the
	// application.
	// This avoids the rule to call it again with the -same- data after
	// GNUTLS_E_AGAIN.
	void CheckResumeFailedReadWrite();
	bool m_lastReadFailed;
	bool m_lastWriteFailed;
	unsigned int m_writeSkip;

	// Peek data
	char* m_peekData;
	unsigned int m_peekDataLen;

	gnutls_datum_t m_implicitTrustedCert;

	bool m_socket_eof;

	bool m_require_root_trust;
};

#endif //__TLSSOCKET_H__
