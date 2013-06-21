#ifndef __NOTIFICATION_H__
#define __NOTIFICATION_H__

// Notification overview
// ---------------------

// To inform the application about what's happening, the engine sends
// some notifications to the application using events.
// For example the event table entry could look like this:
//     EVT_FZ_NOTIFICATION(wxID_ANY, CMainFrame::OnEngineEvent)
// and the handler function has the following declaration:
//     void OnEngineEvent(wxEvent& event);
// You can get the engine which sent the event by calling
//     event.GetEventObject()
// Whenever you get a notification event,
// CFileZillaEngine::GetNextNotification has to be called until it returns 0,
// or you will lose important notifications or your memory will fill with
// pending notifications.

// A special class of notifications are the asynchronous requests. These
// requests have to be answered. Once proessed, call
// CFileZillaEngine::SetAsyncRequestReply to continue the current operation.

#include "local_path.h"

extern const wxEventType fzEVT_NOTIFICATION;
#define EVT_FZ_NOTIFICATION(id, fn) \
	DECLARE_EVENT_TABLE_ENTRY(      \
		fzEVT_NOTIFICATION, id, -1, \
		(wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxEventFunction, &fn ), \
		(wxObject *) NULL           \
	),

class wxFzEvent : public wxEvent
{
public:
	wxFzEvent(int id = wxID_ANY);
	virtual wxEvent *Clone() const;
};

enum NotificationId
{
	nId_logmsg,				// notification about new messages for the message log
	nId_operation,			// operation reply codes
	nId_connection,			// connection information: connects, disconnects, timeouts etc..
	nId_transferstatus,		// transfer information: bytes transferes, transfer speed and such
	nId_listing,			// directory listings
	nId_asyncrequest,		// asynchronous request
	nId_active,				// sent if data gets either received or sent
	nId_data,				// for memory downloads, indicates that new data is available.
	nId_sftp_encryption,	// information about key exchange, encryption algorithms and so on for SFTP
	nId_local_dir_created	// local directory has been created
};

// Async request IDs
enum RequestId
{
	reqId_fileexists,		// Target file already exists, awaiting further instructions
	reqId_interactiveLogin, // gives a challenge prompt for a password
	reqId_hostkey,			// used only by SSH/SFTP to indicate new host key
	reqId_hostkeyChanged,	// used only by SSH/SFTP to indicate changed host key
	reqId_certificate		// sent after a successful TLS/SSL handshake to allow certificate
							// validation
};

class CNotification
{
public:
	CNotification();
	virtual ~CNotification();
	virtual enum NotificationId GetID() const = 0;
};

class CLogmsgNotification : public CNotification
{
public:
	CLogmsgNotification();
	virtual ~CLogmsgNotification();
	virtual enum NotificationId GetID() const;

	wxString msg;
	enum MessageType msgType; // Type of message, see logging.h for details
};

// If CFileZillaEngine does return with FZ_REPLY_WOULDBLOCK, you will receive
// a nId_operation notification once the operation ends.
class COperationNotification : public CNotification
{
public:
	COperationNotification();
	virtual ~COperationNotification();
	virtual enum NotificationId GetID() const;

	int nReplyCode;
	enum Command commandId;
};

// You get this type of notification everytime a directory listing has been
// requested explicitely or when a directory listing was retrieved implicitely
// during another operation, e.g. file transfers.
class CDirectoryListing;
class CDirectoryListingNotification : public CNotification
{
public:
	CDirectoryListingNotification(const CServerPath& path, const bool modified = false, const bool failed = false);
	virtual ~CDirectoryListingNotification();
	virtual enum NotificationId GetID() const;
	bool Modified() const { return m_modified; }
	bool Failed() const { return m_failed; }
	const CServerPath GetPath() const { return m_path; }

protected:
	bool m_modified;
	bool m_failed;
	CServerPath m_path;
};

class CAsyncRequestNotification : public CNotification
{
public:
	CAsyncRequestNotification();
	virtual ~CAsyncRequestNotification();
	virtual enum NotificationId GetID() const;

	virtual enum RequestId GetRequestID() const = 0;

	unsigned int requestNumber; // Do never change this
};

class CFileExistsNotification : public CAsyncRequestNotification
{
public:
	CFileExistsNotification();
	virtual ~CFileExistsNotification();
	virtual enum RequestId GetRequestID() const;

	bool download;

	wxString localFile;
	wxLongLong localSize;
	wxDateTime localTime;

	wxString remoteFile;
	CServerPath remotePath;
	wxLongLong remoteSize;
	wxDateTime remoteTime;

	bool ascii;

	bool canResume;

	// overwriteAction will be set by the request handler
	enum OverwriteAction
	{
		unknown = -1,
		ask,
		overwrite,
		overwriteNewer,	// Overwrite if source file is newer than target file
		overwriteSize,	// Overwrite if source file is is different in size than target file
		overwriteSizeOrNewer,	// Overwrite if source file is different in size or newer than target file
		resume, // Overwrites if cannot be resumed
		rename,
		skip,

		ACTION_COUNT
	};

	// Set overwriteAction to the desired action
	enum OverwriteAction overwriteAction;

	// Set to new filename if overwriteAction is rename. Might trigger further
	// file exists notifications if new target file exists as well.
	wxString newName;
};

class CInteractiveLoginNotification : public CAsyncRequestNotification
{
public:
	CInteractiveLoginNotification(const wxString& challenge);
	virtual ~CInteractiveLoginNotification();
	virtual enum RequestId GetRequestID() const;

	// Set to true if you have set a password
	bool passwordSet;

	// Set password by calling server.SetUser
	CServer server;

	const wxString& GetChallenge() const { return m_challenge; }

protected:
	// Password prompt string as given by the server
	const wxString m_challenge;
};

// Indicate network action.
class CActiveNotification : public CNotification
{
public:
	CActiveNotification(int direction);
	virtual ~CActiveNotification();
	virtual enum NotificationId GetID() const;
	int GetDirection() const { return m_direction; }
protected:
	const int m_direction;
};

class CTransferStatus
{
public:
	wxDateTime started;
	wxFileOffset totalSize;		// Total size of the file to transfer, -1 if unknown
	wxFileOffset startOffset;
	wxFileOffset currentOffset;

	// True on download notifications iff currentOffset != startOffset.
	// True on FTP upload notifications iff currentOffset != startOffset
	// AND after the first accepted data after the first wxSOCKET_WOULDBLOCK.
	// SFTP uploads: Set to true if currentOffset >= startOffset + 65536.
	bool madeProgress;

	bool list;
};

class CTransferStatusNotification : public CNotification
{
public:
	CTransferStatusNotification(CTransferStatus *pStatus);
	virtual ~CTransferStatusNotification();
	virtual enum  NotificationId GetID() const;

	const CTransferStatus *GetStatus() const;

protected:
	CTransferStatus *m_pStatus;
};

// Notification about new or changed hostkeys, only used by SSH/SFTP transfers.
// GetRequestID() returns either reqId_hostkey or reqId_hostkeyChanged
class CHostKeyNotification : public CAsyncRequestNotification
{
public:
	CHostKeyNotification(wxString host, int port, wxString fingerprint, bool changed = false);
	virtual ~CHostKeyNotification();
	virtual enum RequestId GetRequestID() const;

	wxString GetHost() const;
	int GetPort() const;
	wxString GetFingerprint() const;

	// Set to true if you trust the server
	bool m_trust;

	// If m_truest is true, set this to true to always trust this server
	// in future.
	bool m_alwaysTrust;

protected:

	const wxString m_host;
	const int m_port;
	const wxString m_fingerprint;
	const bool m_changed;
};

class CDataNotification : public CNotification
{
public:
	CDataNotification(char* pData, int len);
	virtual ~CDataNotification();
	virtual enum NotificationId GetID() const { return nId_data; }

	char* Detach(int& len);

protected:
	char* m_pData;
	unsigned int m_len;
};

class CCertificate
{
public:
	CCertificate();

	CCertificate(
		const unsigned char* rawData, unsigned int len,
		wxDateTime activationTime, wxDateTime expirationTime,
		const wxString& serial,
		const wxString& pkalgoname, unsigned int bits,
		const wxString& signalgoname,
		const wxString& fingerprint_md5,
		const wxString& fingerprint_sha1,
		const wxString& subject,
		const wxString& issuer);
	CCertificate(const CCertificate& op);
	virtual ~CCertificate();

	const unsigned char* GetRawData(unsigned int& len) const { len = m_len; return m_rawData; }
	wxDateTime GetActivationTime() const { return m_activationTime; }
	wxDateTime GetExpirationTime() const { return m_expirationTime; }

	const wxString& GetSerial() const { return m_serial; }
	const wxString& GetPkAlgoName() const { return m_pkalgoname; }
	unsigned int GetPkAlgoBits() const { return m_pkalgobits; }

	const wxString& GetSignatureAlgorithm() const { return m_signalgoname; }

	const wxString& GetFingerPrintMD5() const { return m_fingerprint_md5; }
	const wxString& GetFingerPrintSHA1() const { return m_fingerprint_sha1; }

	const wxString& GetSubject() const { return m_subject; }
	const wxString& GetIssuer() const { return m_issuer; }

	CCertificate& operator=(const CCertificate &op);

private:
	wxDateTime m_activationTime;
	wxDateTime m_expirationTime;

	unsigned char* m_rawData;
	unsigned int m_len;

	wxString m_serial;
	wxString m_pkalgoname;
	unsigned int m_pkalgobits;

	wxString m_signalgoname;

	wxString m_fingerprint_md5;
	wxString m_fingerprint_sha1;

	wxString m_subject;
	wxString m_issuer;
};

class CCertificateNotification : public CAsyncRequestNotification
{
public:
	CCertificateNotification(const wxString& host, unsigned int port,
		const wxString& protocol,
		const wxString& keyExchange,
		const wxString& sessionCipher,
		const wxString& sessionMac,
		const std::vector<CCertificate> &certificates);
	virtual ~CCertificateNotification();
	virtual enum RequestId GetRequestID() const { return reqId_certificate; }

	const wxString& GetHost() const { return m_host; }
	unsigned int GetPort() const { return m_port; }

	const wxString& GetSessionCipher() const { return m_sessionCipher; }
	const wxString& GetSessionMac() const { return m_sessionMac; }

	bool m_trusted;

	const std::vector<CCertificate> GetCertificates() const { return m_certificates; }

	const wxString& GetProtocol() const { return m_protocol; }
	const wxString& GetKeyExchange() const { return m_keyExchange; }

protected:
	wxString m_host;
	unsigned int m_port;

	wxString m_protocol;
	wxString m_keyExchange;
	wxString m_sessionCipher;
	wxString m_sessionMac;

	std::vector<CCertificate> m_certificates;
};

class CSftpEncryptionNotification : public CNotification
{
public:
	virtual enum NotificationId GetID() const { return nId_sftp_encryption; }

	wxString hostKey;
	wxString kexAlgorithm;
	wxString kexHash;
	wxString cipherClientToServer;
	wxString cipherServerToClient;
	wxString macClientToServer;
	wxString macServerToClient;
};

class CLocalDirCreatedNotification : public CNotification
{
public:
	virtual enum NotificationId GetID() const { return nId_local_dir_created; }

	CLocalPath dir;
};

#endif
