// Copyright (C) 2003-2009 Dolphin Project.

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

#include "Common.h"

#include "VertexLoader.h"
#include "VertexLoader_Position.h"
#include "../../../Core/VideoCommon/Src/VertexLoader_Normal.h"
#include "../../../Core/VideoCommon/Src/VertexLoader_Color.h"
#include "../../../Core/VideoCommon/Src/VertexLoader_TextCoord.h"

#include "CPMemLoader.h"
#include "XFMemLoader.h"

#include "TransformUnit.h"
#include "SetupUnit.h"
#include "Statistics.h"
#include "NativeVertexWriter.h"
#include "VertexFormatConverter.h"
#include "../../../Core/VideoCommon/Src/DataReader.h"

// Vertex loaders read these
int tcIndex;
int colIndex;
int colElements[2];
float posScale;
float tcScale[8];


VertexLoader::VertexLoader() :
    m_VertexSize(0),
    m_NumAttributeLoaders(0)
 {
     VertexLoader_Normal::Init();
	 VertexLoader_Position::Init();
	 VertexLoader_TextCoord::Init();

     m_SetupUnit = new SetupUnit;     
 }

VertexLoader::~VertexLoader()
{
    delete m_SetupUnit;
    m_SetupUnit = NULL;
}

void VertexLoader::SetFormat(u8 attributeIndex, u8 primitiveType)
{
    m_CurrentVat = &g_VtxAttr[attributeIndex];

    posScale = 1.0f / float(1 << m_CurrentVat->g0.PosFrac);
    tcScale[0] = 1.0f / float(1 << m_CurrentVat->g0.Tex0Frac);
    tcScale[1] = 1.0f / float(1 << m_CurrentVat->g1.Tex1Frac);
    tcScale[2] = 1.0f / float(1 << m_CurrentVat->g1.Tex2Frac);
    tcScale[3] = 1.0f / float(1 << m_CurrentVat->g1.Tex3Frac);
    tcScale[4] = 1.0f / float(1 << m_CurrentVat->g2.Tex4Frac);
    tcScale[5] = 1.0f / float(1 << m_CurrentVat->g2.Tex5Frac);
    tcScale[6] = 1.0f / float(1 << m_CurrentVat->g2.Tex6Frac);
    tcScale[7] = 1.0f / float(1 << m_CurrentVat->g2.Tex7Frac);

    //TexMtx
    const int tmDesc[8] = {
        g_VtxDesc.Tex0MatIdx, g_VtxDesc.Tex1MatIdx, g_VtxDesc.Tex2MatIdx, g_VtxDesc.Tex3MatIdx,
		g_VtxDesc.Tex4MatIdx, g_VtxDesc.Tex5MatIdx, g_VtxDesc.Tex6MatIdx, g_VtxDesc.Tex7MatIdx
	};
    // Colors
	const int colDesc[2] = {g_VtxDesc.Color0, g_VtxDesc.Color1};
    colElements[0] = m_CurrentVat->g0.Color0Elements;
    colElements[1] = m_CurrentVat->g0.Color1Elements;
    const int colComp[2] = {m_CurrentVat->g0.Color0Comp, m_CurrentVat->g0.Color1Comp};
	// TextureCoord
	const int tcDesc[8] = {
		g_VtxDesc.Tex0Coord, g_VtxDesc.Tex1Coord, g_VtxDesc.Tex2Coord, g_VtxDesc.Tex3Coord,
		g_VtxDesc.Tex4Coord, g_VtxDesc.Tex5Coord, g_VtxDesc.Tex6Coord, (g_VtxDesc.Hex >> 31) & 3
	};
    const int tcElements[8] = {
        m_CurrentVat->g0.Tex0CoordElements, m_CurrentVat->g1.Tex1CoordElements, m_CurrentVat->g1.Tex2CoordElements, 
        m_CurrentVat->g1.Tex3CoordElements, m_CurrentVat->g1.Tex4CoordElements, m_CurrentVat->g2.Tex5CoordElements,
        m_CurrentVat->g2.Tex6CoordElements, m_CurrentVat->g2.Tex7CoordElements
    };

    const int tcFormat[8] = {
        m_CurrentVat->g0.Tex0CoordFormat, m_CurrentVat->g1.Tex1CoordFormat, m_CurrentVat->g1.Tex2CoordFormat, 
        m_CurrentVat->g1.Tex3CoordFormat, m_CurrentVat->g1.Tex4CoordFormat, m_CurrentVat->g2.Tex5CoordFormat,
        m_CurrentVat->g2.Tex6CoordFormat, m_CurrentVat->g2.Tex7CoordFormat
    };

    m_VertexSize = 0;
	
	// Reset pipeline
	m_positionLoader = NULL;
    m_normalLoader = NULL;
    m_NumAttributeLoaders = 0;

    // Reset vertex
    // matrix index from xfregs or cp memory?
    _assert_msg_(VIDEO, xfregs.MatrixIndexA.PosNormalMtxIdx == MatrixIndexA.PosNormalMtxIdx, "Matrix indices don't match");
    //_assert_msg_(VIDEO, xfregs.MatrixIndexA.Tex0MtxIdx == MatrixIndexA.Tex0MtxIdx, "Matrix indices don't match");
    //_assert_msg_(VIDEO, xfregs.MatrixIndexA.Tex1MtxIdx == MatrixIndexA.Tex1MtxIdx, "Matrix indices don't match");
    _assert_msg_(VIDEO, xfregs.MatrixIndexA.Tex2MtxIdx == MatrixIndexA.Tex2MtxIdx, "Matrix indices don't match");
    _assert_msg_(VIDEO, xfregs.MatrixIndexA.Tex3MtxIdx == MatrixIndexA.Tex3MtxIdx, "Matrix indices don't match");
    _assert_msg_(VIDEO, xfregs.MatrixIndexB.Tex4MtxIdx == MatrixIndexB.Tex4MtxIdx, "Matrix indices don't match");
    _assert_msg_(VIDEO, xfregs.MatrixIndexB.Tex5MtxIdx == MatrixIndexB.Tex5MtxIdx, "Matrix indices don't match");
    _assert_msg_(VIDEO, xfregs.MatrixIndexB.Tex6MtxIdx == MatrixIndexB.Tex6MtxIdx, "Matrix indices don't match");
    _assert_msg_(VIDEO, xfregs.MatrixIndexB.Tex7MtxIdx == MatrixIndexB.Tex7MtxIdx, "Matrix indices don't match");
    m_Vertex.posMtx = xfregs.MatrixIndexA.PosNormalMtxIdx;
    m_Vertex.texMtx[0] = xfregs.MatrixIndexA.Tex0MtxIdx;
    m_Vertex.texMtx[1] = xfregs.MatrixIndexA.Tex1MtxIdx;
    m_Vertex.texMtx[2] = xfregs.MatrixIndexA.Tex2MtxIdx;
    m_Vertex.texMtx[3] = xfregs.MatrixIndexA.Tex3MtxIdx;
    m_Vertex.texMtx[4] = xfregs.MatrixIndexB.Tex4MtxIdx;
    m_Vertex.texMtx[5] = xfregs.MatrixIndexB.Tex5MtxIdx;
    m_Vertex.texMtx[6] = xfregs.MatrixIndexB.Tex6MtxIdx;
    m_Vertex.texMtx[7] = xfregs.MatrixIndexB.Tex7MtxIdx;
    /*m_Vertex.posMtx = MatrixIndexA.PosNormalMtxIdx;
    m_Vertex.texMtx[0] = MatrixIndexA.Tex0MtxIdx;
    m_Vertex.texMtx[1] = MatrixIndexA.Tex1MtxIdx;
    m_Vertex.texMtx[2] = MatrixIndexA.Tex2MtxIdx;
    m_Vertex.texMtx[3] = MatrixIndexA.Tex3MtxIdx;
    m_Vertex.texMtx[4] = MatrixIndexB.Tex4MtxIdx;
    m_Vertex.texMtx[5] = MatrixIndexB.Tex5MtxIdx;
    m_Vertex.texMtx[6] = MatrixIndexB.Tex6MtxIdx;
    m_Vertex.texMtx[7] = MatrixIndexB.Tex7MtxIdx;*/

    if (g_VtxDesc.PosMatIdx != NOT_PRESENT) {
        AddAttributeLoader(LoadPosMtx);
        m_VertexSize++;
    }

    for (int i = 0; i < 8; ++i) {
        if (tmDesc[i] != NOT_PRESENT)
        {
            AddAttributeLoader(LoadTexMtx, i);
            m_VertexSize++;
        }
    }

	// Write vertex position loader
	m_positionLoader = VertexLoader_Position::GetFunction(g_VtxDesc.Position, m_CurrentVat->g0.PosFormat, m_CurrentVat->g0.PosElements);
	m_VertexSize += VertexLoader_Position::GetSize(g_VtxDesc.Position, m_CurrentVat->g0.PosFormat, m_CurrentVat->g0.PosElements);
	AddAttributeLoader(LoadPosition);

	// Normals
	if (g_VtxDesc.Normal != NOT_PRESENT) {
		m_VertexSize += VertexLoader_Normal::GetSize(g_VtxDesc.Normal, m_CurrentVat->g0.NormalFormat, m_CurrentVat->g0.NormalElements, m_CurrentVat->g0.NormalIndex3);
		m_normalLoader = VertexLoader_Normal::GetFunction(g_VtxDesc.Normal, m_CurrentVat->g0.NormalFormat, m_CurrentVat->g0.NormalElements, m_CurrentVat->g0.NormalIndex3, true);
		if (m_normalLoader == 0)
		{
			ERROR_LOG(VIDEO, "VertexLoader_Normal::GetFunction returned zero!");
		}
        AddAttributeLoader(LoadNormal);

        switch (m_CurrentVat->g0.NormalFormat) {
		case FORMAT_UBYTE:	
		case FORMAT_BYTE:
            if (m_CurrentVat->g0.NormalElements)
                m_normalConverter = VertexFormatConverter::LoadNormal3_Byte;
            else
                m_normalConverter = VertexFormatConverter::LoadNormal1_Byte;
			break;
		case FORMAT_USHORT:
		case FORMAT_SHORT:
			if (m_CurrentVat->g0.NormalElements)
                m_normalConverter = VertexFormatConverter::LoadNormal3_Short;
            else
                m_normalConverter = VertexFormatConverter::LoadNormal1_Short;
			break;
		case FORMAT_FLOAT:
			if (m_CurrentVat->g0.NormalElements)
                m_normalConverter = VertexFormatConverter::LoadNormal3_Float;
            else
                m_normalConverter = VertexFormatConverter::LoadNormal1_Float;
			break;
		default: _assert_(0); break;
		}
    }

	for (int i = 0; i < 2; i++) {
		switch (colDesc[i])
		{
		case NOT_PRESENT:
            m_colorLoader[i] = NULL;
			break;
		case DIRECT:
			switch (colComp[i])
			{
			case FORMAT_16B_565:	m_VertexSize += 2; m_colorLoader[i] = (Color_ReadDirect_16b_565); break;
			case FORMAT_24B_888:	m_VertexSize += 3; m_colorLoader[i] = (Color_ReadDirect_24b_888); break;
			case FORMAT_32B_888x:	m_VertexSize += 4; m_colorLoader[i] = (Color_ReadDirect_32b_888x); break;
			case FORMAT_16B_4444:	m_VertexSize += 2; m_colorLoader[i] = (Color_ReadDirect_16b_4444); break;
			case FORMAT_24B_6666:	m_VertexSize += 3; m_colorLoader[i] = (Color_ReadDirect_24b_6666); break;
			case FORMAT_32B_8888:	m_VertexSize += 4; m_colorLoader[i] = (Color_ReadDirect_32b_8888); break;
			default: _assert_(0); break;
			}
            AddAttributeLoader(LoadColor, i);
			break;
		case INDEX8:	
			m_VertexSize += 1;
			switch (colComp[i])
			{
			case FORMAT_16B_565:	m_colorLoader[i] = (Color_ReadIndex8_16b_565); break;
			case FORMAT_24B_888:	m_colorLoader[i] = (Color_ReadIndex8_24b_888); break;
			case FORMAT_32B_888x:	m_colorLoader[i] = (Color_ReadIndex8_32b_888x); break;
			case FORMAT_16B_4444:	m_colorLoader[i] = (Color_ReadIndex8_16b_4444); break;
			case FORMAT_24B_6666:	m_colorLoader[i] = (Color_ReadIndex8_24b_6666); break;
			case FORMAT_32B_8888:	m_colorLoader[i] = (Color_ReadIndex8_32b_8888); break;
			default: _assert_(0); break;
			}
            AddAttributeLoader(LoadColor, i);
			break;
		case INDEX16:
			m_VertexSize += 2;
			switch (colComp[i])
			{
			case FORMAT_16B_565:	m_colorLoader[i] = (Color_ReadIndex16_16b_565); break;
			case FORMAT_24B_888:	m_colorLoader[i] = (Color_ReadIndex16_24b_888); break;
			case FORMAT_32B_888x:	m_colorLoader[i] = (Color_ReadIndex16_32b_888x); break;
			case FORMAT_16B_4444:	m_colorLoader[i] = (Color_ReadIndex16_16b_4444); break;
			case FORMAT_24B_6666:	m_colorLoader[i] = (Color_ReadIndex16_24b_6666); break;
			case FORMAT_32B_8888:	m_colorLoader[i] = (Color_ReadIndex16_32b_8888); break;
			default: _assert_(0); break;
			}
            AddAttributeLoader(LoadColor, i);
			break;
		}
	}

	// Texture matrix indices (remove if corresponding texture coordinate isn't enabled)
	for (int i = 0; i < 8; i++) {
		const int desc = tcDesc[i];
		const int format = tcFormat[i];
		const int elements = tcElements[i];
		_assert_msg_(VIDEO, NOT_PRESENT <= desc && desc <= INDEX16, "Invalid texture coordinates description!\n(desc = %d)", desc);
		_assert_msg_(VIDEO, FORMAT_UBYTE <= format && format <= FORMAT_FLOAT, "Invalid texture coordinates format!\n(format = %d)", format);
		_assert_msg_(VIDEO, 0 <= elements && elements <= 1, "Invalid number of texture coordinates elemnts!\n(elements = %d)", elements);

		m_texCoordLoader[i] = VertexLoader_TextCoord::GetFunction(desc, format, elements);
		m_VertexSize += VertexLoader_TextCoord::GetSize(desc, format, elements);
		if (m_texCoordLoader[i])
			AddAttributeLoader(LoadTexCoord, i);
    }

	// special case if only pos and tex coord 0 and tex coord input is AB11
	m_TexGenSpecialCase =
		((g_VtxDesc.Hex & 0x60600L) == g_VtxDesc.Hex) && // only pos and tex coord 0
		(g_VtxDesc.Tex0Coord != NOT_PRESENT) &&
		(xfregs.texMtxInfo[0].inputform == XF_TEXINPUT_AB11);


    m_SetupUnit->Init(primitiveType);
}


void VertexLoader::LoadVertex()
{
    for (int i = 0; i < m_NumAttributeLoaders; i++)
        m_AttributeLoaders[i].loader(this, &m_Vertex, m_AttributeLoaders[i].index);

    OutputVertexData* outVertex = m_SetupUnit->GetVertex();

    // transform input data
    TransformUnit::TransformPosition(&m_Vertex, outVertex);

    if (g_VtxDesc.Normal != NOT_PRESENT)
    {
        TransformUnit::TransformNormal(&m_Vertex, m_CurrentVat->g0.NormalElements, outVertex);
    }

    TransformUnit::TransformColor(&m_Vertex, outVertex);

    TransformUnit::TransformTexCoord(&m_Vertex, outVertex, m_TexGenSpecialCase);

    m_SetupUnit->SetupVertex();

    INCSTAT(stats.thisFrame.numVerticesLoaded)
}

void VertexLoader::AddAttributeLoader(AttributeLoader loader, u8 index)
{
    _assert_msg_(VIDEO, m_NumAttributeLoaders < 21, "Too many attribute loaders");
    m_AttributeLoaders[m_NumAttributeLoaders].loader = loader;
    m_AttributeLoaders[m_NumAttributeLoaders++].index = index;
}

void VertexLoader::LoadPosMtx(VertexLoader *vertexLoader, InputVertexData *vertex, u8 unused)
{
    vertex->posMtx = DataReadU8() & 0x3f;
}

void VertexLoader::LoadTexMtx(VertexLoader *vertexLoader, InputVertexData *vertex, u8 index)
{
    vertex->texMtx[index] = DataReadU8() & 0x3f;
}    

void VertexLoader::LoadPosition(VertexLoader *vertexLoader, InputVertexData *vertex, u8 unused)
{
    VertexManager::s_pCurBufferPointer = (u8*)&vertex->position;
    vertexLoader->m_positionLoader();
}

void VertexLoader::LoadNormal(VertexLoader *vertexLoader, InputVertexData *vertex, u8 unused)
{
    u8 buffer[3*3*4];

    VertexManager::s_pCurBufferPointer = buffer;
    vertexLoader->m_normalLoader();

    // the common vertex loader loads data as bytes, shorts or floats so an extra step is needed to make it floats
    vertexLoader->m_normalConverter(vertex, buffer);
}

void VertexLoader::LoadColor(VertexLoader *vertexLoader, InputVertexData *vertex, u8 index)
{
    u32 color;
    VertexManager::s_pCurBufferPointer = (u8*)&color;
    colIndex = index;
    vertexLoader->m_colorLoader[index]();

    // rgba -> abgr
    *(u32*)vertex->color[index] = Common::swap32(color);
}

void VertexLoader::LoadTexCoord(VertexLoader *vertexLoader, InputVertexData *vertex, u8 index)
{
    VertexManager::s_pCurBufferPointer = (u8*)&vertex->texCoords[index];
    tcIndex = index;
    vertexLoader->m_texCoordLoader[index]();
}


