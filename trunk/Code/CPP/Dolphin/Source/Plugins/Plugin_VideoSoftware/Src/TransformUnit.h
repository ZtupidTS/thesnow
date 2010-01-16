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

#ifndef _TRANSFORM_UNIT_H_
#define _TRANSFORM_UNIT_H_

struct InputVertexData;
struct OutputVertexData;

namespace TransformUnit
{
    void MultiplyVec2Mat24(const float *vec, const float *mat, float *result);
    void MultiplyVec2Mat34(const float *vec, const float *mat, float *result);
    void MultiplyVec3Mat33(const float *vec, const float *mat, float *result);
    void MultiplyVec3Mat34(const float *vec, const float *mat, float *result);

    void TransformPosition(const InputVertexData *src, OutputVertexData *dst);
    void TransformNormal(const InputVertexData *src, bool nbt, OutputVertexData *dst);
    void TransformColor(const InputVertexData *src, OutputVertexData *dst);
    void TransformTexCoord(const InputVertexData *src, OutputVertexData *dst, bool specialCase);
}

#endif
