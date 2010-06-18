/*	ZeroGS KOSMOS
 *	Copyright (C) 2005-2006 zerofrog@gmail.com
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	02111-1307	USA
 */

#ifndef GIFTRANSFER_H_INCLUDED
#define GIFTRANSFER_H_INCLUDED

#include "Regs.h"
#include "Util.h"

// If you notice bugs in the newest revisions, you might try disabling this,
// to see if they are related.
#define NEW_GIF_TRANSFER
enum GIF_FLG
{
	GIF_FLG_PACKED	= 0,
	GIF_FLG_REGLIST	= 1,
	GIF_FLG_IMAGE	= 2,
	GIF_FLG_IMAGE2	= 3
};

//
// GIFTag
union GIFTag
{
	u64 ai64[2];
	u32 ai32[4];

	struct
	{
		u32 NLOOP : 15;
		u32 EOP : 1;
		u32 _PAD1 : 16;
		u32 _PAD2 : 14;
		u32 PRE : 1;
		u32 PRIM : 11;
		u32 FLG : 2; // enum GIF_FLG
		u32 NREG : 4;
		u64 REGS : 64;
	};

	void set(u32 *data)
	{
		for (int i = 0; i <= 3; i++)
		{
			ai32[i] = data[i];
		}
	}

	GIFTag(u32 *data)
	{
		set(data);
	}

	GIFTag(){ ai64[0] = 0; ai64[1] = 0; }
};

// EE part. Data transfer packet description

typedef struct
{
#ifdef NEW_GIF_TRANSFER
	u32 mode;
	int reg;
	u64 regs;
	u32 nloop;
	int eop;
	int nreg;
	u32 adonly;
	GIFTag tag;
#else
	int mode;
	int regn;
	u64 regs;
	int nloop;
	int eop;
	int nreg;
	u32 adonly;
	GIFTag tag;
#endif

#ifdef NEW_GIF_TRANSFER
	void setTag(u32 *data)
	{
		tag.set(data);

		nloop	= tag.NLOOP;
		eop		= tag.EOP;
		mode	= tag.FLG;

		// Hmm....
		nreg	= tag.NREG << 2;
		if (nreg == 0) nreg = 64;
		regs = tag.REGS;
		reg = 0;

		//      ZZLog::GS_Log("GIFtag: %8.8lx_%8.8lx_%8.8lx_%8.8lx: EOP=%d, NLOOP=%x, FLG=%x, NREG=%d, PRE=%d",
		//                      data[3], data[2], data[1], data[0],
		//                      path->eop, path->nloop, mode, path->nreg, tag.PRE);
	}

	u32 GetReg()
	{
		return (regs >> reg) & 0xf;
	}

	bool StepReg()
	{
		reg += 4;

		if (reg == nreg)
		{
			reg = 0;
			nloop--;

			if (nloop == 0) return false;
		}

		return true;
	}

#else
	void setTag(u32 *data)
	{
		tag.set(data);

		nloop   = tag.NLOOP;
		eop     = tag.EOP;
		u32 tagpre              = tag.PRE;
		u32 tagprim             = tag.PRIM;
		u32 tagflg              = tag.FLG;

		// Hmm....
		nreg    = tag.NREG << 2;
		if (nreg == 0) nreg = 64;

		//      ZZLog::GS_Log("GIFtag: %8.8lx_%8.8lx_%8.8lx_%8.8lx: EOP=%d, NLOOP=%x, FLG=%x, NREG=%d, PRE=%d",
		//                      data[3], data[2], data[1], data[0],
		//                      path->eop, path->nloop, tagflg, path->nreg, tagpre);

		mode = tagflg;

		switch (mode)
		{
			case GIF_FLG_PACKED:
				regs = *(u64 *)(data + 2);
				regn = 0;
				if (tagpre) GIFRegHandlerPRIM((u32*)&tagprim);
				break;

			case GIF_FLG_REGLIST:
				regs = *(u64 *)(data + 2);
				regn = 0;
				break;
		}
	}

#endif
} pathInfo;

void _GSgifPacket(pathInfo *path, u32 *pMem);
void _GSgifRegList(pathInfo *path, u32 *pMem);
void _GSgifTransfer(pathInfo *path, u32 *pMem, u32 size);

extern GIFRegHandler g_GIFPackedRegHandlers[];
extern GIFRegHandler g_GIFRegHandlers[];

#endif // GIFTRANSFER_H_INCLUDED
