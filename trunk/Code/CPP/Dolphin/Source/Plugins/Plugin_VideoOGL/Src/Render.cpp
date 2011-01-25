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

#include "Globals.h"
#include "Thread.h"
#include "Atomic.h"

#include <vector>
#include <cmath>
#include <cstdio>

#include "GLUtil.h"

#include "FileUtil.h"

#ifdef _WIN32
#include <mmsystem.h>
#endif

#include "CommonPaths.h"
#include "VideoConfig.h"
#include "Profiler.h"
#include "Statistics.h"
#include "ImageWrite.h"
#include "PixelEngine.h"
#include "Render.h"
#include "OpcodeDecoding.h"
#include "BPStructs.h"
#include "TextureCache.h"
#include "RasterFont.h"
#include "VertexShaderGen.h"
#include "DLCache.h"
#include "PixelShaderCache.h"
#include "PixelShaderManager.h"
#include "VertexShaderCache.h"
#include "VertexShaderManager.h"
#include "VertexLoaderManager.h"
#include "VertexLoader.h"
#include "PostProcessing.h"
#include "TextureConverter.h"
#include "OnScreenDisplay.h"
#include "Timer.h"
#include "StringUtil.h"
#include "FramebufferManager.h"
#include "Fifo.h"
#include "Debugger.h"

#include "main.h" // Local
#ifdef _WIN32
#include "EmuWindow.h"
#endif
#if defined _WIN32 || defined HAVE_LIBAV
#include "AVIDump.h"
#endif

#if defined(HAVE_WX) && HAVE_WX
#include <wx/image.h>
#endif

// Declarations and definitions
// ----------------------------
int s_fps=0;

#if defined HAVE_CG && HAVE_CG
CGcontext g_cgcontext;
CGprofile g_cgvProf;
CGprofile g_cgfProf;
#endif

RasterFont* s_pfont = NULL;

#if defined _WIN32 || defined HAVE_LIBAV
static bool s_bAVIDumping = false;
#else
static FILE* f_pFrameDump;
#endif

// 1 for no MSAA. Use s_MSAASamples > 1 to check for MSAA.
static int s_MSAASamples = 1;
static int s_MSAACoverageSamples = 0;

bool s_bHaveFramebufferBlit = false; // export to FramebufferManager.cpp
static bool s_bHaveCoverageMSAA = false;
static u32 s_blendMode;

#if defined(HAVE_WX) && HAVE_WX
static Common::Thread *scrshotThread = 0;
#endif

#ifdef _WIN32
extern int OSDChoice, OSDTime, OSDInternalW, OSDInternalH;
#else
int OSDChoice, OSDTime, OSDInternalW, OSDInternalH;
#endif

namespace
{

#if defined(HAVE_WX) && HAVE_WX
// Screenshot thread struct
typedef struct
{
	int W, H;
	std::string filename;
	wxImage *img;
} ScrStrct;
#endif

static const GLenum glSrcFactors[8] =
{
	GL_ZERO,
	GL_ONE,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA, // NOTE: If dual-source blending is enabled, use SRC1_ALPHA
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

static const GLenum glDestFactors[8] = {
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA, // NOTE: If dual-source blending is enabled, use SRC1_ALPHA
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

static const GLenum glCmpFuncs[8] = {
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

static const GLenum glLogicOpCodes[16] = {
	GL_CLEAR,
	GL_AND,
	GL_AND_REVERSE,
	GL_COPY,
	GL_AND_INVERTED,
	GL_NOOP,
	GL_XOR,
	GL_OR,
	GL_NOR,
	GL_EQUIV,
	GL_INVERT,
	GL_OR_REVERSE,
	GL_COPY_INVERTED,
	GL_OR_INVERTED,
	GL_NAND,
	GL_SET
};

#if defined HAVE_CG && HAVE_CG
void HandleCgError(CGcontext ctx, CGerror err, void* appdata)
{
	DEBUG_LOG(VIDEO, "Cg error: %s", cgGetErrorString(err));
	const char* listing = cgGetLastListing(g_cgcontext);
	if (listing != NULL)
		DEBUG_LOG(VIDEO, "    last listing: %s", listing);
}
#endif

} // namespace
void VideoConfig::UpdateProjectionHack()
{
	::UpdateProjectionHack(g_Config.iPhackvalue);
}

namespace OGL
{

// Init functions
Renderer::Renderer()
{
	bool bSuccess = true;
	s_blendMode = 0;
	s_MSAACoverageSamples = 0;
	GLint numvertexattribs = 0;

	switch (g_ActiveConfig.iMultisampleMode)
	{
		case MULTISAMPLE_OFF:
			s_MSAASamples = 1;
			break;
		case MULTISAMPLE_2X:
			s_MSAASamples = 2;
			break;
		case MULTISAMPLE_4X:
			s_MSAASamples = 4;
			break;
		case MULTISAMPLE_8X:
			s_MSAASamples = 8;
			break;
		case MULTISAMPLE_CSAA_8X:
			s_MSAASamples = 4;
			s_MSAACoverageSamples = 8;
			break;
		case MULTISAMPLE_CSAA_8XQ:
			s_MSAASamples = 8;
			s_MSAACoverageSamples = 8;
			break;
		case MULTISAMPLE_CSAA_16X:
			s_MSAASamples = 4;
			s_MSAACoverageSamples = 16;
			break;
		case MULTISAMPLE_CSAA_16XQ:
			s_MSAASamples = 8;
			s_MSAACoverageSamples = 16;
			break;
		default:
			s_MSAASamples = 1;
			break;
	}

#if defined HAVE_CG && HAVE_CG
	g_cgcontext = cgCreateContext();
	cgGetError();
	cgSetErrorHandler(HandleCgError, NULL);
#endif

	// Look for required extensions.
	const char *ptoken = (const char*)glGetString(GL_EXTENSIONS);
	if (!ptoken)
	{
		PanicAlert("Your OpenGL Driver seems to be not working.\n"
				"Please make sure your drivers are up-to-date and\n"
				"that your video hardware is OpenGL 2.x compatible.");
		return;	// TODO: fail
	}

	INFO_LOG(VIDEO, "Supported OpenGL Extensions:");
	INFO_LOG(VIDEO, "%s", ptoken);  // write to the log file
	INFO_LOG(VIDEO, "\n");

	OSD::AddMessage(StringFromFormat("Video Info: %s, %s, %s",
				glGetString(GL_VENDOR),
				glGetString(GL_RENDERER),
				glGetString(GL_VERSION)).c_str(), 5000);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numvertexattribs);
	if (numvertexattribs < 11)
	{
		ERROR_LOG(VIDEO, "GPU: OGL ERROR: Number of attributes %d not enough.\n"
				"GPU: Does your video card support OpenGL 2.x?",
				numvertexattribs);
		bSuccess = false;
	}

	// Init extension support.
	if (glewInit() != GLEW_OK)
	{
		ERROR_LOG(VIDEO, "glewInit() failed! Does your video card support OpenGL 2.x?");
		return;	// TODO: fail
	}

	if (!GLEW_EXT_framebuffer_object)
	{
		ERROR_LOG(VIDEO, "GPU: ERROR: Need GL_EXT_framebufer_object for multiple render targets.\n"
				"GPU: Does your video card support OpenGL 2.x?");
		bSuccess = false;
	}

	if (!GLEW_EXT_secondary_color)
	{
		ERROR_LOG(VIDEO, "GPU: OGL ERROR: Need GL_EXT_secondary_color.\n"
				"GPU: Does your video card support OpenGL 2.x?");
		bSuccess = false;
	}

	s_bHaveFramebufferBlit = strstr(ptoken, "GL_EXT_framebuffer_blit") != NULL;
	if (!s_bHaveFramebufferBlit)
	{
		// MSAA ain't gonna work. turn it off if enabled.
		s_MSAASamples = 1;
	}

	s_bHaveCoverageMSAA = strstr(ptoken, "GL_NV_framebuffer_multisample_coverage") != NULL;
	if (!s_bHaveCoverageMSAA)
	{
		s_MSAACoverageSamples = 0;
	}

	if (!bSuccess)
		return;	// TODO: fail

	// Decide frambuffer size
	s_backbuffer_width = (int)OpenGL_GetBackbufferWidth();
	s_backbuffer_height = (int)OpenGL_GetBackbufferHeight();

	// Handle VSync on/off
#if defined USE_WX && USE_WX
	// TODO: FILL IN
#elif defined _WIN32
	if (WGLEW_EXT_swap_control)
		wglSwapIntervalEXT(g_ActiveConfig.bVSync ? 1 : 0);
	else
		ERROR_LOG(VIDEO, "No support for SwapInterval (framerate clamped to monitor refresh rate).");
#elif defined(HAVE_X11) && HAVE_X11
	if (glXSwapIntervalSGI)
		glXSwapIntervalSGI(g_ActiveConfig.bVSync ? 1 : 0);
	else
		ERROR_LOG(VIDEO, "No support for SwapInterval (framerate clamped to monitor refresh rate).");
#endif

	// check the max texture width and height
	GLint max_texture_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&max_texture_size);
	if (max_texture_size < 1024)
		ERROR_LOG(VIDEO, "GL_MAX_TEXTURE_SIZE too small at %i - must be at least 1024.",
				max_texture_size);

	if (GL_REPORT_ERROR() != GL_NO_ERROR)
		bSuccess = false;

	if (glDrawBuffers == NULL && !GLEW_ARB_draw_buffers)
		glDrawBuffers = glDrawBuffersARB;

	if (!GLEW_ARB_texture_non_power_of_two)
		WARN_LOG(VIDEO, "ARB_texture_non_power_of_two not supported.");

	s_XFB_width = MAX_XFB_WIDTH;
	s_XFB_height = MAX_XFB_HEIGHT;

	TargetRectangle dst_rect;
	ComputeDrawRectangle(s_backbuffer_width, s_backbuffer_height, false, &dst_rect);

	CalculateXYScale(dst_rect);

	s_LastEFBScale = g_ActiveConfig.iEFBScale;
	CalculateTargetSize();

	// Because of the fixed framebuffer size we need to disable the resolution
	// options while running
	g_Config.bRunning = true;

	if (GL_REPORT_ERROR() != GL_NO_ERROR)
		bSuccess = false;

	// Initialize the FramebufferManager
	g_framebuffer_manager = new FramebufferManager(s_target_width, s_target_height,
			s_MSAASamples, s_MSAACoverageSamples);

	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	if (GL_REPORT_ERROR() != GL_NO_ERROR)
		bSuccess = false;

	s_pfont = new RasterFont();

#if defined HAVE_CG && HAVE_CG
	// load the effect, find the best profiles (if any)
	if (cgGLIsProfileSupported(CG_PROFILE_ARBVP1) != CG_TRUE)
	{
		ERROR_LOG(VIDEO, "arbvp1 not supported");
		return;	// TODO: fail
	}

	if (cgGLIsProfileSupported(CG_PROFILE_ARBFP1) != CG_TRUE)
	{
		ERROR_LOG(VIDEO, "arbfp1 not supported");
		return;	// TODO: fail
	}

	g_cgvProf = cgGLGetLatestProfile(CG_GL_VERTEX);
	g_cgfProf = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	if (strstr((const char*)glGetString(GL_VENDOR), "Humper") == NULL)
	{
#if CG_VERSION_NUM == 2100
	// A bug was introduced in Cg2.1's handling of very large profile option values
	// so this will not work on ATI. ATI returns MAXINT = 2147483647 (0x7fffffff)
	// which is correct in OpenGL but Cg fails to handle it properly. As a result
	// -1 is used by Cg resulting (signedness incorrect) and compilation fails.
		if (strstr((const char*)glGetString(GL_VENDOR), "ATI") == NULL)
#endif
		{
			cgGLSetOptimalOptions(g_cgvProf);
			cgGLSetOptimalOptions(g_cgfProf);
		}
	}
#endif	// HAVE_CG

	int nenvvertparams, nenvfragparams, naddrregisters[2];
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB,
			GL_MAX_PROGRAM_ENV_PARAMETERS_ARB,
			(GLint *)&nenvvertparams);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,
			GL_MAX_PROGRAM_ENV_PARAMETERS_ARB,
			(GLint *)&nenvfragparams);
	glGetProgramivARB(GL_VERTEX_PROGRAM_ARB,
			GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,
			(GLint *)&naddrregisters[0]);
	glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB,
			GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB,
			(GLint *)&naddrregisters[1]);
	DEBUG_LOG(VIDEO, "Max program env parameters: vert=%d, frag=%d",
			nenvvertparams, nenvfragparams);
	DEBUG_LOG(VIDEO, "Max program address register parameters: vert=%d, frag=%d",
			naddrregisters[0], naddrregisters[1]);

	if (nenvvertparams < 238)
		ERROR_LOG(VIDEO, "Not enough vertex shader environment constants!!");

#if defined HAVE_CG && HAVE_CG
	INFO_LOG(VIDEO, "Max buffer sizes: %d %d",
		cgGetProgramBufferMaxSize(g_cgvProf),
		cgGetProgramBufferMaxSize(g_cgfProf));
#ifndef _DEBUG
	cgGLSetDebugMode(GL_FALSE);
#endif
#endif

	glStencilFunc(GL_ALWAYS, 0, 0);
	glBlendFunc(GL_ONE, GL_ONE);

	glViewport(0, 0, GetTargetWidth(), GetTargetHeight()); // Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_LEQUAL);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);  // 4-byte pixel alignment

	glDisable(GL_STENCIL_TEST);
	glEnable(GL_SCISSOR_TEST);

	glScissor(0, 0, GetTargetWidth(), GetTargetHeight());
	glBlendColorEXT(0, 0, 0, 0.5f);
	glClearDepth(1.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// legacy multitexturing: select texture channel only.
	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	UpdateActiveConfig();

	//return GL_REPORT_ERROR() == GL_NO_ERROR && bSuccess;
	return;
}

Renderer::~Renderer()
{
	g_Config.bRunning = false;
	UpdateActiveConfig();
	delete s_pfont;
	s_pfont = 0;

#if defined HAVE_CG && HAVE_CG
	if (g_cgcontext)
	{
		cgDestroyContext(g_cgcontext);
		g_cgcontext = 0;
	}
#endif

#if defined(HAVE_WX) && HAVE_WX
	if (scrshotThread)
		delete scrshotThread;
#endif

	delete g_framebuffer_manager;

#if defined _WIN32 || defined HAVE_LIBAV
	if(s_bAVIDumping)
		AVIDump::Stop();
#else
	if(f_pFrameDump != NULL)
		fclose(f_pFrameDump);
#endif
}

// Create On-Screen-Messages
void Renderer::DrawDebugInfo()
{
	// Reset viewport for drawing text
	glViewport(0, 0, OpenGL_GetBackbufferWidth(), OpenGL_GetBackbufferHeight());
	// Draw various messages on the screen, like FPS, statistics, etc.
	char debugtext_buffer[8192];
	char *p = debugtext_buffer;
	p[0] = 0;

	if (g_ActiveConfig.bShowFPS)
		p+=sprintf(p, "FPS: %d\n", s_fps);

	if (g_ActiveConfig.bShowEFBCopyRegions)
	{
		// Store Line Size
		GLfloat lSize;
		glGetFloatv(GL_LINE_WIDTH, &lSize);

		// Set Line Size
		glLineWidth(3.0f);

		glBegin(GL_LINES);

		// Draw EFB copy regions rectangles
		for (std::vector<EFBRectangle>::const_iterator it = stats.efb_regions.begin();
			it != stats.efb_regions.end(); ++it)
		{
			GLfloat halfWidth = EFB_WIDTH / 2.0f;
			GLfloat halfHeight = EFB_HEIGHT / 2.0f;
			GLfloat x =  (GLfloat) -1.0f + ((GLfloat)it->left / halfWidth);
			GLfloat y =  (GLfloat) 1.0f - ((GLfloat)it->top / halfHeight);
			GLfloat x2 = (GLfloat) -1.0f + ((GLfloat)it->right / halfWidth);
			GLfloat y2 = (GLfloat) 1.0f - ((GLfloat)it->bottom / halfHeight);

			// Draw shadow of rect
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex2f(x, y - 0.01);  glVertex2f(x2, y - 0.01);
			glVertex2f(x, y2 - 0.01); glVertex2f(x2, y2 - 0.01);
			glVertex2f(x + 0.005, y);  glVertex2f(x + 0.005, y2);
			glVertex2f(x2 + 0.005, y); glVertex2f(x2 + 0.005, y2);

			// Draw rect
			glColor3f(0.0f, 1.0f, 1.0f);
			glVertex2f(x, y);  glVertex2f(x2, y);
			glVertex2f(x, y2); glVertex2f(x2, y2);
			glVertex2f(x, y);  glVertex2f(x, y2);
			glVertex2f(x2, y); glVertex2f(x2, y2);
		}

		glEnd();

		// Restore Line Size
		glLineWidth(lSize);

		// Clear stored regions
		stats.efb_regions.clear();
	}

	if (g_ActiveConfig.bOverlayStats)
		p = Statistics::ToString(p);

	if (g_ActiveConfig.bOverlayProjStats)
		p = Statistics::ToStringProj(p);

	// Render a shadow, and then the text.
	if (p != debugtext_buffer)
	{
		Renderer::RenderText(debugtext_buffer, 21, 21, 0xDD000000);
		Renderer::RenderText(debugtext_buffer, 20, 20, 0xFF00FFFF);
	}
}

void Renderer::RenderText(const char *text, int left, int top, u32 color)
{
	const int nBackbufferWidth = (int)OpenGL_GetBackbufferWidth();
	const int nBackbufferHeight = (int)OpenGL_GetBackbufferHeight();
	
	glColor4f(((color>>16) & 0xff)/255.0f, ((color>> 8) & 0xff)/255.0f,
		((color>> 0) & 0xff)/255.0f, ((color>>24) & 0xFF)/255.0f);

	s_pfont->printMultilineText(text,
		left * 2.0f / (float)nBackbufferWidth - 1,
		1 - top * 2.0f / (float)nBackbufferHeight,
		0, nBackbufferWidth, nBackbufferHeight);

	GL_REPORT_ERRORD();
}

TargetRectangle Renderer::ConvertEFBRectangle(const EFBRectangle& rc)
{
	TargetRectangle result;
	result.left   = EFBToScaledX(rc.left) + TargetStrideX();
	result.top    = EFBToScaledY(EFB_HEIGHT - rc.top) + TargetStrideY();
	result.right  = EFBToScaledX(rc.right) - (TargetStrideX() * 2);
	result.bottom = EFBToScaledY(EFB_HEIGHT - rc.bottom) - (TargetStrideY() * 2);
	return result;
}

// Function: This function handles the OpenGL glScissor() function
// ----------------------------
// Call browser: OpcodeDecoding.cpp ExecuteDisplayList > Decode() > LoadBPReg()
//		case 0x52 > SetScissorRect()
// ----------------------------
// bpmem.scissorTL.x, y = 342x342
// bpmem.scissorBR.x, y = 981x821
// Renderer::GetTargetHeight() = the fixed ini file setting
// donkopunchstania - it appears scissorBR is the bottom right pixel inside the scissor box
// therefore the width and height are (scissorBR + 1) - scissorTL
bool Renderer::SetScissorRect()
{
	MathUtil::Rectangle<float> rc;
	GetScissorRect(rc);

	if (rc.left < 0) rc.left = 0;
	if (rc.top < 0) rc.top = 0;
	if (rc.right > EFB_WIDTH) rc.right = EFB_WIDTH;
	if (rc.bottom > EFB_HEIGHT) rc.bottom = EFB_HEIGHT;

	if (rc.left > rc.right)
	{
		int temp = rc.right;
		rc.right = rc.left;
		rc.left = temp;
	}
	if (rc.top > rc.bottom)
	{
		int temp = rc.bottom;
		rc.bottom = rc.top;
		rc.top = temp;
	}

	// Check that the coordinates are good
	if (rc.right != rc.left && rc.bottom != rc.top)
	{
		glScissor(
			EFBToScaledX(rc.left), // x = 0 for example
			EFBToScaledY(EFB_HEIGHT - rc.bottom), // y = 0 for example
			EFBToScaledX(rc.right - rc.left), // width = 640 for example
			EFBToScaledY(rc.bottom - rc.top)); // height = 480 for example
		return true;
	}
	else
	{
		glScissor(
			0,
			0,
			Renderer::GetTargetWidth(),
			Renderer::GetTargetHeight()
			);
	}
	return false;
}

void Renderer::SetColorMask()
{
	// Only enable alpha channel if it's supported by the current EFB format
	GLenum ColorMask = GL_FALSE, AlphaMask = GL_FALSE;
	if (bpmem.blendmode.colorupdate)
		ColorMask = GL_TRUE;
	if (bpmem.blendmode.alphaupdate && (bpmem.zcontrol.pixel_format == PIXELFMT_RGBA6_Z24))
		AlphaMask = GL_TRUE;
	glColorMask(ColorMask,  ColorMask,  ColorMask,  AlphaMask);
}

// This function allows the CPU to directly access the EFB.
// There are EFB peeks (which will read the color or depth of a pixel)
// and EFB pokes (which will change the color or depth of a pixel).
//
// The behavior of EFB peeks can only be modified by:
//	- GX_PokeAlphaRead
// The behavior of EFB pokes can be modified by:
//	- GX_PokeAlphaMode (TODO)
//	- GX_PokeAlphaUpdate (TODO)
//	- GX_PokeBlendMode (TODO)
//	- GX_PokeColorUpdate (TODO)
//	- GX_PokeDither (TODO)
//	- GX_PokeDstAlpha (TODO)
//	- GX_PokeZMode (TODO)
u32 Renderer::AccessEFB(EFBAccessType type, u32 x, u32 y, u32 poke_data)
{
	if (!g_ActiveConfig.bEFBAccessEnable)
		return 0;

	// Get the rectangular target region covered by the EFB pixel
	EFBRectangle efbPixelRc;
	efbPixelRc.left = x;
	efbPixelRc.top = y;
	efbPixelRc.right = x + 1;
	efbPixelRc.bottom = y + 1;

	TargetRectangle targetPixelRc = ConvertEFBRectangle(efbPixelRc);

	// TODO (FIX) : currently, AA path is broken/offset and doesn't return the correct pixel
	switch (type)
	{
	case PEEK_Z:
		{
			if (s_MSAASamples > 1)
			{
				// Resolve our rectangle.
				FramebufferManager::GetEFBDepthTexture(efbPixelRc);
				glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, FramebufferManager::GetResolvedFramebuffer());
			}

			// Sample from the center of the target region.
			int srcX = (targetPixelRc.left + targetPixelRc.right) / 2;
			int srcY = (targetPixelRc.top + targetPixelRc.bottom) / 2;

			u32 z = 0;
			glReadPixels(srcX, srcY, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &z);
			GL_REPORT_ERRORD();

			// Scale the 32-bit value returned by glReadPixels to a 24-bit
			// value (GC uses a 24-bit Z-buffer).
			// TODO: in RE0 this value is often off by one, which causes lighting to disappear
			if(bpmem.zcontrol.pixel_format == PIXELFMT_RGB565_Z16)
			{
				// if Z is in 16 bit format you must return a 16 bit integer
				z = z >> 16;
			}
			else
			{
				z = z >> 8;
			}
			return z;
		}

	case PEEK_COLOR: // GXPeekARGB
		{
			// Although it may sound strange, this really is A8R8G8B8 and not RGBA or 24-bit...

			// Tested in Killer 7, the first 8bits represent the alpha value which is used to
			// determine if we're aiming at an enemy (0x80 / 0x88) or not (0x70)
			// Wind Waker is also using it for the pictograph to determine the color of each pixel

			if (s_MSAASamples > 1)
			{
				// Resolve our rectangle.
				FramebufferManager::GetEFBColorTexture(efbPixelRc);
				glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, FramebufferManager::GetResolvedFramebuffer());
			}

			// Sample from the center of the target region.
			int srcX = (targetPixelRc.left + targetPixelRc.right) / 2;
			int srcY = (targetPixelRc.top + targetPixelRc.bottom) / 2;

			// Read back pixel in BGRA format, then byteswap to get GameCube's ARGB Format.
			u32 color = 0;
			glReadPixels(srcX, srcY, 1, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &color);
			GL_REPORT_ERRORD();

			// check what to do with the alpha channel (GX_PokeAlphaRead)
			PixelEngine::UPEAlphaReadReg alpha_read_mode;
			PixelEngine::Read16((u16&)alpha_read_mode, PE_ALPHAREAD);

			if (bpmem.zcontrol.pixel_format == PIXELFMT_RGBA6_Z24)
			{
				color = RGBA8ToRGBA6ToRGBA8(color);
			}
			else if (bpmem.zcontrol.pixel_format == PIXELFMT_RGB565_Z16)
			{
				color = RGBA8ToRGB565ToRGBA8(color);
			}
			if(bpmem.zcontrol.pixel_format != PIXELFMT_RGBA6_Z24)
			{
				color |= 0xFF000000;
			}
			if(alpha_read_mode.ReadMode == 2) return color; // GX_READ_NONE
			else if(alpha_read_mode.ReadMode == 1) return (color | 0xFF000000); // GX_READ_FF
			else /*if(alpha_read_mode.ReadMode == 0)*/ return (color & 0x00FFFFFF); // GX_READ_00
		}

	case POKE_COLOR:
	case POKE_Z:
		// TODO: Implement. One way is to draw a tiny pixel-sized rectangle at
		// the exact location. Note: EFB pokes are susceptible to Z-buffering
		// and perhaps blending.
		//WARN_LOG(VIDEOINTERFACE, "This is probably some kind of software rendering");
		break;

	default:
		break;
	}

	return 0;
}

// Called from VertexShaderManager
void Renderer::UpdateViewport()
{
	// reversed gxsetviewport(xorig, yorig, width, height, nearz, farz)
	// [0] = width/2
	// [1] = height/2
	// [2] = 16777215 * (farz - nearz)
	// [3] = xorig + width/2 + 342
	// [4] = yorig + height/2 + 342
	// [5] = 16777215 * farz

	int scissorXOff = bpmem.scissorOffset.x << 1;
	int scissorYOff = bpmem.scissorOffset.y << 1;

	// TODO: ceil, floor or just cast to int?
	int X = EFBToScaledX((int)ceil(xfregs.rawViewport[3] - xfregs.rawViewport[0] - (float)scissorXOff));
	int Y = EFBToScaledY((int)ceil((float)EFB_HEIGHT - xfregs.rawViewport[4] + xfregs.rawViewport[1] + (float)scissorYOff));
	int Width = EFBToScaledX((int)ceil(2.0f * xfregs.rawViewport[0]));
	int Height = EFBToScaledY((int)ceil(-2.0f * xfregs.rawViewport[1]));
	double GLNear = (xfregs.rawViewport[5] - xfregs.rawViewport[2]) / 16777216.0f;
	double GLFar = xfregs.rawViewport[5] / 16777216.0f;
	if (Width < 0)
	{
		X += Width;
		Width *= -1;
	}
	if (Height < 0)
	{
		Y += Height;
		Height *= -1;
	}
	// Update the view port
	glViewport(X, Y, Width, Height);
	glDepthRange(GLNear, GLFar);
}

void Renderer::ClearScreen(const EFBRectangle& rc, bool colorEnable, bool alphaEnable, bool zEnable, u32 color, u32 z)
{
	ResetAPIState();

	GLenum ColorMask = GL_FALSE, AlphaMask = GL_FALSE;
	if (colorEnable) ColorMask = GL_TRUE;
	if (alphaEnable) AlphaMask = GL_TRUE;
	glColorMask(ColorMask,  ColorMask,  ColorMask,  AlphaMask);

	if (zEnable)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_ALWAYS);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_NEVER);
	}

	// Update viewport for clearing the picture
	TargetRectangle targetRc = ConvertEFBRectangle(rc);
	glViewport(targetRc.left, targetRc.bottom, targetRc.GetWidth(), targetRc.GetHeight());
	glDepthRange(0.0, (float)(z & 0xFFFFFF) / float(0xFFFFFF));

	glColor4f((float)((color >> 16) & 0xFF) / 255.0f,
				(float)((color >> 8) & 0xFF) / 255.0f,
				(float)(color & 0xFF) / 255.0f,
				(float)((color >> 24) & 0xFF) / 255.0f);
	glBegin(GL_QUADS);
	glVertex3f(-1.f, -1.f, 1.f);
	glVertex3f(-1.f,  1.f, 1.f);
	glVertex3f( 1.f,  1.f, 1.f);
	glVertex3f( 1.f, -1.f, 1.f);
	glEnd();

	RestoreAPIState();
}

void Renderer::ReinterpretPixelData(unsigned int convtype)
{
	// TODO
}

void Renderer::SetBlendMode(bool forceUpdate)
{
	// blend mode bit mask
	// 0 - blend enable
	// 2 - reverse subtract enable (else add)
	// 3-5 - srcRGB function
	// 6-8 - dstRGB function

	u32 newval = bpmem.blendmode.subtract << 2;

	if (bpmem.blendmode.subtract)
		newval |= 0x0049;   // enable blending src 1 dst 1
	else if (bpmem.blendmode.blendenable)
	{
		newval |= 1;    // enable blending
		newval |= bpmem.blendmode.srcfactor << 3;
		newval |= bpmem.blendmode.dstfactor << 6;
	}

	u32 changes = forceUpdate ? 0xFFFFFFFF : newval ^ s_blendMode;

#ifdef USE_DUAL_SOURCE_BLEND
	bool useDstAlpha = !g_ActiveConfig.bDstAlphaPass && bpmem.dstalpha.enable && bpmem.blendmode.alphaupdate
		&& bpmem.zcontrol.pixel_format == PIXELFMT_RGBA6_Z24;
	bool useDualSource = useDstAlpha && GLEW_ARB_blend_func_extended;
#endif

	if (changes & 1)
		// blend enable change
		(newval & 1) ? glEnable(GL_BLEND) : glDisable(GL_BLEND);

	if (changes & 4)
	{
#ifdef USE_DUAL_SOURCE_BLEND
		// subtract enable change
		GLenum equation = newval & 4 ? GL_FUNC_REVERSE_SUBTRACT : GL_FUNC_ADD;
		GLenum equationAlpha = useDualSource ? GL_FUNC_ADD : equation;
		glBlendEquationSeparate(equation, equationAlpha);
#else
		glBlendEquation(newval & 4 ? GL_FUNC_REVERSE_SUBTRACT : GL_FUNC_ADD);
#endif
	}

	if (changes & 0x1F8)
	{
#ifdef USE_DUAL_SOURCE_BLEND
		GLenum srcFactor = glSrcFactors[(newval >> 3) & 7];
		GLenum srcFactorAlpha = srcFactor;
		GLenum dstFactor = glDestFactors[(newval >> 6) & 7];
		GLenum dstFactorAlpha = dstFactor;
		if (useDualSource)
		{
			srcFactorAlpha = GL_ONE;
			dstFactorAlpha = GL_ZERO;

			if (srcFactor == GL_SRC_ALPHA)
				srcFactor = GL_SRC1_ALPHA;
			else if (srcFactor == GL_ONE_MINUS_SRC_ALPHA)
				srcFactor = GL_ONE_MINUS_SRC1_ALPHA;

			if (dstFactor == GL_SRC_ALPHA)
				dstFactor = GL_SRC1_ALPHA;
			else if (dstFactor == GL_ONE_MINUS_SRC_ALPHA)
				dstFactor = GL_ONE_MINUS_SRC1_ALPHA;
		}

		// blend RGB change
		glBlendFuncSeparate(srcFactor, dstFactor, srcFactorAlpha, dstFactorAlpha);
#else
		glBlendFunc(glSrcFactors[(newval >> 3) & 7], glDestFactors[(newval >> 6) & 7]);
#endif
	}

	s_blendMode = newval;
}

// This function has the final picture. We adjust the aspect ratio here.
void Renderer::Swap(u32 xfbAddr, FieldType field, u32 fbWidth, u32 fbHeight,const EFBRectangle& rc,float Gamma)
{
	if (g_bSkipCurrentFrame || (!XFBWrited && (!g_ActiveConfig.bUseXFB || !g_ActiveConfig.bUseRealXFB)) || !fbWidth || !fbHeight)
	{
		g_VideoInitialize.pCopiedToXFB(false);
		return;
	}
	// this function is called after the XFB field is changed, not after
	// EFB is copied to XFB. In this way, flickering is reduced in games
	// and seems to also give more FPS in ZTP

	if (field == FIELD_LOWER) xfbAddr -= fbWidth * 2;
	u32 xfbCount = 0;
	const XFBSourceBase* const* xfbSourceList = FramebufferManager::GetXFBSource(xfbAddr, fbWidth, fbHeight, xfbCount);
	if ((!xfbSourceList || xfbCount == 0) && g_ActiveConfig.bUseXFB && !g_ActiveConfig.bUseRealXFB)
	{
		g_VideoInitialize.pCopiedToXFB(false);
		return;
	}

	ResetAPIState();

	DVSTARTPROFILE();
	TargetRectangle dst_rect;
	ComputeDrawRectangle(s_backbuffer_width, s_backbuffer_height, true, &dst_rect);

	// Make sure that the wireframe setting doesn't screw up the screen copy.
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Textured triangles are necessary because of post-processing shaders

	// Disable all other stages
	for (int i = 1; i < 8; ++i)
		OGL::TextureCache::DisableStage(i);

	// Update GLViewPort
	glViewport(dst_rect.left, dst_rect.bottom, dst_rect.GetWidth(), dst_rect.GetHeight());

	GL_REPORT_ERRORD();

	// Copy the framebuffer to screen.

	// Texture map s_xfbTexture onto the main buffer
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	// Use linear filtering.
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// We must call ApplyShader here even if no post proc is selected - it takes
	// care of disabling it in that case. It returns false in case of no post processing.
	bool applyShader = PostProcessing::ApplyShader();

	const XFBSourceBase* xfbSource = NULL;

	if(g_ActiveConfig.bUseXFB)
	{
		// draw each xfb source
		// Render to the real buffer now.
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // switch to the window backbuffer

		for (u32 i = 0; i < xfbCount; ++i)
		{
			xfbSource = xfbSourceList[i];

			MathUtil::Rectangle<float> drawRc;

			if (!g_ActiveConfig.bUseRealXFB)
			{
				// use virtual xfb with offset
				int xfbHeight = xfbSource->srcHeight;
				int xfbWidth = xfbSource->srcWidth;
				int hOffset = ((s32)xfbSource->srcAddr - (s32)xfbAddr) / ((s32)fbWidth * 2);

				drawRc.top = 1.0f - (2.0f * (hOffset) / (float)fbHeight);
				drawRc.bottom = 1.0f - (2.0f * (hOffset + xfbHeight) / (float)fbHeight);
				drawRc.left = -(xfbWidth / (float)fbWidth);
				drawRc.right = (xfbWidth / (float)fbWidth);

				// The following code disables auto stretch.  Kept for reference.
				// scale draw area for a 1 to 1 pixel mapping with the draw target
				//float vScale = (float)fbHeight / (float)dst_rect.GetHeight();
				//float hScale = (float)fbWidth / (float)dst_rect.GetWidth();
				//drawRc.top *= vScale;
				//drawRc.bottom *= vScale;
				//drawRc.left *= hScale;
				//drawRc.right *= hScale;
			}
			else
			{
				drawRc.top = 1;
				drawRc.bottom = -1;
				drawRc.left = -1;
				drawRc.right = 1;
			}

			// Tell the OSD Menu about the current internal resolution
			OSDInternalW = xfbSource->sourceRc.GetWidth(); OSDInternalH = xfbSource->sourceRc.GetHeight();

			MathUtil::Rectangle<float> sourceRc;
			sourceRc.left = xfbSource->sourceRc.left;
			sourceRc.right = xfbSource->sourceRc.right;
			sourceRc.top = xfbSource->sourceRc.top;
			sourceRc.bottom = xfbSource->sourceRc.bottom;

			xfbSource->Draw(sourceRc, drawRc, 0, 0);

			// We must call ApplyShader here even if no post proc is selected.
			// It takes care of disabling it in that case. It returns false in
			// case of no post processing.
			if (applyShader)
				PixelShaderCache::DisableShader();
		}
	}
	else
	{
		TargetRectangle targetRc = ConvertEFBRectangle(rc);
		GLuint read_texture = FramebufferManager::ResolveAndGetRenderTarget(rc);
		// Render to the real buffer now.
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // switch to the window backbuffer
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, read_texture);
		if (applyShader)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(targetRc.left, targetRc.bottom);
			glMultiTexCoord2fARB(GL_TEXTURE1, 0, 0);
			glVertex2f(-1, -1);

			glTexCoord2f(targetRc.left, targetRc.top);
			glMultiTexCoord2fARB(GL_TEXTURE1, 0, 1);
			glVertex2f(-1,  1);

			glTexCoord2f(targetRc.right, targetRc.top);
			glMultiTexCoord2fARB(GL_TEXTURE1, 1, 1);
			glVertex2f( 1,  1);

			glTexCoord2f(targetRc.right, targetRc.bottom);
			glMultiTexCoord2fARB(GL_TEXTURE1, 1, 0);
			glVertex2f( 1, -1);
			glEnd();
			PixelShaderCache::DisableShader();
		}
		else
		{
			glBegin(GL_QUADS);
			glTexCoord2f(targetRc.left, targetRc.bottom);
			glVertex2f(-1, -1);

			glTexCoord2f(targetRc.left, targetRc.top);
			glVertex2f(-1, 1);

			glTexCoord2f(targetRc.right, targetRc.top);
			glVertex2f( 1, 1);

			glTexCoord2f(targetRc.right, targetRc.bottom);
			glVertex2f( 1, -1);
			glEnd();
		}
	}

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
	OGL::TextureCache::DisableStage(0);

	// Wireframe
	if (g_ActiveConfig.bWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Save screenshot
	if (s_bScreenshot)
	{
		s_criticalScreenshot.Enter();
		SaveScreenshot(s_sScreenshotName, dst_rect);
		// Reset settings
		s_sScreenshotName.clear();
		s_bScreenshot = false;
		s_criticalScreenshot.Leave();
	}

	// Frame dumps are handled a little differently in Windows
#if defined _WIN32 || defined HAVE_LIBAV
	if (g_ActiveConfig.bDumpFrames)
	{
		s_criticalScreenshot.Enter();
		int w = dst_rect.GetWidth();
		int h = dst_rect.GetHeight();
		u8 *data = new u8[3 * w * h];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(dst_rect.left, dst_rect.bottom, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);
		if (GL_REPORT_ERROR() == GL_NO_ERROR && w > 0 && h > 0)
		{
			if (!s_bLastFrameDumped)
			{
				#ifdef _WIN32
					s_bAVIDumping = AVIDump::Start(EmuWindow::GetParentWnd(), w, h);
				#else
					s_bAVIDumping = AVIDump::Start(w, h);
				#endif
				if (!s_bAVIDumping)
					OSD::AddMessage("AVIDump Start failed", 2000);
				else
				{
					OSD::AddMessage(StringFromFormat(
								"Dumping Frames to \"%sframedump0.avi\" (%dx%d RGB24)",
								File::GetUserPath(D_DUMPFRAMES_IDX), w, h).c_str(), 2000);
				}
			}
			if (s_bAVIDumping)
			{
				#ifdef _WIN32
					AVIDump::AddFrame((char *) data);
				#else
					FlipImageData(data, w, h);
					AVIDump::AddFrame(data);
				#endif
			}

			s_bLastFrameDumped = true;
		}
		else
			NOTICE_LOG(VIDEO, "Error reading framebuffer");

		delete[] data;
		s_criticalScreenshot.Leave();
	}
	else
	{
		if (s_bLastFrameDumped && s_bAVIDumping)
		{
			AVIDump::Stop();
			s_bAVIDumping = false;
			OSD::AddMessage("Stop dumping frames", 2000);
		}
		s_bLastFrameDumped = false;
	}
#else
	if (g_ActiveConfig.bDumpFrames)
	{
		s_criticalScreenshot.Enter();
		char movie_file_name[255];
		int w = dst_rect.GetWidth();
		int h = dst_rect.GetHeight();
		u8 *data = new u8[3 * w * h];
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(dst_rect.left, dst_rect.bottom, w, h, GL_BGR, GL_UNSIGNED_BYTE, data);
		if (GL_REPORT_ERROR() == GL_NO_ERROR)
		{
			if (!s_bLastFrameDumped)
			{
				sprintf(movie_file_name, "%sframedump.raw", File::GetUserPath(D_DUMPFRAMES_IDX));
				f_pFrameDump = fopen(movie_file_name, "wb");
				if (f_pFrameDump == NULL)
					OSD::AddMessage("Error opening framedump.raw for writing.", 2000);
				else
				{
					char msg [255];
					sprintf(msg, "Dumping Frames to \"%s\" (%dx%d RGB24)", movie_file_name, w, h);
					OSD::AddMessage(msg, 2000);
				}
			}
			if (f_pFrameDump != NULL)
			{
				FlipImageData(data, w, h);
				fwrite(data, w * 3, h, f_pFrameDump);
				fflush(f_pFrameDump);
			}
			s_bLastFrameDumped = true;
		}

		delete[] data;
		s_criticalScreenshot.Leave();
	}
	else
	{
		if (s_bLastFrameDumped && f_pFrameDump != NULL)
		{
			fclose(f_pFrameDump);
			f_pFrameDump = NULL;
		}
		s_bLastFrameDumped = false;
	}
#endif

	// Finish up the current frame, print some stats

	SetWindowSize(fbWidth, fbHeight);

	OpenGL_Update(); // just updates the render window position and the backbuffer size
	
	bool xfbchanged = false;

	if (s_XFB_width != fbWidth || s_XFB_height != fbHeight)
	{
		xfbchanged = true;
		s_XFB_width = fbWidth;
		s_XFB_height = fbHeight;
		if (s_XFB_width < 1) s_XFB_width = MAX_XFB_WIDTH;
		if (s_XFB_width > MAX_XFB_WIDTH) s_XFB_width = MAX_XFB_WIDTH;
		if (s_XFB_height < 1) s_XFB_height = MAX_XFB_HEIGHT;
		if (s_XFB_height > MAX_XFB_HEIGHT) s_XFB_height = MAX_XFB_HEIGHT;
	}

	bool WindowResized = false;
	int W = (int)OpenGL_GetBackbufferWidth();
	int H = (int)OpenGL_GetBackbufferHeight();
	if (W != s_backbuffer_width || H != s_backbuffer_height || s_LastEFBScale != g_ActiveConfig.iEFBScale)
	{
		WindowResized = true;
		s_backbuffer_width = W;
		s_backbuffer_height = H;
		s_LastEFBScale = g_ActiveConfig.iEFBScale;
	}

	if (xfbchanged || WindowResized)
	{
		ComputeDrawRectangle(s_backbuffer_width, s_backbuffer_height, false, &dst_rect);

		CalculateXYScale(dst_rect);

		if (CalculateTargetSize())
		{
			delete g_framebuffer_manager;
			g_framebuffer_manager = new FramebufferManager(s_target_width, s_target_height,
				s_MSAASamples, s_MSAACoverageSamples);
			glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}
	}

	// Place messages on the picture, then copy it to the screen
	// ---------------------------------------------------------------------
	// Count FPS.
	// -------------
	static int fpscount = 0;
	static unsigned long lasttime = 0;
	if (Common::Timer::GetTimeMs() - lasttime >= 1000)
	{
		lasttime = Common::Timer::GetTimeMs();
		s_fps = fpscount;
		fpscount = 0;
	}
	if (XFBWrited)
		++fpscount;
	// ---------------------------------------------------------------------
	GL_REPORT_ERRORD();

	DrawDebugText();
	DrawDebugInfo();

	GL_REPORT_ERRORD();

	// Get the status of the Blend mode
	GLboolean blend_enabled = glIsEnabled(GL_BLEND);
	glDisable(GL_BLEND);
	OSD::DrawMessages();
	if (blend_enabled)
		glEnable(GL_BLEND);
	GL_REPORT_ERRORD();

#if defined(DVPROFILE)
	if (g_bWriteProfile)
	{
		//g_bWriteProfile = 0;
		static int framenum = 0;
		const int UPDATE_FRAMES = 8;
		if (++framenum >= UPDATE_FRAMES)
		{
			DVProfWrite("prof.txt", UPDATE_FRAMES);
			DVProfClear();
			framenum = 0;
		}
	}
#endif

	// Copy the rendered frame to the real window
	OpenGL_SwapBuffers();

	GL_REPORT_ERRORD();

	// Clear framebuffer
	if(!g_ActiveConfig.bAnaglyphStereo)
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	GL_REPORT_ERRORD();

	// Clean out old stuff from caches. It's not worth it to clean out the shader caches.
	DLCache::ProgressiveCleanup();
	TextureCache::Cleanup();

	frameCount++;

	GFX_DEBUGGER_PAUSE_AT(NEXT_FRAME, true);

	// Begin new frame
	// Set default viewport and scissor, for the clear to work correctly
	// New frame
	stats.ResetFrame();

	// Render to the framebuffer.
	FramebufferManager::SetFramebuffer(0);

	GL_REPORT_ERRORD();

	RestoreAPIState();

	GL_REPORT_ERRORD();
	g_Config.iSaveTargetId = 0;

	// reload textures if these settings changed
	if (g_Config.bSafeTextureCache != g_ActiveConfig.bSafeTextureCache ||
		g_Config.bUseNativeMips != g_ActiveConfig.bUseNativeMips)
		TextureCache::Invalidate(false);

	if (g_Config.bCopyEFBToTexture != g_ActiveConfig.bCopyEFBToTexture)
		TextureCache::ClearRenderTargets();

	UpdateActiveConfig();

	// For testing zbuffer targets.
	// Renderer::SetZBufferRender();
	// SaveTexture("tex.tga", GL_TEXTURE_RECTANGLE_ARB, s_FakeZTarget,
	//	      GetTargetWidth(), GetTargetHeight());
	g_VideoInitialize.pCopiedToXFB(XFBWrited || (g_ActiveConfig.bUseXFB && g_ActiveConfig.bUseRealXFB));
	XFBWrited = false;
}

// ALWAYS call RestoreAPIState for each ResetAPIState call you're doing
void Renderer::ResetAPIState()
{
	// Gets us to a reasonably sane state where it's possible to do things like
	// image copies with textured quads, etc.
	VertexShaderCache::DisableShader();
	PixelShaderCache::DisableShader();
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Renderer::RestoreAPIState()
{
	// Gets us back into a more game-like state.
	glEnable(GL_SCISSOR_TEST);
	SetGenerationMode();
	SetScissorRect();
	SetColorMask();
	SetDepthMode();
	SetBlendMode(true);
	UpdateViewport();

	VertexShaderCache::SetCurrentShader(0);
	PixelShaderCache::SetCurrentShader(0);
}

void Renderer::SetGenerationMode()
{
	// none, ccw, cw, ccw
	if (bpmem.genMode.cullmode > 0)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(bpmem.genMode.cullmode == 2 ? GL_CCW : GL_CW);
	}
	else
		glDisable(GL_CULL_FACE);
}

void Renderer::SetDepthMode()
{
	if (bpmem.zmode.testenable)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(bpmem.zmode.updateenable ? GL_TRUE : GL_FALSE);
		glDepthFunc(glCmpFuncs[bpmem.zmode.func]);
	}
	else
	{
		// if the test is disabled write is disabled too
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	}
}

void Renderer::SetLogicOpMode()
{
	if (bpmem.blendmode.logicopenable && bpmem.blendmode.logicmode != 3)
	{
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(glLogicOpCodes[bpmem.blendmode.logicmode]);
	}
	else
	{
		glDisable(GL_COLOR_LOGIC_OP);
	}
}

void Renderer::SetDitherMode()
{
	if (bpmem.blendmode.dither)
		glEnable(GL_DITHER);
	else
		glDisable(GL_DITHER);
}

void Renderer::SetLineWidth()
{
	float fratio = xfregs.rawViewport[0] != 0 ?
		((float)Renderer::GetTargetWidth() / EFB_WIDTH) : 1.0f;
	if (bpmem.lineptwidth.linesize > 0)
		// scale by ratio of widths
		glLineWidth((float)bpmem.lineptwidth.linesize * fratio / 6.0f);
	if (bpmem.lineptwidth.pointsize > 0)
		glPointSize((float)bpmem.lineptwidth.pointsize * fratio / 6.0f);
}

void Renderer::SetSamplerState(int stage, int texindex)
{
	// TODO
}

void Renderer::SetInterlacingMode()
{
	// TODO
}

void Renderer::FlipImageData(u8 *data, int w, int h)
{
	// Flip image upside down. Damn OpenGL.
	for (int y = 0; y < h / 2; y++)
	{
		for(int x = 0; x < w; x++)
		{
			std::swap(data[(y * w + x) * 3],     data[((h - 1 - y) * w + x) * 3]);
			std::swap(data[(y * w + x) * 3 + 1], data[((h - 1 - y) * w + x) * 3 + 1]);
			std::swap(data[(y * w + x) * 3 + 2], data[((h - 1 - y) * w + x) * 3 + 2]);
		}
	}
}

}

#if defined(HAVE_WX) && HAVE_WX
THREAD_RETURN TakeScreenshot(void *pArgs)
{
	ScrStrct *threadStruct = (ScrStrct *)pArgs;

	// These will contain the final image size
	float FloatW = (float)threadStruct->W;
	float FloatH = (float)threadStruct->H;

	// Handle aspect ratio for the final ScrStrct to look exactly like what's on screen.
	if (g_ActiveConfig.iAspectRatio != ASPECT_STRETCH)
	{
		bool use16_9 = g_VideoInitialize.bAutoAspectIs16_9;

		// Check for force-settings and override.
		if (g_ActiveConfig.iAspectRatio == ASPECT_FORCE_16_9)
			use16_9 = true;
		else if (g_ActiveConfig.iAspectRatio == ASPECT_FORCE_4_3)
			use16_9 = false;

		float Ratio = (FloatW / FloatH) / (!use16_9 ? (4.0f / 3.0f) : (16.0f / 9.0f));

		// If ratio > 1 the picture is too wide and we have to limit the width.
		if (Ratio > 1)
			FloatW /= Ratio;
		// ratio == 1 or the image is too high, we have to limit the height.
		else
			FloatH *= Ratio;

		// This is a bit expensive on high resolutions
		threadStruct->img->Rescale((int)FloatW, (int)FloatH, wxIMAGE_QUALITY_HIGH);
	}

	// Save the screenshot and finally kill the wxImage object
	// This is really expensive when saving to PNG, but not at all when using BMP
	threadStruct->img->SaveFile(wxString::FromAscii(threadStruct->filename.c_str()),
		wxBITMAP_TYPE_PNG);
	threadStruct->img->Destroy();

	// Show success messages
	OSD::AddMessage(StringFromFormat("Saved %i x %i %s", (int)FloatW, (int)FloatH,
		threadStruct->filename.c_str()).c_str(), 2000);
	delete threadStruct;

	return 0;
}
#endif

namespace OGL
{

bool Renderer::SaveScreenshot(const std::string &filename, const TargetRectangle &back_rc)
{
	u32 W = back_rc.GetWidth();
	u32 H = back_rc.GetHeight();
	u8 *data = new u8[3 * W * H];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glReadPixels(back_rc.left, back_rc.bottom, W, H, GL_RGB, GL_UNSIGNED_BYTE, data);

	// Show failure message
	if (GL_REPORT_ERROR() != GL_NO_ERROR)
	{
		OSD::AddMessage("Error capturing or saving screenshot.", 2000);
		return false;
	}

	// Turn image upside down
	FlipImageData(data, W, H);

#if defined(HAVE_WX) && HAVE_WX
	// Create wxImage
	wxImage *a = new wxImage(W, H, data);

	if (scrshotThread)
	{
		delete scrshotThread;
		scrshotThread = NULL;
	}

	ScrStrct *threadStruct = new ScrStrct;
	threadStruct->filename = filename;
	threadStruct->img = a;
	threadStruct->H = H; threadStruct->W = W;

	scrshotThread = new Common::Thread(TakeScreenshot, threadStruct);
#ifdef _WIN32
	scrshotThread->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
#endif
	bool result = true;

	OSD::AddMessage("Saving Screenshot... ", 2000);

#else
	bool result = SaveTGA(filename.c_str(), W, H, data);
	delete[] data;
#endif

	return result;
}

void Renderer::SetWindowSize(int width, int height)
{
	if (width < 1)
		width = 1;
	if (height < 1)
		height = 1;

	// Scale the window size by the EFB scale.
	CalculateTargetScale(width, height, width, height);

	g_VideoInitialize.pRequestWindowSize(width, height);
}

}
