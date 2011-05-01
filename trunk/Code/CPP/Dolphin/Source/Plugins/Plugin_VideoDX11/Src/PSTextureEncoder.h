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

#ifndef _PSTEXTUREENCODER_H
#define _PSTEXTUREENCODER_H

#include "TextureEncoder.h"
#include "D3DUtil.h"

namespace DX11
{

class PSTextureEncoder : public TextureEncoder
{
public:
	PSTextureEncoder();
	~PSTextureEncoder();

	size_t Encode(u8* dst, unsigned int dstFormat,
		unsigned int srcFormat, const EFBRectangle& srcRect, bool isIntensity,
		bool scaleByHalf);

private:
	bool m_ready;

	SharedPtr<ID3D11Texture2D> m_out;
	ID3D11RenderTargetView* m_outRTV;
	SharedPtr<ID3D11Texture2D> m_outStage;
	SharedPtr<ID3D11Buffer> m_encodeParams;
	SharedPtr<ID3D11Buffer> m_quad;
	SharedPtr<ID3D11VertexShader> m_vShader;
	SharedPtr<ID3D11InputLayout> m_quadLayout;
	SharedPtr<ID3D11BlendState> m_efbEncodeBlendState;
	ID3D11DepthStencilState* m_efbEncodeDepthState;
	ID3D11RasterizerState* m_efbEncodeRastState;
	ID3D11SamplerState* m_efbSampler;

	// Stuff only used in static-linking mode (SM4.0-compatible)

	bool InitStaticMode();
	bool SetStaticShader(unsigned int dstFormat, unsigned int srcFormat,
		bool isIntensity, bool scaleByHalf);

	typedef unsigned int ComboKey; // Key for a shader combination

	ComboKey MakeComboKey(unsigned int dstFormat, unsigned int srcFormat,
		bool isIntensity, bool scaleByHalf)
	{
		return (dstFormat << 4) | (srcFormat << 2) | (isIntensity ? (1<<1) : 0)
			| (scaleByHalf ? (1<<0) : 0);
	}

	typedef std::map<ComboKey, SharedPtr<ID3D11PixelShader>> ComboMap;

	ComboMap m_staticShaders;

	// Stuff only used for dynamic-linking mode (SM5.0+, available as soon as
	// Microsoft fixes their bloody HLSL compiler)
	
	bool InitDynamicMode();
	bool SetDynamicShader(unsigned int dstFormat, unsigned int srcFormat,
		bool isIntensity, bool scaleByHalf);

	SharedPtr<ID3D11PixelShader> m_dynamicShader;
	ID3D11ClassLinkage* m_classLinkage;

	// Interface slots
	UINT m_fetchSlot;
	UINT m_scaledFetchSlot;
	UINT m_intensitySlot;
	UINT m_generatorSlot;

	// Class instances
	// Fetch: 0 is RGB, 1 is RGBA, 2 is RGB565, 3 is Z
	ID3D11ClassInstance* m_fetchClass[4];
	// ScaledFetch: 0 is off, 1 is on
	ID3D11ClassInstance* m_scaledFetchClass[2];
	// Intensity: 0 is off, 1 is on
	ID3D11ClassInstance* m_intensityClass[2];
	// Generator: one for each dst format, 16 total
	ID3D11ClassInstance* m_generatorClass[16];

	std::vector<ID3D11ClassInstance*> m_linkageArray;
};

}

#endif
