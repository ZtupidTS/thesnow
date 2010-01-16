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

// ---------------------------------------------------------------------------------------------
// GC graphics pipeline
// ---------------------------------------------------------------------------------------------
// 3d commands are issued through the fifo. The gpu draws to the 2MB EFB.
// The efb can be copied back into ram in two forms: as textures or as XFB.
// The XFB is the region in RAM that the VI chip scans out to the television.
// So, after all rendering to EFB is done, the image is copied into one of two XFBs in RAM.
// Next frame, that one is scanned out and the other one gets the copy. = double buffering.
// ---------------------------------------------------------------------------------------------

#ifndef _COMMON_RENDER_H_
#define _COMMON_RENDER_H_

#include "VideoCommon.h"
#include "MathUtil.h"
#include "pluginspecs_video.h"

// TODO: Move these out of here.
extern int frameCount;
extern int OSDChoice, OSDTime, OSDInternalW, OSDInternalH;

static int s_fps=0;

// Renderer really isn't a very good name for this class - it's more like "Misc".
// The long term goal is to get rid of this class and replace it with others that make
// more sense.
class Renderer
{
public:
	static bool Init();
	static void Shutdown();

	// What's the real difference between these? Too similar names.
	static void ResetAPIState();
	static void RestoreAPIState();

	static void ReinitView();

	static void SetColorMask();
	static void SetBlendMode(bool forceUpdate);
	static bool SetScissorRect();
	static void SetGenerationMode();
	static void SetDepthMode();
	static void SetLogicOpMode();
	static void SetDitherMode();
	static void SetLineWidth();
	static void SetSamplerState(int stage,int texindex);
	static void SetInterlacingMode();
	// Live resolution change
	static bool Allow2x();
	static bool AllowCustom();

	// Render target management
	static int GetFrameBufferWidth();
	static int GetFrameBufferHeight();
	static int GetCustomWidth();
	static int GetCustomHeight();
	static int GetTargetWidth();
	static int GetTargetHeight();
	static int GetFullTargetWidth();
	static int GetFullTargetHeight();

	// Multiply any 2D EFB coordinates by these when rendering.
	static float GetTargetScaleX();
    static float GetTargetScaleY();

	static TargetRectangle ConvertEFBRectangle(const EFBRectangle& rc);

	static u32 AccessEFB(EFBAccessType type, int x, int y);

	// Random utilities
    static void RenderText(const char* pstr, int left, int top, u32 color);
	static void DrawDebugText();
	static void SetScreenshot(const char *filename);
	static void FlipImageData(u8 *data, int w, int h);
	static bool SaveRenderTarget(const char *filename, int w, int h, int YOffset = 0);

	static void ClearScreen(const EFBRectangle& rc, bool colorEnable, bool alphaEnable, bool zEnable, u32 color, u32 z);
	static void RenderToXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc);

    // Finish up the current frame, print some stats
    static void Swap(u32 xfbAddr, FieldType field, u32 fbWidth, u32 fbHeight);
};

void UpdateViewport();

#endif // _COMMON_RENDER_H_
