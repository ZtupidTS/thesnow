//  Copyright (C) 2003-2004 Dennis Syrovatsky. All Rights Reserved.
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

#include "stdhdrs.h"
#include "vncviewer.h"

#include "FileInfo.h"
#include "FileInfoEx.h"
#include "FileTransferTypes.h"

int 
CompareFileInfoEx(const void *F, const void *S)
{
	FILEINFOEX *pF = (FILEINFOEX *) F;
	FILEINFOEX *pS = (FILEINFOEX *) S;

	return 0;
}

FileInfoEx::FileInfoEx()
{
	 m_numEntries = 0;
	 m_pEntries = NULL;
}

FileInfoEx::~FileInfoEx()
{
	free();
}

void
FileInfoEx::add(FileInfoEx *pFIEx)
{
	for (unsigned int i = 0; i < pFIEx->getNumEntries(); i++) {
		add(pFIEx->getLocPathAt(i), pFIEx->getRemPathAt(i),	pFIEx->getLocNameAt(i), 
			pFIEx->getRemNameAt(i), pFIEx->getSizeAt(i), pFIEx->getDataAt(i), pFIEx->getFlagsAt(i));
	}
}

void
FileInfoEx::add(char *pLocPath, char *pRemPath, FileInfo *pFI, unsigned int flags)
{
	char locPath[MAX_PATH];
	char remPath[MAX_PATH];
	strcpy(locPath, pLocPath);
	strcpy(remPath, pRemPath);

	for (unsigned int i = 0; i < pFI->getNumEntries(); i++) {
		add(locPath, remPath, pFI->getNameAt(i), pFI->getNameAt(i), 
			pFI->getSizeAt(i), pFI->getDataAt(i), (pFI->getFlagsAt(i) | flags));
	}
}

void 
FileInfoEx::add(char *pLocPath, char *pRemPath, char *pLocName, char *pRemName, 
			  unsigned int size, unsigned int data, unsigned int flags)
{
	FILEINFOEX *pTemporary = new FILEINFOEX[m_numEntries + 1];
	if (m_numEntries != 0) 
		memcpy(pTemporary, m_pEntries, m_numEntries * sizeof(FILEINFOEX));

	strcpy(pTemporary[m_numEntries].locPath, pLocPath);
	strcpy(pTemporary[m_numEntries].locName, pLocName);
	strcpy(pTemporary[m_numEntries].remPath, pRemPath);
	strcpy(pTemporary[m_numEntries].remName, pRemName);
	
	pTemporary[m_numEntries].info.size = size;
	pTemporary[m_numEntries].info.data = data;
	pTemporary[m_numEntries].info.flags = flags;

	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_numEntries++;
}

char *
FileInfoEx::getLocPathAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].locPath;
	}
	return NULL;
}

char *
FileInfoEx::getLocNameAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].locName;
	}
	return NULL;
}

char *
FileInfoEx::getRemPathAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].remPath;
	}
	return NULL;
}

char *
FileInfoEx::getRemNameAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].remName;
	}
	return NULL;
}

char *
FileInfoEx::getFullLocPathAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		sprintf(m_szFullLocPath, "%s\\%s", getLocPathAt(number), getLocNameAt(number));
		return m_szFullLocPath;
	}
	return NULL;
}

char *
FileInfoEx::getFullRemPathAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		sprintf(m_szFullRemPath, "%s\\%s", getRemPathAt(number), getRemNameAt(number));
		return m_szFullRemPath;
	}
	return NULL;
}

SIZEDATAFLAGSINFO * 
FileInfoEx::getSizeDataFlagsAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return &m_pEntries[number].info;
	}
	return NULL;
}

bool 
FileInfoEx::setLocPathAt(unsigned int number, char *pName)
{
	if ((number >= 0) && (number < m_numEntries)) {
		strcpy(m_pEntries[number].locPath, pName);
		return true;
	}
	return false;
}

bool 
FileInfoEx::setLocNameAt(unsigned int number, char *pName)
{
	if ((number >= 0) && (number < m_numEntries)) {
		strcpy(m_pEntries[number].locName, pName);
		return true;
	}
	return false;
}

bool 
FileInfoEx::setRemPathAt(unsigned int number, char *pName)
{
	if ((number >= 0) && (number < m_numEntries)) {
		strcpy(m_pEntries[number].remPath, pName);
		return true;
	}
	return false;
}

bool 
FileInfoEx::setRemNameAt(unsigned int number, char *pName)
{
	if ((number >= 0) && (number < m_numEntries)) {
		strcpy(m_pEntries[number].remName, pName);
		return true;
	}
	return false;
}

unsigned int
FileInfoEx::getSizeAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].info.size;
	}
	return 0;
}

unsigned int
FileInfoEx::getDataAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].info.data;
	}
	return 0;
}

unsigned int 
FileInfoEx::getFlagsAt(unsigned int number)
{
	if ((number >= 0) && (number < m_numEntries)) {
		return m_pEntries[number].info.flags;
	}
	return 0;
}
	
bool 
FileInfoEx::setSizeAt(unsigned int number, unsigned int value)
{
	if ((number >= 0) && (number < m_numEntries)) {
		m_pEntries[number].info.size = value;
		return true;
	}
	return false;
}

bool 
FileInfoEx::setDataAt(unsigned int number, unsigned int value)
{
	if ((number >= 0) && (number < m_numEntries)) {
		m_pEntries[number].info.data = value;
		return true;
	}
	return false;
}

bool 
FileInfoEx::setFlagsAt(unsigned int number, unsigned int value)
{
	if ((number >= 0) && (number < m_numEntries)) {
		m_pEntries[number].info.flags = value;
		return true;
	}
	return false;
}

bool 
FileInfoEx::deleteAt(unsigned int number)
{
	if ((number >= m_numEntries) || (number < 0)) return false;
	
	FILEINFOEX *pTemporary = new FILEINFOEX[m_numEntries - 1];
	
	if (number == 0) {
		memcpy(pTemporary, &m_pEntries[1], (m_numEntries - 1) * sizeof(FILEINFOEX));
	} else {
		memcpy(pTemporary, m_pEntries, number * sizeof(FILEINFOEX));
		if (number != (m_numEntries - 1)) 
			memcpy(&pTemporary[number], &m_pEntries[number + 1], (m_numEntries - number - 1) * sizeof(FILEINFOEX));
	}
	
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_pEntries = pTemporary;
	pTemporary = NULL;
	m_numEntries--;
	return true;
}
	
unsigned int 
FileInfoEx::getNumEntries()
{
	return m_numEntries;
}

void 
FileInfoEx::sort()
{
	qsort(m_pEntries, m_numEntries, sizeof(FILEINFOEX), CompareFileInfoEx);
}

void 
FileInfoEx::free()
{
	if (m_pEntries != NULL) {
		delete [] m_pEntries;
		m_pEntries = NULL;
	}
	m_numEntries = 0;
}
