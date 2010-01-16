
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.

// $Id: horizonSettingsDialog.h 1892 2004-10-06 14:20:15Z const_k $

class horizonSettingsDialog ;

#ifndef __HORIZON_SETTINGS_DIALOG_H
#define __HORIZON_SETTINGS_DIALOG_H

#include "stdhdrs.h"

#include "vncServerSingleton.h"

class horizonProperties ;

// pre-declare to avoid undefined base class error
class horizonSharedArea ;

//
// class definition
//

class horizonSettingsDialog
{
public:
	horizonSettingsDialog() ;
	virtual ~horizonSettingsDialog() ;

	virtual bool Show( void ) = 0 ;

	virtual bool isVisible( void ) { return m_dlgvisible ; } 
	virtual bool wasOKClicked( void ) { return m_ok_clicked ; } 
	virtual bool wasFirstTimeThrough( void ) { return m_times_through == 1 ; } 

protected:
	horizonSettingsDialog( const horizonSettingsDialog& rhs ) ;
	const horizonSettingsDialog& operator=( const horizonSettingsDialog& rhs ) ;

	// pointers to properties and server objects
	horizonProperties* m_properties ;
	vncServer* m_server ;
	
	// shared area panel
	horizonSharedArea* m_shareddtarea ;

	// was okay clicked?
	bool m_ok_clicked ;	

	// is the dialog already visible?
	static bool m_dlgvisible ;
	
	// first time through?
	static unsigned int m_times_through ;

} ;

#endif // __HORIZON_SETTINGS_DIALOG_H
