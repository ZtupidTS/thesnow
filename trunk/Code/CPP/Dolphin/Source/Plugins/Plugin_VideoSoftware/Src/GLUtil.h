// Copyright (C) 2003-2009 Dolphin Project.

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


#ifndef _GLINIT_H_
#define _GLINIT_H_

#include <string>
#include "SWVideoConfig.h"

#ifdef _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/wglew.h>
#elif defined HAVE_X11 && HAVE_X11
#include <GL/glxew.h>
#include <GL/gl.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#elif defined __APPLE__
#include <GL/glew.h>
#import <AppKit/AppKit.h>
#endif

#if defined USE_WX && USE_WX
#include "wx/wx.h"
#include "wx/glcanvas.h"
#endif

#ifndef GL_DEPTH24_STENCIL8_EXT // allows FBOs to support stencils
#define GL_DEPTH_STENCIL_EXT 0x84F9
#define GL_UNSIGNED_INT_24_8_EXT 0x84FA
#define GL_DEPTH24_STENCIL8_EXT 0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT 0x88F1
#endif

#ifndef _WIN32

#include <sys/types.h>

typedef struct {
#if defined(USE_WX) && USE_WX
	wxGLCanvas *glCanvas;
	wxGLContext *glCtxt;
	wxPanel *panel;
#elif defined(HAVE_X11) && HAVE_X11
	int screen;
	Window win;
	Window parent;
	Display *dpy;
	GLXContext ctx;
	XSetWindowAttributes attr;
	int x, y;
	unsigned int width, height;
#endif 
} GLWindow;

extern GLWindow GLWin;

#endif

// Public OpenGL util

// Initialization / upkeep
bool OpenGL_Create(int _width, int _height);
void OpenGL_Shutdown();
void OpenGL_Update();
bool OpenGL_MakeCurrent();
void OpenGL_SwapBuffers();

// Get status
u32 OpenGL_GetBackbufferWidth();
u32 OpenGL_GetBackbufferHeight();

// Set things
void OpenGL_SetWindowText(const char *text);

// Error reporting - use the convenient macros.
void OpenGL_ReportARBProgramError();
GLuint OpenGL_ReportGLError(const char *function, const char *file, int line);
bool OpenGL_ReportFBOError(const char *function, const char *file, int line);

#if defined(_DEBUG) || defined(DEBUGFAST)
#define GL_REPORT_ERROR()         OpenGL_ReportGLError        (__FUNCTION__, __FILE__, __LINE__)
#define GL_REPORT_PROGRAM_ERROR() OpenGL_ReportARBProgramError()
#define GL_REPORT_FBO_ERROR()     OpenGL_ReportFBOError       (__FUNCTION__, __FILE__, __LINE__)
#define GL_REPORT_ERRORD() OpenGL_ReportGLError(__FUNCTION__, __FILE__, __LINE__)
#else
#define GL_REPORT_ERROR() GL_NO_ERROR
#define GL_REPORT_PROGRAM_ERROR()
#define GL_REPORT_FBO_ERROR()
#define GL_REPORT_ERRORD()
#endif

#endif  // _GLINIT_H_
