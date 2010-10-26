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

#include "GS.h"
#include "GSLocalMemory.h"
#include "GSDrawingContext.h"
#include "GSDrawingEnvironment.h"
#include "GSVertex.h"
#include "GSVertexList.h"
#include "GSUtil.h"
#include "GSDirtyRect.h"
#include "GSPerfMon.h"
#include "GSVector.h"
#include "GSDevice.h"
#include "GSCrc.h"
#include "GSAlignedClass.h"
#include "GSDump.h"

// Set this to 1 to enable a switch statement instead of a LUT for the packed register handler
// in the GifTransfer code.  Switch statement is probably faster, but it isn't fully implemented
// yet (not properly supporting frameskipping).
#define UsePackedRegSwitch 0

class GSState : public GSAlignedClass<16>
{
#if !UsePackedRegSwitch
	typedef void (GSState::*GIFPackedRegHandler)(const GIFPackedReg* r);
	GIFPackedRegHandler m_fpGIFPackedRegHandlers[16];
#endif

	void GIFPackedRegHandlerNull(const GIFPackedReg* r);
	void GIFPackedRegHandlerRGBA(const GIFPackedReg* r);
	void GIFPackedRegHandlerSTQ(const GIFPackedReg* r);
	void GIFPackedRegHandlerUV(const GIFPackedReg* r);
	void GIFPackedRegHandlerXYZF2(const GIFPackedReg* r);
	void GIFPackedRegHandlerXYZ2(const GIFPackedReg* r);
	void GIFPackedRegHandlerFOG(const GIFPackedReg* r);
	void GIFPackedRegHandlerA_D(const GIFPackedReg* r);
	void GIFPackedRegHandlerNOP(const GIFPackedReg* r);

	typedef void (GSState::*GIFRegHandler)(const GIFReg* r);

	GIFRegHandler m_fpGIFRegHandlers[256];

	void ApplyTEX0( uint i, GIFRegTEX0& TEX0 );
	void ApplyPRIM(const GIFRegPRIM& PRIM);

	void GIFRegHandlerNull(const GIFReg* r);
	void GIFRegHandlerPRIM(const GIFReg* r);
	void GIFRegHandlerRGBAQ(const GIFReg* r);
	void GIFRegHandlerST(const GIFReg* r);
	void GIFRegHandlerUV(const GIFReg* r);
	void GIFRegHandlerXYZF2(const GIFReg* r);
	void GIFRegHandlerXYZ2(const GIFReg* r);
	template<int i> void GIFRegHandlerTEX0(const GIFReg* r);
	template<int i> void GIFRegHandlerCLAMP(const GIFReg* r);
	void GIFRegHandlerFOG(const GIFReg* r);
	void GIFRegHandlerXYZF3(const GIFReg* r);
	void GIFRegHandlerXYZ3(const GIFReg* r);
	void GIFRegHandlerNOP(const GIFReg* r);
	template<int i> void GIFRegHandlerTEX1(const GIFReg* r);
	template<int i> void GIFRegHandlerTEX2(const GIFReg* r);
	template<int i> void GIFRegHandlerXYOFFSET(const GIFReg* r);
	void GIFRegHandlerPRMODECONT(const GIFReg* r);
	void GIFRegHandlerPRMODE(const GIFReg* r);
	void GIFRegHandlerTEXCLUT(const GIFReg* r);
	void GIFRegHandlerSCANMSK(const GIFReg* r);
	template<int i> void GIFRegHandlerMIPTBP1(const GIFReg* r);
	template<int i> void GIFRegHandlerMIPTBP2(const GIFReg* r);
	void GIFRegHandlerTEXA(const GIFReg* r);
	void GIFRegHandlerFOGCOL(const GIFReg* r);
	void GIFRegHandlerTEXFLUSH(const GIFReg* r);
	template<int i> void GIFRegHandlerSCISSOR(const GIFReg* r);
	template<int i> void GIFRegHandlerALPHA(const GIFReg* r);
	void GIFRegHandlerDIMX(const GIFReg* r);
	void GIFRegHandlerDTHE(const GIFReg* r);
	void GIFRegHandlerCOLCLAMP(const GIFReg* r);
	template<int i> void GIFRegHandlerTEST(const GIFReg* r);
	void GIFRegHandlerPABE(const GIFReg* r);
	template<int i> void GIFRegHandlerFBA(const GIFReg* r);
	template<int i> void GIFRegHandlerFRAME(const GIFReg* r);
	template<int i> void GIFRegHandlerZBUF(const GIFReg* r);
	void GIFRegHandlerBITBLTBUF(const GIFReg* r);
	void GIFRegHandlerTRXPOS(const GIFReg* r);
	void GIFRegHandlerTRXREG(const GIFReg* r);
	void GIFRegHandlerTRXDIR(const GIFReg* r);
	void GIFRegHandlerHWREG(const GIFReg* r);
	void GIFRegHandlerSIGNAL(const GIFReg* r);
	void GIFRegHandlerFINISH(const GIFReg* r);
	void GIFRegHandlerLABEL(const GIFReg* r);

	int m_version;
	int m_sssize;

	bool m_mt;
	void (*m_irq)();
	bool m_path3hack;

	struct GSTransferBuffer
	{
		int x, y;
		int start, end, total;
		bool overflow;
		uint8* buff;

		GSTransferBuffer();
		virtual ~GSTransferBuffer();

		void Init(int tx, int ty);
		bool Update(int tw, int th, int bpp, int& len);

	} m_tr;

	void FlushWrite();

protected:
	bool IsBadFrame(int& skip, int UserHacks_SkipDraw);

	typedef void (GSState::*DrawingKickPtr)(bool skip);

	DrawingKickPtr m_dk[8];

	template<class T> void InitVertexKick()
	{
		m_dk[GS_POINTLIST]			= (DrawingKickPtr)&T::DrawingKick<GS_POINTLIST>;
		m_dk[GS_LINELIST]			= (DrawingKickPtr)&T::DrawingKick<GS_LINELIST>;
		m_dk[GS_LINESTRIP]			= (DrawingKickPtr)&T::DrawingKick<GS_LINESTRIP>;
		m_dk[GS_TRIANGLELIST]		= (DrawingKickPtr)&T::DrawingKick<GS_TRIANGLELIST>;
		m_dk[GS_TRIANGLESTRIP]		= (DrawingKickPtr)&T::DrawingKick<GS_TRIANGLESTRIP>;
		m_dk[GS_TRIANGLEFAN]		= (DrawingKickPtr)&T::DrawingKick<GS_TRIANGLEFAN>;
		m_dk[GS_SPRITE]				= (DrawingKickPtr)&T::DrawingKick<GS_SPRITE>;
		m_dk[GS_INVALID]			= &GSState::DrawingKickNull;
	}

	void DrawingKickNull(bool skip)
	{
		ASSERT(0);
	}

	virtual void DoVertexKick()=0;

	__fi void VertexKick(bool skip)
	{
		DoVertexKick();
		(this->*m_dk[PRIM->PRIM])(skip);
	}

public:
	GIFPath m_path[4];
	GIFRegPRIM* PRIM;
	GSPrivRegSet* m_regs;
	GSLocalMemory m_mem;
	GSDrawingEnvironment m_env;
	GSDrawingContext* m_context;
	GSVertex m_v;
	float m_q;
	uint32 m_vprim;

	GSPerfMon m_perfmon;
	uint32 m_crc;
	int m_options;
	int m_frameskip;
	bool m_framelimit;
	CRC::Game m_game;
	GSDump m_dump;

public:
	GSState();
	virtual ~GSState();

	void ResetHandlers();

	GSVector4i GetDisplayRect(int i = -1);
	GSVector4i GetFrameRect(int i = -1);
	GSVector2i GetDeviceSize(int i = -1);

	bool IsEnabled(int i);

	int GetFPS();

	virtual void Reset();
	virtual void Flush();
	virtual void FlushPrim() = 0;
	virtual void ResetPrim() = 0;
	virtual void InvalidateVideoMem(const GIFRegBITBLTBUF& BITBLTBUF, const GSVector4i& r) {}
	virtual void InvalidateLocalMem(const GIFRegBITBLTBUF& BITBLTBUF, const GSVector4i& r) {}
	virtual void InvalidateTextureCache() {}

	void Move();
	void Write(const uint8* mem, int len);
	void Read(uint8* mem, int len);

	void SoftReset(uint32 mask);
	void WriteCSR(uint32 csr) {m_regs->CSR.u32[1] = csr;}
	void ReadFIFO(uint8* mem, int size);
	template<int index> void Transfer(const uint8* mem, uint32 size);
	int Freeze(GSFreezeData* fd, bool sizeonly);
	int Defrost(const GSFreezeData* fd);
	void GetLastTag(uint32* tag) {*tag = m_path3hack; m_path3hack = 0;}
	virtual void SetGameCRC(uint32 crc, int options);
	void SetFrameSkip(int skip);
	void SetRegsMem(uint8* basemem);
	void SetIrqCallback(void (*irq)());
	void SetMultithreaded(bool isMT=true);
};

