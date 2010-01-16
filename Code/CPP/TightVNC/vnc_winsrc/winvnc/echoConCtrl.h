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

#if !defined(ECHOCONCTRL_H)
#define ECHOCONCTRL_H

#include "echoConnection.h"
#include "echoTypes.h"

class echoConCtrl  
{
public:
	echoConCtrl();
	virtual ~echoConCtrl();

	bool initialize(int callbackPort);
	void disconnectAll();
	void destroy();

	bool add(ECHOPROP *echoProp);
	bool del(ECHOPROP *echoProp);

	bool getEntriesAt(int number, ECHOPROP *echoProp);

	int isExists(ECHOPROP *echoProp);

	int getNumEntries() { return m_NumEntries; }

	bool isLocalProxyEnable() { return m_bLocalProxyEnabled; }

	bool isInitialized();
	bool isEncryptionPossible();
	int  isEncrypted() { return m_encrypted; }
	bool setEncryption(int status);

	bool setLocalProxyInfo(ECHOPROP *echoProp);
	bool getLocalProxyInfo(ECHOPROP *echoProp);

	int isConnected(ECHOPROP *echoProp);
	char *getConnectionStatus(ECHOPROP *echoProp);
	char *getEchoWareVersion();
	char *getIPbyName(char *server);

	void allowEchoConnection(int status);
	int getEnableEchoConnection() { return m_enableEchoConnection; }
	char *getDefaultPort() { return m_echoConnection.getDefaultPort(); }

	DWORD getLastError() { return m_dwLastError; }

	void copyConnectionParams(ECHOPROP *dest, ECHOPROP *source);
	bool parseConnectionString(char *pConnectionString, char *pServer, char *pPort);

	bool setCallbackPort(int port);

	bool connectAll();

private:
	echoConnection m_echoConnection;

	ECHOPROP * m_pEntries;
	int m_NumEntries;
	DWORD m_dwLastError;

	ECHOPROP m_localProxyInfo;

	int  m_encrypted;
	int  m_enableEchoConnection;
	bool m_bEncryptionPossible;
	bool m_bLocalProxyEnabled;

	int m_callbackPort;

	char m_szVersionStatus[ID_STRING_SIZE];

	void free();
	void deleteAt(int number);
	void makeEchoWareVersion();
};

#endif // !defined(ECHOCONCTRL_H)
