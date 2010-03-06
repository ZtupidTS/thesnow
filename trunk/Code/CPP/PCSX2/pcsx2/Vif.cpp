/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2009  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "Common.h"
#include "Vif.h"
#include "Vif_Dma.h"
#include "newVif.h"
#include "GS.h"
#include "Gif.h"

vifStruct  vif0;
vifStruct  vif1;
Path3Modes Path3progress = STOPPED_MODE;

void vif0Init() { initNewVif(0); }
void vif1Init() { initNewVif(1); }

void vif0Reset()
{
	/* Reset the whole VIF, meaning the internal pcsx2 vars and all the registers */
	memzero(vif0);
	memzero(*vif0Regs);

	psHu64(VIF0_FIFO) = 0;
	psHu64(VIF0_FIFO + 8) = 0;
	
	vif0Regs->stat.VPS = VPS_IDLE;
	vif0Regs->stat.FQC = 0;

	vif0.done = true;

	resetNewVif(0);
}

void vif1Reset()
{
	/* Reset the whole VIF, meaning the internal pcsx2 vars, and all the registers */
	memzero(vif1);
	memzero(*vif1Regs);

	psHu64(VIF1_FIFO) = 0;
	psHu64(VIF1_FIFO + 8) = 0;

	vif1Regs->stat.VPS = VPS_IDLE;
	vif1Regs->stat.FQC = 0; // FQC=0

	vif1.done = true;
	cpuRegs.interrupt &= ~((1 << 1) | (1 << 10)); //Stop all vif1 DMA's

	resetNewVif(1);
}

void SaveStateBase::vif0Freeze()
{
	FreezeTag("VIFdma");
	Freeze(g_vifCycles); // Dunno if this one is needed, but whatever, it's small. :)
	Freeze(g_vifmask);	 // mask settings for VIF0 and VIF1
	Freeze(vif0);

	Freeze(nVif[0].bSize);
	FreezeMem(nVif[0].buffer, nVif[0].bSize);
}

void SaveStateBase::vif1Freeze()
{
	Freeze(vif1);

	Freeze(nVif[1].bSize);
	FreezeMem(nVif[1].buffer, nVif[1].bSize);
}

//------------------------------------------------------------------
// Vif0/Vif1 Write32
//------------------------------------------------------------------

extern bool _chainVIF0();
extern bool _VIF0chain();

_f void vif0FBRST(u32 value) {
	VIF_LOG("VIF0_FBRST write32 0x%8.8x", value);

	if (value & 0x1) // Reset Vif.
	{
		//Console.WriteLn("Vif0 Reset %x", vif0Regs->stat._u32);

		memzero(vif0);
		vif0ch->qwc = 0; //?
		cpuRegs.interrupt &= ~1; //Stop all vif0 DMA's
		psHu64(VIF0_FIFO) = 0;
		psHu64(VIF0_FIFO + 8) = 0;
		vif0.done = true;
		vif0Regs->err.reset();
		vif0Regs->stat.clear_flags(VIF0_STAT_FQC | VIF0_STAT_INT | VIF0_STAT_VSS | VIF0_STAT_VIS | VIF0_STAT_VFS | VIF0_STAT_VPS); // FQC=0
	}

	if (value & 0x2) // Forcebreak Vif,
	{
		/* I guess we should stop the VIF dma here, but not 100% sure (linuz) */
		cpuRegs.interrupt &= ~1; //Stop all vif0 DMA's
		vif0Regs->stat.VFS = true;
		vif0Regs->stat.VPS = VPS_IDLE;
		vif0.vifstalled = true;
		Console.WriteLn("vif0 force break");
	}

	if (value & 0x4) // Stop Vif.
	{
		// Not completely sure about this, can't remember what game, used this, but 'draining' the VIF helped it, instead of
		//  just stoppin the VIF (linuz).
		vif0Regs->stat.VSS = true;
		vif0Regs->stat.VPS = VPS_IDLE;
		vif0.vifstalled = true;
	}

	if (value & 0x8) // Cancel Vif Stall.
	{
		bool cancel = false;

		/* Cancel stall, first check if there is a stall to cancel, and then clear VIF0_STAT VSS|VFS|VIS|INT|ER0|ER1 bits */
		if (vif0Regs->stat.test(VIF0_STAT_VSS | VIF0_STAT_VIS | VIF0_STAT_VFS))
			cancel = true;

		vif0Regs->stat.clear_flags(VIF0_STAT_VSS | VIF0_STAT_VFS | VIF0_STAT_VIS |
				    VIF0_STAT_INT | VIF0_STAT_ER0 | VIF0_STAT_ER1);
		if (cancel)
		{
			if (vif0.vifstalled)
			{
				g_vifCycles = 0;

				// loop necessary for spiderman
				if (vif0.stallontag)
					_chainVIF0();
				else
					_VIF0chain();

				vif0ch->chcr.STR = true;
				CPU_INT(DMAC_VIF0, g_vifCycles); // Gets the timing right - Flatout
			}
		}
	}
}

_f void vif1FBRST(u32 value) {
	VIF_LOG("VIF1_FBRST write32 0x%8.8x", value);

	if (FBRST(value).RST) // Reset Vif.
	{
		memzero(vif1);
		cpuRegs.interrupt &= ~((1 << 1) | (1 << 10)); //Stop all vif1 DMA's
		vif1ch->qwc = 0; //?
		psHu64(VIF1_FIFO) = 0;
		psHu64(VIF1_FIFO + 8) = 0;
		vif1.done = true;

		if(vif1Regs->mskpath3)
		{
			vif1Regs->mskpath3 = 0;
			gifRegs->stat.IMT = false;
			if (gif->chcr.STR) CPU_INT(DMAC_GIF, 4);
		}

		vif1Regs->err.reset();
		vif1.inprogress = 0;
		vif1Regs->stat.FQC = 0;
		vif1Regs->stat.clear_flags(VIF1_STAT_FDR | VIF1_STAT_INT | VIF1_STAT_VSS | VIF1_STAT_VIS | VIF1_STAT_VFS | VIF1_STAT_VPS);
	}

	if (FBRST(value).FBK) // Forcebreak Vif.
	{
		/* I guess we should stop the VIF dma here, but not 100% sure (linuz) */
		vif1Regs->stat.VFS = true;
		vif1Regs->stat.VPS = VPS_IDLE;
		cpuRegs.interrupt &= ~((1 << 1) | (1 << 10)); //Stop all vif1 DMA's
		vif1.vifstalled = true;
		Console.WriteLn("vif1 force break");
	}

	if (FBRST(value).STP) // Stop Vif.
	{
		// Not completely sure about this, can't remember what game used this, but 'draining' the VIF helped it, instead of
		//   just stoppin the VIF (linuz).
		vif1Regs->stat.VSS = true;
		vif1Regs->stat.VPS = VPS_IDLE;
		cpuRegs.interrupt &= ~((1 << 1) | (1 << 10)); //Stop all vif1 DMA's
		vif1.vifstalled = true;
	}

	if (FBRST(value).STC) // Cancel Vif Stall.
	{
		bool cancel = false;

		/* Cancel stall, first check if there is a stall to cancel, and then clear VIF1_STAT VSS|VFS|VIS|INT|ER0|ER1 bits */
		if (vif1Regs->stat.test(VIF1_STAT_VSS | VIF1_STAT_VIS | VIF1_STAT_VFS))
		{
			cancel = true;
		}

		vif1Regs->stat.clear_flags(VIF1_STAT_VSS | VIF1_STAT_VFS | VIF1_STAT_VIS |
				VIF1_STAT_INT | VIF1_STAT_ER0 | VIF1_STAT_ER1);

		if (cancel)
		{
			if (vif1.vifstalled)
			{
				g_vifCycles = 0;
				// loop necessary for spiderman
				switch(dmacRegs->ctrl.MFD)
				{
				    case MFD_VIF1:
                        //Console.WriteLn("MFIFO Stall");
                        CPU_INT(10, vif1ch->qwc * BIAS);
                        break;

                    case NO_MFD:
                    case MFD_RESERVED:
                    case MFD_GIF: // Wonder if this should be with VIF?
                        // Gets the timing right - Flatout
                        CPU_INT(DMAC_VIF1, vif1ch->qwc * BIAS);
                        break;
				}
				
				vif1ch->chcr.STR = true;
			}
		}
	}
}

_f void vif1STAT(u32 value) {
	VIF_LOG("VIF1_STAT write32 0x%8.8x", value);

	/* Only FDR bit is writable, so mask the rest */
	if ((vif1Regs->stat.FDR) ^ ((tVIF_STAT&)value).FDR) {
		// different so can't be stalled
		if (vif1Regs->stat.test(VIF1_STAT_INT | VIF1_STAT_VSS | VIF1_STAT_VIS | VIF1_STAT_VFS)) {
			DevCon.WriteLn("changing dir when vif1 fifo stalled");
		}
	}

	vif1Regs->stat.FDR = VIF_STAT(value).FDR;
	
	if (vif1Regs->stat.FDR) // Vif transferring to memory.
	{
	    // Hack but it checks this is true before transfer? (fatal frame)
		vif1Regs->stat.FQC = 0x1;
	}
	else // Memory transferring to Vif.
	{
		vif1ch->qwc = 0;
		vif1.vifstalled = false;
		vif1.done = true;
		vif1Regs->stat.FQC = 0;
	}
}

#define caseVif(x) (idx ? VIF1_##x : VIF0_##x)

_vifT void vifWrite32(u32 mem, u32 value) {
	switch (mem) {
		case caseVif(MARK):
			VIF_LOG("VIF%d_MARK write32 0x%8.8x", idx, value);
			vifXRegs->stat.MRK = false;
			vifXRegs->mark	   = value;
			break;

		case caseVif(FBRST):
			if (!idx) vif0FBRST(value);
			else	  vif1FBRST(value);
			break;

		case caseVif(ERR):
			VIF_LOG("VIF%d_ERR write32 0x%8.8x", idx, value);
			vifXRegs->err.write(value);
			break;

		case caseVif(STAT):
			if (idx) { // Only Vif1 does this stuff?
				vif1STAT(value);
			}
			else {
				Console.WriteLn("Unknown Vif%d write to %x", idx, mem);
				psHu32(mem) = value;
			}
			break;

		case caseVif(MODE):
			vifXRegs->mode = value;
			break;

		case caseVif(R0):
		case caseVif(R1):
		case caseVif(R2):
		case caseVif(R3):
			if (!idx) g_vifmask.Row0[ (mem>>4)&3 ]   = value;
			else	  g_vifmask.Row1[ (mem>>4)&3 ]   = value;
			((u32*)&vifXRegs->r0)   [((mem>>4)&3)*4] = value;
			break;

		case caseVif(C0):
		case caseVif(C1):
		case caseVif(C2):
		case caseVif(C3):
			if (!idx) g_vifmask.Col0[ (mem>>4)&3 ]   = value;
			else	  g_vifmask.Col1[ (mem>>4)&3 ]   = value;
			((u32*)&vifXRegs->c0)   [((mem>>4)&3)*4] = value;
			break;

		default:
			Console.WriteLn("Unknown Vif%d write to %x", idx, mem);
			psHu32(mem) = value;
			break;
	}
	/* Other registers are read-only so do nothing for them */
}

void vif0Write32(u32 mem, u32 value) { vifWrite32<0>(mem, value); }
void vif1Write32(u32 mem, u32 value) { vifWrite32<1>(mem, value); }
