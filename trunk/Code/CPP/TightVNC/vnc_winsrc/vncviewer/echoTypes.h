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

#ifndef _VNC_ECHO_TYPES_H__
#define _VNC_ECHO_TYPES_H__

#include "windows.h"

#define ID_STRING_SIZE 255

#define MAX_ECHO_SERVERS 16

#define ID_ECHO_CONNECTION_TYPE_NONE 0
#define ID_ECHO_CONNECTION_TYPE_AUTO 1

#define ID_ECHO_ERROR_SUCCESS		100
#define ID_ECHO_ERROR_UNKNOWN		101
#define ID_ECHO_ERROR_MAX_SERVERS	102

#define ID_ECHO_ERROR_ALREADY_EXIST	103
#define ID_ECHO_ERROR_NOT_EXIST		104

#define ID_ECHO_ERROR_LIB_MISSING			105
#define ID_ECHO_ERROR_LIB_NOT_INITIALIZED	106
#define ID_ECHO_ERROR_CREATE_OBJECT_FAILED	107

#define ID_ECHO_ERROR_WRONG_ADDRESS 111
#define ID_ECHO_ERROR_WRONG_LOGIN   112
#define ID_ECHO_ERROR_CHANNEL_EXIST 113

#define ID_ECHO_ERROR_CANT_RESOLVE_ADDR 115

#define MASK_ECHO_STATUS_NO_CONNECTION					0
#define MASK_ECHO_STATUS_AUTH_CHANNEL_CONNECTING		1
#define MASK_ECHO_STATUS_AUTH_CHANNEL_ESTABLISHED		2
#define MASK_ECHO_STATUS_PARTNER_SEARCH					4
#define MASK_ECHO_STATUS_RELAY_CHANNEL_CONNECTING		8
#define MASK_ECHO_STATUS_RELAY_CHANNEL_ESTABLISHED_1	16
#define MASK_ECHO_STATUS_RELAY_CHANNEL_ESTABLISHED_2	32

typedef struct tagECHOPROP
{
    char server[ID_STRING_SIZE];
	char port[ID_STRING_SIZE];
	char username[ID_STRING_SIZE];
	char pwd[ID_STRING_SIZE];
	char ipaddr[ID_STRING_SIZE];
	int connectionType;
} ECHOPROP;

typedef char* (*LPFN_ECHOWARE_GET_DLLVERSION)                   
              (void);
typedef bool  (*LPFN_ECHOWARE_INITIALIZE_PROXYDLL)              
              ();
typedef void  (*LPFN_ECHOWARE_SET_LOGGING_OPTIONS)              
              (BOOL, char*);
typedef bool  (*LPFN_ECHOWARE_SET_PORT_FOR_OFFLOADING_DATA)     
              (int);
typedef void* (*LPFN_ECHOWARE_CREATE_PROXY_INFO_CLASS_OBJECT)   
              ();
typedef void  (*LPFN_ECHOWARE_DELETE_PROXY_INFO_CLASS_OBJECT)   
              (void*);
typedef void  (*LPFN_ECHOWARE_AUTO_CONNECT)                     
              ();
typedef int   (*LPFN_ECHOWARE_CONNECT_PROXY)                    
              (void*);
typedef bool  (*LPFN_ECHOWARE_DISCONNECT_PROXY)                 
              (void*);
typedef bool  (*LPFN_ECHOWARE_DISCONNECT_ALL_PROXIES)           
              ();
typedef int   (*LPFN_ECHOWARE_ESTABLISH_NEW_DATA_CHANNEL)       
              (void*, char*);

typedef void  (*LPFN_ECHOWARE_SET_ENCRYPTION_LEVEL)
			  (int, void*);
typedef void  (*LPFN_ECHOWARE_SET_LOCAL_PROXY_INFO)
			  (char*, char*, char*, char*);

#endif // _VNC_ECHO_TYPES_H__
