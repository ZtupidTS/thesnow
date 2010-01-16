//  Copyright (C) 2001-2004 HorizonLive.com, Inc. All Rights Reserved.
//  Copyright (C) 2001-2004 TightVNC Team. All Rights Reserved.
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

// vncDesktop implementation

// System headers
#include "stdhdrs.h"
#include <omnithread.h>

#include <wingdi.h>

// Custom headers
#include "WinVNC.h"
#include "VNCHooks\VNCHooks.h"
#include "vncServer.h"
#include "vncRegion.h"
#include "rectlist.h"
#include "vncDesktop.h"
#include "vncService.h"
#include "WallpaperUtils.h"
#include "TsSessions.h"

#if (_MSC_VER>= 1300)
#include <fstream>
#else
#include <fstream.h>
#endif

// Constants
const UINT RFB_SCREEN_UPDATE = RegisterWindowMessage("WinVNC.Update.DrawRect");
const UINT RFB_COPYRECT_UPDATE = RegisterWindowMessage("WinVNC.Update.CopyRect");
const UINT RFB_MOUSE_UPDATE = RegisterWindowMessage("WinVNC.Update.Mouse");
// Messages for blocking remote input events
const UINT RFB_LOCAL_KEYBOARD = RegisterWindowMessage("WinVNC.Local.Keyboard");
const UINT RFB_LOCAL_MOUSE = RegisterWindowMessage("WinVNC.Local.Mouse");

const char szDesktopSink[] = "WinVNC desktop sink";

// Atoms
const char *VNC_WINDOWPOS_ATOMNAME = "VNCHooks.CopyRect.WindowPos";
ATOM VNC_WINDOWPOS_ATOM = NULL;

// The desktop handler thread
// This handles the messages posted by RFBLib to the vncDesktop window

class vncDesktopThread : public omni_thread
{
public:
	vncDesktopThread() { m_returnsig = NULL; }
protected:
	~vncDesktopThread() { if (m_returnsig != NULL) delete m_returnsig; }
public:
	virtual BOOL Init(vncDesktop *desktop, vncServer *server);
	virtual void *run_undetached(void *arg);
	virtual void ReturnVal(BOOL result);

protected:
	vncServer *m_server;
	vncDesktop *m_desktop;

	omni_mutex m_returnLock;
	omni_condition *m_returnsig;
	BOOL m_return;
	BOOL m_returnset;
};

BOOL
vncDesktopThread::Init(vncDesktop *desktop, vncServer *server)
{
	// Save the server pointer
	m_server = server;
	m_desktop = desktop;

	// set the remote event grace period
	m_desktop->m_remote_event_gp.setPeriod( m_server->DisableTime() );

	m_returnset = FALSE;
	m_returnsig = new omni_condition(&m_returnLock);

	// Start the thread
	start_undetached();

	// Wait for the thread to let us know if it failed to init
	{	omni_mutex_lock l(m_returnLock);

		while (!m_returnset)
		{
			m_returnsig->wait();
		}
	}

	return m_return;
}

void
vncDesktopThread::ReturnVal(BOOL result)
{
	omni_mutex_lock l(m_returnLock);

	m_returnset = TRUE;
	m_return = result;
	m_returnsig->signal();
}

void *
vncDesktopThread::run_undetached(void *arg)
{
	// Save the thread's "home" desktop, under NT (no effect under 9x)
	HDESK home_desktop = GetThreadDesktop(GetCurrentThreadId());

	// Try to make session zero the console session
	if (!inConsoleSession())
		setConsoleSession();

	// Attempt to initialise and return success or failure
	if (!m_desktop->Startup()) {
		vncService::SelectHDESK(home_desktop);
		ReturnVal(FALSE);
		return NULL;
	}

	RECT rect;
	if (m_server->WindowShared()) {
		GetWindowRect(m_server->GetWindowShared(), &rect);
	} else if (m_server->ScreenAreaShared()) {
		rect = m_server->GetScreenAreaRect();
	} else {
		rect = m_desktop->m_bmrect;
	}

	IntersectRect(&rect, &rect, &m_desktop->m_bmrect);
	m_server->SetSharedRect(rect);

	// Succeeded to initialise ok
	ReturnVal(TRUE);

	// START PROCESSING DESKTOP MESSAGES

	// We set a flag inside the desktop handler here, to indicate it's now safe
	// to handle clipboard messages
	m_desktop->SetClipboardActive(TRUE);

	MSG msg;
	while (TRUE)
	{

		if (!PeekMessage(&msg, m_desktop->Window(), NULL, NULL, PM_REMOVE))
		{
			// Whenever the message queue becomes empty, we check to see whether
			// there are updates to be passed to clients (first we make sure
			// that scheduled wallpaper removal is complete).
			if (!m_server->WallpaperWait()) {
				if (!m_desktop->CheckUpdates())
					break;
			}

			// Now wait for more messages to be queued
			if (!WaitMessage()) {
				vnclog.Print(LL_INTERR, VNCLOG("WaitMessage() failed\n"));
				break;
			}
		}
		else if (msg.message == RFB_SCREEN_UPDATE)
		{
			// An area of the screen has changed (ignore if we have a driver)
			RECT rect;
			rect.left =	(SHORT)LOWORD(msg.wParam);
			rect.top = (SHORT)HIWORD(msg.wParam);
			rect.right = (SHORT)LOWORD(msg.lParam);
			rect.bottom = (SHORT)HIWORD(msg.lParam);
			m_desktop->m_changed_rgn.AddRect(rect);
		}
		else if (msg.message == RFB_MOUSE_UPDATE)
		{
			// Save the cursor ID
			m_desktop->SetCursor((HCURSOR) msg.wParam);
		}
		else if (msg.message == RFB_LOCAL_KEYBOARD)
		{
			// Block remote input events if necessary
			if (vncService::IsWin95()) {
				m_server->SetKeyboardCounter(-1);
				if (m_server->KeyboardCounter() < 0) {
					m_desktop->m_remote_event_gp.restart();
				}
			} else {
				m_desktop->m_remote_event_gp.restart();
			}
		}
		else if (msg.message == RFB_LOCAL_MOUSE)
		{
			// Block remote input events if necessary
			if (vncService::IsWin95()) {
				if (msg.wParam == WM_MOUSEMOVE) {
					m_server->SetMouseCounter(-1, msg.pt, true);
				} else {
					m_server->SetMouseCounter(-1, msg.pt, false);
				}
				if ( 
					m_server->MouseCounter() < 0
					&& m_desktop->m_remote_event_gp.getStartTime() == 0
				) 
				{
					m_desktop->m_remote_event_gp.restart();
				}
			} else {
				m_desktop->m_remote_event_gp.restart();
			}
		}
		else if (msg.message == WM_QUIT)
		{
			break;
		}
		else
		{
			// Process any other messages normally
			DispatchMessage(&msg);
		}
		
		// FIXME: It's not necessary to do this on receiving _each_ message.
		// Check timer to unblock remote input events if necessary
		if ( m_server->LocalInputPriority() )
		{
			if ( m_desktop->m_remote_event_gp.isPeriodExpired() == true )
			{
				m_server->BlockRemoteInput(false);
				m_server->SetKeyboardCounter(0);
				m_server->SetMouseCounter(0, msg.pt, false);
			}
			else
			{
				// default to blocking remote input
				m_server->BlockRemoteInput(true);
			}
		}	
	}

	m_desktop->SetClipboardActive(FALSE);
	
	vnclog.Print(LL_INTINFO, VNCLOG("quitting desktop server thread\n"));

	// Clear all the hooks and close windows, etc.
	m_desktop->Shutdown();
	// Turn on the screen.
	m_desktop->BlankScreen(FALSE);

	// Clear the shift modifier keys, now that there are no remote clients
	vncKeymap::ClearShiftKeys();

	// Switch back into our home desktop, under NT (no effect under 9x)
	vncService::SelectHDESK(home_desktop);

	return NULL;
}

// Implementation of the vncDesktop class

vncDesktop::vncDesktop()
{
	m_thread = NULL;

	m_hwnd = NULL;
	m_timer_blank_screen = 0;
	m_hnextviewer = NULL;
	m_hcursor = NULL;

	m_displaychanged = FALSE;

	m_hrootdc = NULL;
	m_hmemdc = NULL;
	m_membitmap = NULL;

	m_initialClipBoardSeen = FALSE;

	// Vars for Will Dean's DIBsection patch
	m_DIBbits = NULL;
	m_freemainbuff = FALSE;
	m_formatmunged = FALSE;
	m_mainbuff = NULL;
	m_backbuff = NULL;

	m_clipboard_active = FALSE;
	m_hooks_active = FALSE;
	m_hooks_may_change = FALSE;
	lpDevMode = NULL;
	m_copyrect_set = FALSE;

	m_timer_blank_screen = 0;

}

vncDesktop::~vncDesktop()
{
	vnclog.Print(LL_INTINFO, VNCLOG("killing desktop server\n"));

	// If we created a thread then here we delete it
	// The thread itself does most of the cleanup
	if(m_thread != NULL)
	{
		// Post a close message to quit our message handler thread
		PostMessage(Window(), WM_QUIT, 0, 0);

		// Join with the desktop handler thread
		void *returnval;
		m_thread->join(&returnval);
		m_thread = NULL;
	}

	// Let's call Shutdown just in case something went wrong...
	Shutdown();
}

// Routine to startup and install all the hooks and stuff
BOOL
vncDesktop::Startup()
{
	// Currently, we just check whether we're in the console session, and
	//   fail if not
	if (!inConsoleSession()) {
		vnclog.Print(LL_INTERR, VNCLOG("Console is not session zero - reconnect to restore Console session"));
		return FALSE;
	}

	KillScreenSaver();

	// Initialise the Desktop object
	if (!InitDesktop()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to InitDesktop()\n"));
		return FALSE;
	}

	if (!InitBitmap()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to InitBitmap()\n"));
		return FALSE;
	}

	if (!ThunkBitmapInfo()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to ThunkBitmapInfo()\n"));
		return FALSE;
	}

	if (!SetPixFormat()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to SetPixFormat()\n"));
		return FALSE;
	}

	if (!CreateBuffers()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to CreateBuffers()\n"));
		return FALSE;
	}

	if (!SetPixShifts()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to SetPixShifts()\n"));
		return FALSE;
	}

	if (!SetPalette()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to SetPalette()\n"));
		return FALSE;
	}

	if (!InitWindow()) {
		vnclog.Print(LL_INTINFO, VNCLOG("unable to InitWindow()\n"));
		return FALSE;
	}

	// Add the system hook
	ActivateHooks();
	m_hooks_may_change = true;

	// Start up the keyboard and mouse filters
	SetKeyboardFilterHook(m_server->LocalInputsDisabled());
	SetMouseFilterHook(m_server->LocalInputsDisabled());

	// Start up the keyboard and mouse hooks  for 
	// local event priority over remote impl.
	if (m_server->LocalInputPriority())
		SetLocalInputPriorityHook(true);

	// If necessary, start a separate timer to preserve the diplay turned off.
	UpdateBlankScreenTimer();

	// Get hold of the WindowPos atom!
	if ((VNC_WINDOWPOS_ATOM = GlobalAddAtom(VNC_WINDOWPOS_ATOMNAME)) == 0) {
		vnclog.Print(LL_INTERR, VNCLOG("GlobalAddAtom() failed.\n"));
		return FALSE;
	}

  startupUpdateHandler();

	// Everything is ok, so return TRUE
	return TRUE;
}

// Routine to shutdown all the hooks and stuff
BOOL
vncDesktop::Shutdown()
{
  shutdownUpdateHandler();

	// If we created timers then kill them
	if (m_timer_blank_screen) {
		KillTimer(Window(), TIMER_BLANK_SCREEN);
		m_timer_blank_screen = 0;
	}

	// If we created a window then kill it and the hooks
	if (m_hwnd != NULL)
	{	
		//Remove the system hooks
		//Unset keyboard and mouse hooks
		SetLocalInputPriorityHook(false);
		m_hooks_may_change = false;
		ShutdownHooks();

		// Stop the keyboard and mouse filters
		SetKeyboardFilterHook(false);
		SetMouseFilterHook(false);

		// The window is being closed - remove it from the viewer list
		ChangeClipboardChain(m_hwnd, m_hnextviewer);

		// Close the hook window
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
		m_hnextviewer = NULL;
	}

	// Now free all the bitmap stuff
	if (m_hrootdc != NULL)
	{
		// Release our device context
		if(ReleaseDC(NULL, m_hrootdc) == 0)
		{
			vnclog.Print(LL_INTERR, VNCLOG("failed to ReleaseDC\n"));
		}
		m_hrootdc = NULL;
	}
	if (m_hmemdc != NULL)
	{
		// Release our device context
		if (!DeleteDC(m_hmemdc))
		{
			vnclog.Print(LL_INTERR, VNCLOG("failed to DeleteDC\n"));
		}
		m_hmemdc = NULL;
	}
	if (m_membitmap != NULL)
	{
		// Release the custom bitmap, if any
		if (!DeleteObject(m_membitmap))
		{
			vnclog.Print(LL_INTERR, VNCLOG("failed to DeleteObject\n"));
		}
		m_membitmap = NULL;
	}

	// Free back buffer
	if (m_backbuff != NULL)
	{
		delete [] m_backbuff;
		m_backbuff = NULL;
	}

	if (m_freemainbuff) {
		// Slow blits were enabled - free the slow blit buffer
		if (m_mainbuff != NULL)
		{
			delete [] m_mainbuff;
			m_mainbuff = NULL;
		}
	}


	// Free the WindowPos atom!
	if (VNC_WINDOWPOS_ATOM != NULL) {
		if (GlobalDeleteAtom(VNC_WINDOWPOS_ATOM) != 0) {
			vnclog.Print(LL_INTERR, VNCLOG("failed to delete atom!\n"));
		}
	}

	return TRUE;
}

// Routines to set/unset hooks via VNCHooks.dll

void
vncDesktop::ActivateHooks()
{
	//
	// Disable hooks if DontSetHooks() and PollFullScreen() are TRUE.
	//

	if (m_server->DontSetHooks() && m_server->PollFullScreen()) {
		vnclog.Print(LL_INTINFO, VNCLOG("disabling system hooks\n"));
		ShutdownHooks();
		return;
	}

	//
	// Attempt to enable hooks.
	//

	vnclog.Print(LL_INTINFO, VNCLOG("attempting to enable system hooks\n"));

	// Hooks are already active
	if (m_hooks_active) {
		vnclog.Print(LL_INTINFO, VNCLOG("system hooks were already enabled\n"));
		return;
	}

	// Enable hooks
	m_hooks_active = SetHook(m_hwnd,
							 RFB_SCREEN_UPDATE,
							 RFB_COPYRECT_UPDATE,
							 RFB_MOUSE_UPDATE);

	// Handle activation errors
	if (!m_hooks_active) {
		vnclog.Print(LL_INTERR, VNCLOG("failed to enabled system hooks\n"));

		// Switch on full screen polling, so they can see something, at least...
		m_server->PollFullScreen(TRUE);
		return;
	}

	// Report success
	vnclog.Print(LL_INTINFO, VNCLOG("successfully enabled system hooks\n"));

	// Update m_hooks_active flag
	m_hooks_active = TRUE;
}

void
vncDesktop::ShutdownHooks()
{
	// deactivate hooks, if necessary
	if (m_hooks_active)
		UnSetHook(m_hwnd);

	// update m_hooks_active flag
	m_hooks_active = FALSE;
}

void
vncDesktop::TryActivateHooks()
{
	if (m_hooks_may_change)
		ActivateHooks();
}

// Routine to ensure we're on the correct NT desktop

BOOL
vncDesktop::InitDesktop()
{
	if (vncService::InputDesktopSelected())
		return TRUE;

	// Ask for the current input desktop
	return vncService::SelectDesktop(NULL);
}

// Routine used to close the screen saver, if it's active...

BOOL CALLBACK
KillScreenSaverFunc(HWND hwnd, LPARAM lParam)
{
	char buffer[256];

	// - ONLY try to close Screen-saver windows!!!
	if ((GetClassName(hwnd, buffer, 256) != 0) &&
		(strcmp(buffer, "WindowsScreenSaverClass") == 0))
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	return TRUE;
}

void
vncDesktop::KillScreenSaver()
{
	OSVERSIONINFO osversioninfo;
	osversioninfo.dwOSVersionInfoSize = sizeof(osversioninfo);

	// Get the current OS version
	if (!GetVersionEx(&osversioninfo))
		return;

	vnclog.Print(LL_INTINFO, VNCLOG("KillScreenSaver...\n"));

	// How to kill the screen saver depends on the OS
	switch (osversioninfo.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			// Windows 95

			// Fidn the ScreenSaverClass window
			HWND hsswnd = FindWindow ("WindowsScreenSaverClass", NULL);
			if (hsswnd != NULL)
				PostMessage(hsswnd, WM_CLOSE, 0, 0); 
			break;
		} 
	case VER_PLATFORM_WIN32_NT:
		{
			// Windows NT

			// Find the screensaver desktop
			HDESK hDesk = OpenDesktop(
				"Screen-saver",
				0,
				FALSE,
				DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS
				);
			if (hDesk != NULL)
			{
				vnclog.Print(LL_INTINFO, VNCLOG("Killing ScreenSaver\n"));

				// Close all windows on the screen saver desktop
				EnumDesktopWindows(hDesk, (WNDENUMPROC) &KillScreenSaverFunc, 0);
				CloseDesktop(hDesk);
				// Pause long enough for the screen-saver to close
				//Sleep(2000);
				// Reset the screen saver so it can run again
				SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE); 
			}
			break;
		}
	}
}

BOOL
vncDesktop::InitBitmap()
{
	// Get the device context for the whole screen and find it's size
	m_hrootdc = ::GetDC(NULL);
	if (m_hrootdc == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("GetDC() failed, error=%d\n"), GetLastError());
		return FALSE;
	}

	m_bmrect.left = m_bmrect.top = 0;
	m_bmrect.right = GetDeviceCaps(m_hrootdc, HORZRES);
	m_bmrect.bottom = GetDeviceCaps(m_hrootdc, VERTRES);
	vnclog.Print(LL_INTINFO, VNCLOG("bitmap dimensions are %dx%d\n"), m_bmrect.right, m_bmrect.bottom);

	// Create a compatible memory DC
	m_hmemdc = CreateCompatibleDC(m_hrootdc);
	if (m_hmemdc == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("CreateCompatibleDC() failed, error=%d\n"),
					 GetLastError());
		return FALSE;
	}

	// Check that the device capabilities are ok
	if ((GetDeviceCaps(m_hrootdc, RASTERCAPS) & RC_BITBLT) == 0)
	{
		MessageBox(
			NULL,
			"vncDesktop : root device doesn't support BitBlt\n"
			"WinVNC cannot be used with this graphic device driver",
			szAppName,
			MB_ICONSTOP | MB_OK
			);
		return FALSE;
	}
	if ((GetDeviceCaps(m_hmemdc, RASTERCAPS) & RC_DI_BITMAP) == 0)
	{
		MessageBox(
			NULL,
			"vncDesktop : memory device doesn't support GetDIBits\n"
			"WinVNC cannot be used with this graphics device driver",
			szAppName,
			MB_ICONSTOP | MB_OK
			);
		return FALSE;
	}

	// Create the bitmap to be compatible with the ROOT DC!!!
	m_membitmap = CreateCompatibleBitmap(m_hrootdc, m_bmrect.right, m_bmrect.bottom);
	if (m_membitmap == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("failed to create memory bitmap, error=%d\n"),
					 GetLastError());
		return FALSE;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("created memory bitmap\n"));

	// Get the bitmap's format and colour details
	int result;
	m_bminfo.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bminfo.bmi.bmiHeader.biBitCount = 0;
	result = ::GetDIBits(m_hmemdc, m_membitmap, 0, 1, NULL, &m_bminfo.bmi, DIB_RGB_COLORS);
	if (result == 0) {
		return FALSE;
	}
	result = ::GetDIBits(m_hmemdc, m_membitmap, 0, 1, NULL, &m_bminfo.bmi, DIB_RGB_COLORS);
	if (result == 0) {
		return FALSE;
	}
	vnclog.Print(LL_INTINFO, VNCLOG("got bitmap format\n"));
	vnclog.Print(LL_INTINFO, VNCLOG("DBG:display context has %d planes!\n"), GetDeviceCaps(m_hrootdc, PLANES));
	vnclog.Print(LL_INTINFO, VNCLOG("DBG:memory context has %d planes!\n"), GetDeviceCaps(m_hmemdc, PLANES));
	if (GetDeviceCaps(m_hmemdc, PLANES) != 1)
	{
		MessageBox(
			NULL,
			"vncDesktop : current display is PLANAR, not CHUNKY!\n"
			"WinVNC cannot be used with this graphics device driver",
			szAppName,
			MB_ICONSTOP | MB_OK
			);
		return FALSE;
	}

	// Henceforth we want to use a top-down scanning representation
	m_bminfo.bmi.bmiHeader.biHeight = - abs(m_bminfo.bmi.bmiHeader.biHeight);

	// Is the bitmap palette-based or truecolour?
	m_bminfo.truecolour = (GetDeviceCaps(m_hmemdc, RASTERCAPS) & RC_PALETTE) == 0;

	return TRUE;
}


BOOL
vncDesktop::ThunkBitmapInfo()
{
	// If we leave the pixel format intact, the blits can be optimised (Will Dean's patch)
	m_formatmunged = FALSE;

	// HACK ***.  Optimised blits don't work with palette-based displays, yet
	if (!m_bminfo.truecolour) {
		m_formatmunged = TRUE;
	}

	// Attempt to force the actual format into one we can handle
	// We can handle 8-bit-palette and 16/32-bit-truecolour modes
	switch (m_bminfo.bmi.bmiHeader.biBitCount)
	{
	case 1:
	case 4:
		vnclog.Print(LL_INTINFO, VNCLOG("DBG:used/bits/planes/comp/size "
										"= %d/%d/%d/%d/%d\n"),
					 (int)m_bminfo.bmi.bmiHeader.biClrUsed,
					 (int)m_bminfo.bmi.bmiHeader.biBitCount,
					 (int)m_bminfo.bmi.bmiHeader.biPlanes,
					 (int)m_bminfo.bmi.bmiHeader.biCompression,
					 (int)m_bminfo.bmi.bmiHeader.biSizeImage);
		
		// Correct the BITMAPINFO header to the format we actually want
		m_bminfo.bmi.bmiHeader.biClrUsed = 0;
		m_bminfo.bmi.bmiHeader.biPlanes = 1;
		m_bminfo.bmi.bmiHeader.biCompression = BI_RGB;
		m_bminfo.bmi.bmiHeader.biBitCount = 8;
		m_bminfo.bmi.bmiHeader.biSizeImage =
			abs((m_bminfo.bmi.bmiHeader.biWidth *
				m_bminfo.bmi.bmiHeader.biHeight *
				m_bminfo.bmi.bmiHeader.biBitCount)/ 8);
		m_bminfo.bmi.bmiHeader.biClrImportant = 0;
		m_bminfo.truecolour = FALSE;

		// Display format is non-VNC compatible - use the slow blit method
		m_formatmunged = TRUE;
		break;	
	case 24:
		// Update the bitmapinfo header
		m_bminfo.bmi.bmiHeader.biBitCount = 32;
		m_bminfo.bmi.bmiHeader.biPlanes = 1;
		m_bminfo.bmi.bmiHeader.biCompression = BI_RGB;
		m_bminfo.bmi.bmiHeader.biSizeImage =
			abs((m_bminfo.bmi.bmiHeader.biWidth *
				m_bminfo.bmi.bmiHeader.biHeight *
				m_bminfo.bmi.bmiHeader.biBitCount)/ 8);
		// Display format is non-VNC compatible - use the slow blit method
		m_formatmunged = TRUE;
		break;
	}

	return TRUE;
}

BOOL
vncDesktop::SetPixFormat()
{
	// Examine the bitmapinfo structure to obtain the current pixel format
	m_scrinfo.format.trueColour = m_bminfo.truecolour;
	m_scrinfo.format.bigEndian = 0;

	// Set up the native buffer width, height and format
	m_scrinfo.framebufferWidth = (CARD16) (m_bmrect.right - m_bmrect.left);		// Swap endian before actually sending
	m_scrinfo.framebufferHeight = (CARD16) (m_bmrect.bottom - m_bmrect.top);	// Swap endian before actually sending
	m_scrinfo.format.bitsPerPixel = (CARD8) m_bminfo.bmi.bmiHeader.biBitCount;
	m_scrinfo.format.depth        = (CARD8) m_bminfo.bmi.bmiHeader.biBitCount;

	
	// Calculate the number of bytes per row
	m_bytesPerRow = m_scrinfo.framebufferWidth * m_scrinfo.format.bitsPerPixel / 8;

	return TRUE;
}

BOOL
vncDesktop::SetPixShifts()
{
	// Sort out the colour shifts, etc.
	DWORD redMask=0, blueMask=0, greenMask = 0;

	switch (m_bminfo.bmi.bmiHeader.biBitCount)
	{
	case 16:
		// Standard 16-bit display
		if (m_bminfo.bmi.bmiHeader.biCompression == BI_RGB)
		{
			// each word single pixel 5-5-5
			redMask = 0x7c00; greenMask = 0x03e0; blueMask = 0x001f;
		}
		else
		{
			if (m_bminfo.bmi.bmiHeader.biCompression == BI_BITFIELDS)
			{
				redMask =   *(DWORD *) &m_bminfo.bmi.bmiColors[0];
				greenMask = *(DWORD *) &m_bminfo.bmi.bmiColors[1];
				blueMask =  *(DWORD *) &m_bminfo.bmi.bmiColors[2];
			}
		}
		break;

	case 32:
		// Standard 24/32 bit displays
		if (m_bminfo.bmi.bmiHeader.biCompression == BI_RGB)
		{
			redMask = 0xff0000;
			greenMask = 0xff00;
			blueMask = 0x00ff;

			// The real color depth is 24 bits in this case. If the depth
			// is set to 32, the Tight encoder shows worse performance.
			m_scrinfo.format.depth = 24;
		}
		else
		{
			if (m_bminfo.bmi.bmiHeader.biCompression == BI_BITFIELDS)
			{
				redMask =   *(DWORD *) &m_bminfo.bmi.bmiColors[0];
				greenMask = *(DWORD *) &m_bminfo.bmi.bmiColors[1];
				blueMask =  *(DWORD *) &m_bminfo.bmi.bmiColors[2];
			}
		}
		break;

	default:
		// Other pixel formats are only valid if they're palette-based
		if (m_bminfo.truecolour)
		{
			vnclog.Print(LL_INTERR, "unsupported truecolour pixel format for SetPixShifts()\n");
			return FALSE;
		}
		vnclog.Print(LL_INTINFO, VNCLOG("DBG:palette-based desktop in SetPixShifts()\n"));
		return TRUE;
	}

	// Convert the data we just retrieved
	MaskToMaxAndShift(redMask, m_scrinfo.format.redMax, m_scrinfo.format.redShift);
	MaskToMaxAndShift(greenMask, m_scrinfo.format.greenMax, m_scrinfo.format.greenShift);
	MaskToMaxAndShift(blueMask, m_scrinfo.format.blueMax, m_scrinfo.format.blueShift);

	vnclog.Print(LL_INTINFO, VNCLOG("DBG:true-color desktop in SetPixShifts()\n"));
	return TRUE;
}

BOOL
vncDesktop::SetPalette()
{
	// Lock the current display palette into the memory DC we're holding
	// *** CHECK THIS FOR LEAKS!
	if (!m_bminfo.truecolour)
	{
		LOGPALETTE *palette;
		UINT size = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);

		palette = (LOGPALETTE *) new char[size];
		if (palette == NULL) {
			vnclog.Print(LL_INTERR, VNCLOG("error allocating palette\n"));
			return FALSE;
		}

		// Initialise the structure
		palette->palVersion = 0x300;
		palette->palNumEntries = 256;

		// Get the system colours
		if (GetSystemPaletteEntries(m_hrootdc, 0, 256, palette->palPalEntry) == 0)
		{
			vnclog.Print(LL_INTERR, VNCLOG("GetSystemPaletteEntries() failed.\n"));
			delete [] palette;
			return FALSE;
		}

		// Create a palette from those
		HPALETTE pal = CreatePalette(palette);
		if (pal == NULL)
		{
			vnclog.Print(LL_INTERR, VNCLOG("CreatePalette() failed.\n"));
			delete [] palette;
			return FALSE;
		}

		// Select the palette into our memory DC
		HPALETTE oldpalette = SelectPalette(m_hmemdc, pal, FALSE);
		if (oldpalette == NULL)
		{
			vnclog.Print(LL_INTERR, VNCLOG("SelectPalette() failed.\n"));
			delete [] palette;
			DeleteObject(pal);
			return FALSE;
		}

		// Worked, so realise the palette
		if (RealizePalette(m_hmemdc) == GDI_ERROR)
			vnclog.Print(LL_INTWARN, VNCLOG("warning - failed to RealizePalette\n"));

		// It worked!
		delete [] palette;
		DeleteObject(oldpalette);

		vnclog.Print(LL_INTINFO, VNCLOG("initialised palette OK\n"));
		return TRUE;
	}

	// Not a palette based local screen - forget it!
	vnclog.Print(LL_INTINFO, VNCLOG("no palette data for truecolour display\n"));
	return TRUE;
}

LRESULT CALLBACK DesktopWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

ATOM m_wndClass = 0;

BOOL
vncDesktop::InitWindow()
{
	if (m_wndClass == 0) {
		// Create the window class
		WNDCLASSEX wndclass;

		wndclass.cbSize			= sizeof(wndclass);
		wndclass.style			= 0;
		wndclass.lpfnWndProc	= &DesktopWndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= hAppInstance;
		wndclass.hIcon			= NULL;
		wndclass.hCursor		= NULL;
		wndclass.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName	= (const char *) NULL;
		wndclass.lpszClassName	= szDesktopSink;
		wndclass.hIconSm		= NULL;

		// Register it
		m_wndClass = RegisterClassEx(&wndclass);
	}

	// And create a window
	m_hwnd = CreateWindow(szDesktopSink,
				"WinVNC",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				400, 200,
				NULL,
				NULL,
				hAppInstance,
				NULL);

	if (m_hwnd == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("CreateWindow() failed.\n"));
		return FALSE;
	}

	// Set the "this" pointer for the window
	SetWindowLong(m_hwnd, GWL_USERDATA, (long)this);

	// Enable clipboard hooking
	m_hnextviewer = SetClipboardViewer(m_hwnd);

	return TRUE;
}

BOOL
vncDesktop::CreateBuffers()
{
	vnclog.Print(LL_INTINFO, VNCLOG("attempting to create main and back buffers\n"));

	// Create a new DIB section ***
	HBITMAP tempbitmap = CreateDIBSection(m_hmemdc, &m_bminfo.bmi, DIB_RGB_COLORS, &m_DIBbits, NULL, 0);

	m_freemainbuff = false;

	if (tempbitmap == NULL || m_formatmunged) {
		vnclog.Print(LL_INTWARN, VNCLOG("failed to build DIB section - reverting to slow blits\n"));
		m_DIBbits = NULL;
		// create our own buffer to copy blits through
		if ((m_mainbuff = new BYTE [ScreenBuffSize()]) == NULL) {
				vnclog.Print(LL_INTERR, VNCLOG("unable to allocate main buffer[%d]\n"), ScreenBuffSize());
				return FALSE;
		}
		m_freemainbuff = true;
		if ((m_backbuff = new BYTE [ScreenBuffSize()]) == NULL) {
			vnclog.Print(LL_INTERR, VNCLOG("unable to allocate back buffer[%d]\n"), ScreenBuffSize());
			return FALSE;
		}
		return TRUE;
	}
	
	// Create our own buffer to copy blits through
	if ((m_backbuff = new BYTE [ScreenBuffSize()]) == NULL) {
		vnclog.Print(LL_INTERR, VNCLOG("unable to allocate back buffer[%d]\n"), ScreenBuffSize());
		return FALSE;
	}

	// Delete the old memory bitmap
	if (m_membitmap != NULL) {
		DeleteObject(m_membitmap);
		m_membitmap = NULL;
	}

	// Replace old membitmap with DIB section
	m_membitmap = tempbitmap;
	m_mainbuff = (BYTE *)m_DIBbits;
	vnclog.Print(LL_INTINFO, VNCLOG("enabled fast DIBsection blits OK\n"));
	return TRUE;
}

BOOL
vncDesktop::Init(vncServer *server)
{
	vnclog.Print(LL_INTINFO, VNCLOG("initialising desktop server\n"));

	// Save the server pointer
	m_server = server;

	// Load in the arrow cursor
	m_hdefcursor = LoadCursor(NULL, IDC_ARROW);
	m_hcursor = m_hdefcursor;

	// Spawn a thread to handle that window's message queue
	vncDesktopThread *thread = new vncDesktopThread;
	if (thread == NULL)
		return FALSE;
	m_thread = thread;
	return thread->Init(this, m_server);
}

void
vncDesktop::RequestUpdate()
{
  // FIXME: Check what this function is used for.
}

int
vncDesktop::ScreenBuffSize()
{
	return m_scrinfo.format.bitsPerPixel/8 *
		m_scrinfo.framebufferWidth *
		m_scrinfo.framebufferHeight;
}

void
vncDesktop::FillDisplayInfo(rfbServerInitMsg *scrinfo)
{
	memcpy(scrinfo, &m_scrinfo, sz_rfbServerInitMsg);
}

// Function to capture an area of the screen immediately prior to sending
// an update.
void
vncDesktop::CaptureScreen(RECT &rect, BYTE *scrBuff)
{
	
	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	// Finish drawing anything in this thread 
	// Wish we could do this for the whole system - maybe we should
	// do something with LockWindowUpdate here.
	GdiFlush();

	// Select the memory bitmap into the memory DC
	HBITMAP oldbitmap;
	if ((oldbitmap = (HBITMAP) SelectObject(m_hmemdc, m_membitmap)) == NULL)
		return;

	// Capture screen into bitmap
	BOOL blitok;

	if (m_server->GetApplication()) {
		vncRegion capturergn, blackrgn;
		capturergn.Clear();
		blackrgn.Clear();
		
		blackrgn.AddRect(rect);
		BOOL find = FALSE;
		HWND hforegr = GetWindow(GetForegroundWindow(), GW_HWNDLAST);
		while (hforegr != NULL) {			
			vncRegion wintmprgn, captmprgn;
			RECT win;
			GetWindowRect(hforegr, &win);				
			wintmprgn.AddRect(win);
			captmprgn.AddRect(rect);
			wintmprgn.Intersect(captmprgn);
			DWORD style = GetWindowLong(hforegr, GWL_STYLE);
			DWORD procforegr;
			GetWindowThreadProcessId(hforegr, &procforegr);
			if (style & WS_VISIBLE) {
				if (procforegr == m_server->GetWindowIdProcess()) {				
					capturergn.Combine(wintmprgn);
					find = TRUE;
				} else {
					capturergn.Subtract(wintmprgn);
				}
			}
			hforegr = GetWindow(hforegr, GW_HWNDPREV);	
		}
		if (!find)
			capturergn.Clear();
		blackrgn.Subtract(capturergn);		
		if (!capturergn.IsEmpty()) {
			rectlist capturerects;
			m_server->GetBlackRegion()->Subtract(capturergn);
			capturergn.Rectangles(capturerects);
			rectlist::iterator i;
			for (i = capturerects.begin(); i != capturerects.end(); i++) {
				blitok = BitBlt(m_hmemdc, (*i).left, (*i).top,
								(*i).right - (*i).left, (*i).bottom - (*i).top,
								m_hrootdc, (*i).left, (*i).top,	SRCCOPY);	
			}
		}
		if (!blackrgn.IsEmpty()) {
			rectlist blackrects;
			m_server->GetBlackRegion()->Combine(blackrgn);
			blackrgn.Rectangles(blackrects);
			rectlist::iterator i;
			for (i = blackrects.begin(); i != blackrects.end(); i++) {
				blitok = PatBlt(m_hmemdc, (*i).left, (*i).top, 
								(*i).right - (*i).left, 
								(*i).bottom - (*i).top, BLACKNESS);
			}
		}
	} else {
		blitok = BitBlt(m_hmemdc, rect.left, rect.top,
								rect.right - rect.left, rect.bottom - rect.top,
								m_hrootdc, rect.left, rect.top,	SRCCOPY);
	}
	// Select the old bitmap back into the memory DC
	SelectObject(m_hmemdc, oldbitmap);
	
	if (blitok) {
		// Copy the new data to the screen buffer (CopyToBuffer optimises this if possible)
		CopyToBuffer(rect, scrBuff);
	}

}

// Add the mouse pointer to the buffer
void
vncDesktop::CaptureMouse(BYTE *scrBuff, UINT scrBuffSize)
{
	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	POINT CursorPos;
	ICONINFO IconInfo;

	// If the mouse cursor handle is invalid then forget it
	if (m_hcursor == NULL)
		return;

	// Get the cursor position
	if (!GetCursorPos(&CursorPos))
		return;
	
	// Translate position for hotspot
	if (GetIconInfo(m_hcursor, &IconInfo))
	{
		
		CursorPos.x -= ((int) IconInfo.xHotspot);
		CursorPos.y -= ((int) IconInfo.yHotspot);
		
		if (IconInfo.hbmMask != NULL)
			DeleteObject(IconInfo.hbmMask);
		if (IconInfo.hbmColor != NULL)
			DeleteObject(IconInfo.hbmColor);
	}
	

	// Select the memory bitmap into the memory DC
	HBITMAP oldbitmap;
	if ((oldbitmap = (HBITMAP) SelectObject(m_hmemdc, m_membitmap)) == NULL)
		return;

	// Draw the cursor
	DrawIconEx(
		m_hmemdc,									// handle to device context 
		CursorPos.x, CursorPos.y,
		m_hcursor,									// handle to icon to draw 
		0,0,										// width of the icon 
		0,											// index of frame in animated cursor 
		NULL,										// handle to background brush 
		DI_NORMAL									// icon-drawing flags 
		);

	// Select the old bitmap back into the memory DC
	SelectObject(m_hmemdc, oldbitmap);

	// Save the bounding rectangle
	m_cursorpos.left = CursorPos.x;
	m_cursorpos.top = CursorPos.y;
	m_cursorpos.right = CursorPos.x + GetSystemMetrics(SM_CXCURSOR);
	m_cursorpos.bottom = CursorPos.y + GetSystemMetrics(SM_CYCURSOR);
	
	// Clip the bounding rect to the screen
	RECT screen = m_server->GetSharedRect();
	// Copy the mouse cursor into the screen buffer, if any of it is visible
	if (IntersectRect(&m_cursorpos, &m_cursorpos, &screen))
		CopyToBuffer(m_cursorpos, scrBuff);
}

// Obtain cursor image data in server's local format.
// The length of databuf[] should be at least (width * height * 4).
BOOL
vncDesktop::GetRichCursorData(BYTE *databuf, HCURSOR hcursor, int width, int height)
{
	// Protect the memory bitmap (is it really necessary here?)
	omni_mutex_lock l(m_bitbltlock);

	// Create bitmap, select it into memory DC
	HBITMAP membitmap = CreateCompatibleBitmap(m_hrootdc, width, height);
	if (membitmap == NULL) {
		return FALSE;
	}
	HBITMAP oldbitmap = (HBITMAP) SelectObject(m_hmemdc, membitmap);
	if (oldbitmap == NULL) {
		DeleteObject(membitmap);
		return FALSE;
	}

	// Draw the cursor
	DrawIconEx(m_hmemdc, 0, 0, hcursor, 0, 0, 0, NULL, DI_IMAGE);
	SelectObject(m_hmemdc, oldbitmap);

	// Prepare BITMAPINFO structure (copy most m_bminfo fields)
	BITMAPINFO *bmi = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	memcpy(bmi, &m_bminfo.bmi, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = -height;

	// Clear data buffer and extract RGB data
	memset(databuf, 0x00, width * height * 4);
	int lines = GetDIBits(m_hmemdc, membitmap, 0, height, databuf, bmi, DIB_RGB_COLORS);

	// Cleanup
	free(bmi);
	DeleteObject(membitmap);

	return (lines != 0);
}

// Return the current mouse pointer position
RECT
vncDesktop::MouseRect()
{
	return m_cursorpos;
}

void
vncDesktop::SetCursor(HCURSOR cursor)
{
	if (cursor == NULL)
		m_hcursor = m_hdefcursor;
	else
		m_hcursor = cursor;
}

void
vncDesktop::UpdateCursor()
{
#if(WINVER>=0x0500)
    CURSORINFO info;
    info.cbSize = sizeof(CURSORINFO);
    if (!GetCursorInfo(&info)) {
	vnclog.Print(LL_INTERR, VNCLOG("GetCursorInfo() failed.\n"));
	return;
    }
    m_hcursor = info.hCursor;
#endif
}

//
// DEBUG: Some visualization for LF->CRLF conversion code.
//
/*
static void ShowClipText(char *caption, char *text)
{
	int len = strlen(text);
	char *out_text = new char[len * 4 + 8];
	int pos = 0;

	out_text[pos++] = '"';
	for (int i = 0; i < len; i++) {
		if (text[i] == '\r') {
			strcpy(&out_text[pos], "\\r");
			pos += 2;
		} else if (text[i] == '\n') {
			strcpy(&out_text[pos], "\\n");
			pos += 2;
		} else if (text[i] < ' ') {
			strcpy(&out_text[pos], "\\?");
			pos += 2;
		} else {
			out_text[pos++] = text[i];
		}
	}
	out_text[pos++] = '"';
	out_text[pos++] = '\0';

	MessageBox(NULL, out_text, caption, MB_OK);

	delete[] out_text;
}
*/

//
// Convert text from Unix (LF only) format to CR+LF.
// NOTE: The size of dst[] buffer must be at least (strlen(src) * 2 + 1).
//

void
vncDesktop::ConvertClipText(char *dst, const char *src)
{
	const char *ptr0 = src;
	const char *ptr1;
	int dst_pos = 0;

	while ((ptr1 = strchr(ptr0, '\n')) != NULL) {
		// Copy the string before the LF
		if (ptr1 != ptr0) {
			memcpy(&dst[dst_pos], ptr0, ptr1 - ptr0);
			dst_pos += ptr1 - ptr0;
		}
		// Don't insert CR if there is one already
		if (ptr1 == ptr0 || *(ptr1 - 1) != '\r') {
			dst[dst_pos++] = '\r';
		}
		// Append LF
		dst[dst_pos++] = '\n';
		// Next position in the source text
		ptr0 = ptr1 + 1;
	}
	// Copy the last string with no LF, but with '\0'
	memcpy(&dst[dst_pos], ptr0, &src[strlen(src)] - ptr0 + 1);
}

//
// Manipulation of the clipboard
//

void
vncDesktop::SetClipText(LPSTR text)
{
	// Open the system clipboard
	if (OpenClipboard(m_hwnd))
	{
		// Empty it
		if (EmptyClipboard())
		{
			HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
									  strlen(text) * 2 + 1);

			if (hMem != NULL)
			{
				LPSTR pMem = (char*)GlobalLock(hMem);

				// Get the data (with line endings converted to CR+LF)
				ConvertClipText(pMem, text);

				// Tell the clipboard
				GlobalUnlock(hMem);
				SetClipboardData(CF_TEXT, hMem);
			}
		}
	}

	// Now close it
	CloseClipboard();
}

// INTERNAL METHODS

inline void
vncDesktop::MaskToMaxAndShift(DWORD mask, CARD16 &max, CARD8 &shift)
{
	for (shift = 0; (mask & 1) == 0; shift++)
		mask >>= 1;
	max = (CARD16) mask;
}

// Copy data from the memory bitmap into a buffer
void
vncDesktop::CopyToBuffer(RECT &rect, BYTE *destbuff)
{
	// Are we being asked to blit from the DIBsection to itself?
	if (destbuff == m_DIBbits) {
		// Yes.  Ignore the request!
		return;
	}

	// Protect the memory bitmap
	omni_mutex_lock l(m_bitbltlock);

	int y_inv;
	BYTE * destbuffpos;

	// Calculate the scanline-ordered y position to copy from
	y_inv = m_scrinfo.framebufferHeight-rect.top-(rect.bottom-rect.top);

	// Calculate where in the output buffer to put the data
	destbuffpos = destbuff + (m_bytesPerRow * rect.top);

	// Set the number of bytes for GetDIBits to actually write
	// NOTE : GetDIBits pads the destination buffer if biSizeImage < no. of bytes required
	m_bminfo.bmi.bmiHeader.biSizeImage = (rect.bottom-rect.top) * m_bytesPerRow;

	// Get the actual bits from the bitmap into the bit buffer
	// If fast (DIBsection) blits are disabled then use the old GetDIBits technique
	if (m_DIBbits == NULL) {
		if (GetDIBits(m_hmemdc, m_membitmap, y_inv,
					(rect.bottom-rect.top), destbuffpos,
					&m_bminfo.bmi, DIB_RGB_COLORS) == 0)
		{
#ifdef _MSC_VER
			_RPT1(_CRT_WARN, "vncDesktop : [1] GetDIBits failed! %d\n", GetLastError());
			_RPT3(_CRT_WARN, "vncDesktop : thread = %d, DC = %d, bitmap = %d\n", omni_thread::self(), m_hmemdc, m_membitmap);
			_RPT2(_CRT_WARN, "vncDesktop : y = %d, height = %d\n", y_inv, (rect.bottom-rect.top));
#endif
		}
	} else {
		// Fast blits are enabled.  [I have a sneaking suspicion this will never get used, unless
		// something weird goes wrong in the code.  It's here to keep the function general, though!]

		int bytesPerPixel = m_scrinfo.format.bitsPerPixel / 8;
		BYTE *srcbuffpos = (BYTE*)m_DIBbits;

		srcbuffpos += (m_bytesPerRow * rect.top) + (bytesPerPixel * rect.left);
		destbuffpos += bytesPerPixel * rect.left;

		int widthBytes = (rect.right-rect.left) * bytesPerPixel;

		for(int y = rect.top; y < rect.bottom; y++)
		{
			memcpy(destbuffpos, srcbuffpos, widthBytes);
			srcbuffpos += m_bytesPerRow;
			destbuffpos += m_bytesPerRow;
		}
	}
}

// Callback routine used internally to catch window movement...
BOOL CALLBACK
EnumWindowsFnCopyRect(HWND hwnd, LPARAM arg)
{

	//For excluding the popup windows
	if ((GetWindowLong( hwnd, GWL_STYLE) & WS_POPUP) ==0)
	{
	
		HANDLE prop = GetProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
		if (prop != NULL) {
			
			if (IsWindowVisible(hwnd)) {
				
				RECT dest;
				POINT source;

				// Get the window rectangle
				if (GetWindowRect(hwnd, &dest)) {
					// Old position
					source.x = (SHORT) LOWORD(prop);
					source.y = (SHORT) HIWORD(prop);

					// Got the destination position.  Now send to clients!
					if ((source.x != dest.left) || (source.y != dest.top)) {
						// Update the property entry
						SHORT x = (SHORT) dest.left;
						SHORT y = (SHORT) dest.top;
						SetProp(hwnd,
							(LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0),
							(HANDLE) MAKELONG(x, y));

						// Store of the copyrect 
						((vncDesktop*)arg)->CopyRect(dest, source);
						
					}
				} else {
					RemoveProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
				}
			} else {
				RemoveProp(hwnd, (LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0));
			}
		} else {
			// If the window has become visible then save its position!
			if (IsWindowVisible(hwnd)) {
				RECT dest;

				if (GetWindowRect(hwnd, &dest)) {
					SHORT x = (SHORT) dest.left;
					SHORT y = (SHORT) dest.top;
					SetProp(hwnd,
						(LPCTSTR) MAKELONG(VNC_WINDOWPOS_ATOM, 0),
						(HANDLE) MAKELONG(x, y));
				}
			}
		}
	}
	return TRUE;
}


void
vncDesktop::SetLocalInputDisableHook(BOOL enable)
{
	SetKeyboardFilterHook(enable);
	SetMouseFilterHook(enable);
}

void
vncDesktop::SetLocalInputPriorityHook(BOOL enable)
{
	if (vncService::IsWin95()) {
		SetKeyboardPriorityHook(m_hwnd,enable,RFB_LOCAL_KEYBOARD);
		SetMousePriorityHook(m_hwnd,enable,RFB_LOCAL_MOUSE);
	} else {
		SetKeyboardPriorityLLHook(m_hwnd,enable,RFB_LOCAL_KEYBOARD);
		SetMousePriorityLLHook(m_hwnd,enable,RFB_LOCAL_MOUSE);
	}

	if (!enable)
		m_server->BlockRemoteInput(false);
}

// Routine to find out which windows have moved
void
vncDesktop::CalcCopyRects()
{
	// Enumerate all the desktop windows for movement
	EnumWindows((WNDENUMPROC)EnumWindowsFnCopyRect, (LPARAM) this);
}


// Window procedure for the Desktop window
LRESULT CALLBACK
DesktopWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	vncDesktop *_this = (vncDesktop*)GetWindowLong(hwnd, GWL_USERDATA);

	switch (iMsg)
	{

		// GENERAL

	case WM_DISPLAYCHANGE:
		// The display resolution is changing

		// We must kick off any clients since their screen size will be wrong
		_this->m_displaychanged = TRUE;
		return 0;

	case WM_SYSCOLORCHANGE:
	case WM_PALETTECHANGED:
		// The palette colours have changed, so tell the server

		// Get the system palette
		if (!_this->SetPalette())
			PostQuitMessage(0);
		// Update any palette-based clients, too
		_this->m_server->UpdatePalette();
		return 0;

	case WM_TIMER:
		switch (wParam) {
		case vncDesktop::TIMER_BLANK_SCREEN:
			if (_this->m_server->GetBlankScreen())
				_this->BlankScreen(TRUE);
			break;
		case vncDesktop::TIMER_RESTORE_SCREEN:
			_this->BlankScreen(FALSE);
			break;
		}
		return 0;

		// CLIPBOARD MESSAGES

	case WM_CHANGECBCHAIN:
		// The clipboard chain has changed - check our nextviewer handle
		if ((HWND)wParam == _this->m_hnextviewer)
			_this->m_hnextviewer = (HWND)lParam;
		else
			if (_this->m_hnextviewer != NULL)
				SendMessage(_this->m_hnextviewer,
							WM_CHANGECBCHAIN,
							wParam, lParam);

		return 0;

	case WM_DRAWCLIPBOARD:
		// The clipboard contents have changed
		if((GetClipboardOwner() != _this->Window()) &&
		    _this->m_initialClipBoardSeen &&
			_this->m_clipboard_active)
		{
			LPSTR cliptext = NULL;

			// Open the clipboard
			if (OpenClipboard(_this->Window()))
			{
				// Get the clipboard data
				HGLOBAL cliphandle = GetClipboardData(CF_TEXT);
				if (cliphandle != NULL)
				{
					LPSTR clipdata = (LPSTR) GlobalLock(cliphandle);

					// Copy it into a new buffer
					if (clipdata == NULL)
						cliptext = NULL;
					else
						cliptext = strdup(clipdata);

					// Release the buffer and close the clipboard
					GlobalUnlock(cliphandle);
				}

				CloseClipboard();
			}

			if (cliptext != NULL)
			{
				int cliplen = strlen(cliptext);
				LPSTR unixtext = (char *)malloc(cliplen+1);

				// Replace CR-LF with LF - never send CR-LF on the wire,
				// since Unix won't like it
				int unixpos=0;
				for (int x=0; x<cliplen; x++)
				{
					if (cliptext[x] != '\x0d')
					{
						unixtext[unixpos] = cliptext[x];
						unixpos++;
					}
				}
				unixtext[unixpos] = 0;

				// Free the clip text
				free(cliptext);
				cliptext = NULL;

				// Now send the unix text to the server
				_this->m_server->UpdateClipText(unixtext);

				free(unixtext);
			}
		}

		_this->m_initialClipBoardSeen = TRUE;

		if (_this->m_hnextviewer != NULL)
		{
			// Pass the message to the next window in clipboard viewer chain.  
			return SendMessage(_this->m_hnextviewer, WM_DRAWCLIPBOARD, 0,0); 
		}

		return 0;

	default:
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
	}
}

BOOL
vncDesktop::CheckUpdates()
{
	// Update the state of blank screen timer
	UpdateBlankScreenTimer();

	// Has the display resolution or desktop changed?
	if (m_displaychanged || !vncService::InputDesktopSelected() || !inConsoleSession())
	{
		vnclog.Print(LL_STATE, VNCLOG("display resolution or desktop changed.\n"));

		rfbServerInitMsg oldscrinfo = m_scrinfo;
		m_displaychanged = FALSE;

		// Attempt to close the old hooks
		if (!Shutdown()) {
			vnclog.Print(LL_INTERR, VNCLOG("failed to close desktop server.\n"));
			m_server->KillAuthClients();
			return FALSE;
		}

		if (!Startup()) {
			vnclog.Print(LL_INTERR, VNCLOG("failed to re-start desktop server.\n"));
			m_server->KillAuthClients();
			return FALSE;
		}

		// Check if the screen info has changed
		vnclog.Print(LL_INTINFO,
					 VNCLOG("SCR: old screen format %dx%dx%d\n"),
					 oldscrinfo.framebufferWidth,
					 oldscrinfo.framebufferHeight,
					 oldscrinfo.format.bitsPerPixel);
		vnclog.Print(LL_INTINFO,
					 VNCLOG("SCR: new screen format %dx%dx%d\n"),
					 m_scrinfo.framebufferWidth,
					 m_scrinfo.framebufferHeight,
					 m_scrinfo.format.bitsPerPixel);
		if (memcmp(&m_scrinfo, &oldscrinfo, sizeof(oldscrinfo)) != 0) {
			vnclog.Print(LL_INTINFO, VNCLOG("screen format has changed.\n"));
		}

		// Call this regardless of screen format change
		m_server->UpdateLocalFormat();

		// Add a full screen update to all the clients
		m_changed_rgn.AddRect(m_bmrect);
		m_server->UpdatePalette();
	}

	// EXAMINE THE SHARED AREA / WINDOW

	RECT rect = m_server->GetSharedRect();
	RECT new_rect;
	
	if (m_server->WindowShared()) {
		HWND hwnd = m_server->GetWindowShared();
		GetWindowRect(hwnd, &new_rect);
	} else if (m_server->ScreenAreaShared()) {
		new_rect = m_server->GetScreenAreaRect();
	} else {
		new_rect = m_bmrect;
	}

	if ((m_server->WindowShared() || m_server->GetApplication()) &&
		m_server->GetWindowShared() == NULL) {
		// Disconnect clients if the shared window has dissapeared.
		// FIXME: Make this behavior configurable.
		MessageBox(NULL, "You have exited an application that is being\n"
				   "viewed/controlled from a remote PC. Exiting this\n"
				   "application will terminate the session with the remote PC.",
				   "Warning", MB_ICONWARNING | MB_OK);
		vnclog.Print(LL_CONNERR, VNCLOG("shared window not found - disconnecting clients\n"));
		m_server->KillAuthClients();
		return FALSE;
	}

	// intersect the shared rect with the desktop rect
	IntersectRect(&new_rect, &new_rect, &m_bmrect);

	// Disconnect clients if the shared window is empty (dissapeared).
	// FIXME: Make this behavior configurable.
	if (new_rect.right - new_rect.left == 0 ||
		new_rect.bottom - new_rect.top == 0) {
		vnclog.Print(LL_CONNERR, VNCLOG("shared window empty - disconnecting clients\n"));
		m_server->KillAuthClients();
		return FALSE;
	}
		
	// Update screen size if required
	if (!EqualRect(&new_rect, &rect)) {
		m_server->SetSharedRect(new_rect);
		bool sendnewfb = false;

		if ( rect.right - rect.left != new_rect.right - new_rect.left ||
			 rect.bottom - rect.top != new_rect.bottom - new_rect.top )
			sendnewfb = true;

		// FIXME: We should not send NewFBSize if a client
		//        did not send framebuffer update request.
		m_server->SetNewFBSize(sendnewfb);
		m_changed_rgn.Clear();
		return TRUE;
	}		

	// DETERMINE THE CHANGED RECTS

	// If we have clients full region requests
	if (m_server->FullRgnRequested()) {
		// Capture screen to main buffer
		CaptureScreen(rect, m_mainbuff);	
	}

	// If we have incremental update requests
	if (m_server->IncrRgnRequested()) {

		vncRegion rgn;

		// Check for moved windows
		if (m_server->FullScreen())
			CalcCopyRects();

		if (m_copyrect_set) {
			// Send copyrect to all clients
			m_server->CopyRect(m_copyrect_rect, m_copyrect_src);
			m_copyrect_set = false;

			// Copy new window rect to main buffer
			CaptureScreen(m_copyrect_rect, m_mainbuff);
			// Copy old window rect to back buffer
			CopyRectToBuffer(m_copyrect_rect, m_copyrect_src);
			// Get changed pixels to rgn
			GetChangedRegion(rgn,m_copyrect_rect);
			RECT rect;
			rect.left= m_copyrect_src.x;
			rect.top = m_copyrect_src.y;
			rect.right = rect.left + (m_copyrect_rect.right - m_copyrect_rect.left);
			rect.bottom = rect.top + (m_copyrect_rect.bottom - m_copyrect_rect.top);
			// Refresh old window rect
			m_changed_rgn.AddRect(rect);					
			// Don't refresh new window rect
			m_changed_rgn.SubtractRect(m_copyrect_rect);				
		} 

		// Get only desktop area
		vncRegion temprgn;
		temprgn.Clear();
		temprgn.AddRect(rect);
		m_changed_rgn.Intersect(temprgn);

		// Get list of rectangles for checking
		rectlist rectsToScan;
		m_changed_rgn.Rectangles(rectsToScan);

		// Capture and check them
		CheckRects(rgn, rectsToScan);

		// Update the mouse
		if (m_server->DontSetHooks())
		    UpdateCursor();
		m_server->UpdateMouse();

		// Send changed region data to all clients
		m_server->UpdateRegion(rgn);

		// Clear changed region
		m_changed_rgn.Clear();
	}

	// TRIGGER THE UPDATE

	if (m_server->FullRgnRequested() || m_server->IncrRgnRequested())
		m_server->TriggerUpdate();

	return TRUE;
}

inline void
vncDesktop::CheckRects(vncRegion &rgn, rectlist &rects)
{
	rectlist::iterator i;

	for (i = rects.begin(); i != rects.end(); i++)
	{
		// Copy data to the main buffer
		// FIXME: Maybe call CaptureScreen() just once?
		//        Check what would be more efficient.
		CaptureScreen(*i, m_mainbuff);

		// Check for changes in the rectangle
		GetChangedRegion(rgn, *i);
	}
}

// This notably improves performance when using Visual C++ 6.0 compiler
#pragma function(memcpy, memcmp)

static const int BLOCK_SIZE = 32;

void
vncDesktop::GetChangedRegion(vncRegion &rgn, const RECT &rect)
{
	const UINT bytesPerPixel = m_scrinfo.format.bitsPerPixel / 8;
	const int bytes_per_scanline = (rect.right - rect.left) * bytesPerPixel;

	const int offset = rect.top * m_bytesPerRow + rect.left * bytesPerPixel;
	unsigned char *o_ptr = m_backbuff + offset;
	unsigned char *n_ptr = m_mainbuff + offset;

	RECT new_rect = rect;

	// Fast processing for small rectangles
	if ( rect.right - rect.left <= BLOCK_SIZE &&
		 rect.bottom - rect.top <= BLOCK_SIZE ) {
		for (int y = rect.top; y < rect.bottom; y++) {
			if (memcmp(o_ptr, n_ptr, bytes_per_scanline) != 0) {
				new_rect.top = y;
				UpdateChangedSubRect(rgn, new_rect);
				break;
			}
			o_ptr += m_bytesPerRow;
			n_ptr += m_bytesPerRow;
		}
		return;
	}

	// Process bigger rectangles
	new_rect.top = -1;
	for (int y = rect.top; y < rect.bottom; y++) {
		if (memcmp(o_ptr, n_ptr, bytes_per_scanline) != 0) {
			if (new_rect.top == -1) {
				new_rect.top = y;
			}
			// Skip a number of lines after a non-matched one
			int n = BLOCK_SIZE / 2 - 1;
			y += n;
			o_ptr += n * m_bytesPerRow;
			n_ptr += n * m_bytesPerRow;
		} else {
			if (new_rect.top != -1) {
				new_rect.bottom = y;
				UpdateChangedRect(rgn, new_rect);
				new_rect.top = -1;
			}
		}
		o_ptr += m_bytesPerRow;
		n_ptr += m_bytesPerRow;
	}
	if (new_rect.top != -1) {
		new_rect.bottom = rect.bottom;
		UpdateChangedRect(rgn, new_rect);
	}
}

void
vncDesktop::UpdateChangedRect(vncRegion &rgn, const RECT &rect)
{
	// Pass small rectangles directly to UpdateChangedSubRect
	if ( rect.right - rect.left <= BLOCK_SIZE &&
		 rect.bottom - rect.top <= BLOCK_SIZE ) {
		UpdateChangedSubRect(rgn, rect);
		return;
	}

	const UINT bytesPerPixel = m_scrinfo.format.bitsPerPixel / 8;

	RECT new_rect;
	int x, y, ay;

	// Scan down the rectangle
	const int offset = rect.top * m_bytesPerRow + rect.left * bytesPerPixel;
	unsigned char *o_topleft_ptr = m_backbuff + offset;
	unsigned char *n_topleft_ptr = m_mainbuff + offset;

	for (y = rect.top; y < rect.bottom; y += BLOCK_SIZE)
	{
		// Work out way down the bitmap
		unsigned char *o_row_ptr = o_topleft_ptr;
		unsigned char *n_row_ptr = n_topleft_ptr;

		const int blockbottom = Min(y + BLOCK_SIZE, rect.bottom);
		new_rect.bottom = blockbottom;
		new_rect.left = -1;

		for (x = rect.left; x < rect.right; x += BLOCK_SIZE)
		{
			// Work our way across the row
			unsigned char *n_block_ptr = n_row_ptr;
			unsigned char *o_block_ptr = o_row_ptr;

			const UINT blockright = Min(x + BLOCK_SIZE, rect.right);
			const UINT bytesPerBlockRow = (blockright-x) * bytesPerPixel;

			// Scan this block
			for (ay = y; ay < blockbottom; ay++) {
				if (memcmp(n_block_ptr, o_block_ptr, bytesPerBlockRow) != 0)
					break;
				n_block_ptr += m_bytesPerRow;
				o_block_ptr += m_bytesPerRow;
			}
			if (ay < blockbottom) {
				// There were changes, so this block will need to be updated
				if (new_rect.left == -1) {
					new_rect.left = x;
					new_rect.top = ay;
				} else if (ay < new_rect.top) {
					new_rect.top = ay;
				}
			} else {
				// No changes in this block, process previous changed blocks if any
				if (new_rect.left != -1) {
					new_rect.right = x;
					UpdateChangedSubRect(rgn, new_rect);
					new_rect.left = -1;
				}
			}

			o_row_ptr += bytesPerBlockRow;
			n_row_ptr += bytesPerBlockRow;
		}

		if (new_rect.left != -1) {
			new_rect.right = rect.right;
			UpdateChangedSubRect(rgn, new_rect);
		}

		o_topleft_ptr += m_bytesPerRow * BLOCK_SIZE;
		n_topleft_ptr += m_bytesPerRow * BLOCK_SIZE;
	}
}

void
vncDesktop::UpdateChangedSubRect(vncRegion &rgn, const RECT &rect)
{
	const UINT bytesPerPixel = m_scrinfo.format.bitsPerPixel / 8;
	int bytes_in_row = (rect.right - rect.left) * bytesPerPixel;
	int y, i;

	// Exclude unchanged scan lines at the bottom
	int offset = (rect.bottom - 1) * m_bytesPerRow + rect.left * bytesPerPixel;
	unsigned char *o_ptr = m_backbuff + offset;
	unsigned char *n_ptr = m_mainbuff + offset;
	RECT final_rect = rect;
	final_rect.bottom = rect.top + 1;
	for (y = rect.bottom - 1; y > rect.top; y--) {
		if (memcmp(o_ptr, n_ptr, bytes_in_row) != 0) {
			final_rect.bottom = y + 1;
			break;
		}
		n_ptr -= m_bytesPerRow;
		o_ptr -= m_bytesPerRow;
	}

	// Exclude unchanged pixels at left and right sides
	offset = final_rect.top * m_bytesPerRow + final_rect.left * bytesPerPixel;
	o_ptr = m_backbuff + offset;
	n_ptr = m_mainbuff + offset;
	int left_delta = bytes_in_row - 1;
	int right_delta = 0;
	for (y = final_rect.top; y < final_rect.bottom; y++) {
		for (i = 0; i < bytes_in_row - 1; i++) {
			if (n_ptr[i] != o_ptr[i]) {
				if (i < left_delta)
					left_delta = i;
				break;
			}
		}
		for (i = bytes_in_row - 1; i > 0; i--) {
			if (n_ptr[i] != o_ptr[i]) {
				if (i > right_delta)
					right_delta = i;
				break;
			}
		}
		n_ptr += m_bytesPerRow;
		o_ptr += m_bytesPerRow;
	}
	final_rect.right = final_rect.left + right_delta / bytesPerPixel + 1;
	final_rect.left += left_delta / bytesPerPixel;

	// Update the rectangle
	rgn.AddRect(final_rect);

	// Copy the changes to the back buffer
	offset = final_rect.top * m_bytesPerRow + final_rect.left * bytesPerPixel;
	o_ptr = m_backbuff + offset;
	n_ptr = m_mainbuff + offset;
	bytes_in_row = (final_rect.right - final_rect.left) * bytesPerPixel;
	for (y = final_rect.top; y < final_rect.bottom; y++) {
		memcpy(o_ptr, n_ptr, bytes_in_row);
		n_ptr += m_bytesPerRow;
		o_ptr += m_bytesPerRow;
	}
}

void vncDesktop::CopyRect(const RECT &rcDest, const POINT &ptSrc)
{
	const int offset_x = rcDest.left - ptSrc.x;
	const int offset_y = rcDest.top - ptSrc.y;

	// Clip the destination to the screen
	RECT destrect;
	if (!IntersectRect(&destrect, &rcDest, &m_server->GetSharedRect()))
		return;

	// NOTE: This is important. Each pixel in destrect is either salvaged
	//       by copyrect or became dirty.
	m_changed_rgn.AddRect(destrect);

	// Work out the source rectangle
	RECT srcrect;
	srcrect.left = destrect.left - offset_x;
	srcrect.top = destrect.top - offset_y;
	srcrect.right = srcrect.left + destrect.right - destrect.left;
	srcrect.bottom = srcrect.top + destrect.bottom - destrect.top;

	// Clip the source to the screen
	RECT srcrect2;
	if (!IntersectRect(&srcrect2, &srcrect, &m_server->GetSharedRect()))
		return;

	destrect.left += (srcrect2.left - srcrect.left);
	destrect.top += (srcrect2.top - srcrect.top);
	destrect.right = srcrect2.right - srcrect2.left + destrect.left;
	destrect.bottom = srcrect2.bottom - srcrect2.top + destrect.top;

	if ( destrect.right - destrect.left >= 16 &&
		 destrect.bottom - destrect.top >= 16 ) {
		m_changed_rgn.SubtractRect(destrect);

		m_copyrect_rect = destrect;
		m_copyrect_src.x = srcrect2.left;
		m_copyrect_src.y = srcrect2.top;
		m_copyrect_set = TRUE;

		//DPF(("CopyRect: (%d, %d) (%d, %d, %d, %d)\n",
		//	m_copyrect_src.x,
		//	m_copyrect_src.y,
		//	m_copyrect_rect.left,
		//	m_copyrect_rect.top,
		//	m_copyrect_rect.right,
		//	m_copyrect_rect.bottom));
	}
}

//
// Copy the data from one rectangle of the back buffer to another.
//

void vncDesktop::CopyRectToBuffer(const RECT &dest, const POINT &source)
{
	const int src_x = source.x - m_bmrect.left;
	const int src_y = source.y - m_bmrect.top;
	_ASSERTE(src_x >= 0);
	_ASSERTE(src_y >= 0);

	const int dst_x = dest.left - m_bmrect.left;
	const int dst_y = dest.top - m_bmrect.top;
	_ASSERTE(dst_x >= 0);
	_ASSERTE(dst_y >= 0);

	const unsigned int bytesPerPixel = m_scrinfo.format.bitsPerPixel / 8;
	const unsigned int bytesPerLine = (dest.right - dest.left) * bytesPerPixel;

	BYTE *srcptr = m_backbuff + src_y * m_bytesPerRow + src_x * bytesPerPixel;
	BYTE *destptr = m_backbuff + dst_y * m_bytesPerRow + dst_x * bytesPerPixel;

	if (dst_y < src_y) {
		for (int y = dest.top; y < dest.bottom; y++) {
			memmove(destptr, srcptr, bytesPerLine);
			srcptr+=m_bytesPerRow;
			destptr+=m_bytesPerRow;
		}
	} else {
		srcptr += m_bytesPerRow * (dest.bottom - dest.top - 1);
		destptr += m_bytesPerRow * (dest.bottom - dest.top - 1);
		for (int y = dest.bottom; y > dest.top; y--) {
			memmove(destptr, srcptr, bytesPerLine);
			srcptr-=m_bytesPerRow;
			destptr-=m_bytesPerRow;
		}
	}
}

void
vncDesktop::UpdateBlankScreenTimer()
{
	BOOL active = m_server->GetBlankScreen();
	if (active && !m_timer_blank_screen) {
		m_timer_blank_screen = SetTimer(Window(), TIMER_BLANK_SCREEN, 50, NULL);
	} else if (!active && m_timer_blank_screen) {
		KillTimer(Window(), TIMER_BLANK_SCREEN);
		m_timer_blank_screen = 0;
		PostMessage(m_hwnd, WM_TIMER, TIMER_RESTORE_SCREEN, 0);
	}
}

void
vncDesktop::BlankScreen(BOOL set)
{
	if (set) {
		SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1, NULL, 0);
		SendMessage(GetDesktopWindow(), WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)2);
	} else {
		SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, 0);
		SendMessage(GetDesktopWindow(), WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)-1);
	}
}

void vncDesktop::startupUpdateHandler()
{
  m_updateHandler = new UpdateHandler;
  m_updateHandler->setOutUpdateListener(this);
  m_updateHandler->execute();
}

void vncDesktop::shutdownUpdateHandler()
{
  delete m_updateHandler;
}
