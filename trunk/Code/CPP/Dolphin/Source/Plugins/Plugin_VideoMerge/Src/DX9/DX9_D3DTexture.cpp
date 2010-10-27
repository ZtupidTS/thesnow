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

#include "DX9_D3DBase.h"
#include "DX9_D3DTexture.h"

namespace DX9
{

namespace D3D
{

LPDIRECT3DTEXTURE9 CreateTexture2D(const u8* buffer, const int width, const int height, const int pitch, D3DFORMAT fmt, bool swap_r_b, int levels)
{
	u32* pBuffer = (u32*)buffer;
	LPDIRECT3DTEXTURE9 pTexture;

	// crazy bitmagic, sorry :)
	bool isPow2 = !((width&(width-1)) || (height&(height-1)));
	bool bExpand = false;

	if (fmt == D3DFMT_A8P8) {
		fmt = D3DFMT_A8L8;
		bExpand = true;
	}

	HRESULT hr;
	// TODO(ector): Allow mipmaps for non-pow textures on newer cards?
	// TODO(ector): Use the game-specified mipmaps?
	if (levels > 0)
		hr = dev->CreateTexture(width, height, levels, 0, fmt, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		hr = dev->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, fmt, D3DPOOL_MANAGED, &pTexture, NULL);

	if (FAILED(hr))
		return 0;
	int level = 0;
	D3DLOCKED_RECT Lock;
	pTexture->LockRect(level, &Lock, NULL, 0);
	switch (fmt) 
	{
	case D3DFMT_L8:
	case D3DFMT_A8:
	case D3DFMT_A4L4:
		{
			const u8 *pIn = buffer;
			for (int y = 0; y < height; y++)
			{
				u8* pBits = ((u8*)Lock.pBits + (y * Lock.Pitch));
				memcpy(pBits, pIn, width);
				pIn += pitch;
			}
		}
		break;
	case D3DFMT_R5G6B5:
		{
			const u16 *pIn = (u16*)buffer;
			for (int y = 0; y < height; y++)
			{
				u16* pBits = (u16*)((u8*)Lock.pBits + (y * Lock.Pitch));
				memcpy(pBits, pIn, width * 2);
				pIn += pitch;
			}
		}
		break;
	case D3DFMT_A8L8:
		{
			if (bExpand) { // I8
				const u8 *pIn = buffer;
				// TODO(XK): Find a better way that does not involve either unpacking
				//           or downsampling (i.e. A4L4)
				for (int y = 0; y < height; y++)
				{
					u8* pBits = ((u8*)Lock.pBits + (y * Lock.Pitch));
					for(int i = 0; i < width * 2; i += 2) {
						pBits[i] = pIn[i / 2];
						pBits[i + 1] = pIn[i / 2];
					}
					pIn += pitch;
				}
			} else { // IA8
				const u16 *pIn = (u16*)buffer;

				for (int y = 0; y < height; y++)
				{
					u16* pBits = (u16*)((u8*)Lock.pBits + (y * Lock.Pitch));
					memcpy(pBits, pIn, width * 2);
					pIn += pitch;
				}
			}
		}
		break;
	case D3DFMT_A8R8G8B8:
		{
			if(pitch * 4 == Lock.Pitch && !swap_r_b)
			{
				memcpy(Lock.pBits,buffer,Lock.Pitch*height);
			}
			else
			{
				u32* pIn = pBuffer;
				if (!swap_r_b) {
					for (int y = 0; y < height; y++)
					{
						u32 *pBits = (u32*)((u8*)Lock.pBits + (y * Lock.Pitch));
						memcpy(pBits, pIn, width * 4);
						pIn += pitch;
					}
				} else {
					for (int y = 0; y < height; y++)
					{
						u8 *pIn8 = (u8 *)pIn;
						u8 *pBits = (u8 *)((u8*)Lock.pBits + (y * Lock.Pitch));
						for (int x = 0; x < width * 4; x += 4) {
							pBits[x + 0] = pIn8[x + 2];
							pBits[x + 1] = pIn8[x + 1];
							pBits[x + 2] = pIn8[x + 0];
							pBits[x + 3] = pIn8[x + 3];
						}
						pIn += pitch;
					}
				}
			}
		}
		break;
	case D3DFMT_DXT1:
		memcpy(Lock.pBits,buffer,((width+3)/4)*((height+3)/4)*8);
		break;
	default:
		PanicAlert("D3D: Invalid texture format %i", fmt);
	}
	pTexture->UnlockRect(level); 
	return pTexture;
}

LPDIRECT3DTEXTURE9 CreateOnlyTexture2D(const int width, const int height, D3DFORMAT fmt)
{
	LPDIRECT3DTEXTURE9 pTexture;
	// crazy bitmagic, sorry :)
	bool isPow2 = !((width&(width-1)) || (height&(height-1)));
	bool bExpand = false;
	HRESULT hr;
	// TODO(ector): Allow mipmaps for non-pow textures on newer cards?
	// TODO(ector): Use the game-specified mipmaps?
	if (!isPow2)
		hr = dev->CreateTexture(width, height, 1, 0, fmt, D3DPOOL_MANAGED, &pTexture, NULL);
	else
		hr = dev->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, fmt, D3DPOOL_MANAGED, &pTexture, NULL);

	if (FAILED(hr))
		return 0;
	return pTexture;
}

void ReplaceTexture2D(LPDIRECT3DTEXTURE9 pTexture, const u8* buffer, const int width, const int height, const int pitch, D3DFORMAT fmt, bool swap_r_b, int level)
{
	u32* pBuffer = (u32*)buffer;
	D3DLOCKED_RECT Lock;
	pTexture->LockRect(level, &Lock, NULL, 0);
	u32* pIn = pBuffer;

	bool bExpand = false;

	if (fmt == D3DFMT_A8P8) {
		fmt = D3DFMT_A8L8;
		bExpand = true;
	}
	switch (fmt) 
	{
	case D3DFMT_A8R8G8B8:
		if(pitch * 4 == Lock.Pitch && !swap_r_b)
		{
			memcpy(Lock.pBits, pBuffer, Lock.Pitch*height);
		}
		else if (!swap_r_b)
		{
			for (int y = 0; y < height; y++)
			{
				u32 *pBits = (u32*)((u8*)Lock.pBits + (y * Lock.Pitch));
				memcpy(pBits, pIn, width * 4);
				pIn += pitch;
			}
		}
		else
		{
			for (int y = 0; y < height; y++)
			{
				u8 *pIn8 = (u8 *)pIn;
				u8 *pBits = (u8 *)((u8*)Lock.pBits + (y * Lock.Pitch));
				for (int x = 0; x < width * 4; x += 4)
				{
					pBits[x + 0] = pIn8[x + 2];
					pBits[x + 1] = pIn8[x + 1];
					pBits[x + 2] = pIn8[x + 0];
					pBits[x + 3] = pIn8[x + 3];
				}
				pIn += pitch;
			}
		}
		break;
	case D3DFMT_L8:
	case D3DFMT_A8:
	case D3DFMT_A4L4:
		{
			const u8 *pIn = buffer;
			for (int y = 0; y < height; y++)
			{
				u8* pBits = ((u8*)Lock.pBits + (y * Lock.Pitch));
				memcpy(pBits, pIn, width);
				pIn += pitch;
			}
		}
		break;
	case D3DFMT_R5G6B5:
		{
			const u16 *pIn = (u16*)buffer;
			for (int y = 0; y < height; y++)
			{
				u16* pBits = (u16*)((u8*)Lock.pBits + (y * Lock.Pitch));
				memcpy(pBits, pIn, width * 2);
				pIn += pitch;
			}
		}
		break;
	case D3DFMT_A8L8:
		{
			if (bExpand) { // I8
				const u8 *pIn = buffer;
				// TODO(XK): Find a better way that does not involve either unpacking
				//           or downsampling (i.e. A4L4)
				for (int y = 0; y < height; y++)
				{
					u8* pBits = ((u8*)Lock.pBits + (y * Lock.Pitch));
					for(int i = 0; i < width * 2; i += 2) {
						pBits[i] = pIn[i / 2];
						pBits[i + 1] = pIn[i / 2];
					}
					pIn += pitch;
				}
			} else { // IA8
				const u16 *pIn = (u16*)buffer;

				for (int y = 0; y < height; y++)
				{
					u16* pBits = (u16*)((u8*)Lock.pBits + (y * Lock.Pitch));
					memcpy(pBits, pIn, width * 2);
					pIn += pitch;
				}
			}
		}
		break;
	case D3DFMT_DXT1:
		memcpy(Lock.pBits,buffer,((width+3)/4)*((height+3)/4)*8);
		break;
	}
	pTexture->UnlockRect(level); 
}

}  // namespace

}
