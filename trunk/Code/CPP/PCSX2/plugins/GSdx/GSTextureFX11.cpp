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
#include "GSDevice11.h"
#include "resource.h"

bool GSDevice11::CreateTextureFX()
{
	HRESULT hr;

	D3D11_BUFFER_DESC bd;

	memset(&bd, 0, sizeof(bd));

	bd.ByteWidth = sizeof(VSConstantBuffer);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_dev->CreateBuffer(&bd, NULL, &m_vs_cb);

	if(FAILED(hr)) return false;

	memset(&bd, 0, sizeof(bd));

	bd.ByteWidth = sizeof(PSConstantBuffer);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = m_dev->CreateBuffer(&bd, NULL, &m_ps_cb);

	if(FAILED(hr)) return false;

	D3D11_SAMPLER_DESC sd;

	memset(&sd, 0, sizeof(sd));

	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.MaxLOD = FLT_MAX;
	sd.MaxAnisotropy = 16; 
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;

	hr = m_dev->CreateSamplerState(&sd, &m_palette_ss);

	if(FAILED(hr)) return false;

	// create layout

	VSSelector sel;
	VSConstantBuffer cb;

	SetupVS(sel, &cb);

	//

	return true;
}

void GSDevice11::SetupIA(const void* vertices, int count, int prim)
{
	IASetVertexBuffer(vertices, sizeof(GSVertexHW11), count);
	IASetInputLayout(m_il);
	IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)prim);
}

void GSDevice11::SetupVS(VSSelector sel, const VSConstantBuffer* cb)
{
	hash_map<uint32, CComPtr<ID3D11VertexShader> >::const_iterator i = m_vs.find(sel);

	if(i == m_vs.end())
	{
		string str[3];

		str[0] = format("%d", sel.bppz);
		str[1] = format("%d", sel.tme);
		str[2] = format("%d", sel.fst);

		D3D11_SHADER_MACRO macro[] =
		{
			{"VS_BPPZ", str[0].c_str()},
			{"VS_TME", str[1].c_str()},
			{"VS_FST", str[2].c_str()},
			{NULL, NULL},
		};

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"POSITION", 0, DXGI_FORMAT_R16G16_UINT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"POSITION", 1, DXGI_FORMAT_R32_UINT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 1, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		CComPtr<ID3D11InputLayout> il;
		CComPtr<ID3D11VertexShader> vs;

		CompileShader(IDR_TFX_FX, "vs_main", macro, &vs, layout, countof(layout), &il);

		if(m_il == NULL)
		{
			m_il = il;
		}

		m_vs[sel] = vs;

		i = m_vs.find(sel);
	}

	if(m_vs_cb_cache.Update(cb))
	{
		ID3D11DeviceContext* ctx = m_ctx;

		ctx->UpdateSubresource(m_vs_cb, 0, NULL, cb, 0, 0);
	}

	VSSetShader(i->second, m_vs_cb);
}

void GSDevice11::SetupGS(GSSelector sel)
{
	ID3D11GeometryShader* gs = NULL;

	if(sel.prim > 0 && (sel.iip == 0 || sel.prim == 3)) // geometry shader works in every case, but not needed
	{
		hash_map<uint32, CComPtr<ID3D11GeometryShader> >::const_iterator i = m_gs.find(sel);

		if(i != m_gs.end())
		{
			gs = i->second;
		}
		else
		{
			string str[2];

			str[0] = format("%d", sel.iip);
			str[1] = format("%d", sel.prim);

			D3D11_SHADER_MACRO macro[] =
			{
				{"GS_IIP", str[0].c_str()},
				{"GS_PRIM", str[1].c_str()},
				{NULL, NULL},
			};

			CompileShader(IDR_TFX_FX, "gs_main", macro, &gs);

			m_gs[sel] = gs;
		}
	}

	GSSetShader(gs);
}

void GSDevice11::SetupPS(PSSelector sel, const PSConstantBuffer* cb, PSSamplerSelector ssel)
{
	hash_map<uint32, CComPtr<ID3D11PixelShader> >::const_iterator i = m_ps.find(sel);

	if(i == m_ps.end())
	{
		string str[13];

		str[0] = format("%d", sel.fst);
		str[1] = format("%d", sel.wms);
		str[2] = format("%d", sel.wmt);
		str[3] = format("%d", sel.fmt);
		str[4] = format("%d", sel.aem);
		str[5] = format("%d", sel.tfx);
		str[6] = format("%d", sel.tcc);
		str[7] = format("%d", sel.atst);
		str[8] = format("%d", sel.fog);
		str[9] = format("%d", sel.clr1);
		str[10] = format("%d", sel.fba);
		str[11] = format("%d", sel.aout);
		str[12] = format("%d", sel.ltf);

		D3D11_SHADER_MACRO macro[] =
		{
			{"PS_FST", str[0].c_str()},
			{"PS_WMS", str[1].c_str()},
			{"PS_WMT", str[2].c_str()},
			{"PS_FMT", str[3].c_str()},
			{"PS_AEM", str[4].c_str()},
			{"PS_TFX", str[5].c_str()},
			{"PS_TCC", str[6].c_str()},
			{"PS_ATST", str[7].c_str()},
			{"PS_FOG", str[8].c_str()},
			{"PS_CLR1", str[9].c_str()},
			{"PS_FBA", str[10].c_str()},
			{"PS_AOUT", str[11].c_str()},
			{"PS_LTF", str[12].c_str()},
			{NULL, NULL},
		};

		CComPtr<ID3D11PixelShader> ps;
		
		CompileShader(IDR_TFX_FX, "ps_main", macro, &ps);

		m_ps[sel] = ps;

		i = m_ps.find(sel);
	}

	if(m_ps_cb_cache.Update(cb))
	{
		ID3D11DeviceContext* ctx = m_ctx;

		ctx->UpdateSubresource(m_ps_cb, 0, NULL, cb, 0, 0);
	}

	PSSetShader(i->second, m_ps_cb);

	ID3D11SamplerState* ss0 = NULL;
	ID3D11SamplerState* ss1 = NULL;

	if(sel.tfx != 4)
	{
		if(!(sel.fmt < 3 && sel.wms < 3 && sel.wmt < 3))
		{
			ssel.ltf = 0;
		}

		hash_map<uint32, CComPtr<ID3D11SamplerState> >::const_iterator i = m_ps_ss.find(ssel);

		if(i != m_ps_ss.end())
		{
			ss0 = i->second;
		}
		else
		{
			D3D11_SAMPLER_DESC sd;

			memset(&sd, 0, sizeof(sd));

			sd.Filter = ssel.ltf ? D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;

			sd.AddressU = ssel.tau ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.AddressV = ssel.tav ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
			sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

			sd.MaxLOD = FLT_MAX;
			sd.MaxAnisotropy = 16; 
			sd.ComparisonFunc = D3D11_COMPARISON_NEVER;

			m_dev->CreateSamplerState(&sd, &ss0);

			m_ps_ss[ssel] = ss0;
		}

		if(sel.fmt >= 3)
		{
			ss1 = m_palette_ss;
		}
	}

	PSSetSamplerState(ss0, ss1);
}

void GSDevice11::SetupOM(OMDepthStencilSelector dssel, OMBlendSelector bsel, uint8 afix)
{
	hash_map<uint32, CComPtr<ID3D11DepthStencilState> >::const_iterator i = m_om_dss.find(dssel);

	if(i == m_om_dss.end())
	{
		D3D11_DEPTH_STENCIL_DESC dsd;

		memset(&dsd, 0, sizeof(dsd));

		if(dssel.date)
		{
			dsd.StencilEnable = true;
			dsd.StencilReadMask = 1;
			dsd.StencilWriteMask = 1;
			dsd.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
			dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		}

		if(dssel.ztst != ZTST_ALWAYS || dssel.zwe)
		{
			static const D3D11_COMPARISON_FUNC ztst[] = 
			{
				D3D11_COMPARISON_NEVER, 
				D3D11_COMPARISON_ALWAYS, 
				D3D11_COMPARISON_GREATER_EQUAL, 
				D3D11_COMPARISON_GREATER
			};

			dsd.DepthEnable = true;
			dsd.DepthWriteMask = dssel.zwe ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
			dsd.DepthFunc = ztst[dssel.ztst];
		}

		CComPtr<ID3D11DepthStencilState> dss;

		m_dev->CreateDepthStencilState(&dsd, &dss);

		m_om_dss[dssel] = dss;

		i = m_om_dss.find(dssel);
	}

	OMSetDepthStencilState(i->second, 1);

	hash_map<uint32, CComPtr<ID3D11BlendState> >::const_iterator j = m_om_bs.find(bsel);

	if(j == m_om_bs.end())
	{
		D3D11_BLEND_DESC bd;

		memset(&bd, 0, sizeof(bd));

		bd.RenderTarget[0].BlendEnable = bsel.abe;

		if(bsel.abe)
		{
			// (A:Cs/Cd/0 - B:Cs/Cd/0) * C:As/Ad/FIX + D:Cs/Cd/0

			static const struct {int bogus; D3D11_BLEND_OP op; D3D11_BLEND src, dst;} map[3*3*3*3] =
			{
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 0000: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 0001: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 0002: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 0010: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 0011: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 0012: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 0020: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 0021: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 0022: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{1, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_SRC1_ALPHA},		// * 0100: (Cs - Cd)*As + Cs ==> Cs*(As + 1) - Cd*As
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_INV_SRC1_ALPHA},		// 0101: (Cs - Cd)*As + Cd ==> Cs*As + Cd*(1 - As)
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_SRC1_ALPHA},		// 0102: (Cs - Cd)*As + 0 ==> Cs*As - Cd*As
				{1, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA},		// * 0110: (Cs - Cd)*Ad + Cs ==> Cs*(Ad + 1) - Cd*Ad
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA},		// 0111: (Cs - Cd)*Ad + Cd ==> Cs*Ad + Cd*(1 - Ad)
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA},			// 0112: (Cs - Cd)*Ad + 0 ==> Cs*Ad - Cd*Ad
				{1, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_BLEND_FACTOR},	// * 0120: (Cs - Cd)*F + Cs ==> Cs*(F + 1) - Cd*F
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_INV_BLEND_FACTOR},	// 0121: (Cs - Cd)*F + Cd ==> Cs*F + Cd*(1 - F)
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_BLEND_FACTOR},	// 0122: (Cs - Cd)*F + 0 ==> Cs*F - Cd*F
				{1, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_ZERO},					// * 0200: (Cs - 0)*As + Cs ==> Cs*(As + 1)
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_ONE},					// 0201: (Cs - 0)*As + Cd ==> Cs*As + Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_ZERO},					// 0202: (Cs - 0)*As + 0 ==> Cs*As
				{1, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_ZERO},					// * 0210: (Cs - 0)*Ad + Cs ==> Cs*(Ad + 1)
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ONE},					// 0211: (Cs - 0)*Ad + Cd ==> Cs*Ad + Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO},					// 0212: (Cs - 0)*Ad + 0 ==> Cs*Ad
				{1, D3D11_BLEND_OP_ADD, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_ZERO},				// * 0220: (Cs - 0)*F + Cs ==> Cs*(F + 1)
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_ONE},					// 0221: (Cs - 0)*F + Cd ==> Cs*F + Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_ZERO},				// 0222: (Cs - 0)*F + 0 ==> Cs*F
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_SRC1_ALPHA, D3D11_BLEND_SRC1_ALPHA},		// 1000: (Cd - Cs)*As + Cs ==> Cd*As + Cs*(1 - As)
				{1, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_SRC1_ALPHA},	// * 1001: (Cd - Cs)*As + Cd ==> Cd*(As + 1) - Cs*As
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_SRC1_ALPHA},	// 1002: (Cd - Cs)*As + 0 ==> Cd*As - Cs*As
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA},		// 1010: (Cd - Cs)*Ad + Cs ==> Cd*Ad + Cs*(1 - Ad)
				{1, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA},	// * 1011: (Cd - Cs)*Ad + Cd ==> Cd*(Ad + 1) - Cs*Ad
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA},	// 1012: (Cd - Cs)*Ad + 0 ==> Cd*Ad - Cs*Ad
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_BLEND_FACTOR, D3D11_BLEND_BLEND_FACTOR},	// 1020: (Cd - Cs)*F + Cs ==> Cd*F + Cs*(1 - F)
				{1, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_BLEND_FACTOR},// * 1021: (Cd - Cs)*F + Cd ==> Cd*(F + 1) - Cs*F
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_BLEND_FACTOR},// 1022: (Cd - Cs)*F + 0 ==> Cd*F - Cs*F
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 1100: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 1101: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 1102: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 1110: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 1111: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 1112: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 1120: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 1121: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 1122: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_SRC1_ALPHA},					// 1200: (Cd - 0)*As + Cs ==> Cs + Cd*As
				{2, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC1_ALPHA},			// ** 1201: (Cd - 0)*As + Cd ==> Cd*(1 + As)  // ffxii main menu background glow effect
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_SRC1_ALPHA},					// 1202: (Cd - 0)*As + 0 ==> Cd*As
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_DEST_ALPHA},					// 1210: (Cd - 0)*Ad + Cs ==> Cs + Cd*Ad
				{2, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_DEST_ALPHA},			// ** 1211: (Cd - 0)*Ad + Cd ==> Cd*(1 + Ad)
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_DEST_ALPHA},					// 1212: (Cd - 0)*Ad + 0 ==> Cd*Ad
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_BLEND_FACTOR},					// 1220: (Cd - 0)*F + Cs ==> Cs + Cd*F
				{2, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_BLEND_FACTOR},			// ** 1221: (Cd - 0)*F + Cd ==> Cd*(1 + F)
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_BLEND_FACTOR},				// 1222: (Cd - 0)*F + 0 ==> Cd*F
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_SRC1_ALPHA, D3D11_BLEND_ZERO},				// 2000: (0 - Cs)*As + Cs ==> Cs*(1 - As)
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_ONE},			// 2001: (0 - Cs)*As + Cd ==> Cd - Cs*As
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_SRC1_ALPHA, D3D11_BLEND_ZERO},			// 2002: (0 - Cs)*As + 0 ==> 0 - Cs*As
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ZERO},				// 2010: (0 - Cs)*Ad + Cs ==> Cs*(1 - Ad)
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ONE},			// 2011: (0 - Cs)*Ad + Cd ==> Cd - Cs*Ad
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO},			// 2012: (0 - Cs)*Ad + 0 ==> 0 - Cs*Ad
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_BLEND_FACTOR, D3D11_BLEND_ZERO},			// 2020: (0 - Cs)*F + Cs ==> Cs*(1 - F)
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_ONE},		// 2021: (0 - Cs)*F + Cd ==> Cd - Cs*F
				{0, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_ZERO},		// 2022: (0 - Cs)*F + 0 ==> 0 - Cs*F
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_SRC1_ALPHA},				// 2100: (0 - Cd)*As + Cs ==> Cs - Cd*As
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_INV_SRC1_ALPHA},				// 2101: (0 - Cd)*As + Cd ==> Cd*(1 - As)
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_ZERO, D3D11_BLEND_SRC1_ALPHA},				// 2102: (0 - Cd)*As + 0 ==> 0 - Cd*As
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_DEST_ALPHA},				// 2110: (0 - Cd)*Ad + Cs ==> Cs - Cd*Ad
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_INV_DEST_ALPHA},				// 2111: (0 - Cd)*Ad + Cd ==> Cd*(1 - Ad)
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_DEST_ALPHA},				// 2112: (0 - Cd)*Ad + 0 ==> 0 - Cd*Ad
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_BLEND_FACTOR},			// 2120: (0 - Cd)*F + Cs ==> Cs - Cd*F
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_INV_BLEND_FACTOR},			// 2121: (0 - Cd)*F + Cd ==> Cd*(1 - F)
				{0, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_BLEND_FACTOR},			// 2122: (0 - Cd)*F + 0 ==> 0 - Cd*F
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 2200: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 2201: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 2202: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 2210: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 2211: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 2212: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO},							// 2220: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cs ==> Cs
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ONE},							// 2221: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + Cd ==> Cd
				{0, D3D11_BLEND_OP_ADD, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO},						// 2222: (Cs/Cd/0 - Cs/Cd/0)*As/Ad/F + 0 ==> 0
			};

			// bogus: 0100, 0110, 0120, 0200, 0210, 0220, 1001, 1011, 1021

			// tricky: 1201, 1211, 1221
			//
			// Source.rgb = float3(1, 1, 1);
			// 1201 Cd*(1 + As) => Source * Dest color + Dest * Source1 alpha
			// 1211 Cd*(1 + Ad) => Source * Dest color + Dest * Dest alpha
			// 1221 Cd*(1 + F) => Source * Dest color + Dest * Factor

			int i = ((bsel.a * 3 + bsel.b) * 3 + bsel.c) * 3 + bsel.d;

			bd.RenderTarget[0].BlendOp = map[i].op;
			bd.RenderTarget[0].SrcBlend = map[i].src;
			bd.RenderTarget[0].DestBlend = map[i].dst;
			bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

			if(map[i].bogus == 1)
			{
				ASSERT(0);

				(bsel.a == 0 ? bd.RenderTarget[0].SrcBlend : bd.RenderTarget[0].DestBlend) = D3D11_BLEND_ONE;
			}
		}

		if(bsel.wr) bd.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
		if(bsel.wg) bd.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		if(bsel.wb) bd.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		if(bsel.wa) bd.RenderTarget[0].RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

		CComPtr<ID3D11BlendState> bs;

		m_dev->CreateBlendState(&bd, &bs);

		m_om_bs[bsel] = bs;

		j = m_om_bs.find(bsel);
	}

	OMSetBlendState(j->second, (float)(int)afix / 0x80);
}
