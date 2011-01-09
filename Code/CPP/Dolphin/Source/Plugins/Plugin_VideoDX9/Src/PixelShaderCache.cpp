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

#include <map>
#include <set>

#include "Common.h"
#include "Hash.h"
#include "FileUtil.h"
#include "LinearDiskCache.h"

#include "Globals.h"
#include "D3DBase.h"
#include "D3DShader.h"
#include "Statistics.h"
#include "VideoConfig.h"
#include "PixelShaderGen.h"
#include "PixelShaderManager.h"
#include "PixelShaderCache.h"
#include "VertexLoader.h"
#include "BPMemory.h"
#include "XFMemory.h"
#include "ImageWrite.h"
#include "Debugger.h"

PixelShaderCache::PSCache PixelShaderCache::PixelShaders;
const PixelShaderCache::PSCacheEntry *PixelShaderCache::last_entry;

static LinearDiskCache<PIXELSHADERUID, u8> g_ps_disk_cache;
static std::set<u32> unique_shaders;

#define MAX_SSAA_SHADERS 3
enum
{
	COPY_TYPE_DIRECT,
	COPY_TYPE_MATRIXCOLOR,
	NUM_COPY_TYPES
};
enum
{
	DEPTH_CONVERSION_TYPE_NONE,
	DEPTH_CONVERSION_TYPE_ON,
	NUM_DEPTH_CONVERSION_TYPES
};

static LPDIRECT3DPIXELSHADER9 s_CopyProgram[NUM_COPY_TYPES][NUM_DEPTH_CONVERSION_TYPES][MAX_SSAA_SHADERS];
static LPDIRECT3DPIXELSHADER9 s_ClearProgram = NULL;
static LPDIRECT3DPIXELSHADER9 s_rgba6_to_rgb8 = NULL;
static LPDIRECT3DPIXELSHADER9 s_rgb8_to_rgba6 = NULL;

LPDIRECT3DPIXELSHADER9 PixelShaderCache::GetColorMatrixProgram(int SSAAMode)
{
	return s_CopyProgram[COPY_TYPE_MATRIXCOLOR][DEPTH_CONVERSION_TYPE_NONE][SSAAMode % MAX_SSAA_SHADERS];
}

LPDIRECT3DPIXELSHADER9 PixelShaderCache::GetDepthMatrixProgram(int SSAAMode, bool depthConversion)
{
	return s_CopyProgram[COPY_TYPE_MATRIXCOLOR][depthConversion ? DEPTH_CONVERSION_TYPE_ON : DEPTH_CONVERSION_TYPE_NONE][SSAAMode % MAX_SSAA_SHADERS];
}

LPDIRECT3DPIXELSHADER9 PixelShaderCache::GetColorCopyProgram(int SSAAMode)
{
	return s_CopyProgram[COPY_TYPE_DIRECT][DEPTH_CONVERSION_TYPE_NONE][SSAAMode % MAX_SSAA_SHADERS];
}

LPDIRECT3DPIXELSHADER9 PixelShaderCache::GetClearProgram()
{
	return s_ClearProgram;
}

static LPDIRECT3DPIXELSHADER9 s_rgb8 = NULL;
static LPDIRECT3DPIXELSHADER9 s_rgba6 = NULL;

LPDIRECT3DPIXELSHADER9 PixelShaderCache::ReinterpRGBA6ToRGB8()
{
	const char code[] =
	{
		"uniform sampler samp0 : register(s0);\n"
		"void main(\n"
		"			out float4 ocol0 : COLOR0,\n"
		"			in float2 uv0 : TEXCOORD0){\n"
		"	ocol0 = tex2D(samp0,uv0);\n"
		"	float4 src6 = trunc(ocol0 * 63.f);\n"
		"	ocol0.r = src6.r*4.f + trunc(src6.g/16.f);\n" // dst8r = (src6r<<2)|(src6g>>4);
		"	ocol0.g = frac(src6.g/16.f)*16.f*16.f+trunc(src6.b/4.f);\n" // dst8g = ((src6g&0xF)<<4)|(src6b>>2);
		"	ocol0.b = frac(src6.b/4.f)*4.f*64.f+src6.a;\n" // dst8b = ((src6b&0x3)<<6)|src6a;
		"	ocol0.a = 255.f;\n"
		"	ocol0 /= 255.f;\n"
		"}\n"
	};
	if (!s_rgba6_to_rgb8) s_rgba6_to_rgb8 = D3D::CompileAndCreatePixelShader(code, (int)strlen(code));
	return s_rgba6_to_rgb8;
}

LPDIRECT3DPIXELSHADER9 PixelShaderCache::ReinterpRGB8ToRGBA6()
{
	const char code[] =
	{
		"uniform sampler samp0 : register(s0);\n"
		"void main(\n"
		"			out float4 ocol0 : COLOR0,\n"
		"			in float2 uv0 : TEXCOORD0){\n"
		"	ocol0 = tex2D(samp0,uv0);\n"
		"	float4 src8 = trunc(ocol0*255.f);\n"
		"	ocol0.r = (src8.r/4.f);\n" // dst6r = src8r>>2;
		"	ocol0.g = frac(src8.r/4.f)*4.f*16.f + (src8.g/16.f);\n" // dst6g = ((src8r&0x3)<<4)|(src8g>>4);
		"	ocol0.b = frac(src8.g/16.f)*16.f*4.f + (src8.b/64.f);\n" // dst6b = ((src8g&0xF)<<2)|(src8b>>6);
		"	ocol0.a = frac(src8.b/64.f)*64.f;\n" // dst6a = src8b&0x3F;
		"	ocol0 /= 63.f;\n"
		"}\n"
	};
	if (!s_rgb8_to_rgba6) s_rgb8_to_rgba6 = D3D::CompileAndCreatePixelShader(code, (int)strlen(code));
	return s_rgb8_to_rgba6;
}

void SetPSConstant4f(unsigned int const_number, float f1, float f2, float f3, float f4)
{
	float f[4] = { f1, f2, f3, f4 };
	D3D::dev->SetPixelShaderConstantF(const_number, f, 1);
}

void SetPSConstant4fv(unsigned int const_number, const float *f)
{
	D3D::dev->SetPixelShaderConstantF(const_number, f, 1);
}

void SetMultiPSConstant4fv(unsigned int const_number, unsigned int count, const float *f)
{
	D3D::dev->SetPixelShaderConstantF(const_number, f, count);
}

class PixelShaderCacheInserter : public LinearDiskCacheReader<PIXELSHADERUID, u8>
{
public:
	void Read(const PIXELSHADERUID &key, const u8 *value, u32 value_size)
	{
		PixelShaderCache::InsertByteCode(key, value, value_size, false);
	}
};

#define WRITE p+=sprintf

static LPDIRECT3DPIXELSHADER9 CreateCopyShader(int copyMatrixType, int depthConversionType, int SSAAMode)
{
	//Used for Copy/resolve the color buffer
	//Color conversion Programs
	//Depth copy programs
	// this should create the same shaders as before (plus some extras added for DF16), just... more manageably than listing the full program for each combination
	char text[3072];

	setlocale(LC_NUMERIC, "C"); // Reset locale for compilation
	text[sizeof(text) - 1] = 0x7C;  // canary

	char* p = text;
	WRITE(p, "// Copy/Color Matrix/Depth Matrix shader (matrix=%d, depth=%d, ssaa=%d)\n", copyMatrixType, depthConversionType, SSAAMode);

	WRITE(p, "uniform sampler samp0 : register(s0);\n");
	if(copyMatrixType == COPY_TYPE_MATRIXCOLOR)
		WRITE(p, "uniform float4 cColMatrix[7] : register(c%d);\n", C_COLORMATRIX);
	WRITE(p, "void main(\n"
	         "out float4 ocol0 : COLOR0,\n");

	switch(SSAAMode % MAX_SSAA_SHADERS)
	{
	case 0: // 1 Sample
		WRITE(p, "in float2 uv0 : TEXCOORD0,\n"
		         "in float uv1 : TEXCOORD1){\n"
		         "float4 texcol = tex2D(samp0,uv0.xy);\n");
		break;
	case 1: // 1 Samples SSAA
		WRITE(p, "in float4 uv0 : TEXCOORD0,\n"
		         "in float uv1 : TEXCOORD1){\n"
		         "float4 texcol = tex2D(samp0,uv0.xy);\n");
		break;
	case 2: // 4 Samples SSAA
		WRITE(p, "in float4 uv0 : TEXCOORD0,\n"
		         "in float uv1 : TEXCOORD1,\n"
		         "in float4 uv2 : TEXCOORD2,\n"
		         "in float4 uv3 : TEXCOORD3){\n"
		         "float4 texcol = (tex2D(samp0,uv2.xy) + tex2D(samp0,uv2.wz) + tex2D(samp0,uv3.xy) + tex2D(samp0,uv3.wz))*0.25f;\n");
		break;
	}

	if(depthConversionType != DEPTH_CONVERSION_TYPE_NONE)
	{
		WRITE(p, "float4 EncodedDepth = frac((texcol.r * (16777215.0f/16777216.0f)) * float4(1.0f,256.0f,256.0f*256.0f,1.0f));\n"
		         "texcol = round(EncodedDepth * (16777216.0f/16777215.0f) * float4(255.0f,255.0f,255.0f,15.0f)) / float4(255.0f,255.0f,255.0f,15.0f);\n");
	}
	else
	{
		//Apply Gamma Correction
		WRITE(p, "texcol = pow(texcol,uv1.xxxx);\n");		
	}

	if(copyMatrixType == COPY_TYPE_MATRIXCOLOR)
	{
		if(depthConversionType == DEPTH_CONVERSION_TYPE_NONE)
			WRITE(p, "texcol = round(texcol * cColMatrix[5])*cColMatrix[6];\n");		

		WRITE(p, "ocol0 = float4(dot(texcol,cColMatrix[0]),dot(texcol,cColMatrix[1]),dot(texcol,cColMatrix[2]),dot(texcol,cColMatrix[3])) + cColMatrix[4];\n");		
	}
	else
		WRITE(p, "ocol0 = texcol;\n");

	WRITE(p, "}\n");
	if (text[sizeof(text) - 1] != 0x7C)
		PanicAlert("PixelShaderCache copy shader generator - buffer too small, canary has been eaten!");

	setlocale(LC_NUMERIC, ""); // restore locale
	return D3D::CompileAndCreatePixelShader(text, (int)strlen(text));	
}

void PixelShaderCache::Init()
{
	//program used for clear screen
	{
		char pprog[3072];
		sprintf(pprog, "void main(\n"
							"out float4 ocol0 : COLOR0,\n"
							" in float4 incol0 : COLOR0){\n"
							"ocol0 = incol0;\n"
							"}\n");
		s_ClearProgram = D3D::CompileAndCreatePixelShader(pprog, (int)strlen(pprog));	
	}

	int shaderModel = ((D3D::GetCaps().PixelShaderVersion >> 8) & 0xFF);
	int maxConstants = (shaderModel < 3) ? 32 : ((shaderModel < 4) ? 224 : 65536);	

	// other screen copy/convert programs
	for(int copyMatrixType = 0; copyMatrixType < NUM_COPY_TYPES; copyMatrixType++)
	{
		for(int depthType = 0; depthType < NUM_DEPTH_CONVERSION_TYPES; depthType++)
		{
			for(int ssaaMode = 0; ssaaMode < MAX_SSAA_SHADERS; ssaaMode++)
			{
				if(ssaaMode && !s_CopyProgram[copyMatrixType][depthType][ssaaMode-1]
				|| depthType && !s_CopyProgram[copyMatrixType][depthType-1][ssaaMode]
				|| copyMatrixType && !s_CopyProgram[copyMatrixType-1][depthType][ssaaMode])
				{
					// if it failed at a lower setting, it's going to fail here for the same reason it did there,
					// so skip this attempt to avoid duplicate error messages.
					s_CopyProgram[copyMatrixType][depthType][ssaaMode] = NULL;
				}
				else
				{
					s_CopyProgram[copyMatrixType][depthType][ssaaMode] = CreateCopyShader(copyMatrixType, depthType, ssaaMode);
				}
			}
		}
	}

	Clear();

	if (!File::Exists(File::GetUserPath(D_SHADERCACHE_IDX)))
		File::CreateDir(File::GetUserPath(D_SHADERCACHE_IDX));

	SETSTAT(stats.numPixelShadersCreated, 0);
	SETSTAT(stats.numPixelShadersAlive, 0);

	char cache_filename[MAX_PATH];
	sprintf(cache_filename, "%sdx9-%s-ps.cache", File::GetUserPath(D_SHADERCACHE_IDX), globals->unique_id);
	PixelShaderCacheInserter inserter;
	g_ps_disk_cache.OpenAndRead(cache_filename, inserter);
}

// ONLY to be used during shutdown.
void PixelShaderCache::Clear()
{
	for (PSCache::iterator iter = PixelShaders.begin(); iter != PixelShaders.end(); iter++)
		iter->second.Destroy();
	PixelShaders.clear(); 

	memset(&last_pixel_shader_uid, 0xFF, sizeof(last_pixel_shader_uid));
}

void PixelShaderCache::Shutdown()
{
	for(int copyMatrixType = 0; copyMatrixType < NUM_COPY_TYPES; copyMatrixType++)
		for(int depthType = 0; depthType < NUM_DEPTH_CONVERSION_TYPES; depthType++)
			for(int ssaaMode = 0; ssaaMode < MAX_SSAA_SHADERS; ssaaMode++)
				if(s_CopyProgram[copyMatrixType][depthType][ssaaMode]
					&& (copyMatrixType == 0 || s_CopyProgram[copyMatrixType][depthType][ssaaMode] != s_CopyProgram[copyMatrixType-1][depthType][ssaaMode]))
					s_CopyProgram[copyMatrixType][depthType][ssaaMode]->Release();

	for(int copyMatrixType = 0; copyMatrixType < NUM_COPY_TYPES; copyMatrixType++)
		for(int depthType = 0; depthType < NUM_DEPTH_CONVERSION_TYPES; depthType++)
			for(int ssaaMode = 0; ssaaMode < MAX_SSAA_SHADERS; ssaaMode++)
				s_CopyProgram[copyMatrixType][depthType][ssaaMode] = NULL;

	if (s_ClearProgram) s_ClearProgram->Release();
	s_ClearProgram = NULL;
	if (s_rgb8_to_rgba6) s_rgb8_to_rgba6->Release();
	s_rgb8_to_rgba6 = NULL;
	if (s_rgba6_to_rgb8) s_rgba6_to_rgb8->Release();
	s_rgba6_to_rgb8 = NULL;

	
	Clear();
	g_ps_disk_cache.Sync();
	g_ps_disk_cache.Close();

	unique_shaders.clear();
}

bool PixelShaderCache::SetShader(DSTALPHA_MODE dstAlphaMode, u32 components)
{
	PIXELSHADERUID uid;
	GetPixelShaderId(&uid, dstAlphaMode);

	// Check if the shader is already set
	if (uid == last_pixel_shader_uid && PixelShaders[uid].frameCount == frameCount)
	{
		PSCache::const_iterator iter = PixelShaders.find(uid);
		GFX_DEBUGGER_PAUSE_AT(NEXT_PIXEL_SHADER_CHANGE, true);
		return (iter != PixelShaders.end() && iter->second.shader);
	}

	memcpy(&last_pixel_shader_uid, &uid, sizeof(PIXELSHADERUID));

	// Check if the shader is already in the cache
	PSCache::iterator iter;
	iter = PixelShaders.find(uid);
	if (iter != PixelShaders.end())
	{
		iter->second.frameCount = frameCount;
		const PSCacheEntry &entry = iter->second;
		last_entry = &entry;
		
		if (entry.shader) D3D::SetPixelShader(entry.shader);
		GFX_DEBUGGER_PAUSE_AT(NEXT_PIXEL_SHADER_CHANGE, true);
		return (entry.shader != NULL);
	}

	int shaderModel = ((D3D::GetCaps().PixelShaderVersion >> 8) & 0xFF);
	int maxConstants = (shaderModel < 3) ? 32 : ((shaderModel < 4) ? 224 : 65536);

	// Need to compile a new shader
	const char *code = GeneratePixelShaderCode(dstAlphaMode, API_D3D9, maxConstants, components);

	u32 code_hash = HashAdler32((const u8 *)code, strlen(code));
	unique_shaders.insert(code_hash);
	SETSTAT(stats.numUniquePixelShaders, unique_shaders.size());

#if defined(_DEBUG) || defined(DEBUGFAST)
	if (g_ActiveConfig.iLog & CONF_SAVESHADERS && code) {	
		static int counter = 0;
		char szTemp[MAX_PATH];
		sprintf(szTemp, "%sps_%04i.txt", File::GetUserPath(D_DUMP_IDX), counter++);
		
		SaveData(szTemp, code);
	}
#endif

	u8 *bytecode = 0;
	int bytecodelen = 0;
	if (!D3D::CompilePixelShader(code, (int)strlen(code), &bytecode, &bytecodelen)) {
		if (g_ActiveConfig.bShowShaderErrors)
		{
			PanicAlert("Failed to compile Pixel Shader:\n\n%s", code);
			static int counter = 0;
			char szTemp[MAX_PATH];
			sprintf(szTemp, "%sBADps_%04i.txt", File::GetUserPath(D_DUMP_IDX), counter++);			
			SaveData(szTemp, code);
		}
		GFX_DEBUGGER_PAUSE_AT(NEXT_ERROR, true);
		return false;
	}

	// Insert the bytecode into the caches
	g_ps_disk_cache.Append(uid, bytecode, bytecodelen);
	g_ps_disk_cache.Sync();

	// And insert it into the shader cache.
	bool result = InsertByteCode(uid, bytecode, bytecodelen, true);
	delete [] bytecode;

	GFX_DEBUGGER_PAUSE_AT(NEXT_PIXEL_SHADER_CHANGE, true);
	return result;
}

bool PixelShaderCache::InsertByteCode(const PIXELSHADERUID &uid, const u8 *bytecode, int bytecodelen, bool activate)
{
	LPDIRECT3DPIXELSHADER9 shader = D3D::CreatePixelShaderFromByteCode(bytecode, bytecodelen);

	// Make an entry in the table
	PSCacheEntry newentry;
	newentry.shader = shader;
	newentry.frameCount = frameCount;
	PixelShaders[uid] = newentry;
	last_entry = &PixelShaders[uid];

	if (!shader) {
		// INCSTAT(stats.numPixelShadersFailed);
		return false;
	}

	INCSTAT(stats.numPixelShadersCreated);
	SETSTAT(stats.numPixelShadersAlive, PixelShaders.size());
	if (activate)
	{
		D3D::SetPixelShader(shader);
	}
	return true;
}
