/*  ZZ Open GL graphics plugin
 *  Copyright (c)2009-2010 zeydlitz@gmail.com, arcum42@gmail.com
 *  Based on Zerofrog's ZeroGS KOSMOS (c)2005-2008
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "GS.h"
#include "zerogs.h"


#ifdef GL_X11_WINDOW

#include <X11/Xlib.h>

bool GLWindow::CreateWindow(void *pDisplay)
{
	glDisplay = XOpenDisplay(0);
	glScreen = DefaultScreen(glDisplay);

	if (pDisplay == NULL) return false;

	*(Display**)pDisplay = glDisplay;

	return true;
}

bool GLWindow::ReleaseWindow()
{
	if (context)
	{
		if (!glXMakeCurrent(glDisplay, None, NULL))
		{
			ZZLog::Error_Log("Could not release drawing context.");
		}

		glXDestroyContext(glDisplay, context);

		context = NULL;
	}

	/* switch back to original desktop resolution if we were in fullscreen */
	if (glDisplay != NULL)
	{
		if (fullScreen)
		{
			XF86VidModeSwitchToMode(glDisplay, glScreen, &deskMode);
			XF86VidModeSetViewPort(glDisplay, glScreen, 0, 0);
		}
	}

	return true;
}

void GLWindow::CloseWindow()
{
	conf.x = x;
	conf.y = y;
	SaveConfig();

	if (glDisplay != NULL)
	{
		XCloseDisplay(glDisplay);
		glDisplay = NULL;
	}
}

bool GLWindow::DisplayWindow(int _width, int _height)
{
	int i;
	XVisualInfo *vi;
	Colormap cmap;
	int dpyWidth, dpyHeight;
	int glxMajorVersion, glxMinorVersion;
	int vidModeMajorVersion, vidModeMinorVersion;
	Atom wmDelete;
	Window winDummy;
	unsigned int borderDummy;

	x = conf.x;
	y = conf.y;

	// attributes for a single buffered visual in RGBA format with at least
	// 8 bits per color and a 24 bit depth buffer
	int attrListSgl[] = {GLX_RGBA, GLX_RED_SIZE, 8,
						 GLX_GREEN_SIZE, 8,
						 GLX_BLUE_SIZE, 8,
						 GLX_DEPTH_SIZE, 24,
						 None
						};

	// attributes for a double buffered visual in RGBA format with at least
	// 8 bits per color and a 24 bit depth buffer
	int attrListDbl[] = { GLX_RGBA, GLX_DOUBLEBUFFER,
						  GLX_RED_SIZE, 8,
						  GLX_GREEN_SIZE, 8,
						  GLX_BLUE_SIZE, 8,
						  GLX_DEPTH_SIZE, 24,
						  None
						};

	GLWin.fullScreen = (conf.fullscreen());

	/* get an appropriate visual */
	vi = glXChooseVisual(glDisplay, glScreen, attrListDbl);

	if (vi == NULL)
	{
		vi = glXChooseVisual(glDisplay, glScreen, attrListSgl);
		doubleBuffered = false;
		ZZLog::Error_Log("Only Singlebuffered Visual!");
	}
	else
	{
		doubleBuffered = true;
		ZZLog::Error_Log("Got Doublebuffered Visual!");
	}

	if (vi == NULL)
	{
		ZZLog::Error_Log("Failed to get buffered Visual!");
		return false;
	}

	glXQueryVersion(glDisplay, &glxMajorVersion, &glxMinorVersion);

	ZZLog::Error_Log("glX-Version %d.%d", glxMajorVersion, glxMinorVersion);

	/* create a GLX context */
	context = glXCreateContext(glDisplay, vi, NULL, GL_TRUE);

	/* create a color map */
	cmap = XCreateColormap(glDisplay, RootWindow(glDisplay, vi->screen),
						   vi->visual, AllocNone);
	attr.colormap = cmap;
	attr.border_pixel = 0;

	// get a connection
	XF86VidModeQueryVersion(glDisplay, &vidModeMajorVersion, &vidModeMinorVersion);

	if (fullScreen)
	{
		XF86VidModeModeInfo **modes = NULL;
		int modeNum = 0;
		int bestMode = 0;

		// set best mode to current
		bestMode = 0;
		ZZLog::Error_Log("XF86VidModeExtension-Version %d.%d.", vidModeMajorVersion, vidModeMinorVersion);
		XF86VidModeGetAllModeLines(glDisplay, glScreen, &modeNum, &modes);

		if (modeNum > 0 && modes != NULL)
		{
			/* save desktop-resolution before switching modes */
			deskMode = *modes[0];

			/* look for mode with requested resolution */

			for (i = 0; i < modeNum; i++)
			{
				if ((modes[i]->hdisplay == _width) && (modes[i]->vdisplay == _height))
				{
					bestMode = i;
				}
			}

			XF86VidModeSwitchToMode(glDisplay, glScreen, modes[bestMode]);

			XF86VidModeSetViewPort(glDisplay, glScreen, 0, 0);
			dpyWidth = modes[bestMode]->hdisplay;
			dpyHeight = modes[bestMode]->vdisplay;
			ZZLog::Error_Log("Resolution %dx%d.", dpyWidth, dpyHeight);
			XFree(modes);

			/* create a fullscreen window */
			attr.override_redirect = True;
			attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
			glWindow = XCreateWindow(glDisplay, RootWindow(glDisplay, vi->screen),
									 0, 0, dpyWidth, dpyHeight, 0, vi->depth, InputOutput, vi->visual,
									 CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
									 &attr);
			XWarpPointer(glDisplay, None, glWindow, 0, 0, 0, 0, 0, 0);
			XMapRaised(glDisplay, glWindow);
			XGrabKeyboard(glDisplay, glWindow, True, GrabModeAsync, GrabModeAsync, CurrentTime);
			XGrabPointer(glDisplay, glWindow, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, glWindow, None, CurrentTime);
		}
		else
		{
			ZZLog::Error_Log("Failed to start fullscreen. If you received the \n"
							 "\"XFree86-VidModeExtension\" extension is missing, add\n"
							 "Load \"extmod\"\n"
							 "to your X configuration file (under the Module Section)");
			fullScreen = false;
		}
	}

	if (!fullScreen)
	{
		// create a window in window mode
		attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
		glWindow = XCreateWindow(glDisplay, RootWindow(glDisplay, vi->screen),
								 0, 0, _width, _height, 0, vi->depth, InputOutput, vi->visual,
								 CWBorderPixel | CWColormap | CWEventMask, &attr);

		// only set window title and handle wm_delete_events if in windowed mode
		wmDelete = XInternAtom(glDisplay, "WM_DELETE_WINDOW", True);

		XSetWMProtocols(glDisplay, glWindow, &wmDelete, 1);
		XSetStandardProperties(glDisplay, glWindow, "ZZOgl-PG", "ZZOgl-PG", None, NULL, 0, NULL);
		XMapRaised(glDisplay, glWindow);
		XMoveWindow(glDisplay, glWindow, x, y);
	}

	// connect the glx-context to the window
	glXMakeCurrent(glDisplay, glWindow, context);

	XGetGeometry(glDisplay, glWindow, &winDummy, &x, &y, &width, &height, &borderDummy, &depth);

	ZZLog::Error_Log("Depth %d", depth);

	if (glXIsDirect(glDisplay, context))
		ZZLog::Error_Log("You have Direct Rendering!");
	else
		ZZLog::Error_Log("No Direct Rendering possible!");

	// better for pad plugin key input (thc)
	XSelectInput(glDisplay, glWindow, ExposureMask | KeyPressMask | KeyReleaseMask |
				 ButtonPressMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask |
				 FocusChangeMask);

	return true;
}

void GLWindow::SwapGLBuffers()
{
	glXSwapBuffers(glDisplay, glWindow);
}

void GLWindow::SetTitle(char *strtitle)
{
	XTextProperty prop;
	memset(&prop, 0, sizeof(prop));
	char* ptitle = strtitle;

	if (XStringListToTextProperty(&ptitle, 1, &prop))
		XSetWMName(glDisplay, glWindow, &prop);

	XFree(prop.value);
}

void GLWindow::ResizeCheck()
{
	XEvent event;

	while (XCheckTypedEvent(glDisplay, ConfigureNotify, &event))
	{
		if ((event.xconfigure.width != width) || (event.xconfigure.height != height))
		{
			ZeroGS::ChangeWindowSize(event.xconfigure.width, event.xconfigure.height);
			width = event.xconfigure.width;
			height = event.xconfigure.height;
		}

		if ((event.xconfigure.x != x) || (event.xconfigure.y != y))
		{
			x = event.xconfigure.x;
			y = event.xconfigure.y;
		}
	}
}

#endif
