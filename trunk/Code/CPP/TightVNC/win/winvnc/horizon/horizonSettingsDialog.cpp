
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonSettingsDialog.cpp 1892 2004-10-06 14:20:15Z const_k $

#include "horizonSettingsDialog.h"
#include "horizonProperties.h"

//
// class implemtentation
//

// initialize static member
unsigned int horizonSettingsDialog::m_times_through = 0 ;
bool horizonSettingsDialog::m_dlgvisible = false ;

horizonSettingsDialog::horizonSettingsDialog() 
	: m_properties( horizonProperties::GetInstance() ),
		m_server( vncServerSingleton::GetInstance() ),
		m_ok_clicked( false )
{
}

horizonSettingsDialog::~horizonSettingsDialog()
{
}
