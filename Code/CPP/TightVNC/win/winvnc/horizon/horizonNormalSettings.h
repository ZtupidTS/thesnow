
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonNormalSettings.h 1892 2004-10-06 14:20:15Z const_k $

class horizonNormalSettings ;

#ifndef __HORIZON_NORMAL_SETTINGS_H
#define __HORIZON_NORMAL_SETTINGS_H

#include "stdhdrs.h"

#include "horizonSettingsDialog.h"
#include "horizonProperties.h"

#include "horizonSharedArea.h"

//
// class definition
//

class horizonNormalSettings : public horizonSettingsDialog 
{
public:
	horizonNormalSettings() : horizonSettingsDialog() {} ;
	~horizonNormalSettings() {} ;

	bool Show( void ) ;

	static BOOL CALLBACK DialogProc( HWND hwnd, UINT uMsg, 
		WPARAM wParam, LPARAM lParam ) ;
	
} ;

#endif // __HORIZON_NORMAL_SETTINGS_H
