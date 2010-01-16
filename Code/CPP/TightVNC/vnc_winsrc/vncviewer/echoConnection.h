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

#ifndef ECHO_CONNECTION_H
#define ECHO_CONNECTION_H

#include "echoTypes.h"
#include "interfacedllproxyinfo.h"

class echoConnection
{
public:
	echoConnection();
	~echoConnection();

	bool initialize();
	void destroy();

	char *getDllVersion();
	
	void setLogging(bool bLogging);
	bool setCallbackPort(unsigned int port);

	bool addConnection(ECHOPROP *echoProps);
	bool delConnection(ECHOPROP *echoProps);
	bool establishConnectTo(ECHOPROP *echoProp, char *pPartnerID, int *backPort);

	bool connect(ECHOPROP *echoProp);
	bool disconnect(ECHOPROP *echoProp);

	bool connectAll();
	bool disconnectAll();
	void deleteAll();
	void setEncryptionAll(int status);
	
	bool getStatus(ECHOPROP *echoProp, int *status);
	char *getStatusString(ECHOPROP *echoProp);
	bool isInitialized() { return m_bInitialized; }

	bool isLocalProxyEnable() { return m_bLocalProxyEnabled; }
	bool setLocalProxyInfo(ECHOPROP *echoProp);

	char *getDefaultPort() { return (char *)szDefaultPort; }

	DWORD getLastError() { return m_dwLastError; }

private:

	class echoProxyInfo : public IDllProxyInfo
	{
	public:
		echoProxyInfo();

		void SetName(const char* name);
		void SetIpPort(const char* ipport);
		void SetIP(const char* ipport);
		void SetPort(const char* port);
		void SetPassword(const char* pass);
		void SetStatus(int Status, bool IsStoring);
		bool SetMyID(const char* MyID);
		void SetReconnectProxy(bool bReconnectProxy);

		const char* GetName()const;
		const char* GetIpPort()const;
		const char* GetIP()const;
		const char* GetPort()const;
		const char* GetPassword()const;
		int GetStatus()const;
		const char* GetMyID()const;

		bool GetReconnectProxy();
		bool SetSocketTimeout(int connectTimeout, int ReceiveTimeout, int SendTimeout);
	};

	HMODULE m_hEchoInst;

	bool initEchoProcAddr();
	void resetEchoObjInfo();
	void resetEchoProcPtrs();

	bool connectTo(echoProxyInfo *proxyInfo);

	bool loadLibrary();
	bool freeLibrary();

	LPFN_ECHOWARE_GET_DLLVERSION                 m_lpfnGetDllVersion;
	LPFN_ECHOWARE_INITIALIZE_PROXYDLL            m_lpfnInitializeProxyDLL;
	LPFN_ECHOWARE_SET_LOGGING_OPTIONS            m_lpfnSetLoggingOptions;
	LPFN_ECHOWARE_SET_PORT_FOR_OFFLOADING_DATA   m_lpfnSetPortForOffLoadingData;
	LPFN_ECHOWARE_CREATE_PROXY_INFO_CLASS_OBJECT m_lpfnCreateProxyInfoClassObject;
	LPFN_ECHOWARE_DELETE_PROXY_INFO_CLASS_OBJECT m_lpfnDeleteProxyInfoClassObject;
	LPFN_ECHOWARE_AUTO_CONNECT                   m_lpfnAutoConnect;
	LPFN_ECHOWARE_CONNECT_PROXY                  m_lpfnConnectProxy;
	LPFN_ECHOWARE_DISCONNECT_PROXY               m_lpfnDisconnectProxy;
	LPFN_ECHOWARE_DISCONNECT_ALL_PROXIES         m_lpfnDisconnectAllProxies;
	LPFN_ECHOWARE_ESTABLISH_NEW_DATA_CHANNEL     m_lpfnEstablishNewDataChannel;
	LPFN_ECHOWARE_SET_ENCRYPTION_LEVEL			 m_lpfnSetEncryptionLevel;
	LPFN_ECHOWARE_SET_LOCAL_PROXY_INFO			 m_lpfnSetLocalProxyInfo;

	echoProxyInfo *getEchoObject(ECHOPROP *echoProp, int *number);

	echoProxyInfo *m_pEchoProxyInfo[MAX_ECHO_SERVERS];

	bool m_bInitialized;
	bool m_bEchoWareEncryptionEnabled;
	bool m_bLocalProxyEnabled;

	char m_szDllVersion[ID_STRING_SIZE];
	char m_szString[ID_STRING_SIZE];

	DWORD m_dwLastError;

	static const char szDefaultPort[];

	static const char disconnected[];
	static const char connecting[];
	static const char connected[];
};

#endif // ECHO_CONNECTION_H
