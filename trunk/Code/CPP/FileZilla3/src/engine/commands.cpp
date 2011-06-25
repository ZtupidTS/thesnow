#include <filezilla.h>

CConnectCommand::CConnectCommand(const CServer &server, bool retry_connecting /*=true*/)
	: m_Server(server)
{
	m_retry_connecting = retry_connecting;
}

const CServer CConnectCommand::GetServer() const
{
	return m_Server;
}

CListCommand::CListCommand(int flags /*=0*/)
	: m_flags(flags)
{
}

CListCommand::CListCommand(CServerPath path, wxString subDir /*=_T("")*/, int flags /*=0*/)
	: m_path(path), m_subDir(subDir), m_flags(flags)
{
}

CServerPath CListCommand::GetPath() const
{
	return m_path;
}

wxString CListCommand::GetSubDir() const
{
	return m_subDir;
}

CFileTransferCommand::CFileTransferCommand(const wxString &localFile, const CServerPath& remotePath,
										   const wxString &remoteFile, bool download,
										   const CFileTransferCommand::t_transferSettings& transferSettings)
	: m_localFile(localFile), m_remotePath(remotePath), m_remoteFile(remoteFile)
{
	m_download = download;
	m_transferSettings = transferSettings;
}

wxString CFileTransferCommand::GetLocalFile() const
{
	return m_localFile;
}

CServerPath CFileTransferCommand::GetRemotePath() const
{
	return m_remotePath;
}

wxString CFileTransferCommand::GetRemoteFile() const
{
	return m_remoteFile;
}

bool CFileTransferCommand::Download() const
{
	return m_download;
}

CRawCommand::CRawCommand(const wxString &command)
{
	m_command = command;
}

wxString CRawCommand::GetCommand() const
{
	return m_command;
}

CDeleteCommand::CDeleteCommand(const CServerPath& path, const std::list<wxString>& files)
	: m_path(path), m_files(files)
{
}

CRemoveDirCommand::CRemoveDirCommand(const CServerPath& path, const wxString& subDir)
	: m_path(path), m_subDir(subDir)
{
}

CMkdirCommand::CMkdirCommand(const CServerPath& path)
	: m_path(path)
{
}

CRenameCommand::CRenameCommand(const CServerPath& fromPath, const wxString& fromFile,
							   const CServerPath& toPath, const wxString& toFile)
{
	m_fromPath = fromPath;
	m_toPath = toPath;
	m_fromFile = fromFile;
	m_toFile = toFile;
}

CChmodCommand::CChmodCommand(const CServerPath& path, const wxString& file, const wxString& permission)
{
	m_path = path;
	m_file = file;
	m_permission = permission;
}
