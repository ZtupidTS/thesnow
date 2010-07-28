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

#ifndef _TEXTURECACHE_H
#define _TEXTURECACHE_H


#include <map>

#include "D3DBase.h"
#include "VideoCommon.h"
#include "BPMemory.h"

class TextureCache
{
public:
	struct TCacheEntry
	{
		LPDIRECT3DTEXTURE9 texture;

		u32 addr;
		u32 size_in_bytes;
		u64 hash;
		u32 paletteHash;
		u32 oldpixel;
		
		int frameCount;
		int w, h, fmt,MipLevels;
		int Scaledw, Scaledh;

		float scaleX, scaleY; // Hires texutres need this

		bool isRenderTarget;
		bool isDynamic;// mofified from cpu
		bool isNonPow2;

		TCacheEntry()
		{
			texture = 0;
			isRenderTarget = 0;
			hash = 0;
			paletteHash = 0;
			oldpixel = 0;
			addr = 0;
			size_in_bytes = 0;
			frameCount = 0;
			scaleX = 1.0f;
			scaleY = 1.0f;
			isNonPow2 = true;
			w = 0;
			h = 0;
			Scaledw = 0;
			Scaledh = 0;
		}
		void Destroy(bool shutdown);
		int IntersectsMemoryRange(u32 range_address, u32 range_size);
	};

private:

	typedef std::map<u32,TCacheEntry> TexCache;

	static u8 *temp;
	static TexCache textures;

public:
	static void Init();
	static void Cleanup();
	static void Shutdown();
	static void Invalidate(bool shutdown);
	static void InvalidateRange(u32 start_address, u32 size);
	static void MakeRangeDynamic(u32 start_address, u32 size);
	static TCacheEntry *Load(int stage, u32 address, int width, int height, int format, int tlutaddr, int tlutfmt,bool UseNativeMips, int maxlevel);
	static void CopyRenderTargetToTexture(u32 address, bool bFromZBuffer, bool bIsIntensityFmt, u32 copyfmt, int bScaleByHalf, const EFBRectangle &source_rect);
};

#endif
