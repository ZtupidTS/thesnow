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

#include "stdafx.h"
#include "GSRenderer.h"

GSRenderer::GSRenderer()
	: GSState()
	, m_vt(this)
	, m_dev(NULL)
	, m_shader(0)
{
	m_GStitleInfoBuffer[0] = 0;

	m_interlace = theApp.GetConfig("interlace", 0);
	m_aspectratio = theApp.GetConfig("aspectratio", 1);
	m_filter = theApp.GetConfig("filter", 1);
	m_vsync = !!theApp.GetConfig("vsync", 0);
	m_aa1 = !!theApp.GetConfig("aa1", 0);
	m_mipmap = !!theApp.GetConfig("mipmap", 1);
	m_fxaa = !!theApp.GetConfig("fxaa", 0);

	s_n = 0;
	s_dump = !!theApp.GetConfig("dump", 0);
	s_save = !!theApp.GetConfig("save", 0);
	s_savez = !!theApp.GetConfig("savez", 0);
	s_saven = theApp.GetConfig("saven", 0);
}

GSRenderer::~GSRenderer()
{
	/*if(m_dev)
	{
		m_dev->Reset(1, 1, GSDevice::Windowed);
	}*/

	delete m_dev;
}

bool GSRenderer::CreateWnd(const string& title, int w, int h)
{
	return m_wnd.Create(title.c_str(), w, h);
}

bool GSRenderer::CreateDevice(GSDevice* dev)
{
	ASSERT(dev);
	ASSERT(!m_dev);

	if(!dev->Create(&m_wnd))
	{
		return false;
	}

	m_dev = dev;
	m_dev->SetVSync(m_vsync && m_framelimit);

	return true;
}

void GSRenderer::ResetDevice()
{
    ResetPrim();

    if(m_dev) m_dev->Reset(1, 1);
}

bool GSRenderer::Merge(int field)
{
	bool en[2];

	GSVector4i fr[2];
	GSVector4i dr[2];

	int baseline = INT_MAX;

	for(int i = 0; i < 2; i++)
	{
		en[i] = IsEnabled(i);

		if(en[i])
		{
			fr[i] = GetFrameRect(i);
			dr[i] = GetDisplayRect(i);

			baseline = min(dr[i].top, baseline);

			// printf("[%d]: %d %d %d %d, %d %d %d %d\n", i, fr[i], dr[i]);
		}
	}

	if(!en[0] && !en[1])
	{
		return false;
	}

	// try to avoid fullscreen blur, could be nice on tv but on a monitor it's like double vision, hurts my eyes (persona 4, guitar hero)
	//
	// NOTE: probably the technique explained in graphtip.pdf (Antialiasing by Supersampling / 4. Reading Odd/Even Scan Lines Separately with the PCRTC then Blending)

	bool samesrc =
		en[0] && en[1] &&
		m_regs->DISP[0].DISPFB.FBP == m_regs->DISP[1].DISPFB.FBP &&
		m_regs->DISP[0].DISPFB.FBW == m_regs->DISP[1].DISPFB.FBW &&
		m_regs->DISP[0].DISPFB.PSM == m_regs->DISP[1].DISPFB.PSM;

	bool blurdetected = false;

	if(samesrc /*&& m_regs->PMODE.SLBG == 0 && m_regs->PMODE.MMOD == 1 && m_regs->PMODE.ALP == 0x80*/)
	{
		if(fr[0].eq(fr[1] + GSVector4i(0, -1, 0, 0)) && dr[0].eq(dr[1] + GSVector4i(0, 0, 0, 1))
		|| fr[1].eq(fr[0] + GSVector4i(0, -1, 0, 0)) && dr[1].eq(dr[0] + GSVector4i(0, 0, 0, 1)))
		{
			// persona 4:
			//
			// fr[0] = 0 0 640 448
			// fr[1] = 0 1 640 448
			// dr[0] = 159 50 779 498
			// dr[1] = 159 50 779 497
			//
			// second image shifted up by 1 pixel and blended over itself
			//
			// god of war:
			//
			// fr[0] = 0 1 512 448
			// fr[1] = 0 0 512 448
			// dr[0] = 127 50 639 497
			// dr[1] = 127 50 639 498
			//
			// same just the first image shifted

			int top = min(fr[0].top, fr[1].top);
			int bottom = max(dr[0].bottom, dr[1].bottom);

			fr[0].top = top;
			fr[1].top = top;
			dr[0].bottom = bottom;
			dr[1].bottom = bottom;

			blurdetected = true;
		}
		else if(dr[0].eq(dr[1]) && (fr[0].eq(fr[1] + GSVector4i(0, 1, 0, 1)) || fr[1].eq(fr[0] + GSVector4i(0, 1, 0, 1))))
		{
			// dq5:
			//
			// fr[0] = 0 1 512 445
			// fr[1] = 0 0 512 444
			// dr[0] = 127 50 639 494
			// dr[1] = 127 50 639 494

			int top = min(fr[0].top, fr[1].top);
			int bottom = min(fr[0].bottom, fr[1].bottom);

			fr[0].top = fr[1].top = top;
			fr[0].bottom = fr[1].bottom = bottom;

			blurdetected = true;
		}
		//printf("samesrc = %d blurdetected = %d\n",samesrc,blurdetected);
	}

	GSVector2i fs(0, 0);
	GSVector2i ds(0, 0);

	GSTexture* tex[2] = {NULL, NULL};

	if(samesrc && fr[0].bottom == fr[1].bottom)
	{
		tex[0] = GetOutput(0);
		tex[1] = tex[0]; // saves one texture fetch
	}
	else
	{
		if(en[0]) tex[0] = GetOutput(0);
		if(en[1]) tex[1] = GetOutput(1);
	}

	GSVector4 src[2];
	GSVector4 dst[2];

	for(int i = 0; i < 2; i++)
	{
		if(!en[i] || !tex[i]) continue;

		GSVector4i r = fr[i];

		// overscan hack

		if(dr[i].height() > 512) // hmm
		{
			int y = GetDeviceSize(i).y;
			if(m_regs->SMODE2.INT && m_regs->SMODE2.FFMD) y /= 2;
			r.bottom = r.top + y;
		}

		GSVector4 scale = GSVector4(tex[i]->GetScale()).xyxy();

		src[i] = GSVector4(r) * scale / GSVector4(tex[i]->GetSize()).xyxy();

		GSVector2 o(0, 0);

		if(dr[i].top - baseline >= 4) // 2?
		{
			o.y = tex[i]->GetScale().y * (dr[i].top - baseline);

			if(m_regs->SMODE2.INT && m_regs->SMODE2.FFMD)
			{
				o.y /= 2;
			}
		}

		dst[i] = GSVector4(o).xyxy() + scale * GSVector4(r.rsize());

		fs.x = max(fs.x, (int)(dst[i].z + 0.5f));
		fs.y = max(fs.y, (int)(dst[i].w + 0.5f));
	}

	ds = fs;

	if(m_regs->SMODE2.INT && m_regs->SMODE2.FFMD)
	{
		ds.y *= 2;
	}

	bool slbg = m_regs->PMODE.SLBG;
	bool mmod = m_regs->PMODE.MMOD;

	if(tex[0] || tex[1])
	{
		if(tex[0] == tex[1] && !slbg && (src[0] == src[1] & dst[0] == dst[1]).alltrue())
		{
			// the two outputs are identical, skip drawing one of them (the one that is alpha blended)

			tex[0] = NULL;
		}

		GSVector4 c = GSVector4((int)m_regs->BGCOLOR.R, (int)m_regs->BGCOLOR.G, (int)m_regs->BGCOLOR.B, (int)m_regs->PMODE.ALP) / 255;

		m_dev->Merge(tex, src, dst, fs, slbg, mmod, c);

		if(m_regs->SMODE2.INT && m_interlace > 0)
		{
			int field2 = 1 - ((m_interlace - 1) & 1);
			int mode = (m_interlace - 1) >> 1;

			m_dev->Interlace(ds, field ^ field2, mode, tex[1] ? tex[1]->GetScale().y : tex[0]->GetScale().y);
		}

		if(m_fxaa)
		{
			m_dev->FXAA();
		}
	}

	return true;
}

void GSRenderer::SetFrameLimit(bool limit)
{
	m_framelimit = limit;

	if(m_dev) m_dev->SetVSync(m_vsync && m_framelimit);
}

void GSRenderer::SetVSync(bool enabled)
{
	m_vsync = enabled;

	if(m_dev) m_dev->SetVSync(m_vsync);
}

void GSRenderer::VSync(int field)
{
	GSPerfMonAutoTimer pmat(m_perfmon);

	m_perfmon.Put(GSPerfMon::Frame);

	Flush();

	if(!m_dev->IsLost(true))
	{
		if(!Merge(field ? 1 : 0))
		{
			return;
		}
	}
	else
	{
		ResetDevice();
	}

	// osd

	if((m_perfmon.GetFrame() & 0x1f) == 0)
	{
		m_perfmon.Update();

		double fps = 1000.0f / m_perfmon.Get(GSPerfMon::Frame);

		GSVector4i r = GetDisplayRect();

		string s;

#ifdef GSTITLEINFO_API_FORCE_VERBOSE
		if (1)//force verbose reply
#else
		if (m_wnd.IsManaged())
#endif
		{
			//GSdx owns the window's title, be verbose.

			string s2 = m_regs->SMODE2.INT ? (string("Interlaced ") + (m_regs->SMODE2.FFMD ? "(frame)" : "(field)")) : "Progressive";

			s = format(
				"%lld | %d x %d | %.2f fps (%d%%) | %s - %s | %s | %d/%d/%d | %d%% CPU | %.2f | %.2f",
				m_perfmon.GetFrame(), r.width(), r.height(), fps, (int)(100.0 * fps / GetFPS()),
				s2.c_str(),
				theApp.m_gs_interlace[m_interlace].name.c_str(),
				theApp.m_gs_aspectratio[m_aspectratio].name.c_str(),
				(int)m_perfmon.Get(GSPerfMon::Quad),
				(int)m_perfmon.Get(GSPerfMon::Prim),
				(int)m_perfmon.Get(GSPerfMon::Draw),
				m_perfmon.CPU(),
				m_perfmon.Get(GSPerfMon::Swizzle) / 1024,
				m_perfmon.Get(GSPerfMon::Unswizzle) / 1024
			);

			double fillrate = m_perfmon.Get(GSPerfMon::Fillrate);

			if(fillrate > 0)
			{
				s += format(" | %.2f mpps", fps * fillrate / (1024 * 1024));
			}

		}
		else
		{
			// Satisfy PCSX2's request for title info: minimal verbosity due to more external title text

			s = format("%dx%d | %s", r.width(), r.height(), theApp.m_gs_interlace[m_interlace].name.c_str());
		}

		if(m_capture.IsCapturing())
		{
			s += " | 记录中...";
		}

		if(m_wnd.IsManaged())
		{
			m_wnd.SetWindowText(s.c_str());
		}
		else
		{
			// note: do not use TryEnterCriticalSection.  It is unnecessary code complication in
			// an area that absolutely does not matter (even if it were 100 times slower, it wouldn't
			// be noticeable).  Besides, these locks are extremely short -- overhead of conditional
			// is way more expensive than just waiting for the CriticalSection in 1 of 10,000,000 tries. --air

			GSAutoLock lock(&m_pGSsetTitle_Crit);

			strncpy(m_GStitleInfoBuffer, s.c_str(), countof(m_GStitleInfoBuffer) - 1);

			m_GStitleInfoBuffer[sizeof(m_GStitleInfoBuffer) - 1] = 0; // make sure null terminated even if text overflows
		}
	}
	else
	{
		// [TODO]
		// We don't have window title rights, or the window has no title,
		// so let's use actual OSD!
	}

	if(m_frameskip)
	{
		return;
	}

	// present

	m_dev->Present(m_wnd.GetClientRect().fit(m_aspectratio), m_shader);

	// snapshot

	if(!m_snapshot.empty())
	{
		bool shift = false;

		#ifdef _WINDOWS

		shift = !!(::GetAsyncKeyState(VK_SHIFT) & 0x8000);

		#endif

		if(!m_dump && shift)
		{
			GSFreezeData fd;
			fd.size = 0;
			fd.data = NULL;
			Freeze(&fd, true);
			fd.data = new uint8[fd.size];
			Freeze(&fd, false);

			m_dump.Open(m_snapshot, m_crc, fd, m_regs);

			delete [] fd.data;
		}

		if(GSTexture* t = m_dev->GetCurrent())
		{
			t->Save(m_snapshot + ".bmp");
		}

		m_snapshot.clear();
	}
	else
	{
		if(m_dump)
		{
            bool control = false;

            #ifdef _WINDOWS

            control = !!(::GetAsyncKeyState(VK_CONTROL) & 0x8000);

            #endif

	    	m_dump.VSync(field, !control, m_regs);
		}
	}

	// capture

	if(m_capture.IsCapturing())
	{
		if(GSTexture* current = m_dev->GetCurrent())
		{
			GSVector2i size = m_capture.GetSize();

			if(GSTexture* offscreen = m_dev->CopyOffscreen(current, GSVector4(0, 0, 1, 1), size.x, size.y))
			{
				GSTexture::GSMap m;

				if(offscreen->Map(m))
				{
					m_capture.DeliverFrame(m.bits, m.pitch, !m_dev->IsRBSwapped());

					offscreen->Unmap();
				}

				m_dev->Recycle(offscreen);
			}
		}
	}
}

bool GSRenderer::MakeSnapshot(const string& path)
{
	if(m_snapshot.empty())
	{
		time_t t = time(NULL);

		char buff[16];

		if(strftime(buff, sizeof(buff), "%Y%m%d%H%M%S", localtime(&t)))
		{
			m_snapshot = format("%s_%s", path.c_str(), buff);
		}
	}

	return true;
}

void GSRenderer::BeginCapture()
{
	m_capture.BeginCapture(GetFPS());
}

void GSRenderer::EndCapture()
{
	m_capture.EndCapture();
}

void GSRenderer::KeyEvent(GSKeyEventData* e)
{
	if(e->type == KEYPRESS)
	{
	    #ifdef _WINDOWS

		int step = (::GetAsyncKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;

		switch(e->key)
		{
		case VK_F5:
			m_interlace = (m_interlace + 7 + step) % 7;
			printf("GSdx: Set deinterlace mode to %d (%s).\n", (int)m_interlace, theApp.m_gs_interlace.at(m_interlace).name.c_str());
			return;
		case VK_F6:
			if( m_wnd.IsManaged() )
				m_aspectratio = (m_aspectratio + 3 + step) % 3;
			return;
		case VK_F7:
			m_shader = (m_shader + 3 + step) % 3;
			printf("GSdx: Set shader %d (%s).\n", (int)m_shader);
			return;
		case VK_DELETE:
			m_aa1 = !m_aa1;
			printf("GSdx: (Software) aa1 is now %s.\n", m_aa1 ? "enabled" : "disabled");
			return;
		case VK_INSERT:
			m_mipmap = !m_mipmap;
			printf("GSdx: (Software) mipmapping is now %s.\n", m_mipmap ? "enabled" : "disabled");
			return;
		case VK_PRIOR:
			m_fxaa = !m_fxaa;
			printf("GSdx: fxaa is now %s.\n", m_fxaa ? "enabled" : "disabled");
			return;
		}

		#else

		// TODO: linux

		#endif
	}
}

void GSRenderer::GetTextureMinMax(GSVector4i& r, const GIFRegTEX0& TEX0, const GIFRegCLAMP& CLAMP, bool linear)
{
	int tw = TEX0.TW;
	int th = TEX0.TH;

	int w = 1 << tw;
	int h = 1 << th;

	GSVector4i tr(0, 0, w, h);

	int wms = CLAMP.WMS;
	int wmt = CLAMP.WMT;

	int minu = (int)CLAMP.MINU;
	int minv = (int)CLAMP.MINV;
	int maxu = (int)CLAMP.MAXU;
	int maxv = (int)CLAMP.MAXV;

	GSVector4i vr = tr;

	switch(wms)
	{
	case CLAMP_REPEAT:
		break;
	case CLAMP_CLAMP:
		break;
	case CLAMP_REGION_CLAMP:
		if(vr.x < minu) vr.x = minu;
		if(vr.z > maxu + 1) vr.z = maxu + 1;
		break;
	case CLAMP_REGION_REPEAT:
		vr.x = maxu;
		vr.z = vr.x + (minu + 1);
		break;
	default:
		__assume(0);
	}

	switch(wmt)
	{
	case CLAMP_REPEAT:
		break;
	case CLAMP_CLAMP:
		break;
	case CLAMP_REGION_CLAMP:
		if(vr.y < minv) vr.y = minv;
		if(vr.w > maxv + 1) vr.w = maxv + 1;
		break;
	case CLAMP_REGION_REPEAT:
		vr.y = maxv;
		vr.w = vr.y + (minv + 1);
		break;
	default:
		__assume(0);
	}

	if(wms + wmt < 6)
	{
		GSVector4 st = m_vt.m_min.t.xyxy(m_vt.m_max.t);

		if(linear)
		{
			st += GSVector4(-0x8000, 0x8000).xxyy();
		}

		GSVector4i uv = GSVector4i(st).sra32(16);

		GSVector4i u, v;

		int mask = 0;

		if(wms == CLAMP_REPEAT || wmt == CLAMP_REPEAT)
		{
			u = uv & GSVector4i::xffffffff().srl32(32 - tw);
			v = uv & GSVector4i::xffffffff().srl32(32 - th);

			GSVector4i uu = uv.sra32(tw);
			GSVector4i vv = uv.sra32(th);

			mask = (uu.upl32(vv) == uu.uph32(vv)).mask();
		}

		uv = uv.rintersect(tr);

		switch(wms)
		{
		case CLAMP_REPEAT:
			if(mask & 0x000f) {if(vr.x < u.x) vr.x = u.x; if(vr.z > u.z + 1) vr.z = u.z + 1;}
			break;
		case CLAMP_CLAMP:
		case CLAMP_REGION_CLAMP:
			if(vr.x < uv.x) vr.x = uv.x;
			if(vr.z > uv.z + 1) vr.z = uv.z + 1;
			break;
		case CLAMP_REGION_REPEAT:
			break;
		default:
			__assume(0);
		}

		switch(wmt)
		{
		case CLAMP_REPEAT:
			if(mask & 0xf000) {if(vr.y < v.y) vr.y = v.y; if(vr.w > v.w + 1) vr.w = v.w + 1;}
			break;
		case CLAMP_CLAMP:
		case CLAMP_REGION_CLAMP:
			if(vr.y < uv.y) vr.y = uv.y;
			if(vr.w > uv.w + 1) vr.w = uv.w + 1;
			break;
		case CLAMP_REGION_REPEAT:
			break;
		default:
			__assume(0);
		}
	}

	r = vr.rintersect(tr);
}

void GSRenderer::GetAlphaMinMax()
{
	if(m_vt.m_alpha.valid)
	{
		return;
	}

	const GSDrawingEnvironment& env = m_env;
	const GSDrawingContext* context = m_context;

	GSVector4i a = m_vt.m_min.c.uph32(m_vt.m_max.c).zzww();

	if(PRIM->TME && context->TEX0.TCC)
	{
		switch(GSLocalMemory::m_psm[context->TEX0.PSM].fmt)
		{
		case 0:
			a.y = 0;
			a.w = 0xff;
			break;
		case 1:
			a.y = env.TEXA.AEM ? 0 : env.TEXA.TA0;
			a.w = env.TEXA.TA0;
			break;
		case 2:
			a.y = env.TEXA.AEM ? 0 : min(env.TEXA.TA0, env.TEXA.TA1);
			a.w = max(env.TEXA.TA0, env.TEXA.TA1);
			break;
		case 3:
			m_mem.m_clut.GetAlphaMinMax32(a.y, a.w);
			break;
		default:
			__assume(0);
		}

		switch(context->TEX0.TFX)
		{
		case TFX_MODULATE:
			a.x = (a.x * a.y) >> 7;
			a.z = (a.z * a.w) >> 7;
			if(a.x > 0xff) a.x = 0xff;
			if(a.z > 0xff) a.z = 0xff;
			break;
		case TFX_DECAL:
			a.x = a.y;
			a.z = a.w;
			break;
		case TFX_HIGHLIGHT:
			a.x = a.x + a.y;
			a.z = a.z + a.w;
			if(a.x > 0xff) a.x = 0xff;
			if(a.z > 0xff) a.z = 0xff;
			break;
		case TFX_HIGHLIGHT2:
			a.x = a.y;
			a.z = a.w;
			break;
		default:
			__assume(0);
		}
	}

	m_vt.m_alpha.min = a.x;
	m_vt.m_alpha.max = a.z;
	m_vt.m_alpha.valid = true;
}

bool GSRenderer::TryAlphaTest(uint32& fm, uint32& zm)
{
	const GSDrawingContext* context = m_context;

	bool pass = true;

	if(context->TEST.ATST == ATST_NEVER)
	{
		pass = false;
	}
	else if(context->TEST.ATST != ATST_ALWAYS)
	{
		GetAlphaMinMax();

		int amin = m_vt.m_alpha.min;
		int amax = m_vt.m_alpha.max;

		int aref = context->TEST.AREF;

		switch(context->TEST.ATST)
		{
		case ATST_NEVER:
			pass = false;
			break;
		case ATST_ALWAYS:
			pass = true;
			break;
		case ATST_LESS:
			if(amax < aref) pass = true;
			else if(amin >= aref) pass = false;
			else return false;
			break;
		case ATST_LEQUAL:
			if(amax <= aref) pass = true;
			else if(amin > aref) pass = false;
			else return false;
			break;
		case ATST_EQUAL:
			if(amin == aref && amax == aref) pass = true;
			else if(amin > aref || amax < aref) pass = false;
			else return false;
			break;
		case ATST_GEQUAL:
			if(amin >= aref) pass = true;
			else if(amax < aref) pass = false;
			else return false;
			break;
		case ATST_GREATER:
			if(amin > aref) pass = true;
			else if(amax <= aref) pass = false;
			else return false;
			break;
		case ATST_NOTEQUAL:
			if(amin == aref && amax == aref) pass = false;
			else if(amin > aref || amax < aref) pass = true;
			else return false;
			break;
		default:
			__assume(0);
		}
	}

	if(!pass)
	{
		switch(context->TEST.AFAIL)
		{
		case AFAIL_KEEP: fm = zm = 0xffffffff; break;
		case AFAIL_FB_ONLY: zm = 0xffffffff; break;
		case AFAIL_ZB_ONLY: fm = 0xffffffff; break;
		case AFAIL_RGB_ONLY: fm |= 0xff000000; zm = 0xffffffff; break;
		default: __assume(0);
		}
	}

	return true;
}

bool GSRenderer::IsOpaque()
{
	if(PRIM->AA1)
	{
		return false;
	}

	if(!PRIM->ABE)
	{
		return true;
	}

	const GSDrawingContext* context = m_context;

	int amin = 0, amax = 0xff;

	if(context->ALPHA.A != context->ALPHA.B)
	{
		if(context->ALPHA.C == 0)
		{
			GetAlphaMinMax();

			amin = m_vt.m_alpha.min;
			amax = m_vt.m_alpha.max;
		}
		else if(context->ALPHA.C == 1)
		{
			if(context->FRAME.PSM == PSM_PSMCT24 || context->FRAME.PSM == PSM_PSMZ24)
			{
				amin = amax = 0x80;
			}
		}
		else if(context->ALPHA.C == 2)
		{
			amin = amax = context->ALPHA.FIX;
		}
	}

	return context->ALPHA.IsOpaque(amin, amax);
}
