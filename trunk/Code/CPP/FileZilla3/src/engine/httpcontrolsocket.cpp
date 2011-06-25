#include <filezilla.h>

#include "ControlSocket.h"
#include "httpcontrolsocket.h"
#include "tlssocket.h"

#include <wx/file.h>

#include <errno.h>

#define FZ_REPLY_REDIRECTED FZ_REPLY_ALREADYCONNECTED

// Connect is special for HTTP: It is done on a per-command basis, so we need
// to establish a connection before each command.
class CHttpConnectOpData : public CConnectOpData
{
public:
	CHttpConnectOpData()
		: tls(false)
	{
	}

	virtual ~CHttpConnectOpData()
	{
	}

	bool tls;
};

class CHttpOpData
{
public:
	CHttpOpData(COpData* pOpData)
		: m_pOpData(pOpData)
	{
		m_gotHeader = false;
		m_responseCode = -1;
		m_redirectionCount = 0;
		m_transferEncoding = unknown;

		m_chunkData.getTrailer = false;
		m_chunkData.size = 0;
		m_chunkData.terminateChunk = false;

		m_totalSize = -1;
		m_receivedData = 0;
	}

	virtual ~CHttpOpData() {}

	bool m_gotHeader;
	int m_responseCode;
	wxString m_responseString;
	wxString m_newLocation;
	wxString m_newHostWithPort;
	int m_redirectionCount;

	wxLongLong m_totalSize;
	wxLongLong m_receivedData;

	COpData* m_pOpData;

	enum transferEncodings
	{
		identity,
		chunked,
		unknown
	};
	enum transferEncodings m_transferEncoding;

	struct t_chunkData
	{
		bool getTrailer;
		bool terminateChunk;
		wxLongLong size;
	} m_chunkData;
};

class CHttpFileTransferOpData : public CFileTransferOpData, public CHttpOpData
{
public:
	CHttpFileTransferOpData(bool is_download, const wxString& local_file, const wxString& remote_file, const CServerPath& remote_path)
		: CFileTransferOpData(is_download, local_file, remote_file, remote_path), CHttpOpData(this)
	{
		pFile = 0;
	}

	virtual ~CHttpFileTransferOpData()
	{
		delete pFile;
	}

	wxFile* pFile;
};

CHttpControlSocket::CHttpControlSocket(CFileZillaEnginePrivate *pEngine)
	: CRealControlSocket(pEngine)
{
	m_pRecvBuffer = 0;
	m_recvBufferPos = 0;
	m_pTlsSocket = 0;
	m_pHttpOpData = 0;
}

CHttpControlSocket::~CHttpControlSocket()
{
	DoClose();
	delete [] m_pRecvBuffer;
}

int CHttpControlSocket::SendNextCommand()
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::SendNextCommand()"));
	if (!m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Warning, _T("SendNextCommand called without active operation"));
		ResetOperation(FZ_REPLY_ERROR);
		return FZ_REPLY_ERROR;
	}

	if (m_pCurOpData->waitForAsyncRequest)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("Waiting for async request, ignoring SendNextCommand"));
		return FZ_REPLY_WOULDBLOCK;
	}

	switch (m_pCurOpData->opId)
	{
	case cmd_transfer:
		return FileTransferSend();
	default:
		LogMessage(__TFILE__, __LINE__, this, ::Debug_Warning, _T("Unknown opID (%d) in SendNextCommand"), m_pCurOpData->opId);
		ResetOperation(FZ_REPLY_INTERNALERROR);
		break;
	}

	return FZ_REPLY_ERROR;
}


int CHttpControlSocket::ContinueConnect()
{
	LogMessage(__TFILE__, __LINE__, this, Debug_Verbose, _T("CHttpControlSocket::ContinueConnect() m_pEngine=%p"), m_pEngine);
	if (GetCurrentCommandId() != cmd_connect ||
		!m_pCurrentServer)
	{
		LogMessage(Debug_Warning, _T("Invalid context for call to ContinueConnect(), cmd=%d, m_pCurrentServer=%p"), GetCurrentCommandId(), m_pCurrentServer);
		return DoClose(FZ_REPLY_INTERNALERROR);
	}

	ResetOperation(FZ_REPLY_OK);
	return FZ_REPLY_OK;
}

bool CHttpControlSocket::SetAsyncRequestReply(CAsyncRequestNotification *pNotification)
{
	if (m_pCurOpData)
	{
		if (!m_pCurOpData->waitForAsyncRequest)
		{
			LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("Not waiting for request reply, ignoring request reply %d"), pNotification->GetRequestID());
			return false;
		}
		m_pCurOpData->waitForAsyncRequest = false;
	}

	switch (pNotification->GetRequestID())
	{
	case reqId_fileexists:
		{
			if (!m_pCurOpData || m_pCurOpData->opId != cmd_transfer)
			{
				LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("No or invalid operation in progress, ignoring request reply %f"), pNotification->GetRequestID());
				return false;
			}

			CFileExistsNotification *pFileExistsNotification = reinterpret_cast<CFileExistsNotification *>(pNotification);
			switch (pFileExistsNotification->overwriteAction)
			{
			case CFileExistsNotification::resume:
				ResetOperation(FZ_REPLY_CRITICALERROR | FZ_REPLY_NOTSUPPORTED);
				break;
			default:
				return SetFileExistsAction(pFileExistsNotification);
			}
		}
		break;
	case reqId_certificate:
		{
			if (!m_pTlsSocket || m_pTlsSocket->GetState() != CTlsSocket::verifycert)
			{
				LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("No or invalid operation in progress, ignoring request reply %d"), pNotification->GetRequestID());
				return false;
			}

			CCertificateNotification* pCertificateNotification = reinterpret_cast<CCertificateNotification *>(pNotification);
			m_pTlsSocket->TrustCurrentCert(pCertificateNotification->m_trusted);
		}
		break;
	default:
		LogMessage(__TFILE__, __LINE__, this, Debug_Warning, _T("Unknown request %d"), pNotification->GetRequestID());
		ResetOperation(FZ_REPLY_INTERNALERROR);
		return false;
	}

	return true;
}

void CHttpControlSocket::OnReceive()
{
	DoReceive();
}

int CHttpControlSocket::DoReceive()
{
	do
	{
		const enum CSocket::SocketState state = m_pSocket->GetState();
		if (state != CSocket::connected && state != CSocket::closing)
			return 0;

		if (!m_pRecvBuffer)
		{
			m_pRecvBuffer = new char[m_recvBufferLen];
			m_recvBufferPos = 0;
		}

		unsigned int len = m_recvBufferLen - m_recvBufferPos;
		int error;
		int read = m_pBackend->Read(m_pRecvBuffer + m_recvBufferPos, len, error);
		if (read == -1)
		{
			if (error != EAGAIN)
			{
				ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
			}
			return 0;
		}

		m_pEngine->SetActive(CFileZillaEngine::recv);

		if (!m_pCurOpData || m_pCurOpData->opId == cmd_connect)
		{
			// Just ignore all further data
			m_recvBufferPos = 0;
			return 0;
		}

		m_recvBufferPos += read;

		if (!m_pHttpOpData->m_gotHeader)
		{
			if (!read)
			{
				ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
				return 0;
			}

			int res = ParseHeader(m_pHttpOpData);
			if ((res & FZ_REPLY_REDIRECTED) == FZ_REPLY_REDIRECTED)
				return FZ_REPLY_REDIRECTED;
			if (res != FZ_REPLY_WOULDBLOCK)
				return 0;
		}
		else if (m_pHttpOpData->m_transferEncoding == CHttpOpData::chunked)
		{
			if (!read)
			{
				ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
				return 0;
			}
			OnChunkedData(m_pHttpOpData);
		}
		else
		{
			if (!read)
			{
				wxASSERT(!m_recvBufferPos);
				ProcessData(0, 0);
				return 0;
			}
			else
			{
				m_pHttpOpData->m_receivedData += m_recvBufferPos;
				ProcessData(m_pRecvBuffer, m_recvBufferPos);
				m_recvBufferPos = 0;
			}
		}
	}
	while (m_pSocket);

	return 0;
}

void CHttpControlSocket::OnConnect()
{
	wxASSERT(GetCurrentCommandId() == cmd_connect);

	CHttpConnectOpData *pData = static_cast<CHttpConnectOpData *>(m_pCurOpData);
	
	if (pData->tls)
	{
		if (!m_pTlsSocket)
		{
			LogMessage(Status, _("Connection established, initializing TLS..."));

			delete m_pBackend;
			m_pTlsSocket = new CTlsSocket(this, m_pSocket, this);
			m_pBackend = m_pTlsSocket;

			if (!m_pTlsSocket->Init())
			{
				LogMessage(::Error, _("Failed to initialize TLS."));
				DoClose();
				return;
			}

			const wxString trusted_rootcert = m_pEngine->GetOptions()->GetOption(OPTION_INTERNAL_ROOTCERT);
			if (trusted_rootcert != _T("") && !m_pTlsSocket->AddTrustedRootCertificate(trusted_rootcert))
			{
				LogMessage(::Error, _("Failed to parse trusted root cert."));
				DoClose();
				return;
			}

			int res = m_pTlsSocket->Handshake();
			if (res == FZ_REPLY_ERROR)
				DoClose();
		}
		else
		{
			LogMessage(Status, _("TLS/SSL connection established, sending HTTP request"));
			ResetOperation(FZ_REPLY_OK);
		}

		return;
	}
	else
	{
		LogMessage(Status, _("Connection established, sending HTTP request"));
		ResetOperation(FZ_REPLY_OK);
	}
}

enum filetransferStates
{
	filetransfer_init = 0,
	filetransfer_waitfileexists,
	filetransfer_transfer
};

int CHttpControlSocket::FileTransfer(const wxString localFile, const CServerPath &remotePath,
							  const wxString &remoteFile, bool download,
							  const CFileTransferCommand::t_transferSettings& transferSettings)
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::FileTransfer()"));

	LogMessage(Status, _("Downloading %s"), remotePath.FormatFilename(remoteFile).c_str());

	if (!download)
	{
		ResetOperation(FZ_REPLY_CRITICALERROR | FZ_REPLY_NOTSUPPORTED);
		return FZ_REPLY_ERROR;
	}

	if (m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("deleting nonzero pData"));
		delete m_pCurOpData;
	}

	CHttpFileTransferOpData *pData = new CHttpFileTransferOpData(download, localFile, remoteFile, remotePath);
	m_pCurOpData = pData;
	m_pHttpOpData = pData;

	if (localFile != _T(""))
	{
		pData->opState = filetransfer_waitfileexists;
		int res = CheckOverwriteFile();
		if (res != FZ_REPLY_OK)
			return res;

		pData->opState = filetransfer_transfer;

		pData->pFile = new wxFile();

		CreateLocalDir(pData->localFile);

		if (!pData->pFile->Open(pData->localFile, wxFile::write))
		{
			LogMessage(::Error, _("Failed to open \"%s\" for writing"), pData->localFile.c_str());
			ResetOperation(FZ_REPLY_ERROR);
			return FZ_REPLY_ERROR;
		}
	}
	else
		pData->opState = filetransfer_transfer;

	int res = InternalConnect(m_pCurrentServer->GetHost(), m_pCurrentServer->GetPort(), m_pCurrentServer->GetProtocol() == HTTPS);
	if (res != FZ_REPLY_OK)
		return res;

	return FileTransferSend();
}

int CHttpControlSocket::FileTransferSubcommandResult(int prevResult)
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::FileTransferSubcommandResult(%d)"), prevResult);

	if (!m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("Empty m_pCurOpData"));
		ResetOperation(FZ_REPLY_INTERNALERROR);
		return FZ_REPLY_ERROR;
	}

	if (prevResult != FZ_REPLY_OK)
	{
		ResetOperation(prevResult);
		return FZ_REPLY_ERROR;
	}

	return FileTransferSend();
}

int CHttpControlSocket::FileTransferSend()
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::FileTransferSend()"));

	if (!m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("Empty m_pCurOpData"));
		ResetOperation(FZ_REPLY_INTERNALERROR);
		return FZ_REPLY_ERROR;
	}

	CHttpFileTransferOpData *pData = static_cast<CHttpFileTransferOpData *>(m_pCurOpData);

	if (pData->opState == filetransfer_waitfileexists)
	{
		pData->opState = filetransfer_transfer;
		pData->pFile = new wxFile();

		CreateLocalDir(pData->localFile);

		if (!pData->pFile->Open(pData->localFile, wxFile::write))
		{
			LogMessage(::Error, _("Failed to open \"%s\" for writing"), pData->localFile.c_str());
			ResetOperation(FZ_REPLY_ERROR);
			return FZ_REPLY_ERROR;
		}

		int res = InternalConnect(m_pCurrentServer->GetHost(), m_pCurrentServer->GetPort(), m_pCurrentServer->GetProtocol() == HTTPS);
		if (res != FZ_REPLY_OK)
			return res;
	}

	wxString location;
	wxString hostWithPort;
	if (pData->m_newLocation == _T(""))
	{
		if (m_pCurrentServer->GetProtocol() == HTTPS)
			location = _T("https://") + m_pCurrentServer->FormatHost() + pData->remotePath.FormatFilename(pData->remoteFile).c_str();
		else
			location = _T("http://") + m_pCurrentServer->FormatHost() + pData->remotePath.FormatFilename(pData->remoteFile).c_str();
		hostWithPort = wxString::Format(_T("%s:%d"), m_pCurrentServer->FormatHost(true).c_str(), m_pCurrentServer->GetPort());
	}
	else
	{
		location = pData->m_newLocation;
		hostWithPort = pData->m_newHostWithPort;
	}

	wxString action = wxString::Format(_T("GET %s HTTP/1.1"), location.c_str());
	LogMessageRaw(Command, action);

	wxString command = wxString::Format(_T("%s\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n\r\n"), action.c_str(), hostWithPort.c_str(), wxString(PACKAGE_STRING, wxConvLocal).c_str());

	const wxWX2MBbuf str = command.mb_str();
	if (!Send(str, strlen(str)))
		return FZ_REPLY_ERROR;

	return FZ_REPLY_WOULDBLOCK;
}

int CHttpControlSocket::InternalConnect(wxString host, unsigned short port, bool tls)
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::InternalConnect()"));

	CHttpConnectOpData* pData = new CHttpConnectOpData;
	pData->pNextOpData = m_pCurOpData;
	m_pCurOpData = pData;
	pData->port = port;
	pData->tls = tls;

	if (!IsIpAddress(host))
		LogMessage(Status, _("Resolving address of %s"), host.c_str());

	pData->host = host;
	return DoInternalConnect();
}

int CHttpControlSocket::DoInternalConnect()
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::DoInternalConnect()"));

	if (!m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("Empty m_pCurOpData"));
		ResetOperation(FZ_REPLY_INTERNALERROR);
		return FZ_REPLY_ERROR;
	}

	CHttpConnectOpData *pData = static_cast<CHttpConnectOpData *>(m_pCurOpData);

	delete m_pBackend;
	m_pBackend = new CSocketBackend(this, m_pSocket);

	int res = m_pSocket->Connect(pData->host, pData->port);
	if (!res)
		return FZ_REPLY_OK;

	if (res && res != EINPROGRESS)
		return ResetOperation(FZ_REPLY_ERROR);
	
	return FZ_REPLY_WOULDBLOCK;
}

int CHttpControlSocket::FileTransferParseResponse(char* p, unsigned int len)
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::FileTransferParseResponse(%p, %d)"), p, len);

	if (!m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Info, _T("Empty m_pCurOpData"));
		ResetOperation(FZ_REPLY_INTERNALERROR);
		return FZ_REPLY_ERROR;
	}

	CHttpFileTransferOpData *pData = static_cast<CHttpFileTransferOpData *>(m_pCurOpData);

	if (!p)
	{
		ResetOperation(FZ_REPLY_OK);
		return FZ_REPLY_OK;
	}

	if (!m_pTransferStatus)
	{
		InitTransferStatus(pData->m_totalSize.GetLo() + ((wxFileOffset)pData->m_totalSize.GetHi() << 32), 0, false);
		SetTransferStatusStartTime();
	}

	if (pData->localFile == _T(""))
	{
		char* q = new char[len];
		memcpy(q, p, len);
		m_pEngine->AddNotification(new CDataNotification(q, len));
	}
	else
	{
		wxASSERT(pData->pFile);

		if (pData->pFile->Write(p, len) != len)
		{
			LogMessage(::Error, _("Failed to write to file %s"), pData->localFile.c_str());
			ResetOperation(FZ_REPLY_ERROR);
			return FZ_REPLY_ERROR;
		}
	}

	UpdateTransferStatus(len);

	return FZ_REPLY_WOULDBLOCK;
}

int CHttpControlSocket::ParseHeader(CHttpOpData* pData)
{
	// Parse the HTTP header.
	// We do just the neccessary parsing and silently ignore most header fields
	// Redirects are supported though if the server sends the Location field.

	while (true)
	{
		// Find line ending
		unsigned int i = 0;
		for (i = 0; (i + 1) < m_recvBufferPos; i++)
		{
			if (m_pRecvBuffer[i] == '\r')
			{
				if (m_pRecvBuffer[i + 1] != '\n')
				{
					LogMessage(::Error, _("Malformed reply, server not sending proper line endings"));
					ResetOperation(FZ_REPLY_ERROR);
					return FZ_REPLY_ERROR;
				}
				break;
			}
		}
		if ((i + 1) >= m_recvBufferPos)
		{
			if (m_recvBufferPos == m_recvBufferLen)
			{
				// We don't support header lines larger than 4096
				LogMessage(::Error, _("Too long header line"));
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}
			return FZ_REPLY_WOULDBLOCK;
		}

		m_pRecvBuffer[i] = 0;
		const wxString& line = wxString(m_pRecvBuffer, wxConvLocal);
		if (line != _T(""))
			LogMessageRaw(Response, line);

		if (pData->m_responseCode == -1)
		{
			pData->m_responseString = line;
			if (m_recvBufferPos < 16 || memcmp(m_pRecvBuffer, "HTTP/1.", 7))
			{
				// Invalid HTTP Status-Line
				LogMessage(::Error, _("Invalid HTTP Response"));
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}

			if (m_pRecvBuffer[9] < '1' || m_pRecvBuffer[9] > '5' ||
				m_pRecvBuffer[10] < '0' || m_pRecvBuffer[10] > '9' ||
				m_pRecvBuffer[11] < '0' || m_pRecvBuffer[11] > '9')
			{
				// Invalid response code
				LogMessage(::Error, _("Invalid response code"));
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}

			pData->m_responseCode = (m_pRecvBuffer[9] - '0') * 100 + (m_pRecvBuffer[10] - '0') * 10 + m_pRecvBuffer[11] - '0';

			if (pData->m_responseCode >= 400)
			{
				// Failed request
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}

			if (pData->m_responseCode == 305)
			{
				// Unsupported redirect
				LogMessage(::Error, _("Unsupported redirect"));
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}
		}
		else
		{
			if (!i)
			{
				// End of header, data from now on

				// Redirect if neccessary
				if (pData->m_responseCode >= 300)
				{
					if (pData->m_redirectionCount++ == 5)
					{
						LogMessage(::Error, _("Too many redirects"));
						ResetOperation(FZ_REPLY_ERROR);
						return FZ_REPLY_ERROR;
					}

					ResetSocket();
					ResetHttpData(pData);

					wxString host;
					enum ServerProtocol protocol;
					int pos;
					if ((pos = pData->m_newLocation.Find(_T("://"))) != -1)
					{
						protocol = CServer::GetProtocolFromPrefix(pData->m_newLocation.Left(pos));
						host = pData->m_newLocation.Mid(pos + 3);
					}
					else
					{
						protocol = HTTP;
						host = pData->m_newLocation;
					}

					if ((pos = host.Find(_T("/"))) != -1)
						host = host.Left(pos);

					unsigned long port;
					if ((pos = host.Find(':', true)) != -1)
					{
						wxString strport = host.Mid(pos + 1);
						if (!strport.ToULong(&port) || port < 1 || port > 65535)
						{
							if (protocol == HTTPS)
								port = 443;
							else
								port = 80;
						}
						host = host.Left(pos);
					}
					else
					{
						if (protocol == HTTPS)
							port = 443;
						else
							port = 80;
					}

					if (host == _T(""))
					{
						// Unsupported redirect
						LogMessage(::Error, _("Redirection to invalid address"));
						ResetOperation(FZ_REPLY_ERROR);
						return FZ_REPLY_ERROR;
					}
					pData->m_newHostWithPort = wxString::Format(_T("%s:%d"), host.c_str(), (int)port);

					// International domain names
					host = ConvertDomainName(host);

					int res = InternalConnect(host, port, protocol == HTTPS);
					if (res == FZ_REPLY_WOULDBLOCK)
						res |= FZ_REPLY_REDIRECTED;
					return res;
				}

				pData->m_gotHeader = true;

				memmove(m_pRecvBuffer, m_pRecvBuffer + 2, m_recvBufferPos - 2);
				m_recvBufferPos -= 2;

				if (m_recvBufferPos)
				{
					int res;
					if (pData->m_transferEncoding == pData->chunked)
						res = OnChunkedData(pData);
					else
					{
						pData->m_receivedData += m_recvBufferPos;
						res = ProcessData(m_pRecvBuffer, m_recvBufferPos);
						m_recvBufferPos = 0;
					}
					return res;
				}

				return FZ_REPLY_WOULDBLOCK;
			}
			if (m_recvBufferPos > 12 && !memcmp(m_pRecvBuffer, "Location: ", 10))
			{
				pData->m_newLocation = wxString(m_pRecvBuffer + 10, wxConvLocal);
			}
			else if (m_recvBufferPos > 21 && !memcmp(m_pRecvBuffer, "Transfer-Encoding: ", 19))
			{
				if (!strcmp(m_pRecvBuffer + 19, "chunked"))
					pData->m_transferEncoding = CHttpOpData::chunked;
				else if (!strcmp(m_pRecvBuffer + 19, "identity"))
					pData->m_transferEncoding = CHttpOpData::identity;
				else
					pData->m_transferEncoding = CHttpOpData::unknown;
			}
			else if (i > 16 && !memcmp(m_pRecvBuffer, "Content-Length: ", 16))
			{
				pData->m_totalSize = 0;
				char* p = m_pRecvBuffer + 16;
				while (*p)
				{
					if (*p < '0' || *p > '9')
					{
						LogMessage(::Error, _("Malformed header: %s"), _("Invalid Content-Length"));
						ResetOperation(FZ_REPLY_ERROR);
						return FZ_REPLY_ERROR;
					}
					pData->m_totalSize = pData->m_totalSize * 10 + *p++ - '0';
				}
			}
		}

		memmove(m_pRecvBuffer, m_pRecvBuffer + i + 2, m_recvBufferPos - i - 2);
		m_recvBufferPos -= i + 2;

		if (!m_recvBufferPos)
			break;
	}

	return FZ_REPLY_WOULDBLOCK;
}

int CHttpControlSocket::OnChunkedData(CHttpOpData* pData)
{
	char* p = m_pRecvBuffer;
	unsigned int len = m_recvBufferPos;

	while (true)
	{
		if (pData->m_chunkData.size != 0)
		{
			unsigned int dataLen = len;
			if (pData->m_chunkData.size < len)
				dataLen = pData->m_chunkData.size.GetLo();
			pData->m_receivedData += dataLen;
			int res = ProcessData(p, dataLen);
			if (res != FZ_REPLY_WOULDBLOCK)
				return res;

            pData->m_chunkData.size -= dataLen;
			p += dataLen;
			len -= dataLen;

			if (pData->m_chunkData.size == 0)
				pData->m_chunkData.terminateChunk = true;

			if (!len)
				break;
		}

		// Find line ending
		unsigned int i = 0;
		for (i = 0; (i + 1) < len; i++)
		{
			if (p[i] == '\r')
			{
				if (p[i + 1] != '\n')
				{
					LogMessage(::Error, _("Malformed chunk data: %s"), _("Wrong line endings"));
					ResetOperation(FZ_REPLY_ERROR);
					return FZ_REPLY_ERROR;
				}
				break;
			}
		}
		if ((i + 1) >= len)
		{
			if (len == m_recvBufferLen)
			{
				// We don't support lines larger than 4096
				LogMessage(::Error, _("Malformed chunk data: %s"), _("Line length exceeded"));
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}
			break;
		}

		p[i] = 0;

		if (pData->m_chunkData.terminateChunk)
		{
			if (i)
			{
				// The chunk data has to end with CRLF. If i is nonzero,
				// it didn't end with just CRLF.
				LogMessage(::Error, _("Malformed chunk data: %s"), _("Chunk data improperly terminated"));
				ResetOperation(FZ_REPLY_ERROR);
				return FZ_REPLY_ERROR;
			}
			pData->m_chunkData.terminateChunk = false;
		}
		else if (pData->m_chunkData.getTrailer)
		{
			if (!i)
			{
				// We're done
				return ProcessData(0, 0);
			}

			// Ignore the trailer
		}
		else
		{
			// Read chunk size
			char* q = p;
			while (*q)
			{
				if (*q >= '0' && *q <= '9')
				{
					pData->m_chunkData.size *= 16;
					pData->m_chunkData.size += *q - '0';
				}
				else if (*q >= 'A' && *q <= 'F')
				{
					pData->m_chunkData.size *= 16;
					pData->m_chunkData.size += *q - 'A' + 10;
				}
				else if (*q >= 'a' && *q <= 'f')
				{
					pData->m_chunkData.size *= 16;
					pData->m_chunkData.size += *q - 'a' + 10;
				}
				else if (*q == ';' || *q == ' ')
					break;
				else
				{
					// Invalid size
					LogMessage(::Error, _("Malformed chunk data: %s"), _("Invalid chunk size"));
					ResetOperation(FZ_REPLY_ERROR);
					return FZ_REPLY_ERROR;
				}
				q++;
			}
			if (pData->m_chunkData.size == 0)
				pData->m_chunkData.getTrailer = true;
		}

		p += i + 2;
		len -= i + 2;

		if (!len)
			break;
	}

	if (p != m_pRecvBuffer)
	{
		memmove(m_pRecvBuffer, p, len);
		m_recvBufferPos = len;
	}

	return FZ_REPLY_WOULDBLOCK;
}

int CHttpControlSocket::ResetOperation(int nErrorCode)
{
	if (m_pCurOpData && m_pCurOpData->opId == cmd_transfer)
	{
		CHttpFileTransferOpData *pData = static_cast<CHttpFileTransferOpData *>(m_pCurOpData);
		delete pData->pFile;
		pData->pFile = 0;
	}

	if (!m_pCurOpData || !m_pCurOpData->pNextOpData)
	{
		if (m_pBackend)
		{
			if (nErrorCode == FZ_REPLY_OK)
				LogMessage(Status, _("Disconnected from server"));
			else
				LogMessage(::Error, _("Disconnected from server"));
		}
		ResetSocket();
		m_pHttpOpData = 0;
	}

	return CControlSocket::ResetOperation(nErrorCode);
}

void CHttpControlSocket::OnClose(int error)
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::OnClose(%d)"), error);

	if (error)
	{
		LogMessage(::Error, _("Disconnected from server: %s"), CSocket::GetErrorDescription(error).c_str());
		ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
		return;
	}
	
	if (DoReceive() == FZ_REPLY_REDIRECTED)
		return; // Socket got closed already

	// HTTP socket isn't connected outside operations
	if (!m_pCurOpData)
		return;

	if (m_pCurOpData->pNextOpData)
	{
		ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
		return;
	}

	if (!m_pHttpOpData->m_gotHeader)
	{
		ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
		return;
	}

	if (m_pHttpOpData->m_transferEncoding == CHttpOpData::chunked)
	{
		if (!m_pHttpOpData->m_chunkData.getTrailer)
		{
			ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
			return;
		}
	}
	else
	{
		if (m_pHttpOpData->m_totalSize != -1 && m_pHttpOpData->m_receivedData != m_pHttpOpData->m_totalSize)
		{
			ResetOperation(FZ_REPLY_ERROR | FZ_REPLY_DISCONNECTED);
			return;
		}
	}

	ProcessData(0, 0);
}

void CHttpControlSocket::ResetHttpData(CHttpOpData* pData)
{
	wxASSERT(pData);

	delete [] m_pRecvBuffer;
	m_pRecvBuffer = 0;

	pData->m_gotHeader = false;
	pData->m_responseCode = -1;
	pData->m_transferEncoding = CHttpOpData::unknown;

	pData->m_chunkData.getTrailer = false;
	pData->m_chunkData.size = 0;
	pData->m_chunkData.terminateChunk = false;

	pData->m_totalSize = -1;
	pData->m_receivedData = 0;
}

int CHttpControlSocket::ProcessData(char* p, int len)
{
	int res;
	enum Command commandId = GetCurrentCommandId();
	switch (commandId)
	{
	case cmd_transfer:
		res = FileTransferParseResponse(p, len);
		break;
	default:
		LogMessage(Debug_Warning, _T("No action for parsing data for command %d"), (int)commandId);
		ResetOperation(FZ_REPLY_INTERNALERROR);
		res = FZ_REPLY_ERROR;
		break;
	}

	wxASSERT(p || !m_pCurOpData);

	return res;
}

int CHttpControlSocket::ParseSubcommandResult(int prevResult)
{
	LogMessage(Debug_Verbose, _T("CHttpControlSocket::SendNextCommand(%d)"), prevResult);
	if (!m_pCurOpData)
	{
		LogMessage(__TFILE__, __LINE__, this, Debug_Warning, _T("SendNextCommand called without active operation"));
		ResetOperation(FZ_REPLY_ERROR);
		return FZ_REPLY_ERROR;
	}

	switch (m_pCurOpData->opId)
	{
	case cmd_transfer:
		return FileTransferSubcommandResult(prevResult);
	default:
		LogMessage(__TFILE__, __LINE__, this, ::Debug_Warning, _T("Unknown opID (%d) in SendNextCommand"), m_pCurOpData->opId);
		ResetOperation(FZ_REPLY_INTERNALERROR);
		break;
	}

	return FZ_REPLY_ERROR;
}

int CHttpControlSocket::Disconnect()
{
	DoClose();
	return FZ_REPLY_OK;
}
