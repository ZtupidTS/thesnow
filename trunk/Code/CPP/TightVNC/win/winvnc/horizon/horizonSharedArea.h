
//  Copyright (C) 2004 Horizon Wimba. All Rights Reserved.
//  Copyright (C) 2002-2003 Constantin Kaplinsky. All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation. All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.

// $Id: horizonSharedArea.h 1896 2004-10-07 21:03:06Z jeffo3 $

class horizonSharedArea ;

#ifndef __HORIZON_SHAREDAREA_H
#define __HORIZON_SHAREDAREA_H

#include "resource.h"

#include "WinVNC.h"
#include "vncServer.h"
#include "vncServerSingleton.h"
#include "vncProperties.h"
#include "MatchWindow.h"

//
// class definition
//

class horizonSharedArea  
{
public:
	horizonSharedArea( HWND hwnd, CMatchWindow *matchwindow ) ;
	virtual ~horizonSharedArea() ;
 	
	bool ApplySharedControls() ;
	void CancelChanges() ;
	
	void FullScreen() ;
	void SharedWindow() ;
	void SharedScreen() ;

	//
	// shared region accessors/mutators
	//
	
	BOOL GetPrefWindowShared() { return m_pref_WindowShared ; } ;
	BOOL GetPrefFullScreen() { return m_pref_FullScreen; }
	BOOL GetPrefScreenAreaShared() { return m_pref_ScreenAreaShared ; } ;
	BOOL GetPrefBlackRgn() { return m_pref_BlackRgn ; } ; // unsupported at this point

	void SetPrefWindowShared( BOOL set ) { m_pref_WindowShared = set ; } ;
	void SetPrefFullScreen( BOOL set ) { m_pref_FullScreen = set ; } ;
	void SetPrefScreenAreaShared( BOOL set ) { m_pref_ScreenAreaShared = set ; } ;
	void SetPrefBlackRgn( BOOL set ) { m_pref_BlackRgn = set ; } ; // unsupported at this point

protected:
	void SetWindowCaption(HWND hWnd);
	HWND hNameAppli;

private:
	void Init();
	static LRESULT CALLBACK BmpWndProc(HWND, UINT, WPARAM, LPARAM);
	static void DrawFrameAroundWindow(HWND hWnd, bool draw);

	HWND m_hwnd;
	LONG m_OldBmpWndProc;
	BOOL m_bCaptured;
	HWND m_KeepHandle;

	vncServer *m_server;

	CMatchWindow *m_pMatchWindow;
	bool m_deleteMatchWindow ;

	// shared region settings
	BOOL m_pref_WindowShared ;
	BOOL m_pref_BlackRgn ;
	BOOL m_pref_FullScreen ;
	BOOL m_pref_ScreenAreaShared ;
	
};

#endif // __HORIZON_SHAREDAREA_H
