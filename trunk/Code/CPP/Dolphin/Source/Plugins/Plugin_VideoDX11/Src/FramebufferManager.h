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

#ifndef _FBMANAGER_D3D_H_
#define _FBMANAGER_D3D_H_

#include "d3d11.h"

#include "FramebufferManagerBase.h"

#include "D3DTexture.h"
#include "XFBEncoder.h"

namespace DX11 {

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
	XFBSource(std::unique_ptr<D3DTexture2D>&& _tex)
		: tex(std::move(_tex))
	{}

	void Draw(const MathUtil::Rectangle<float> &sourcerc,
		const MathUtil::Rectangle<float> &drawrc, int width, int height) const;
	void DecodeToTexture(u32 xfbAddr, u32 fbWidth, u32 fbHeight);
	void CopyEFB(float Gamma);

	std::unique_ptr<D3DTexture2D> const tex;
};

class FramebufferManager : public FramebufferManagerBase
{
public:
	FramebufferManager();
	~FramebufferManager();

	static D3DTexture2D* GetEFBColorTexture() { return m_efb.color_tex.get(); }
	static ID3D11Texture2D* GetEFBColorStagingBuffer() { return m_efb.color_staging_buf; }

	static D3DTexture2D* GetEFBDepthTexture() { return m_efb.depth_tex.get(); }
	static D3DTexture2D* GetEFBDepthReadTexture() { return m_efb.depth_read_texture.get(); }
	static ID3D11Texture2D* GetEFBDepthStagingBuffer() { return m_efb.depth_staging_buf; }

	static D3DTexture2D* GetResolvedEFBColorTexture();
	static D3DTexture2D* GetResolvedEFBDepthTexture();

	static D3DTexture2D* GetEFBColorTempTexture() { return m_efb.color_temp_tex.get(); }
	
	static void SwapReinterpretTexture()
	{
		std::swap(m_efb.color_temp_tex, m_efb.color_tex);
	}

private:
	XFBSourceBase* CreateXFBSource(unsigned int target_width, unsigned int target_height);
	void GetTargetSize(unsigned int *width, unsigned int *height, const EFBRectangle& sourceRc);

	void CopyToRealXFB(u32 xfbAddr, u32 fbWidth, u32 fbHeight, const EFBRectangle& sourceRc,float Gamma);

	static struct Efb
	{
		std::unique_ptr<D3DTexture2D> color_tex;
		SharedPtr<ID3D11Texture2D> color_staging_buf;

		std::unique_ptr<D3DTexture2D> depth_tex;
		SharedPtr<ID3D11Texture2D> depth_staging_buf;
		std::unique_ptr<D3DTexture2D> depth_read_texture;

		std::unique_ptr<D3DTexture2D> color_temp_tex;

		std::unique_ptr<D3DTexture2D> resolved_color_tex;
		std::unique_ptr<D3DTexture2D> resolved_depth_tex;
	} m_efb;

	XFBEncoder m_xfbEncoder;
};

}  // namespace DX11

#endif
