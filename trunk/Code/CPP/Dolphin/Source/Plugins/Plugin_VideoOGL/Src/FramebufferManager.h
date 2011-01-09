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

#ifndef _FRAMEBUFFERMANAGER_H_
#define _FRAMEBUFFERMANAGER_H_

#include "GLUtil.h"
#include "FramebufferManagerBase.h"

// On the GameCube, the game sends a request for the graphics processor to
// transfer its internal EFB (Embedded Framebuffer) to an area in GameCube RAM
// called the XFB (External Framebuffer). The size and location of the XFB is
// decided at the time of the copy, and the format is always YUYV. The video
// interface is given a pointer to the XFB, which will be decoded and
// displayed on the TV.
//
// There are two ways for Dolphin to emulate this:
//
// Real XFB mode:
//
// Dolphin will behave like the GameCube and encode the EFB to
// a portion of GameCube RAM. The emulated video interface will decode the data
// for output to the screen.
//
// Advantages: Behaves exactly like the GameCube.
// Disadvantages: Resolution will be limited.
//
// Virtual XFB mode:
//
// When a request is made to copy the EFB to an XFB, Dolphin
// will remember the RAM location and size of the XFB in a Virtual XFB list.
// The video interface will look up the XFB in the list and use the enhanced
// data stored there, if available.
//
// Advantages: Enables high resolution graphics, better than real hardware.
// Disadvantages: If the GameCube CPU writes directly to the XFB (which is
// possible but uncommon), the Virtual XFB will not capture this information.

// There may be multiple XFBs in GameCube RAM. This is the maximum number to
// virtualize.

struct XFBSource : public XFBSourceBase
{
	XFBSource(GLuint tex) : texture(tex) {}
	~XFBSource() { glDeleteTextures(1, &texture); }

	void CopyEFB(float Gamma);
	void DecodeToTexture(u32 xfbAddr, u32 fbWidth, u32 fbHeight);
	void Draw(const MathUtil::Rectangle<float> &sourcerc,
		const MathUtil::Rectangle<float> &drawrc, int width, int height) const;

	const GLuint texture;
};

class FramebufferManager : public FramebufferManagerBase
{
public:
	FramebufferManager(int targetWidth, int targetHeight, int msaaSamples, int msaaCoverageSamples);
	~FramebufferManager();

	// To get the EFB in texture form, these functions may have to transfer
	// the EFB to a resolved texture first.
	static GLuint GetEFBColorTexture(const EFBRectangle& sourceRc);
	static GLuint GetEFBDepthTexture(const EFBRectangle& sourceRc);

	static GLuint GetEFBFramebuffer() { return m_efbFramebuffer; }
	static GLuint GetXFBFramebuffer() { return m_xfbFramebuffer; }

	// Resolved framebuffer is only used in MSAA mode.
	static GLuint GetResolvedFramebuffer() { return m_resolvedFramebuffer; }

	static void SetFramebuffer(GLuint fb);

	// If in MSAA mode, this will perform a resolve of the specified rectangle, and return the resolve target as a texture ID.
	// Thus, this call may be expensive. Don't repeat it unnecessarily.
	// If not in MSAA mode, will just return the render target texture ID.
	// After calling this, before you render anything else, you MUST bind the framebuffer you want to draw to.
	static GLuint ResolveAndGetRenderTarget(const EFBRectangle &rect);

	// Same as above but for the depth Target.
	// After calling this, before you render anything else, you MUST bind the framebuffer you want to draw to.
    static GLuint ResolveAndGetDepthTarget(const EFBRectangle &rect);

private:
	XFBSourceBase* CreateXFBSource(unsigned int target_width, unsigned int target_height);
	void GetTargetSize(unsigned int *width, unsigned int *height, const EFBRectangle& sourceRc);

	void CopyToRealXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc,float Gamma);

	static int m_targetWidth;
	static int m_targetHeight;
	static int m_msaaSamples;
	static int m_msaaCoverageSamples;

	static GLuint m_efbFramebuffer;
	static GLuint m_efbColor; // Renderbuffer in MSAA mode; Texture otherwise
	static GLuint m_efbDepth; // Renderbuffer in MSAA mode; Texture otherwise

	// Only used in MSAA mode.
	static GLuint m_resolvedFramebuffer;
	static GLuint m_resolvedColorTexture;
	static GLuint m_resolvedDepthTexture;

	static GLuint m_xfbFramebuffer; // Only used in MSAA mode
};

#endif
