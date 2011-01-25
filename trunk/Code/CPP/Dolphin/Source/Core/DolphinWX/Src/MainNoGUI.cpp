// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>

#include "Common.h"
#include "FileUtil.h"

#if defined HAVE_X11 && HAVE_X11
#include <X11/keysym.h>
#include "State.h"
#include "X11Utils.h"
#endif

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif

#include "Core.h"
#include "Host.h"
#include "CPUDetect.h"
#include "Thread.h"
#include "PowerPC/PowerPC.h"
#include "HW/Wiimote.h"

#include "PluginManager.h"
#include "ConfigManager.h"
#include "LogManager.h"
#include "BootManager.h"

bool rendererHasFocus = true;
bool running = true;

void Host_NotifyMapLoaded(){}

void Host_ShowJitResults(unsigned int address){}

Common::Event updateMainFrameEvent;
void Host_Message(int Id)
{
	switch (Id)
	{
		case WM_USER_STOP:
			running = false;
			break;
	}
}

void Host_UpdateTitle(const char* title){};

void Host_UpdateLogDisplay(){}

void Host_UpdateDisasmDialog(){}

void Host_UpdateMainFrame()
{
	updateMainFrameEvent.Set();
}

void Host_UpdateBreakPointView(){}

void Host_UpdateMemoryView(){}

void Host_SetDebugMode(bool){}

void Host_GetRenderWindowSize(int& x, int& y, int& width, int& height)
{
	x = SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowXPos;
	y = SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowYPos;
	width = SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowWidth;
	height = SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowHeight;
}

void Host_RequestRenderWindowSize(int width, int height) {}

bool Host_RendererHasFocus()
{
	return rendererHasFocus;
}

void Host_ConnectWiimote(int wm_idx, bool connect) {}

void Host_SetWaitCursor(bool enable){}

void Host_UpdateStatusBar(const char* _pText, int Filed){}

void Host_SysMessage(const char *fmt, ...)
{
	va_list list;
	char msg[512];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	size_t len = strlen(msg);
	if (msg[len - 1] != '\n') {
		msg[len - 1] = '\n';
		msg[len] = '\0';
	}
	fprintf(stderr, "%s", msg);
}

void Host_SetWiiMoteConnectionState(int _State) {}

#if defined(HAVE_X11) && HAVE_X11
void X11_MainLoop()
{
	bool fullscreen = SConfig::GetInstance().m_LocalCoreStartupParameter.bFullscreen;
	while (Core::GetState() == Core::CORE_UNINITIALIZED)
		updateMainFrameEvent.Wait();

	Display *dpy = XOpenDisplay(0);
	Window win = (Window)Core::GetWindowHandle();
	XSelectInput(dpy, win, KeyPressMask | FocusChangeMask);

#if defined(HAVE_XDG_SCREENSAVER) && HAVE_XDG_SCREENSAVER
	X11Utils::InhibitScreensaver(dpy, win, true);
#endif

#if defined(HAVE_XRANDR) && HAVE_XRANDR
	X11Utils::XRRConfiguration *XRRConfig = new X11Utils::XRRConfiguration(dpy, win);
#endif

	Cursor blankCursor = None;
	if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
	{
		// make a blank cursor
		Pixmap Blank;
		XColor DummyColor;
		char ZeroData[1] = {0};
		Blank = XCreateBitmapFromData (dpy, win, ZeroData, 1, 1);
		blankCursor = XCreatePixmapCursor(dpy, Blank, Blank, &DummyColor, &DummyColor, 0, 0);
		XFreePixmap (dpy, Blank);
		XDefineCursor(dpy, win, blankCursor);
	}

	if (fullscreen)
	{
		X11Utils::EWMH_Fullscreen(dpy, _NET_WM_STATE_TOGGLE);
#if defined(HAVE_XRANDR) && HAVE_XRANDR
		XRRConfig->ToggleDisplayMode(True);
#endif
	}

	// The actual loop
	while (running)
	{
		XEvent event;
		KeySym key;
		for (int num_events = XPending(dpy); num_events > 0; num_events--)
		{
			XNextEvent(dpy, &event);
			switch(event.type)
			{
				case KeyPress:
					key = XLookupKeysym((XKeyEvent*)&event, 0);
					if (key == XK_Escape)
					{
						if (Core::GetState() == Core::CORE_RUN)
						{
							if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
								XUndefineCursor(dpy, win);
							Core::SetState(Core::CORE_PAUSE);
						}
						else
						{
							if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
								XDefineCursor(dpy, win, blankCursor);
							Core::SetState(Core::CORE_RUN);
						}
					}
					else if ((key == XK_Return) && (event.xkey.state & Mod1Mask))
					{
						fullscreen = !fullscreen;
						X11Utils::EWMH_Fullscreen(dpy, _NET_WM_STATE_TOGGLE);
#if defined(HAVE_XRANDR) && HAVE_XRANDR
						XRRConfig->ToggleDisplayMode(fullscreen);
#endif
					}
					else if (key >= XK_F1 && key <= XK_F8)
					{
						int slot_number = key - XK_F1 + 1;
						if (event.xkey.state & ShiftMask)
							State_Save(slot_number);
						else
							State_Load(slot_number);
					}
					else if (key == XK_F9)
						Core::ScreenShot();
					else if (key == XK_F11)
						State_LoadLastSaved();
					else if (key == XK_F12)
					{
						if (event.xkey.state & ShiftMask)
							State_UndoLoadState();
						else
							State_UndoSaveState();
					}
					break;
				case FocusIn:
					rendererHasFocus = true;
					if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor &&
							Core::GetState() != Core::CORE_PAUSE)
						XDefineCursor(dpy, win, blankCursor);
					break;
				case FocusOut:
					rendererHasFocus = false;
					if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
						XUndefineCursor(dpy, win);
					break;
			}
		}
		if (!fullscreen)
		{
			Window winDummy;
			unsigned int borderDummy, depthDummy;
			XGetGeometry(dpy, win, &winDummy,
					&SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowXPos,
					&SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowYPos,
					(unsigned int *)&SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowWidth,
					(unsigned int *)&SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowHeight,
					&borderDummy, &depthDummy);
		}
		usleep(100000);
	}

#if defined(HAVE_XRANDR) && HAVE_XRANDR
	delete XRRConfig;
#endif

#if defined(HAVE_XDG_SCREENSAVER) && HAVE_XDG_SCREENSAVER
	X11Utils::InhibitScreensaver(dpy, win, false);
#endif

	if (SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor)
		XFreeCursor(dpy, blankCursor);
	XCloseDisplay(dpy);
	Core::Stop();
}
#endif

int main(int argc, char* argv[])
{
#ifdef __APPLE__
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSEvent *event = [[NSEvent alloc] init];	
	ProcessSerialNumber psn;
#endif
	int ch, help = 0;
	struct option longopts[] = {
		{ "exec",	no_argument,	NULL,	'e' },
		{ "help",	no_argument,	NULL,	'h' },
		{ "version",	no_argument,	NULL,	'v' },
		{ NULL,		0,		NULL,	0 }
	};

	while ((ch = getopt_long(argc, argv, "eh?v", longopts, 0)) != -1) {
		switch (ch) {
		case 'e':
			break;
		case 'h':
		case '?':
			help = 1;
			break;
		case 'v':
			fprintf(stderr, "%s\n", svn_rev_str);
			return 1;
		}
	}

	if (help == 1 || argc == optind) {
		fprintf(stderr, "%s\n\n", svn_rev_str);
		fprintf(stderr, "A multi-platform Gamecube/Wii emulator\n\n");
		fprintf(stderr, "Usage: %s [-e <file>] [-h] [-v]\n", argv[0]);
		fprintf(stderr, "  -e, --exec	Load the specified file\n");
		fprintf(stderr, "  -h, --help	Show this help message\n");
		fprintf(stderr, "  -v, --help	Print version and exit\n");
		return 1;
	}

	updateMainFrameEvent.Init();
	LogManager::Init();
	SConfig::Init();
	CPluginManager::Init();
	CPluginManager::GetInstance().ScanForPlugins();
	WiimoteReal::LoadSettings();

	// No use running the loop when booting fails
	if (BootManager::BootCore(argv[optind]))
	{
#ifdef __APPLE__
	GetCurrentProcess(&psn);
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);
	SetFrontProcess(&psn);

	if (NSApp == nil) {
		[NSApplication sharedApplication];
		//TODO : Create menu
		[NSApp finishLaunching];
	}

	while (running)
	{
		event = [NSApp nextEventMatchingMask: NSAnyEventMask
			untilDate: [NSDate distantFuture]
			inMode: NSDefaultRunLoopMode dequeue: YES];

		if ([event type] == NSKeyDown &&
			[event modifierFlags] & NSCommandKeyMask &&
			[[event characters] UTF8String][0] == 'q')
		{
			Core::Stop();
			break;
		}

		if ([event type] != NSKeyDown)
			[NSApp sendEvent: event];
	}	

	[event release];
	[pool release];
#elif defined HAVE_X11 && HAVE_X11
		XInitThreads();
		X11_MainLoop();
#else
		while (PowerPC::GetState() != PowerPC::CPU_POWERDOWN)
			updateMainFrameEvent.Wait();
#endif
	}

	updateMainFrameEvent.Shutdown();
	WiimoteReal::Shutdown();
	CPluginManager::Shutdown();
	SConfig::Shutdown();
	LogManager::Shutdown();

	return 0;
}
