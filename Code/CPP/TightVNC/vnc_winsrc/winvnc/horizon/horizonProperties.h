
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonProperties.h 1971 2004-11-30 15:05:43Z jeffo3 $

class horizonProperties ;

#ifndef __HORIZON_PROPERTIES_H
#define __HORIZON_PROPERTIES_H

// replace the vncProperties class
typedef horizonProperties vncProperties ; 

//
// includes
//

#include "stdhdrs.h"
#include <assert.h>

#include <algorithm>
#include <string>

using std::string ;

#include <lmcons.h>
#include <commctrl.h>

#include "WinVNC.h"

#include "vncServerSingleton.h" 

#include "horizonSettingsDialog.h"
#include "horizonAdvancedSettings.h"
#include "horizonNormalSettings.h" 
#include "horizonBasicSettings.h" 
#include "horizonConnect.h"

#include "PollCycleControl.h"
#include "MatchWindow.h"

//
// class definition
//

class horizonProperties
{
public:	
	static horizonProperties* GetInstance( void ) ;

	// Display the settings dialogs
	bool ShowBasic( void ) ;
	bool ShowNormal( void ) ;
	bool ShowAdvanced( void ) ;

	// persistent match window
	CMatchWindow* GetMatchWindow( bool reset_window = false ) ;

private:
	horizonProperties() ;
	~horizonProperties() ;

	horizonProperties( const horizonProperties& rhs ) ;
	const horizonProperties& operator=( const horizonProperties& rhs ) ;

	// pointer to static instance
	static horizonProperties* m_instance ;

	// match window
	CMatchWindow* m_matchwindow ;
} ;

#endif // __HORIZON_PROPERTIES_H
