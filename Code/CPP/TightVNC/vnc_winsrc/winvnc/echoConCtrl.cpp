//  Copyright (C) 2005 Dennis Syrovatsky. All Rights Reserved.
//
//  This file is part of the VNC system.
//
//  The VNC system is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//
// TightVNC distribution homepage on the Web: http://www.tightvnc.com/
//
// If the source code for the VNC system is not available from the place 
// whence you received this file, check http://www.uk.research.att.com/vnc or contact
// the authors on vnc@uk.research.att.com for information on obtaining it.

#include "stdio.h"

#include "echoConCtrl.h"

echoConCtrl::echoConCtrl()
{
	m_NumEntries = 0;
	m_pEntries = NULL;

	DWORD m_dwLastError = ID_ECHO_ERROR_SUCCESS;

	m_encrypted = 0;
	m_enableEchoConnection = 0;
	m_bEncryptionPossible = false;
	m_bLocalProxyEnabled = false;

	m_szVersionStatus[0] = '\0';

	m_callbackPort = 0;
}

echoConCtrl::~echoConCtrl()
{
	destroy();
}

bool 
echoConCtrl::initialize(int callbackPort)
{
	if (!m_echoConnection.initialize()) {
		m_dwLastError = m_echoConnection.getLastError();
		return false;
	}

	if (!setCallbackPort(callbackPort)) {
		m_dwLastError = ID_ECHO_ERROR_UNKNOWN;
		return false;
	}

	m_echoConnection.setLogging(true);

	ZeroMemory(&m_localProxyInfo, sizeof(m_localProxyInfo));

	ECHOPROP echoProp;
	for (int i = 0; i < getNumEntries(); i++) {
		getEntriesAt(i, &echoProp);
		m_echoConnection.addConnection(&echoProp);
	}

	m_dwLastError = ID_ECHO_ERROR_SUCCESS;
	return true;
}

void
echoConCtrl::disconnectAll()
{
	m_echoConnection.disconnectAll();
}

void 
echoConCtrl::destroy()
{
	m_echoConnection.destroy();
	free();
}

bool
echoConCtrl::connectAll()
{
	return m_echoConnection.connectAll();
}

bool 
echoConCtrl::add(ECHOPROP *echoProp)
{
	if (m_NumEntries == MAX_ECHO_SERVERS) {
		m_dwLastError = ID_ECHO_ERROR_MAX_SERVERS;
		return false;
	}

	char *ipAddr = getIPbyName(echoProp->server);
	if (ipAddr != NULL) {
		strcpy(echoProp->ipaddr, ipAddr);
	} else {
		m_dwLastError = ID_ECHO_ERROR_CANT_RESOLVE_ADDR;
		return false;
	}

	if (isExists(echoProp) != -1) {
		m_dwLastError = ID_ECHO_ERROR_ALREADY_EXIST;
		return false;
	}

	ECHOPROP *pTemporary = new ECHOPROP[m_NumEntries + 1];
	if (m_NumEntries != 0) 
		memcpy(pTemporary, m_pEntries, m_NumEntries * sizeof(ECHOPROP));
	
	strcpy(pTemporary[m_NumEntries].server, echoProp->server);
	strcpy(pTemporary[m_NumEntries].port, echoProp->port);
	strcpy(pTemporary[m_NumEntries].username, echoProp->username);
	strcpy(pTemporary[m_NumEntries].pwd, echoProp->pwd);
	strcpy(pTemporary[m_NumEntries].ipaddr, echoProp->ipaddr);
	pTemporary[m_NumEntries].connectionType = echoProp->connectionType;
	
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_NumEntries++;

	if (!m_echoConnection.addConnection(echoProp)) {
		m_dwLastError = m_echoConnection.getLastError();
		return false; 
	} else {
		m_dwLastError = ID_ECHO_ERROR_SUCCESS;
		return true;
	}
}

bool 
echoConCtrl::del(ECHOPROP *echoProp)
{
	int num = isExists(echoProp);
	if (num >= 0) {
		if (!m_echoConnection.delConnection(echoProp)) {
			m_dwLastError = m_echoConnection.getLastError();
			if (m_dwLastError != ID_ECHO_ERROR_LIB_NOT_INITIALIZED)	return false;
		}

		deleteAt(num);

		m_dwLastError = ID_ECHO_ERROR_SUCCESS;
		return true;
	} else {
		m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
		return false;
	}
}

void 
echoConCtrl::deleteAt(int number)
{
	if ((number >= m_NumEntries) || (number < 0)) return;
	
	ECHOPROP *pTemporary = new ECHOPROP[m_NumEntries - 1];
	
	if (number == 0) {
		memcpy(pTemporary, &m_pEntries[1], (m_NumEntries - 1) * sizeof(ECHOPROP));
	} else {
		memcpy(pTemporary, m_pEntries, number * sizeof(ECHOPROP));
		if (number != (m_NumEntries - 1)) memcpy(&pTemporary[number], &m_pEntries[number + 1], (m_NumEntries - number - 1) * sizeof(ECHOPROP));
	}
	
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_NumEntries = m_NumEntries - 1;
}

void 
echoConCtrl::free()
{
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_NumEntries = 0;
}

bool 
echoConCtrl::getEntriesAt(int number, ECHOPROP *echoProp)
{
	if ((number >= m_NumEntries) || (number < 0)) return false;
	
	strcpy(echoProp->server, m_pEntries[number].server);
	strcpy(echoProp->port, m_pEntries[number].port);
	strcpy(echoProp->username, m_pEntries[number].username);
	strcpy(echoProp->pwd, m_pEntries[number].pwd);
	strcpy(echoProp->ipaddr, m_pEntries[number].ipaddr);
	echoProp->connectionType = m_pEntries[number].connectionType;

	return true;
}

int
echoConCtrl::isExists(ECHOPROP *echoProp)
{
	ECHOPROP prop;
	for (int i = 0; i < m_NumEntries; i++) {
		if (getEntriesAt(i, &prop)) {
			if ((strcmp(echoProp->ipaddr, prop.ipaddr) == 0) &&
				(strcmp(echoProp->port, prop.port) == 0)) {
				return i;
			}
		}
	}
	return -1;
}

int
echoConCtrl::isConnected(ECHOPROP *echoProp)
{
	int status;
	m_echoConnection.getStatus(echoProp, &status);
	return status;
}

char * 
echoConCtrl::getConnectionStatus(ECHOPROP *echoProp)
{
	return m_echoConnection.getStatusString(echoProp);
}

bool
echoConCtrl::isInitialized()
{
	return m_echoConnection.isInitialized();
}

void 
echoConCtrl::allowEchoConnection(int status)
{
	m_enableEchoConnection = status;
	if (status == 0) {
		m_echoConnection.disconnectAll();
	} else {
		if ((!m_echoConnection.isInitialized()) && (m_callbackPort != 0)) {
			initialize(m_callbackPort);
		}

		ECHOPROP echoProp;
		for (int i = 0; i < getNumEntries(); i++) {
			getEntriesAt(i, &echoProp);
			if (echoProp.connectionType > 0) m_echoConnection.connect(&echoProp);
		}
	}
}

void 
echoConCtrl::copyConnectionParams(ECHOPROP *dest, ECHOPROP *source)
{
	strcpy(dest->server, source->server);
	strcpy(dest->port, source->port);
	strcpy(dest->username, source->username);
	strcpy(dest->pwd, source->pwd);
	dest->connectionType = source->connectionType;
}

bool 
echoConCtrl::parseConnectionString(char *pConnectionString, char *pServer, char *pPort)
{
	char *pColonPos = strchr(pConnectionString, ':');
	if (pColonPos != NULL) {
		int port;
		if (sscanf(pColonPos + 1, "%d", &port) != 1) return false;
		strcpy(pPort, pColonPos + 1);
		strncpy(pServer, pConnectionString, pColonPos - pConnectionString);
	} else {
		strcpy(pServer, pConnectionString);
		strcpy(pPort, getDefaultPort());
	}
	return true;
}

void 
echoConCtrl::makeEchoWareVersion()
{
	char *szVersion = m_echoConnection.getDllVersion();

	if (strlen(szVersion) == 0) {
		sprintf(m_szVersionStatus, "EchoWare (Version unknown)\nStatus : ");
	} else {
		sprintf(m_szVersionStatus, "EchoWare (Version %s)\nStatus : ", szVersion);
	}

	if (m_echoConnection.isInitialized()) {
		strcat(m_szVersionStatus, "Active");
	} else {
		DWORD dwLastError = m_echoConnection.getLastError();
		if ((dwLastError == ID_ECHO_ERROR_LIB_MISSING) || (dwLastError == ID_ECHO_ERROR_LIB_NOT_INITIALIZED)) {
			strcat(m_szVersionStatus, "Missing");
		} else {
			strcat(m_szVersionStatus, "not Active");
		}
	}
}

char *
echoConCtrl::getEchoWareVersion() 
{ 
	makeEchoWareVersion();
	return m_szVersionStatus; 
}

bool 
echoConCtrl::isEncryptionPossible() 
{
	HMODULE hMod = LoadLibrary(L"libeay32.dll");

	if (hMod) {
		m_bEncryptionPossible = true;
		FreeLibrary(hMod);
	} else {
		m_bEncryptionPossible = false;
	}

	return m_bEncryptionPossible; 
}

bool
echoConCtrl::setEncryption(int status)
{
	if (isEncryptionPossible()) {
		m_echoConnection.setEncryptionAll(status);
		m_encrypted = status;
		return true;
	} else {
		return false;
	}
}

bool 
echoConCtrl::setCallbackPort(int port)
{
	m_callbackPort = port;

	if (m_echoConnection.isInitialized()) 
		return m_echoConnection.setCallbackPort(port);

	m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
	return false;
}

char *
echoConCtrl::getIPbyName(char *server)
{
	if (inet_addr(server) == INADDR_NONE) {
		hostent *hostInfo;
		hostInfo = gethostbyname(server);

		if (hostInfo != NULL) {
			in_addr inAddr;
			memcpy(&inAddr, hostInfo->h_addr_list[0], 4);
			m_dwLastError = ID_ECHO_ERROR_SUCCESS;
			return inet_ntoa(inAddr);
		} else {
			m_dwLastError = ID_ECHO_ERROR_CANT_RESOLVE_ADDR;
			return NULL;
		}
	} else {
		return server;
	}
}

bool 
echoConCtrl::setLocalProxyInfo(ECHOPROP *echoProp)
{
	if (!m_echoConnection.isInitialized()) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return FALSE;
	}

	if ((strlen(echoProp->ipaddr)   >= ID_STRING_SIZE) ||
		(strlen(echoProp->server)   >= ID_STRING_SIZE) ||
		(strlen(echoProp->port)     >= ID_STRING_SIZE) ||
		(strlen(echoProp->username) >= ID_STRING_SIZE) ||
		(strlen(echoProp->pwd)      >= ID_STRING_SIZE)) {
		m_dwLastError = ID_ECHO_ERROR_UNKNOWN;
		return false;
	}

	strcpy(m_localProxyInfo.server, echoProp->server);
	strcpy(m_localProxyInfo.port, echoProp->port);
	strcpy(m_localProxyInfo.username, echoProp->username);
	strcpy(m_localProxyInfo.pwd, echoProp->pwd);

	char *ipAddr = getIPbyName(m_localProxyInfo.server);

	if (ipAddr == NULL) return false;

	strcpy(m_localProxyInfo.ipaddr, ipAddr);

	if (!m_echoConnection.setLocalProxyInfo(&m_localProxyInfo)) {
		m_dwLastError = m_echoConnection.getLastError();
		m_bLocalProxyEnabled = false;
		return false;
	} else {
		m_dwLastError = ID_ECHO_ERROR_SUCCESS;
		m_bLocalProxyEnabled = true;
		return true;
	}
}

bool
echoConCtrl::getLocalProxyInfo(ECHOPROP *echoProp)
{
	if (m_bLocalProxyEnabled) {
		strcpy(echoProp->server, m_localProxyInfo.server);
		strcpy(echoProp->ipaddr, m_localProxyInfo.ipaddr);
		strcpy(echoProp->port, m_localProxyInfo.port);
		strcpy(echoProp->username, m_localProxyInfo.username);
		strcpy(echoProp->pwd, m_localProxyInfo.pwd);
		echoProp->connectionType = m_localProxyInfo.connectionType;
		return true;
	} else {
		return false;
	}
}
