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

#include "stdafx.h"
#include "Profiler.h"
#include "XFStructs.h"
#include "Render.h"
#include "main.h"
#include "VertexManager.h"
#include "Utils.h"

// LoadXFReg 0x10
void LoadXFReg(u32 transferSize, u32 baseAddress, u32 *pData)
{	
    DVSTARTPROFILE();
	u32 address = baseAddress;
	for (int i = 0; i < (int)transferSize; i++)
	{
		address = baseAddress + i;

		// Setup a Matrix
		if (address < 0x1000)
		{
			u32* p1 = &xfmem[address];
			memcpy(p1, &pData[i], transferSize*4);
			i += transferSize;
		}
		else if (address < 0x2000)
		{
			u32 data = pData[i];	
			switch (address)
			{
			case 0x1006:
				//SetGPMetric
				break;
			case 0x1008: //__GXXfVtxSpecs, wrote 0004
				break;
			case 0x1009: //GXSetNumChans (no)
				break;
			case 0x100a: xfregs.colChans[0].ambColor = data; break; //GXSetChanAmbientcolor
			case 0x100b: xfregs.colChans[1].ambColor = data; break; //GXSetChanAmbientcolor
			case 0x100c: xfregs.colChans[0].matColor = data; break; //GXSetChanMatcolor (rgba)
			case 0x100d: xfregs.colChans[1].matColor = data; break; //GXSetChanMatcolor (rgba)

			case 0x100e: xfregs.colChans[0].color.hex = data; break; //color0
			case 0x100f: xfregs.colChans[1].color.hex = data; break; //color1
			case 0x1010: xfregs.colChans[0].alpha.hex = data; break; //alpha0
			case 0x1011: xfregs.colChans[1].alpha.hex = data; break; //alpha1

			case 0x1018:
				break;

			case 0x101a: 
				VertexManager::Flush();
				memcpy(xfregs.rawViewport, &pData[i], sizeof(xfregs.rawViewport)); 
				XFUpdateVP(); 
				i += 6;
				break;

			case 0x1020: 
				VertexManager::Flush();
				memcpy(xfregs.rawProjection, &pData[i], sizeof(xfregs.rawProjection)); 
				XFUpdatePJ(); 
				i += 7;
				return;

			case 0x103f: 
				xfregs.numTexGens = data;
				break;

			case 0x1040: xfregs.texcoords[0].texmtxinfo.hex = data; break;
			case 0x1041: xfregs.texcoords[1].texmtxinfo.hex = data; break;
			case 0x1042: xfregs.texcoords[2].texmtxinfo.hex = data; break;
			case 0x1043: xfregs.texcoords[3].texmtxinfo.hex = data; break;
			case 0x1044: xfregs.texcoords[4].texmtxinfo.hex = data; break;
			case 0x1045: xfregs.texcoords[5].texmtxinfo.hex = data; break;
			case 0x1046: xfregs.texcoords[6].texmtxinfo.hex = data; break;
			case 0x1047: xfregs.texcoords[7].texmtxinfo.hex = data; break;

			case 0x1050: xfregs.texcoords[0].postmtxinfo.hex = data; break;
			case 0x1051: xfregs.texcoords[1].postmtxinfo.hex = data; break;
			case 0x1052: xfregs.texcoords[2].postmtxinfo.hex = data; break;
			case 0x1053: xfregs.texcoords[3].postmtxinfo.hex = data; break;
			case 0x1054: xfregs.texcoords[4].postmtxinfo.hex = data; break;
			case 0x1055: xfregs.texcoords[5].postmtxinfo.hex = data; break;
			case 0x1056: xfregs.texcoords[6].postmtxinfo.hex = data; break;
			case 0x1057: xfregs.texcoords[7].postmtxinfo.hex = data; break;

			default:
				break;
			}
		}
		else if (address>=0x4000)
		{
			// MessageBox(NULL, "1", "1", MB_OK);
			//4010 __GXSetGenMode
		}
	}
}

// Check docs for this sucker...
void LoadIndexedXF(u32 val, int array)
{
    DVSTARTPROFILE();

	int index = val >> 16;
	int address = val & 0xFFF; //check mask
	int size = ((val >> 12) & 0xF)+1;
	//load stuff from array to address in xf mem
	for (int i = 0; i < size; i++)
		xfmem[address + i] = Memory_Read_U32(arraybases[array] + arraystrides[array]*index + i*4);
}

void XFUpdateVP()
{
	Renderer::SetViewport(xfregs.rawViewport); 
}

void XFUpdatePJ()
{
	Renderer::SetProjection(xfregs.rawProjection, 0); 
}
