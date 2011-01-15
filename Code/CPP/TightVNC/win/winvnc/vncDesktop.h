//  Copyright (C) 1999 AT&T Laboratories Cambridge. All Rights Reserved.
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


// vncDesktop object

// The vncDesktop object handles retrieval of data from the
// display buffer.  It also uses the RFBLib DLL to supply
// information on mouse movements and screen updates to
// the server

class vncDesktop;

#if !defined(_WINVNC_VNCDESKTOP)
#define _WINVNC_VNCDESKTOP
#pragma once

// Include files
#include "stdhdrs.h"

#include "vncServer.h"
#include "vncRegion.h"
#include "RectList.h"
#include "translate.h"
#include <omnithread.h>
#include "GracePeriod.h"

#include "screen-updates/UpdateListener.h"
#include "screen-updates/UpdateHandler.h"

// Constants
extern const UINT RFB_SCREEN_UPDATE;
extern const UINT RFB_COPYRECT_UPDATE;
extern const UINT RFB_MOUSE_UPDATE;
extern const char szDesktopSink[];

#define MAX_REG_ENTRY_LEN             (80)

// Class definition

class vncDesktop : public UpdateListener
{

// Fields
public:

// Methods
public:
	// Make the desktop thread & window proc friends
	friend class vncDesktopThread;
	friend LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	// Create/Destroy methods
	vncDesktop();
	~vncDesktop();

	BOOL Init(vncServer *pSrv);

	// Hooking stuff
	void TryActivateHooks();

	// Routine to signal a vncServer to trigger an update
	void RequestUpdate();

	// Screen translation, capture, info
	void FillDisplayInfo(rfbServerInitMsg *scrInfo);
	void SetLocalInputDisableHook(BOOL enable);
	void SetLocalInputPriorityHook(BOOL enable);
	void CaptureScreen(RECT &UpdateArea, BYTE *scrBuff); // could be protected?
	int ScreenBuffSize();
	HWND Window() { return m_hwnd; }

	// Mouse related
	void CaptureMouse(BYTE *scrBuff, UINT scrBuffSize);
	BOOL GetRichCursorData(BYTE *databuf, HCURSOR hcursor, int width, int height);
	RECT MouseRect();
	void SetCursor(HCURSOR cursor);
	HCURSOR GetCursor() { return m_hcursor; }
	void UpdateCursor();

	// Clipboard manipulation
	static void ConvertClipText(char *dst, const char *src);
	void SetClipText(LPSTR text);

	// Method to obtain the DIBsection buffer if fast blits are enabled
	// If they're disabled, it'll return NULL
	inline BYTE *MainBuffer() { return m_mainbuff; }
	void CopyRect(const RECT &dest, const POINT &source);

	BOOL			m_initialClipBoardSeen;

  const FrameBuffer *getFrameBuffer() const;

	// Implementation
protected:

	// Routines to hook and unhook us
	BOOL Startup();
	BOOL Shutdown();
	void ActivateHooks();
	void ShutdownHooks();

	// Init routines called by the child thread
	BOOL InitDesktop();
	void KillScreenSaver();

	BOOL InitBitmap();
	BOOL InitWindow();
	BOOL ThunkBitmapInfo();
	BOOL SetPixFormat();
	BOOL SetPixShifts();
	BOOL InitHooks();
	BOOL SetPalette();

	void CopyToBuffer(RECT &rect, BYTE *scrBuff);
	void CopyRectToBuffer(const RECT &dest, const POINT &source);
	void CalcCopyRects();
	
	// Routine to attempt enabling optimised DIBsection blits
	BOOL CreateBuffers();

	// Convert a bit mask eg. 00111000 to max=7, shift=3
	static void MaskToMaxAndShift(DWORD mask, CARD16 &max, CARD8 &shift);
	
	// Enabling & disabling clipboard handling
	void SetClipboardActive(BOOL active) {m_clipboard_active = active;};

	// Detecting updates
	BOOL CheckUpdates();
	void CheckRects(vncRegion &rgn, rectlist &rects);
	void GetChangedRegion(vncRegion &rgn, const RECT &rect);
	void UpdateChangedRect(vncRegion &rgn, const RECT &rect);
	void UpdateChangedSubRect(vncRegion &rgn, const RECT &rect);

	// Blank screen feature
	void UpdateBlankScreenTimer();
	void BlankScreen(BOOL set);

	// Timer identifiers (the third one is not used in any real timer)
	enum TimerID {
		TIMER_BLANK_SCREEN = 2,
		TIMER_RESTORE_SCREEN = 3
	};

  void startupUpdateHandler();
  void shutdownUpdateHandler();

  virtual void onUpdate(void *pSender) {}

	// DATA

	// Generally useful stuff
	vncServer 		*m_server;
	omni_thread 	*m_thread;
	HWND			m_hwnd;
	UINT			m_timer_blank_screen;
	HWND			m_hnextviewer;
	BOOL			m_clipboard_active;
	BOOL			m_hooks_active;
	BOOL			m_hooks_may_change;

	// device contexts for memory and the screen
	HDC				m_hmemdc;
	HDC				m_hrootdc;

	// New and old bitmaps
	HBITMAP			m_membitmap;
	omni_mutex		m_bitbltlock;

	RECT			m_bmrect;
	struct _BMInfo {
		BOOL			truecolour;
		BITMAPINFO		bmi;
		// Colormap info - comes straight after BITMAPINFO - **HACK**
		RGBQUAD			cmap[256];
	} m_bminfo;

	// Screen info
	rfbServerInitMsg	m_scrinfo;

	// These are the red, green & blue masks for a pixel
	DWORD			m_rMask, m_gMask, m_bMask;

	// This is always handy to have
	int				m_bytesPerRow;

	// Handle of the default cursor
	HCURSOR			m_hcursor;
	// Handle of the basic arrow cursor
	HCURSOR			m_hdefcursor;
	// The current mouse position
	RECT			m_cursorpos;

	// Boolean flag to indicate when the display resolution has changed
	BOOL			m_displaychanged;

	// Extra vars used for the DIBsection optimisation
	VOID			*m_DIBbits;
	BYTE			*m_mainbuff;
	BYTE			*m_backbuff;
	BOOL			m_freemainbuff;
	BOOL			m_formatmunged;

	DEVMODE			*lpDevMode; // *** used for res changes - Jeremy Peaks
	long			origPelsWidth; // *** To set the original resolution
	long			origPelsHeight; // *** - Jeremy Peaks
	
	vncRegion		m_changed_rgn;
	BOOL			m_copyrect_set;
	RECT			m_copyrect_rect;
	POINT			m_copyrect_src;

	GracePeriod m_remote_event_gp;

  UpdateHandler *m_updateHandler;
};

#endif // _WINVNC_VNCDESKTOP
