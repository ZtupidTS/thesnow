/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
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
#include "Gif.h"
#include "Vif_Dma.h"

u16 vifqwc = 0;
u32 g_vifCycles = 0;
u32 g_vu0Cycles = 0;
u32 g_vu1Cycles = 0;
u32 g_packetsizeonvu = 0;

extern u32 g_vifCycles;

static u32 qwctag(u32 mask)
{
	return (dmacRegs.rbor.ADDR + (mask & dmacRegs.rbsr.RMSK));
}

static __fi bool mfifoVIF1rbTransfer()
{
	u32 maddr = dmacRegs.rbor.ADDR;
	u32 msize = dmacRegs.rbor.ADDR + dmacRegs.rbsr.RMSK + 16;
	u16 mfifoqwc = std::min(vif1ch.qwc, vifqwc);
	u32 *src;
	bool ret;

	/* Check if the transfer should wrap around the ring buffer */
	if ((vif1ch.madr + (mfifoqwc << 4)) > (msize))
	{
		int s1 = ((msize) - vif1ch.madr) >> 2;

		SPR_LOG("Split MFIFO");

		/* it does, so first copy 's1' bytes from 'addr' to 'data' */
		src = (u32*)PSM(vif1ch.madr);
		if (src == NULL) return false;

		if (vif1.vifstalled)
			ret = VIF1transfer(src + vif1.irqoffset, s1 - vif1.irqoffset);
		else
			ret = VIF1transfer(src, s1);

		vif1ch.madr = qwctag(vif1ch.madr);
		
		if (ret)
		{
            /* and second copy 's2' bytes from 'maddr' to '&data[s1]' */
            vif1ch.madr = maddr;

            src = (u32*)PSM(maddr);
            if (src == NULL) return false;
            VIF1transfer(src, ((mfifoqwc << 2) - s1));
		}
		vif1ch.madr = qwctag(vif1ch.madr);
		
	}
	else
	{
		SPR_LOG("Direct MFIFO");

		/* it doesn't, so just transfer 'qwc*4' words */
		src = (u32*)PSM(vif1ch.madr);
		if (src == NULL) return false;

		if (vif1.vifstalled)
			ret = VIF1transfer(src + vif1.irqoffset, mfifoqwc * 4 - vif1.irqoffset);
		else
			ret = VIF1transfer(src, mfifoqwc << 2);

		vif1ch.madr = qwctag(vif1ch.madr);
		
	}
	return ret;
}

static __fi void mfifo_VIF1chain()
{
	/* Is QWC = 0? if so there is nothing to transfer */
	if ((vif1ch.qwc == 0))
	{
		vif1.inprogress &= ~1;
		return;
	}

	if (vif1ch.madr >= dmacRegs.rbor.ADDR &&
	        vif1ch.madr <= (dmacRegs.rbor.ADDR + dmacRegs.rbsr.RMSK))
	{
		//Need to exit on mfifo locations, if the madr is matching the madr of spr, we dont have any data left :(

		u16 startqwc = vif1ch.qwc;
		mfifoVIF1rbTransfer();
		vifqwc -= startqwc - vif1ch.qwc;
		
	}
	else
	{
		tDMA_TAG *pMem = dmaGetAddr(vif1ch.madr, !vif1ch.chcr.DIR);
		SPR_LOG("Non-MFIFO Location");

		//No need to exit on non-mfifo as it is indirect anyway, so it can be transferring this while spr refills the mfifo

		if (pMem == NULL) return;

		if (vif1.vifstalled)
			VIF1transfer((u32*)pMem + vif1.irqoffset, vif1ch.qwc * 4 - vif1.irqoffset);
		else
			VIF1transfer((u32*)pMem, vif1ch.qwc << 2);
	}
}



void mfifoVIF1transfer(int qwc)
{
	tDMA_TAG *ptag;

	g_vifCycles = 0;

	if (qwc > 0)
	{
		vifqwc += qwc;
		SPR_LOG("Added %x qw to mfifo, total now %x - Vif CHCR %x Stalled %x done %x", qwc, vifqwc, vif1ch.chcr._u32, vif1.vifstalled, vif1.done);
		if (vif1.inprogress & 0x10)
		{
			if(vif1ch.chcr.STR == true)CPU_INT(DMAC_MFIFO_VIF, 4);

			vif1Regs.stat.FQC = 0x10; // FQC=16
		}
		vif1.inprogress &= ~0x10;

		return;
	}

	if (vif1ch.qwc == 0 && vifqwc > 0)
	{
		ptag = dmaGetAddr(vif1ch.tadr, false);

		if (vif1ch.chcr.TTE)
		{
            bool ret;

			static __aligned16 u128 masked_tag;

			masked_tag._u64[0] = 0;
			masked_tag._u64[1] = *((u64*)ptag + 1);

			VIF_LOG("\tVIF1 SrcChain TTE=1, data = 0x%08x.%08x", masked_tag._u32[3], masked_tag._u32[2]);

			if (vif1.vifstalled)
			{
				ret = VIF1transfer((u32*)&masked_tag + vif1.irqoffset, 4 - vif1.irqoffset, true);  //Transfer Tag on stall
				//ret = VIF1transfer((u32*)ptag + (2 + vif1.irqoffset), 2 - vif1.irqoffset);  //Transfer Tag on stall
			}
			else
			{
				ret = VIF1transfer((u32*)&masked_tag, 4, true);  //Transfer Tag
				//ret = VIF1transfer((u32*)ptag + 2, 2);  //Transfer Tag
			}

			if (!ret && vif1.irqoffset)
			{
				return;        //IRQ set by VIFTransfer
				
			} //else vif1.vifstalled = false;
		}

		vif1.irqoffset = 0;

        vif1ch.unsafeTransfer(ptag);

		vif1ch.madr = ptag[1]._u32;
		vifqwc--;

		SPR_LOG("dmaChain %8.8x_%8.8x size=%d, id=%d, madr=%lx, tadr=%lx mfifo qwc = %x spr0 madr = %x",
        ptag[1]._u32, ptag[0]._u32, vif1ch.qwc, ptag->ID, vif1ch.madr, vif1ch.tadr, vifqwc, spr0ch.madr);

		switch (ptag->ID)
		{
			case TAG_REFE: // Refe - Transfer Packet According to ADDR field
				vif1ch.tadr = qwctag(vif1ch.tadr + 16);
				vif1.done = true;										//End Transfer
				break;

			case TAG_CNT: // CNT - Transfer QWC following the tag.
				vif1ch.madr = qwctag(vif1ch.tadr + 16);						//Set MADR to QW after Tag
				vif1ch.tadr = qwctag(vif1ch.madr + (vif1ch.qwc << 4));			//Set TADR to QW following the data
				vif1.done = false;
				break;

			case TAG_NEXT: // Next - Transfer QWC following tag. TADR = ADDR
			{
				int temp = vif1ch.madr;								//Temporarily Store ADDR
				vif1ch.madr = qwctag(vif1ch.tadr + 16); 					  //Set MADR to QW following the tag
				vif1ch.tadr = temp;								//Copy temporarily stored ADDR to Tag
				if ((temp & dmacRegs.rbsr.RMSK) != dmacRegs.rbor.ADDR) Console.WriteLn("Next tag = %x outside ring %x size %x", temp, psHu32(DMAC_RBOR), psHu32(DMAC_RBSR));
				vif1.done = false;
				break;
			}

			case TAG_REF: // Ref - Transfer QWC from ADDR field
			case TAG_REFS: // Refs - Transfer QWC from ADDR field (Stall Control)
				vif1ch.tadr = qwctag(vif1ch.tadr + 16);							//Set TADR to next tag
				vif1.done = false;
				break;

			case TAG_END: // End - Transfer QWC following the tag
				vif1ch.madr = qwctag(vif1ch.tadr + 16);		//Set MADR to data following the tag
				vif1ch.tadr = qwctag(vif1ch.madr + (vif1ch.qwc << 4));			//Set TADR to QW following the data
				vif1.done = true;										//End Transfer
				break;
		}

		if (vif1ch.chcr.TIE && ptag->IRQ)
		{
			VIF_LOG("dmaIrq Set");
			vif1.done = true;
		}

		vif1Regs.stat.FQC = min(vif1ch.qwc, (u16)16);
		vif1.inprogress |= 1;
	}
	

	SPR_LOG("mfifoVIF1transfer end %x madr %x, tadr %x vifqwc %x", vif1ch.chcr._u32, vif1ch.madr, vif1ch.tadr, vifqwc);
}

void vifMFIFOInterrupt()
{
	g_vifCycles = 0;
	VIF_LOG("vif mfifo interrupt");

	if(GSTransferStatus.PTH2 == STOPPED_MODE && gifRegs.stat.APATH == GIF_APATH2)
	{
		GSTransferStatus.PTH2 = STOPPED_MODE;
		if(gifRegs.stat.DIR == 0)gifRegs.stat.OPH = false;
		gifRegs.stat.APATH = GIF_APATH_IDLE;
		if(gifRegs.stat.P1Q) gsPath1Interrupt();
		/*gifRegs.stat.APATH = GIF_APATH_IDLE;
		if(gifRegs.stat.DIR == 0)gifRegs.stat.OPH = false;*/
	}

	if (schedulepath3msk & 0x10) Vif1MskPath3();

	if(vif1ch.chcr.DIR && CheckPath2GIF(DMAC_MFIFO_VIF) == false) return;
	//We need to check the direction, if it is downloading from the GS, we handle that seperately (KH2 for testing)

	//Simulated GS transfer time done, clear the flags
	
	if (vif1.cmd) 
	{
		if(vif1.done == true && vif1ch.qwc == 0)	vif1Regs.stat.VPS = VPS_WAITING;
	}
	else		 
	{
		vif1Regs.stat.VPS = VPS_IDLE;
	}

	if (vif1.irq && vif1.tag.size == 0)
	{
		vif1Regs.stat.INT = true;
		hwIntcIrq(INTC_VIF1);
		--vif1.irq;

		if (vif1Regs.stat.test(VIF1_STAT_VSS | VIF1_STAT_VIS | VIF1_STAT_VFS))
		{
			/*vif1Regs.stat.FQC = 0; // FQC=0
			vif1ch.chcr.STR = false;*/
			if(vif1ch.qwc > 0 || !vif1.done) return;
		}
	}

	if (vif1.done == false || vif1ch.qwc)
	{
		switch(vif1.inprogress & 1)
		{
			case 0: //Set up transfer
                if (vif1ch.tadr == spr0ch.madr)
				{
				//	Console.WriteLn("Empty 1");
					vifqwc = 0;
					if((vif1.inprogress & 0x10) == 0) 
					{
						hwDmacIrq(DMAC_MFIFO_EMPTY);
						vif1.inprogress |= 0x10;
					}
					vif1Regs.stat.FQC = 0;					
					return;
				}

                mfifoVIF1transfer(0);
                CPU_INT(DMAC_MFIFO_VIF, 4);
				return;

			case 1: //Transfer data
				mfifo_VIF1chain();
				//Sanity check! making sure we always have non-zero values
				CPU_INT(DMAC_MFIFO_VIF, (g_vifCycles == 0 ? 4 : g_vifCycles) );				
				return;
		}
		return;
	}


	//FF7 Dirge of Cerberus seems to like the mfifo to tell it when it's empty, even if it's ending.
	//Doesn't seem to care about the vif1 dma interrupting (possibly disabled the interrupt?)
	if (vif1ch.tadr == spr0ch.madr)
	{
		vifqwc = 0;
		if((vif1.inprogress & 0x10) == 0) 
		{
			hwDmacIrq(DMAC_MFIFO_EMPTY);
			vif1.inprogress |= 0x10;
		}
	}
	vif1.vifstalled = false;
	vif1.done = 1;
	g_vifCycles = 0;
	vif1ch.chcr.STR = false;
	hwDmacIrq(DMAC_VIF1);
	VIF_LOG("vif mfifo dma end");

	vif1Regs.stat.FQC = 0;
}
