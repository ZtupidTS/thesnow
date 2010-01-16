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

#include <d3dx9.h>
#include <string>

#include "VideoConfig.h"
#include "D3DShader.h"

namespace D3D
{

LPDIRECT3DVERTEXSHADER9 CompileVertexShader(const char *code, int len)
{
	//try to compile
	LPD3DXBUFFER shaderBuffer = 0;
	LPD3DXBUFFER errorBuffer = 0;
	LPDIRECT3DVERTEXSHADER9 vShader = 0;
 	HRESULT hr = D3DXCompileShader(code, len, 0, 0, "main", D3D::VertexShaderVersionString(),
						           0, &shaderBuffer, &errorBuffer, 0);
	if (FAILED(hr))
	{
		//compilation error
		if (g_ActiveConfig.bShowShaderErrors) {
			std::string hello = (char*)errorBuffer->GetBufferPointer();
			hello += "\n\n";
			hello += code;
			MessageBoxA(0, hello.c_str(), "Error assembling vertex shader", MB_ICONERROR);
		}
		vShader = 0;
	}
	else if (SUCCEEDED(hr))
	{
		//create it
		HRESULT hr = E_FAIL;
		if (shaderBuffer)
			hr = D3D::dev->CreateVertexShader((DWORD *)shaderBuffer->GetBufferPointer(), &vShader);
		if ((FAILED(hr) || vShader == 0) && g_ActiveConfig.bShowShaderErrors)
		{
			MessageBoxA(0, code, (char*)errorBuffer->GetBufferPointer(), MB_ICONERROR);
		}
	}

	//cleanup
	if (shaderBuffer)
		shaderBuffer->Release();
	if (errorBuffer)
		errorBuffer->Release();
	return vShader;
}

LPDIRECT3DPIXELSHADER9 CompilePixelShader(const char *code, int len)
{
	LPD3DXBUFFER shaderBuffer = 0;
	LPD3DXBUFFER errorBuffer = 0;
	LPDIRECT3DPIXELSHADER9 pShader = 0;

	// Someone:
	// For some reason, I had this kind of errors : "Shader uses texture addressing operations
	// in a dependency chain that is too complex for the target shader model (ps_2_0) to handle."
	HRESULT hr = D3DXCompileShader(code, len, 0, 0, "main", D3D::PixelShaderVersionString(), 
				 		           0, &shaderBuffer, &errorBuffer, 0);

	if (FAILED(hr))
	{
		if (g_ActiveConfig.bShowShaderErrors) {
			std::string hello = (char*)errorBuffer->GetBufferPointer();
			hello += "\n\n";
			hello += code;
			MessageBoxA(0, hello.c_str(), "Error assembling pixel shader", MB_ICONERROR);
		}
		pShader = 0;
	}
	else 
	{
		//create it
		HRESULT hr = D3D::dev->CreatePixelShader((DWORD *)shaderBuffer->GetBufferPointer(), &pShader);
		if ((FAILED(hr) || pShader == 0) && g_ActiveConfig.bShowShaderErrors)
		{
			MessageBoxA(0, "damn", "error creating pixelshader", MB_ICONERROR);
		}
	}

	//cleanup
	if (shaderBuffer)
		shaderBuffer->Release();
	if (errorBuffer)
		errorBuffer->Release();
	return pShader;
}

}  // namespace
