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

#ifndef _PIXELSHADERCACHE_H_
#define _PIXELSHADERCACHE_H_

#include <map>
#include <string>

#include "BPMemory.h"
#include "PixelShaderGen.h"

struct FRAGMENTSHADER
{
	FRAGMENTSHADER() : glprogid(0) { }
	void Destroy()
	{
		if (glprogid)
		{
			glDeleteProgramsARB(1, &glprogid);
			glprogid = 0;
		}
	}
	GLuint glprogid; // opengl program id
#if defined(_DEBUG) || defined(DEBUGFAST) 
	std::string strprog;
#endif
};

class PixelShaderCache
{
	struct PSCacheEntry
	{
		FRAGMENTSHADER shader;
		int frameCount;
		PSCacheEntry() : frameCount(0) {}
		~PSCacheEntry() {}
		void Destroy()
		{
			shader.Destroy();
		}
	};

	typedef std::map<PIXELSHADERUID, PSCacheEntry> PSCache;

	static PSCache pshaders;

	static PIXELSHADERUID s_curuid; // the current pixel shader uid (progressively changed as memory is written)

    static bool s_displayCompileAlert;

	static GLuint CurrentShader;

	static bool ShaderEnabled;

public:
	static void Init();
	static void Shutdown();

	static FRAGMENTSHADER* GetShader(bool dstAlphaEnable);
	static bool CompilePixelShader(FRAGMENTSHADER& ps, const char* pstrprogram);

	static GLuint GetColorMatrixProgram();

    static GLuint GetDepthMatrixProgram();

	static void SetCurrentShader(GLuint Shader);

	static void DisableShader();
	
	static void EnableShader(GLuint Shader);
};

#endif // _PIXELSHADERCACHE_H_
