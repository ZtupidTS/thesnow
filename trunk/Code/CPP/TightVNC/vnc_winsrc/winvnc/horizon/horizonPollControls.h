
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonPollControls.h 1892 2004-10-06 14:20:15Z const_k $

#ifndef __HORIZON_POLLCONTROLS_H
#define __HORIZON_POLLCONTROLS_H

#include "stdhdrs.h"
#include "resource.h"

#include "vncServerSingleton.h"
#include "horizonPollingType.h"

class horizonProperties;

class horizonPollControls  
{
public:
	horizonPollControls( HWND hwnd ) ;
	~horizonPollControls() {} ;

	void Validate() ;
	void Apply() ;

	BOOL HandleEvent( UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	
	//
	// polling accessors/mutators
	//
	
	BOOL GetPrefPollUnderCursor() { return m_pref_PollUnderCursor; }
	BOOL GetPrefPollForeground() { return m_pref_PollForeground; }
	BOOL GetPrefPollFullScreen() { return m_pref_PollFullScreen; }
	BOOL GetPrefPollConsoleOnly() { return m_pref_PollConsoleOnly; }
	BOOL GetPrefPollOnEventOnly() { return m_pref_PollOnEventOnly; }
	BOOL GetPrefPollingCycle() { return m_pref_PollingCycle; }
	BOOL GetPrefDontSetHooks() { return m_pref_DontSetHooks; }
	int GetPollingCycle( void ) { return m_pref_PollingCycle ; } ;

	void SetPrefPollUnderCursor( BOOL set ) { m_pref_PollUnderCursor = set; }
	void SetPrefPollForeground( BOOL set ) { m_pref_PollForeground = set; }
	void SetPrefPollFullScreen( BOOL set ) { m_pref_PollFullScreen = set ; } ;
	void SetPrefPollConsoleOnly( BOOL set ) { m_pref_PollConsoleOnly = set; }
	void SetPrefPollOnEventOnly( BOOL set ) { m_pref_PollOnEventOnly = set; }
	void SetPrefPollingCycle( BOOL set ) { m_pref_PollingCycle = set; }
	void SetPrefDontSetHooks( BOOL set ) { m_pref_DontSetHooks = set; }
	void SetPollingCycle( int cycle ) { m_pref_PollingCycle = cycle ; } ;

private:
	inline void Enable( int id, BOOL enable ) 
	{
		EnableWindow( GetDlgItem( m_hwnd, id), enable ) ;
	} ;

	inline void SetChecked( int id, BOOL checked) 
	{
		SendDlgItemMessage( m_hwnd, id, BM_SETCHECK, checked, 0 ) ; 
	} ;

	inline BOOL IsChecked( int id ) 
	{
		return ( SendDlgItemMessage( m_hwnd, id, BM_GETCHECK, 0, 0 ) == BST_CHECKED ) ;
	} ;

	HWND m_hwnd ;
	vncServer* m_server ;

	// polling settings
	BOOL m_pref_PollUnderCursor ;
	BOOL m_pref_PollForeground ;
	BOOL m_pref_PollFullScreen ;
	BOOL m_pref_PollConsoleOnly ;
	BOOL m_pref_PollOnEventOnly ;
	int m_pref_PollingCycle ;
	BOOL m_pref_DontSetHooks ;

} ;

#endif // __HORIZON_POLLCONTROLS_H
