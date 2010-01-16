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

#include "echoConnection.h"

const char echoConnection::szDefaultPort[]				= "1328";

const char echoConnection::disconnected[]	= "Disconnected";
const char echoConnection::connecting[]		= "Connecting...";
const char echoConnection::connected[]		= "Connected";

echoConnection::echoConnection()
{
	m_bInitialized = false;
	m_bLocalProxyEnabled = false;
	m_bEchoWareEncryptionEnabled = false;
	m_hEchoInst = NULL;
	resetEchoProcPtrs();
	resetEchoObjInfo();
	m_szDllVersion[0] = '\0';
	m_szString[0] = '\0';
}

echoConnection::~echoConnection()
{
}

bool 
echoConnection::initialize()
{
	if (!loadLibrary()) {
		m_dwLastError = ID_ECHO_ERROR_LIB_MISSING;
		return false;
	}

	if (!initEchoProcAddr()) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		freeLibrary();
		return false;
	}
	if (!m_lpfnInitializeProxyDLL()) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	setLogging(false);
	strcpy(m_szDllVersion, m_lpfnGetDllVersion());
	m_bInitialized = true;

	m_dwLastError = ID_ECHO_ERROR_SUCCESS;

	return true;
}

void
echoConnection::destroy()
{
	deleteAll();
	freeLibrary();
	resetEchoProcPtrs();
	resetEchoObjInfo();
	m_bInitialized = false;
}

bool
echoConnection::loadLibrary()
{
	if (m_hEchoInst == NULL) {
		m_hEchoInst = LoadLibrary("echoware.dll");
		if (m_hEchoInst) return true;
	}
	return false;
}

bool
echoConnection::freeLibrary()
{
	if (FreeLibrary(m_hEchoInst)) {
		m_hEchoInst = NULL;
		resetEchoProcPtrs();
		resetEchoObjInfo();
		return true;
	} else {
		return false;
	}
}

char *
echoConnection::getDllVersion()
{
	return m_szDllVersion;
}

void
echoConnection::setLogging(bool bLogging)
{
	m_lpfnSetLoggingOptions(bLogging, NULL);
}

bool
echoConnection::setCallbackPort(unsigned int port)
{
	return m_lpfnSetPortForOffLoadingData(port);
}

bool
echoConnection::connectTo(echoProxyInfo *proxyInfo)
{
	switch (m_lpfnConnectProxy(proxyInfo)) 
		{
		case 0:
			m_dwLastError = ID_ECHO_ERROR_SUCCESS;
			return true;
		case 1:
			m_dwLastError = ID_ECHO_ERROR_WRONG_ADDRESS;
			return false;
		case 2:
			m_dwLastError = ID_ECHO_ERROR_WRONG_LOGIN;
			return false;
		case 3:
			m_dwLastError = ID_ECHO_ERROR_CHANNEL_EXIST;
			return false;
		default:
			m_dwLastError = ID_ECHO_ERROR_UNKNOWN;
			return false;
		}
}

bool
echoConnection::addConnection(ECHOPROP *echoProps)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	if (getEchoObject(echoProps, NULL) != NULL) {
		m_dwLastError = ID_ECHO_ERROR_ALREADY_EXIST;
		return false;
	}

	echoProxyInfo *proxyInfo = NULL;
	proxyInfo = (echoProxyInfo *)m_lpfnCreateProxyInfoClassObject();

	if (proxyInfo == NULL) {
		m_dwLastError = ID_ECHO_ERROR_CREATE_OBJECT_FAILED;
		return false;
	}

	proxyInfo->SetIP(echoProps->ipaddr);
	proxyInfo->SetPort(echoProps->port);
	proxyInfo->SetMyID(echoProps->username);
	proxyInfo->SetPassword(echoProps->pwd);

	for (int i = 0; i < MAX_ECHO_SERVERS; i++) {
		if (m_pEchoProxyInfo[i] == NULL) {
			m_pEchoProxyInfo[i] = proxyInfo;
			break;
		}
	}

	return connectTo(proxyInfo);
}

bool
echoConnection::delConnection(ECHOPROP *echoProps)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	int num = 0;
	echoProxyInfo *echoObj = getEchoObject(echoProps, &num);

	if (echoObj != NULL) {
		if (echoObj->GetStatus() != 0) {
			m_lpfnDisconnectProxy(echoObj);
		}
		m_lpfnDeleteProxyInfoClassObject(echoObj);
		m_pEchoProxyInfo[num] = NULL;
		m_dwLastError = ID_ECHO_ERROR_SUCCESS;
		return true;
	}

	m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
	return false;
}

bool 
echoConnection::connect(ECHOPROP *echoProp)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	echoProxyInfo *echoObj = getEchoObject(echoProp, NULL);

	if (echoObj != NULL) {
		return connectTo(echoObj);
	}

	m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
	return false;
}

bool
echoConnection::disconnect(ECHOPROP *echoProp)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	echoProxyInfo *echoObj = getEchoObject(echoProp, NULL);

	if (echoObj != NULL) {
		if (echoObj->GetStatus() != 0) {
			return m_lpfnDisconnectProxy(echoObj);
		}
	}

	m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
	return false;
}

bool
echoConnection::connectAll()
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	bool bResult = true;

	for (int i = 0; i < MAX_ECHO_SERVERS; i++) {
		if (m_pEchoProxyInfo[i] != NULL) {
			if (!connectTo(m_pEchoProxyInfo[i])) bResult = false;
		}
	}

	return bResult;
}

bool
echoConnection::disconnectAll()
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	return m_lpfnDisconnectAllProxies();
}

void
echoConnection::deleteAll()
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return;
	}

	m_lpfnDisconnectAllProxies();

	for (int i = 0; i < MAX_ECHO_SERVERS; i++) {
		if (m_pEchoProxyInfo[i] != NULL) {
			m_lpfnDeleteProxyInfoClassObject(m_pEchoProxyInfo[i]);
			m_pEchoProxyInfo[i] = NULL;
		}
	}
}

bool
echoConnection::getStatus(ECHOPROP *echoProp, int *status)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		*status = 0;
		return false;
	}

	echoProxyInfo *echoObj = getEchoObject(echoProp, NULL);

	if (echoObj != NULL) {
		*status = echoObj->GetStatus();
		m_dwLastError = ID_ECHO_ERROR_SUCCESS;
		return true;
	} else {
		m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
		return false;
	}
}

char *
echoConnection::getStatusString(ECHOPROP *echoProp)
{
	int status;
	bool bResult = getStatus(echoProp, &status);
	m_szString[0] = '\0';

	if (bResult) {
		if (status == 0) {
			strcpy(m_szString, disconnected);
		} else {
			if (status & MASK_ECHO_STATUS_AUTH_CHANNEL_CONNECTING) {
				strcpy(m_szString, connecting);
			} else {
				if ((status & MASK_ECHO_STATUS_AUTH_CHANNEL_ESTABLISHED) || 
					(status & MASK_ECHO_STATUS_PARTNER_SEARCH) || 
					(status & MASK_ECHO_STATUS_RELAY_CHANNEL_CONNECTING) ||
					(status & MASK_ECHO_STATUS_RELAY_CHANNEL_ESTABLISHED_1) ||
					(status & MASK_ECHO_STATUS_RELAY_CHANNEL_ESTABLISHED_2)) {
						strcpy(m_szString, connected);
				}
			}
		}
		if (strlen(m_szString) == 0) {
			strcpy(m_szString, "Unknown Echo Connection Status");
		}
		return m_szString;
	} else {
		strcpy(m_szString, "Can't get Echo Connection Status");
		return m_szString;
	}
}

void
echoConnection::setEncryptionAll(int status)
{
	if (m_bEchoWareEncryptionEnabled) {
		for (int i = 0; i < MAX_ECHO_SERVERS; i++) {
			if (m_pEchoProxyInfo[i] != NULL) {
				m_lpfnSetEncryptionLevel(status, m_pEchoProxyInfo[i]);
			}
		}
	}
}

bool
echoConnection::establishConnectTo(ECHOPROP *echoProp, char *pPartnerID, int *backPort)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	echoProxyInfo *obj = getEchoObject(echoProp, NULL);

	if (obj != NULL) {
		int port = m_lpfnEstablishNewDataChannel(obj, pPartnerID);
		if (port != 0) {
			*backPort = port;
			m_dwLastError = ID_ECHO_ERROR_SUCCESS;
			return true;		
		} else {
			m_dwLastError = ID_ECHO_ERROR_UNKNOWN;
			return false;		
		}
	} else {
		m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
		return false;
	}
}

echoConnection::echoProxyInfo * 
echoConnection::getEchoObject(ECHOPROP *echoProp, int *number)
{
	for (int i = 0; i < MAX_ECHO_SERVERS; i++) {
		if (m_pEchoProxyInfo[i] != NULL) {
			if ((strcmp(m_pEchoProxyInfo[i]->GetIP(), echoProp->ipaddr) == 0) &&
				(strcmp(m_pEchoProxyInfo[i]->GetPort(), echoProp->port) == 0) &&
				(strcmp(m_pEchoProxyInfo[i]->GetMyID(), echoProp->username) == 0)) {
				if (number != NULL) *number = i;
				m_dwLastError = ID_ECHO_ERROR_SUCCESS;
				return m_pEchoProxyInfo[i];
			}
		}
	}
	m_dwLastError = ID_ECHO_ERROR_NOT_EXIST;
	return NULL;
}

bool
echoConnection::initEchoProcAddr()
{
	bool bResult = true;

	m_lpfnGetDllVersion = (LPFN_ECHOWARE_GET_DLLVERSION) 
						   GetProcAddress(m_hEchoInst, "GetDllVersion");

	if (!m_lpfnGetDllVersion) bResult = false;

	m_lpfnInitializeProxyDLL = (LPFN_ECHOWARE_INITIALIZE_PROXYDLL) 
								GetProcAddress(m_hEchoInst, "InitializeProxyDll");

	if (!m_lpfnInitializeProxyDLL) bResult = false;

	m_lpfnSetLoggingOptions = (LPFN_ECHOWARE_SET_LOGGING_OPTIONS) 
							   GetProcAddress(m_hEchoInst, "SetLoggingOptions");

	if (!m_lpfnSetLoggingOptions) bResult = false;

	m_lpfnSetPortForOffLoadingData = (LPFN_ECHOWARE_SET_PORT_FOR_OFFLOADING_DATA) 
									  GetProcAddress(m_hEchoInst, "SetPortForOffLoadingData");

	if (!m_lpfnSetPortForOffLoadingData) bResult = false;

	m_lpfnCreateProxyInfoClassObject = (LPFN_ECHOWARE_CREATE_PROXY_INFO_CLASS_OBJECT) 
										GetProcAddress(m_hEchoInst, "CreateProxyInfoClassObject");

	if (!m_lpfnCreateProxyInfoClassObject) bResult = false;

	m_lpfnDeleteProxyInfoClassObject = (LPFN_ECHOWARE_DELETE_PROXY_INFO_CLASS_OBJECT) 
										GetProcAddress(m_hEchoInst, "DeleteProxyInfoClassObject");

	if (!m_lpfnDeleteProxyInfoClassObject) bResult = false;

	m_lpfnAutoConnect = (LPFN_ECHOWARE_AUTO_CONNECT) 
						 GetProcAddress(m_hEchoInst, "AutoConnect");

	if (!m_lpfnAutoConnect) bResult = false;

	m_lpfnConnectProxy = (LPFN_ECHOWARE_CONNECT_PROXY) 
						  GetProcAddress(m_hEchoInst, "ConnectProxy");

	if (!m_lpfnConnectProxy) bResult = false;

	m_lpfnDisconnectProxy = (LPFN_ECHOWARE_DISCONNECT_PROXY) 
							 GetProcAddress(m_hEchoInst, "DisconnectProxy");

	if (!m_lpfnDisconnectProxy) bResult = false;

	m_lpfnDisconnectAllProxies = (LPFN_ECHOWARE_DISCONNECT_ALL_PROXIES) 
								  GetProcAddress(m_hEchoInst, "DisconnectAllProxies");

	if (!m_lpfnDisconnectAllProxies) bResult = false;

	m_lpfnEstablishNewDataChannel = (LPFN_ECHOWARE_ESTABLISH_NEW_DATA_CHANNEL) 
									 GetProcAddress(m_hEchoInst, "EstablishNewDataChannel");
	
	if (!m_lpfnEstablishNewDataChannel) bResult = false;

	m_lpfnSetEncryptionLevel = (LPFN_ECHOWARE_SET_ENCRYPTION_LEVEL)
								GetProcAddress(m_hEchoInst, "SetEncryptionLevel");

	if (m_lpfnSetEncryptionLevel) m_bEchoWareEncryptionEnabled = true;

	m_lpfnSetLocalProxyInfo = (LPFN_ECHOWARE_SET_LOCAL_PROXY_INFO)
								GetProcAddress(m_hEchoInst, "SetLocalProxyInfo");

	if (m_lpfnSetLocalProxyInfo) m_bLocalProxyEnabled = true;

	if (!bResult) {
		freeLibrary();
		m_dwLastError = ID_ECHO_ERROR_UNKNOWN;
		return false;
	}

	m_dwLastError = ID_ECHO_ERROR_SUCCESS;
	return true;
}

void
echoConnection::resetEchoProcPtrs()
{
	m_lpfnGetDllVersion              = NULL;
	m_lpfnInitializeProxyDLL         = NULL;
	m_lpfnSetLoggingOptions          = NULL;
	m_lpfnSetPortForOffLoadingData   = NULL;
	m_lpfnCreateProxyInfoClassObject = NULL;
	m_lpfnDeleteProxyInfoClassObject = NULL;
	m_lpfnAutoConnect                = NULL;
	m_lpfnConnectProxy               = NULL;
	m_lpfnDisconnectProxy            = NULL;
	m_lpfnDisconnectAllProxies       = NULL;
	m_lpfnEstablishNewDataChannel    = NULL;
	m_lpfnSetEncryptionLevel		 = NULL;
	m_lpfnSetLocalProxyInfo			 = NULL;
}

void
echoConnection::resetEchoObjInfo()
{
	for (int i = 0; i < MAX_ECHO_SERVERS; i++)
		m_pEchoProxyInfo[i] = NULL;
}

bool
echoConnection::setLocalProxyInfo(ECHOPROP *echoProp)
{
	if (!m_bInitialized) {
		m_dwLastError = ID_ECHO_ERROR_LIB_NOT_INITIALIZED;
		return false;
	}

	if (!m_bLocalProxyEnabled) {
		m_dwLastError = ID_ECHO_ERROR_UNKNOWN;
		return false;
	}

	m_lpfnSetLocalProxyInfo(echoProp->ipaddr, echoProp->port, echoProp->username, echoProp->pwd);
	return true;
}
