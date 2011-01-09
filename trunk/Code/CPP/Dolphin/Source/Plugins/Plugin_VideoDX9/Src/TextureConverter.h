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

#ifndef _TEXTURECONVERTER_H_
#define _TEXTURECONVERTER_H_

#include "VideoCommon.h"
#include "D3DBase.h"
#include "D3DTexture.h"
#include "D3DUtil.h"
#include "D3DShader.h"

// Converts textures between formats
// TODO: support multiple texture formats
namespace TextureConverter
{

void Init();
void Shutdown();

void EncodeToRam(u32 address, bool bFromZBuffer, bool bIsIntensityFmt,
				 u32 copyfmt, int bScaleByHalf, const EFBRectangle& source);

void EncodeToRamYUYV(LPDIRECT3DTEXTURE9 srcTexture, const TargetRectangle& sourceRc,
					 u8* destAddr, int dstWidth, int dstHeight,float Gamma);

void DecodeToTexture(u32 xfbAddr, int srcWidth, int srcHeight, LPDIRECT3DTEXTURE9 destTexture);

u64 EncodeToRamFromTexture(u32 address,LPDIRECT3DTEXTURE9 source_texture, u32 SourceW, u32 SourceH, bool bFromZBuffer, bool bIsIntensityFmt, u32 copyfmt, int bScaleByHalf, const EFBRectangle& source);


}

#endif // _TEXTURECONVERTER_H_
