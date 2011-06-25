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

#include "VertexManager.h"

#include "BPStructs.h"
#include "Common.h"
#include "D3DBase.h"
#include "Debugger.h"
#include "Fifo.h"
#include "FileUtil.h"
#include "main.h"
#include "NativeVertexFormat.h"
#include "OpcodeDecoding.h"
#include "IndexGenerator.h"
#include "PixelShaderManager.h"
#include "PixelShaderCache.h"
#include "Statistics.h"
#include "TextureCache.h"
#include "Tmem.h"
#include "VertexShaderManager.h"
#include "VertexShaderCache.h"
#include "VideoConfig.h"
#include "XFStructs.h"


// internal state for loading vertices
extern NativeVertexFormat *g_nativeVertexFmt;

namespace DX9
{

inline void DumpBadShaders()
{
#if defined(_DEBUG) || defined(DEBUGFAST)
	// TODO: Reimplement!
/*	std::string error_shaders;
	error_shaders.append(VertexShaderCache::GetCurrentShaderCode());
	error_shaders.append(PixelShaderCache::GetCurrentShaderCode());
	char filename[512] = "bad_shader_combo_0.txt";
	int which = 0;
	while (File::Exists(filename))
	{
		which++;
		sprintf(filename, "bad_shader_combo_%i.txt", which);
	}
	File::WriteStringToFile(true, error_shaders, filename);
	PanicAlert("DrawIndexedPrimitiveUP failed. Shaders written to %s", filename);*/
#endif
}

void VertexManager::Draw(int stride)
{
	if (IndexGenerator::GetNumTriangles() > 0)
	{
		if (FAILED(D3D::dev->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST, 
			0, IndexGenerator::GetNumVerts(), IndexGenerator::GetNumTriangles(), 
			TIBuffer, 
			D3DFMT_INDEX16, 
			LocalVBuffer, 
			stride)))
		{
			DumpBadShaders();
		}
		INCSTAT(stats.thisFrame.numIndexedDrawCalls);
	}
	if (IndexGenerator::GetNumLines() > 0)
	{
		if (FAILED(D3D::dev->DrawIndexedPrimitiveUP(
			D3DPT_LINELIST, 
			0, IndexGenerator::GetNumVerts(), IndexGenerator::GetNumLines(), 
			LIBuffer, 
			D3DFMT_INDEX16, 
			LocalVBuffer, 
			stride)))
		{
			DumpBadShaders();
		}
		INCSTAT(stats.thisFrame.numIndexedDrawCalls);
	}
	if (IndexGenerator::GetNumPoints() > 0)
	{
		if (FAILED(D3D::dev->DrawIndexedPrimitiveUP(
			D3DPT_POINTLIST, 
			0, IndexGenerator::GetNumVerts(), IndexGenerator::GetNumPoints(), 
			PIBuffer, 
			D3DFMT_INDEX16, 
			LocalVBuffer, 
			stride)))
		{
			DumpBadShaders();
		}
		INCSTAT(stats.thisFrame.numIndexedDrawCalls);
	}
}

void VertexManager::vFlush()
{
	if (LocalVBuffer == s_pCurBufferPointer) return;
	if (Flushed) return;
	Flushed = true;
	VideoFifo_CheckEFBAccess();

	u32 usedtextures = 0;
	for (u32 i = 0; i < (u32)bpmem.genMode.numtevstages + 1; ++i)
		if (bpmem.tevorders[i / 2].getEnable(i & 1))
			usedtextures |= 1 << bpmem.tevorders[i/2].getTexMap(i & 1);

	if (bpmem.genMode.numindstages > 0)
		for (unsigned int i = 0; i < bpmem.genMode.numtevstages + 1; ++i)
			if (bpmem.tevind[i].IsActive() && bpmem.tevind[i].bt < bpmem.genMode.numindstages)
				usedtextures |= 1 << bpmem.tevindref.getTexMap(bpmem.tevind[i].bt);

	LPDIRECT3DTEXTURE9 bindThese[8] = { 0 };
	for (unsigned int i = 0; i < 8; i++)
	{
		if (usedtextures & (1 << i))
		{
			const FourTexUnits &tex = bpmem.tex[i >> 2];

			u32 ramAddr = tex.texImage3[i&3].image_base << 5;
			u32 width = tex.texImage0[i&3].width+1;
			u32 height = tex.texImage0[i&3].height+1;
			u32 levels = (tex.texMode1[i&3].max_lod >> 4) + 1;
			u32 format = tex.texImage0[i&3].format;
			u32 tlutAddr = (tex.texTlut[i&3].tmem_offset << 9) + TMEM_HALF;
			u32 tlutFormat = tex.texTlut[i&3].tlut_format;

			TCacheEntry* entry = (TCacheEntry*)g_textureCache->LoadEntry(
				ramAddr, width, height, levels, format, tlutAddr, tlutFormat);

			if (entry)
				bindThese[i] = entry->GetTexture();
			else
				ERROR_LOG(VIDEO, "Error loading texture from 0x%.08X", ramAddr);
		}
	}

	// Bind and set samplers down here, because TextureCache::LoadEntry may
	// clobber the render state.
	for (int i = 0; i < 8; ++i)
	{
		if (usedtextures & (1 << i))
		{
			g_renderer->SetSamplerState(i & 3, i >> 2);
			const FourTexUnits &tex = bpmem.tex[i >> 2];
		
			u32 width = tex.texImage0[i&3].width+1;
			u32 height = tex.texImage0[i&3].height+1;
			PixelShaderManager::SetTexDims(i, width, height, 0, 0);

			D3D::SetTexture(i, bindThese[i]);
		}
		else
			D3D::SetTexture(i, NULL);
	}

	// set global constants
	VertexShaderManager::SetConstants();
	PixelShaderManager::SetConstants();

	if (!PixelShaderCache::SetShader(DSTALPHA_NONE,g_nativeVertexFmt->m_components))
	{
		GFX_DEBUGGER_PAUSE_LOG_AT(NEXT_ERROR,true,{printf("Fail to set pixel shader\n");});
		goto shader_fail;
	}
	if (!VertexShaderCache::SetShader(g_nativeVertexFmt->m_components))
	{
		GFX_DEBUGGER_PAUSE_LOG_AT(NEXT_ERROR,true,{printf("Fail to set vertex shader\n");});
		goto shader_fail;

	}

	int stride = g_nativeVertexFmt->GetVertexStride();
	g_nativeVertexFmt->SetupVertexPointers();

	Draw(stride);

	bool useDstAlpha = !g_ActiveConfig.bDstAlphaPass && bpmem.dstalpha.enable && bpmem.blendmode.alphaupdate &&
						bpmem.zcontrol.pixel_format == PIXELFMT_RGBA6_Z24;
	if (useDstAlpha)
	{
		DWORD write = 0;
		if (!PixelShaderCache::SetShader(DSTALPHA_ALPHA_PASS, g_nativeVertexFmt->m_components))
		{
			GFX_DEBUGGER_PAUSE_LOG_AT(NEXT_ERROR,true,{printf("Fail to set pixel shader\n");});
			goto shader_fail;
		}
		// update alpha only
		g_renderer->ApplyState(true);
		Draw(stride);
		g_renderer->RestoreState();
	}
	GFX_DEBUGGER_PAUSE_AT(NEXT_FLUSH, true);

shader_fail:
	ResetBuffer();
}

}
