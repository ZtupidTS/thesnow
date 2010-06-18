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

#ifndef _VERTEXSHADERCACHE_H
#define _VERTEXSHADERCACHE_H

#include "D3DBase.h"

#include <map>
#include <string>

#include "D3DBase.h"
#include "VertexShaderGen.h"

#include "d3dcompiler.h"

class VertexShaderCache
{
public:
	static void Init();
	static void Clear();
	static void Shutdown();
	static bool SetShader(u32 components);

	static ID3D11VertexShader* GetSimpleVertexShader();
	static ID3D11VertexShader* GetClearVertexShader();
	static ID3D11InputLayout* GetSimpleInputLayout();
	static ID3D11InputLayout* GetClearInputLayout();

	static bool VertexShaderCache::InsertByteCode(const VERTEXSHADERUID &uid, D3DBlob* bcodeblob);

private:
	struct VSCacheEntry
	{ 
		ID3D11VertexShader* shader;
		D3DBlob* bytecode; // needed to initialize the input layout
		int frameCount;

		VSCacheEntry() : shader(NULL), bytecode(NULL), frameCount(0) {}
		void SetByteCode(D3DBlob* blob)
		{
			SAFE_RELEASE(bytecode);
			bytecode = blob;
			blob->AddRef();
		}
		void Destroy()
		{
			SAFE_RELEASE(shader);
			SAFE_RELEASE(bytecode);
		}
	};
	typedef std::map<VERTEXSHADERUID, VSCacheEntry> VSCache;
	
	static VSCache vshaders;
	static const VSCacheEntry* last_entry;
};

#endif  // _VERTEXSHADERCACHE_H
