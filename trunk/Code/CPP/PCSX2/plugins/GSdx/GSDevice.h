/*
 *	Copyright (C) 2007-2009 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include "GSWnd.h"
#include "GSTexture.h"
#include "GSVertex.h"
#include "GSAlignedClass.h"

#pragma pack(push, 1)

class MergeConstantBuffer
{
public:
	GSVector4 BGColor;

	MergeConstantBuffer() {memset(this, 0, sizeof(*this));}
};

class InterlaceConstantBuffer
{
public:
	GSVector2 ZrH;
	float hH;
	float _pad[1];

	InterlaceConstantBuffer() {memset(this, 0, sizeof(*this));}
};

#pragma pack(pop)

class GSDevice : public GSAlignedClass<32>
{
	list<GSTexture*> m_pool;

protected:
	GSWnd* m_wnd;
	bool m_vsync;
	bool m_rbswapped;
	GSTexture* m_backbuffer;
	GSTexture* m_merge;
	GSTexture* m_weavebob;
	GSTexture* m_blend;
	GSTexture* m_1x1;
	GSTexture* m_current;
	struct {size_t stride, start, count, limit;} m_vertices;
	unsigned int m_frame; // for ageing the pool

	virtual GSTexture* CreateSurface(int type, int w, int h, bool msaa, int format) = 0;
	virtual GSTexture* FetchSurface(int type, int w, int h, bool msaa, int format);

	virtual void DoMerge(GSTexture* st[2], GSVector4* sr, GSTexture* dt, GSVector4* dr, bool slbg, bool mmod, const GSVector4& c) = 0;
	virtual void DoInterlace(GSTexture* st, GSTexture* dt, int shader, bool linear, float yoffset) = 0;

public:
	GSDevice();
	virtual ~GSDevice();

	void Recycle(GSTexture* t);

	enum {Windowed, Fullscreen, DontCare};

	virtual bool Create(GSWnd* wnd);
	virtual bool Reset(int w, int h);
	virtual bool IsLost(bool update = false) {return false;}
	virtual void Present(const GSVector4i& r, int shader);
	virtual void Present(GSTexture* st, GSTexture* dt, const GSVector4& dr, int shader = 0);
	virtual void Flip() {}

	virtual void SetVSync(bool enable) {m_vsync = enable;}

	virtual void BeginScene() {}
	virtual void DrawPrimitive() {};
	virtual void EndScene();

	virtual void ClearRenderTarget(GSTexture* t, const GSVector4& c) {}
	virtual void ClearRenderTarget(GSTexture* t, uint32 c) {}
	virtual void ClearDepth(GSTexture* t, float c) {}
	virtual void ClearStencil(GSTexture* t, uint8 c) {}

	virtual GSTexture* CreateRenderTarget(int w, int h, bool msaa, int format = 0);
	virtual GSTexture* CreateDepthStencil(int w, int h, bool msaa, int format = 0);
	virtual GSTexture* CreateTexture(int w, int h, int format = 0);
	virtual GSTexture* CreateOffscreen(int w, int h, int format = 0);

	virtual GSTexture* Resolve(GSTexture* t) {return NULL;}

	virtual GSTexture* CopyOffscreen(GSTexture* src, const GSVector4& sr, int w, int h, int format = 0) {return NULL;}

	virtual void CopyRect(GSTexture* st, GSTexture* dt, const GSVector4i& r) {}
	virtual void StretchRect(GSTexture* st, const GSVector4& sr, GSTexture* dt, const GSVector4& dr, int shader = 0, bool linear = true) {}

	void StretchRect(GSTexture* st, GSTexture* dt, const GSVector4& dr, int shader = 0, bool linear = true);

	virtual void PSSetShaderResources(GSTexture* sr0, GSTexture* sr1) {}
	virtual void PSSetShaderResource(int i, GSTexture* sr) {}
	virtual void OMSetRenderTargets(GSTexture* rt, GSTexture* ds, const GSVector4i* scissor = NULL) {}

	GSTexture* GetCurrent();

	void Merge(GSTexture* st[2], GSVector4* sr, GSVector4* dr, const GSVector2i& fs, bool slbg, bool mmod, const GSVector4& c);
	void Interlace(const GSVector2i& ds, int field, int mode, float yoffset);

	bool ResizeTexture(GSTexture** t, int w, int h);

	bool IsRBSwapped() {return m_rbswapped;}

	void AgePool();
};

