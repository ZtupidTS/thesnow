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

#include "Common.h"
#include "FileUtil.h"
#include "LinearDiskCache.h"

#include "Globals.h"
#include "D3DBase.h"
#include "D3DShader.h"
#include "Statistics.h"
#include "VideoConfig.h"
#include "VertexShaderCache.h"
#include "VertexLoader.h"
#include "BPMemory.h"
#include "XFMemory.h"
#include "Debugger.h"

VertexShaderCache::VSCache VertexShaderCache::vshaders;
const VertexShaderCache::VSCacheEntry *VertexShaderCache::last_entry;

#define MAX_SSAA_SHADERS 3

static LPDIRECT3DVERTEXSHADER9 SimpleVertexShader[MAX_SSAA_SHADERS];
static LPDIRECT3DVERTEXSHADER9 ClearVertexShader;

LinearDiskCache<VERTEXSHADERUID, u8> g_vs_disk_cache;

LPDIRECT3DVERTEXSHADER9 VertexShaderCache::GetSimpleVertexShader(int level)
{
	return SimpleVertexShader[level % MAX_SSAA_SHADERS];
}

LPDIRECT3DVERTEXSHADER9 VertexShaderCache::GetClearVertexShader()
{
	return ClearVertexShader;
}

void SetVSConstant4f(unsigned int const_number, float f1, float f2, float f3, float f4)
{
	const float f[4] = { f1, f2, f3, f4 };
	D3D::dev->SetVertexShaderConstantF(const_number, f, 1);
}

void SetVSConstant4fv(unsigned int const_number, const float *f)
{
	D3D::dev->SetVertexShaderConstantF(const_number, f, 1);
}

void SetMultiVSConstant3fv(unsigned int const_number, unsigned int count, const float *f)
{
	float buf[4*C_VENVCONST_END];
	for (unsigned int i = 0; i < count; i++)
	{
		buf[4*i  ] = *f++;
		buf[4*i+1] = *f++;
		buf[4*i+2] = *f++;
		buf[4*i+3] = 0.f;
	}
	D3D::dev->SetVertexShaderConstantF(const_number, buf, count);
}

void SetMultiVSConstant4fv(unsigned int const_number, unsigned int count, const float *f)
{
	D3D::dev->SetVertexShaderConstantF(const_number, f, count);
}

// this class will load the precompiled shaders into our cache
class VertexShaderCacheInserter : public LinearDiskCacheReader<VERTEXSHADERUID, u8>
{
public:
	void Read(const VERTEXSHADERUID &key, const u8 *value, u32 value_size)
	{
		VertexShaderCache::InsertByteCode(key, value, value_size, false);
	}
};

void VertexShaderCache::Init()
{
	char* vProg = new char[2048];
	sprintf(vProg,"struct VSOUTPUT\n"
						"{\n"
						   "float4 vPosition : POSITION;\n"
						   "float2 vTexCoord : TEXCOORD0;\n"
						   "float vTexCoord1 : TEXCOORD1;\n"
						"};\n"
						"VSOUTPUT main(float4 inPosition : POSITION,float2 inTEX0 : TEXCOORD0,float2 inTEX1 : TEXCOORD1,float inTEX2 : TEXCOORD2)\n"
						"{\n"
							"VSOUTPUT OUT;\n"
							"OUT.vPosition = inPosition;\n"
							"OUT.vTexCoord = inTEX0;\n"
							"OUT.vTexCoord1 = inTEX2;\n"
							"return OUT;\n"
						"}\n");

	SimpleVertexShader[0] = D3D::CompileAndCreateVertexShader(vProg, (int)strlen(vProg));

	sprintf(vProg,"struct VSOUTPUT\n"
						"{\n"
						   "float4 vPosition   : POSITION;\n"
						   "float4 vColor0   : COLOR0;\n"						   
						"};\n"
						"VSOUTPUT main(float4 inPosition : POSITION,float4 inColor0: COLOR0)\n"
						"{\n"
							"VSOUTPUT OUT;\n"
							"OUT.vPosition = inPosition;\n"
							"OUT.vColor0 = inColor0;\n"
							"return OUT;\n"
						"}\n");

	ClearVertexShader = D3D::CompileAndCreateVertexShader(vProg, (int)strlen(vProg));
	
	sprintf(vProg,	"struct VSOUTPUT\n"
						"{\n"
						   "float4 vPosition   : POSITION;\n"
						   "float4 vTexCoord   : TEXCOORD0;\n"
						   "float vTexCoord1   : TEXCOORD1;\n"						   
						"};\n"
						"VSOUTPUT main(float4 inPosition : POSITION,float2 inTEX0 : TEXCOORD0,float2 inTEX1 : TEXCOORD1,float inTEX2 : TEXCOORD2)\n"
						"{\n"
						   "VSOUTPUT OUT;"
						   "OUT.vPosition = inPosition;\n"
						   "OUT.vTexCoord  = inTEX0.xyyx;\n"
						   "OUT.vTexCoord1 = inTEX2;\n"
						   "return OUT;\n"
						"}\n");
	SimpleVertexShader[1] = D3D::CompileAndCreateVertexShader(vProg, (int)strlen(vProg));	

	sprintf(vProg,	"struct VSOUTPUT\n"
						"{\n"
						   "float4 vPosition   : POSITION;\n"
						   "float4 vTexCoord   : TEXCOORD0;\n"
						   "float  vTexCoord1   : TEXCOORD1;\n"
						   "float4 vTexCoord2   : TEXCOORD2;\n"   
						   "float4 vTexCoord3   : TEXCOORD3;\n"						   
						"};\n"
						"VSOUTPUT main(float4 inPosition : POSITION,float2 inTEX0 : TEXCOORD0,float2 inTEX1 : TEXCOORD1,float inTEX2 : TEXCOORD2)\n"
						"{\n"
						   "VSOUTPUT OUT;"
						   "OUT.vPosition = inPosition;\n"
						   "OUT.vTexCoord  = inTEX0.xyyx;\n"
						   "OUT.vTexCoord1 = inTEX2.x;\n"
						   "OUT.vTexCoord2 = inTEX0.xyyx + (float4(-1.0f,-0.5f, 1.0f,-0.5f) * inTEX1.xyyx);\n"
						   "OUT.vTexCoord3 = inTEX0.xyyx + (float4( 1.0f, 0.5f,-1.0f, 0.5f) * inTEX1.xyyx);\n"						   
						   "return OUT;\n"
						"}\n");
	SimpleVertexShader[2] = D3D::CompileAndCreateVertexShader(vProg, (int)strlen(vProg));	
	
	Clear();
	delete [] vProg;

	if (!File::Exists(File::GetUserPath(D_SHADERCACHE_IDX)))
		File::CreateDir(File::GetUserPath(D_SHADERCACHE_IDX));

	SETSTAT(stats.numVertexShadersCreated, 0);
	SETSTAT(stats.numVertexShadersAlive, 0);

	char cache_filename[MAX_PATH];
	sprintf(cache_filename, "%sdx9-%s-vs.cache", File::GetUserPath(D_SHADERCACHE_IDX), globals->unique_id);
	VertexShaderCacheInserter inserter;
	g_vs_disk_cache.OpenAndRead(cache_filename, inserter);
}

void VertexShaderCache::Clear()
{
	for (VSCache::iterator iter = vshaders.begin(); iter != vshaders.end(); ++iter)
		iter->second.Destroy();
	vshaders.clear();

	memset(&last_vertex_shader_uid, 0xFF, sizeof(last_vertex_shader_uid));
}

void VertexShaderCache::Shutdown()
{
	for (int i = 0; i < MAX_SSAA_SHADERS; i++)
	{
		if (SimpleVertexShader[i])
			SimpleVertexShader[i]->Release();
		SimpleVertexShader[i] = NULL;
	}

	if (ClearVertexShader)
		ClearVertexShader->Release();
	ClearVertexShader = NULL;
	
	Clear();
	g_vs_disk_cache.Sync();
	g_vs_disk_cache.Close();
}

bool VertexShaderCache::SetShader(u32 components)
{
	VERTEXSHADERUID uid;
	GetVertexShaderId(&uid, components);
	if (uid == last_vertex_shader_uid && vshaders[uid].frameCount == frameCount)
	{
		GFX_DEBUGGER_PAUSE_AT(NEXT_VERTEX_SHADER_CHANGE, true);
		return (vshaders[uid].shader != NULL);
	}

	memcpy(&last_vertex_shader_uid, &uid, sizeof(VERTEXSHADERUID));

	VSCache::iterator iter = vshaders.find(uid);
	if (iter != vshaders.end())
	{
		iter->second.frameCount = frameCount;
		const VSCacheEntry &entry = iter->second;
		last_entry = &entry;

		if (entry.shader) D3D::SetVertexShader(entry.shader);
		GFX_DEBUGGER_PAUSE_AT(NEXT_VERTEX_SHADER_CHANGE, true);
		return (entry.shader != NULL);
	}

	const char *code = GenerateVertexShaderCode(components, API_D3D9);
	u8 *bytecode;
	int bytecodelen;
	if (!D3D::CompileVertexShader(code, (int)strlen(code), &bytecode, &bytecodelen))
	{
		if (g_ActiveConfig.bShowShaderErrors)
		{
			PanicAlert("Failed to compile Vertex Shader:\n\n%s", code);
		}
		GFX_DEBUGGER_PAUSE_AT(NEXT_ERROR, true);
		return false;
	}
	g_vs_disk_cache.Append(uid, bytecode, bytecodelen);
	g_vs_disk_cache.Sync();

	bool result = InsertByteCode(uid, bytecode, bytecodelen, true);
	delete [] bytecode;
	GFX_DEBUGGER_PAUSE_AT(NEXT_VERTEX_SHADER_CHANGE, true);
	return result;
}

bool VertexShaderCache::InsertByteCode(const VERTEXSHADERUID &uid, const u8 *bytecode, int bytecodelen, bool activate) {
	LPDIRECT3DVERTEXSHADER9 shader = D3D::CreateVertexShaderFromByteCode(bytecode, bytecodelen);

	// Make an entry in the table
	VSCacheEntry entry;
	entry.shader = shader;
	entry.frameCount = frameCount;

	vshaders[uid] = entry;
	last_entry = &vshaders[uid];
	if (!shader)
		return false;

	INCSTAT(stats.numVertexShadersCreated);
	SETSTAT(stats.numVertexShadersAlive, (int)vshaders.size());
	if (activate)
	{
		D3D::SetVertexShader(shader);
		return true;
	}
	return false;
}
