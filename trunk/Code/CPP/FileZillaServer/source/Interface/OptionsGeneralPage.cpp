// FileZilla Server - a Windows ftp server

// Copyright (C) 2002-2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// OptionsGeneralPage.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "filezilla server.h"
#include "OptionsDlg.h"
#include "OptionsPage.h"
#include "OptionsGeneralPage.h"
#include <set>

#if defined(_DEBUG) && !defined(MMGR)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld COptionsGeneralPage 


COptionsGeneralPage::COptionsGeneralPage(COptionsDlg *pOptionsDlg, CWnd* pParent /*=NULL*/)
	: COptionsPage(pOptionsDlg, COptionsGeneralPage::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsGeneralPage)
	m_MaxUsers = _T("");
	m_Port = _T("");
	m_Threadnum = _T("");
	m_Timeout = _T("");
	m_NoTransferTimeout = _T("");
	m_LoginTimeout = _T("");
	//}}AFX_DATA_INIT
}


void COptionsGeneralPage::DoDataExchange(CDataExchange* pDX)
{
	COptionsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsGeneralPage)
	DDX_Text(pDX, IDC_MAXUSERS, m_MaxUsers);
	DDV_MaxChars(pDX, m_MaxUsers, 9);
	DDX_Text(pDX, IDC_PORT, m_Port);
	DDX_Text(pDX, IDC_THREADNUM, m_Threadnum);
	DDV_MaxChars(pDX, m_Threadnum, 2);
	DDX_Text(pDX, IDC_TIMEOUT, m_Timeout);
	DDV_MaxChars(pDX, m_Timeout, 4);
	DDX_Text(pDX, IDC_TRANSFERTIMEOUT, m_NoTransferTimeout);
	DDV_MaxChars(pDX, m_NoTransferTimeout, 4);
	DDX_Text(pDX, IDC_LOGINTIMEOUT, m_LoginTimeout);
	DDV_MaxChars(pDX, m_LoginTimeout, 4);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsGeneralPage, COptionsPage)
	//{{AFX_MSG_MAP(COptionsGeneralPage)
		// HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten COptionsGeneralPage 

BOOL COptionsGeneralPage::IsDataValid()
{
	if (!UpdateData(TRUE))
		return FALSE;

	std::set<int> portSet;
	bool valid = true;
	
	CString ports = m_Port;
	ports.TrimLeft(" ,");

	int pos = ports.FindOneOf(_T(" ,"));
	while (pos != -1 && valid)
	{
		int port = atoi(ports.Left(pos));
		if (port < 1 || port > 65535)
		{
			valid = false;
			break;
		}
		else
			portSet.insert(port);
		ports = ports.Mid(pos + 1);
		ports.TrimLeft(" ,");
		pos = ports.FindOneOf(_T(" ,"));
	}
	if (valid && ports != "")
	{
		int port = atoi(ports);
		if (port < 1 || port > 65535)
			valid = false;
		else
			portSet.insert(port);
	}

	if (!valid)
	{
		m_pOptionsDlg->ShowPage(this);
		GetDlgItem(IDC_PORT)->SetFocus();
		AfxMessageBox("Invalid port found, please only enter ports in the range from 1 to 65535.");
		return FALSE;
	}
	int threadnum = atoi(m_Threadnum);
	if (threadnum<1 || threadnum>50)
	{
		m_pOptionsDlg->ShowPage(this);
		GetDlgItem(IDC_THREADNUM)->SetFocus();
		AfxMessageBox("Please enter a value between 1 and 50 for the number of Threads!");
		return FALSE;
	}

	m_Port = "";
	for (std::set<int>::const_iterator iter = portSet.begin(); iter != portSet.end(); iter++)
	{
		CString tmp;
		tmp.Format("%d ", *iter);
		m_Port += tmp;
	}
	m_Port.TrimRight(' ');
	UpdateData(false);

	return TRUE;
}

void COptionsGeneralPage::LoadData()
{
	m_Port = m_pOptionsDlg->GetOption(OPTION_SERVERPORT);
	m_Threadnum.Format("%d", static_cast<int>(m_pOptionsDlg->GetOptionVal(OPTION_THREADNUM)));
	m_MaxUsers.Format("%d", static_cast<int>(m_pOptionsDlg->GetOptionVal(OPTION_MAXUSERS)));
	m_Timeout.Format("%d", static_cast<int>(m_pOptionsDlg->GetOptionVal(OPTION_TIMEOUT)));
	m_NoTransferTimeout.Format("%d", static_cast<int>(m_pOptionsDlg->GetOptionVal(OPTION_NOTRANSFERTIMEOUT)));
	m_LoginTimeout.Format("%d", static_cast<int>(m_pOptionsDlg->GetOptionVal(OPTION_LOGINTIMEOUT)));
}

void COptionsGeneralPage::SaveData()
{
	m_pOptionsDlg->SetOption(OPTION_SERVERPORT, m_Port);
	m_pOptionsDlg->SetOption(OPTION_THREADNUM, atoi(m_Threadnum));
	m_pOptionsDlg->SetOption(OPTION_MAXUSERS, atoi(m_MaxUsers));
	m_pOptionsDlg->SetOption(OPTION_TIMEOUT, atoi(m_Timeout));
	m_pOptionsDlg->SetOption(OPTION_NOTRANSFERTIMEOUT, atoi(m_NoTransferTimeout));
	m_pOptionsDlg->SetOption(OPTION_LOGINTIMEOUT, atoi(m_LoginTimeout));
}