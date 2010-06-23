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


#ifndef __DMAC_H__
#define __DMAC_H__

extern u8  *psH; // hw mem

// Useful enums for some of the fields.
enum pce_values
{
	PCE_NOTHING = 0,
	PCE_RESERVED,
	PCE_DISABLED,
	PCE_ENABLED
};


enum tag_id
{
	TAG_CNTS = 0,
	TAG_REFE = 0, 	// Transfer Packet According to ADDR field, clear STR, and end
	TAG_CNT, 		// Transfer QWC following the tag.
	TAG_NEXT,		// Transfer QWC following tag. TADR = ADDR
	TAG_REF,			// Transfer QWC from ADDR field
	TAG_REFS,		// Transfer QWC from ADDR field (Stall Control)
	TAG_CALL,		// Transfer QWC following the tag, save succeeding tag
	TAG_RET,			// Transfer QWC following the tag, load next tag
	TAG_END			// Transfer QWC following the tag
};

enum mfd_type
{
	NO_MFD = 0,
	MFD_RESERVED,
	MFD_VIF1,
	MFD_GIF
};

enum sts_type
{
	NO_STS = 0,
	STS_SIF0,
	STS_fromSPR,
	STS_fromIPU
};

enum std_type
{
	NO_STD = 0,
	STD_VIF1,
	STD_GIF,
	STD_SIF1
};

enum TransferMode
{
	NORMAL_MODE = 0,
	CHAIN_MODE,
	INTERLEAVE_MODE,
	UNDEFINED_MODE
};

//
// --- DMA ---
//

// Doing double duty as both the top 32 bits *and* the lower 32 bits of a chain tag.
// Theoretically should probably both be in a u64 together, but with the way the
// code is layed out, this is easier for the moment.

union tDMA_TAG {
	struct {
		u32 QWC : 16;
		u32 reserved2 : 10;
		u32 PCE : 2;
		u32 ID : 3;
		u32 IRQ : 1;
	};
	struct {
		u32 ADDR : 31;
		u32 SPR : 1;
	};
	u32 _u32;

	tDMA_TAG(u32 val) { _u32 = val; }
	u16 upper() const { return (_u32 >> 16); }
	u16 lower() const { return (u16)_u32; }
	wxString tag_to_str() const
	{
		switch(ID)
		{
			case TAG_REFE: return wxsFormat(L"REFE %08X", _u32); break;
			case TAG_CNT: return L"CNT"; break;
			case TAG_NEXT: return wxsFormat(L"NEXT %08X", _u32); break;
			case TAG_REF: return wxsFormat(L"REF %08X", _u32); break;
			case TAG_REFS: return wxsFormat(L"REFS %08X", _u32); break;
			case TAG_CALL: return L"CALL"; break;
			case TAG_RET: return L"RET"; break;
			case TAG_END: return L"END"; break;
			default: return L"????"; break;
		}
	}
	void reset() { _u32 = 0; }
};
#define DMA_TAG(value) ((tDMA_TAG)(value))

union tDMA_CHCR {
	struct {
		u32 DIR : 1;        // Direction: 0 - to memory, 1 - from memory. VIF1 & SIF2 only.
		u32 reserved1 : 1;
		u32 MOD : 2;
		u32 ASP : 2;        // ASP1 & ASP2; Address stack pointer. 0, 1, or 2 addresses.
		u32 TTE : 1;        // Tag Transfer Enable. 0 - Disable / 1 - Enable.
		u32 TIE : 1;        // Tag Interrupt Enable. 0 - Disable / 1 - Enable.
		u32 STR : 1;        // Start. 0 while stopping DMA, 1 while it's running.
		u32 reserved2 : 7;
		u32 TAG : 16;
	};
	u32 _u32;

	tDMA_CHCR( u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set(u32 value) { _u32 = value; }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	u16 upper() const { return (_u32 >> 16); }
	u16 lower() const { return (u16)_u32; }
	wxString desc() const { return wxsFormat(L"Chcr: 0x%x", _u32); }
};

#define CHCR(value) ((tDMA_CHCR)(value))

union tDMA_SADR {
	struct {
		u32 ADDR : 14;
		u32 reserved2 : 18;
	};
	u32 _u32;

	tDMA_SADR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Sadr: 0x%x", _u32); }
};

union tDMA_MADR {
	struct {
		u32 ADDR : 31; // Transfer memory address
		u32 SPR : 1; // Memory/SPR Address
	};
	u32 _u32;

	tDMA_MADR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Madr: 0x%x", _u32); }
};

union tDMA_TADR {
	struct {
		u32 ADDR : 31; // Next Tag address
		u32 SPR : 1; // Memory/SPR Address
	};
	u32 _u32;

	tDMA_TADR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Tadr: 0x%x", _u32); }
};

union tDMA_ASR { // The Address Stack Register
	struct {
		u32 ADDR : 31; // Tag memory address
		u32 SPR : 1; // Memory/SPR Address
	};
	u32 _u32;

	tDMA_ASR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Asr: 0x%x", _u32); }
};

union tDMA_QWC {
	struct {
		u32 QWC : 16;
		u32 reserved2 : 16;
	};
	u32 _u32;

	tDMA_QWC(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"QWC: 0x%x", _u32); }
};
static __forceinline void setDmacStat(u32 num);
static __forceinline tDMA_TAG *dmaGetAddr(u32 addr, bool write);
static __forceinline void throwBusError(const char *s);

struct DMACh {
	tDMA_CHCR chcr;
	u32 null0[3];
	u32 madr;
	u32 null1[3];
	u16 qwc; u16 pad;
	u32 null2[3];
	u32 tadr;
	u32 null3[3];
	u32 asr0;
	u32 null4[3];
	u32 asr1;
	u32 null5[11];
	u32 sadr;

	void chcrTransfer(tDMA_TAG* ptag)
	{
	    chcr.TAG = ptag[0].upper();
	}

	void qwcTransfer(tDMA_TAG* ptag)
	{
	    qwc = ptag[0].QWC;
	}

	bool transfer(const char *s, tDMA_TAG* ptag)
	{
		if (ptag == NULL)  					 // Is ptag empty?
		{
			throwBusError(s);
			return false;
		}
	    chcrTransfer(ptag);

        qwcTransfer(ptag);
        return true;
	}

	void unsafeTransfer(tDMA_TAG* ptag)
	{
        chcrTransfer(ptag);
        qwcTransfer(ptag);
	}

	tDMA_TAG *getAddr(u32 addr, u32 num, bool write)
	{
		tDMA_TAG *ptr = dmaGetAddr(addr, write);
		if (ptr == NULL)
		{
			throwBusError("dmaGetAddr");
			setDmacStat(num);
			chcr.STR = false;
		}

		return ptr;
	}

	tDMA_TAG *DMAtransfer(u32 addr, u32 num)
	{
		tDMA_TAG *tag = getAddr(addr, num, false);

		if (tag == NULL) return NULL;

	    chcrTransfer(tag);
        qwcTransfer(tag);
        return tag;
	}

	tDMA_TAG dma_tag() const
	{
		return DMA_TAG(chcr._u32);
	}

	wxString cmq_to_str() const
	{
		return wxsFormat(L"chcr = %lx, madr = %lx, qwc  = %lx", chcr._u32, madr, qwc);
	}

	wxString cmqt_to_str() const
	{
		return wxsFormat(L"chcr = %lx, madr = %lx, qwc  = %lx, tadr = %1x", chcr._u32, madr, qwc, tadr);
	}
};

enum INTCIrqs
{
	INTC_GS = 0,
	INTC_SBUS,
	INTC_VBLANK_S,
	INTC_VBLANK_E,
	INTC_VIF0,
	INTC_VIF1,
	INTC_VU0,
	INTC_VU1,
	INTC_IPU,
	INTC_TIM0,
	INTC_TIM1,
	INTC_TIM2,
	INTC_TIM3,
	INTC_SFIFO,
	INTVU0_WD
};

enum dmac_conditions
{
	DMAC_STAT_SIS	= (1<<13),	 // stall condition
	DMAC_STAT_MEIS	= (1<<14),	 // mfifo empty
	DMAC_STAT_BEIS	= (1<<15),	 // bus error
	DMAC_STAT_SIM	= (1<<29),	 // stall mask
	DMAC_STAT_MEIM	= (1<<30)	 // mfifo mask
};

enum DMACIrqs
{
	DMAC_VIF0	= 0,
	DMAC_VIF1,
	DMAC_GIF,
	DMAC_FROM_IPU,
	DMAC_TO_IPU,
	DMAC_SIF0,
	DMAC_SIF1,
	DMAC_SIF2,
	DMAC_FROM_SPR,
	DMAC_TO_SPR,

	// We're setting error conditions through hwDmacIrq, so these correspond to the conditions above.
	DMAC_STALL_SIS		= 13, // SIS
	DMAC_MFIFO_EMPTY	= 14, // MEIS
	DMAC_BUS_ERROR	= 15      // BEIS
};

//DMA interrupts & masks
enum DMAInter
{
	BEISintr = 0x00008000,
	VIF0intr = 0x00010001,
	VIF1intr = 0x00020002,
	GIFintr =  0x00040004,
	IPU0intr = 0x00080008,
	IPU1intr = 0x00100010,
	SIF0intr = 0x00200020,
	SIF1intr = 0x00400040,
	SIF2intr = 0x00800080,
	SPR0intr = 0x01000100,
	SPR1intr = 0x02000200,
	SISintr  = 0x20002000,
	MEISintr = 0x40004000
};

union tDMAC_QUEUE
{
	struct
	{
	    u16 VIF0 : 1;
	    u16 VIF1 : 1;
	    u16 GIF  : 1;
	    u16 IPU0 : 1;
	    u16 IPU1 : 1;
	    u16 SIF0 : 1;
	    u16 SIF1 : 1;
	    u16 SIF2 : 1;
	    u16 SPR0 : 1;
        u16 SPR1 : 1;
	    u16 SIS  : 1;
	    u16 MEIS : 1;
	    u16 BEIS : 1;
	};
	u16 _u16;

	tDMAC_QUEUE(u16 val) { _u16 = val; }
	void reset() { _u16 = 0; }
	bool empty() const { return (_u16 == 0); }
};

static __forceinline const wxChar* ChcrName(u32 addr)
{
    switch (addr)
    {
        case D0_CHCR: return L"Vif 0";
        case D1_CHCR: return L"Vif 1";
        case D2_CHCR: return L"GIF";
        case D3_CHCR: return L"Ipu 0";
        case D4_CHCR: return L"Ipu 1";
        case D5_CHCR: return L"Sif 0";
        case D6_CHCR: return L"Sif 1";
        case D7_CHCR: return L"Sif 2";
        case D8_CHCR: return L"SPR 0";
        case D9_CHCR: return L"SPR 1";
        default: return L"???";
    }
}

// Believe it or not, making this const can generate compiler warnings in gcc.
static __forceinline int ChannelNumber(u32 addr)
{
    switch (addr)
    {
        case D0_CHCR: return 0;
        case D1_CHCR: return 1;
        case D2_CHCR: return 2;
        case D3_CHCR: return 3;
        case D4_CHCR: return 4;
        case D5_CHCR: return 5;
        case D6_CHCR: return 6;
        case D7_CHCR: return 7;
        case D8_CHCR: return 8;
        case D9_CHCR: return 9;
		default:
		{
			DevCon.Warning("Invalid DMA channel number");
			return 51; // some value
		}
    }
}

union tDMAC_CTRL {
	struct {
		u32 DMAE : 1;       // 0/1 - disables/enables all DMAs
		u32 RELE : 1;       // 0/1 - cycle stealing off/on
		u32 MFD : 2;        // Memory FIFO drain channel (mfd_type)
		u32 STS : 2;        // Stall Control source channel (sts type)
		u32 STD : 2;        // Stall Control drain channel (std_type)
		u32 RCYC : 3;       // Release cycle (8/16/32/64/128/256)
		u32 reserved1 : 21;
	};
	u32 _u32;

	tDMAC_CTRL(u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Ctrl: 0x%x", _u32); }
};

union tDMAC_STAT {
	struct {
		u32 CIS : 10;
		u32 reserved1 : 3;
		u32 SIS : 1;
		u32 MEIS : 1;
		u32 BEIS : 1;
		u32 CIM : 10;
		u32 reserved2 : 3;
		u32 SIM : 1;
		u32 MEIM : 1;
		u32 reserved3 : 1;
	};
	u32 _u32;

	tDMAC_STAT(u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Stat: 0x%x", _u32); }
};

union tDMAC_PCR {
	struct {
		u32 CPC : 10;
		u32 reserved1 : 6;
		u32 CDE : 10;
		u32 reserved2 : 5;
		u32 PCE : 1;
	};
	u32 _u32;

	tDMAC_PCR(u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Pcr: 0x%x", _u32); }
};

union tDMAC_SQWC {
	struct {
		u32 SQWC : 8;
		u32 reserved1 : 8;
		u32 TQWC : 8;
		u32 reserved2 : 8;
	};
	u32 _u32;

	tDMAC_SQWC(u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Sqwc: 0x%x", _u32); }
};

union tDMAC_RBSR {
	struct {
		u32 RMSK : 31;
		u32 reserved1 : 1;
	};
	u32 _u32;

	tDMAC_RBSR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Rbsr: 0x%x", _u32); }
};

union tDMAC_RBOR {
	struct {
		u32 ADDR : 31;
		u32 reserved1 : 1;
	};
	u32 _u32;

	tDMAC_RBOR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Rbor: 0x%x", _u32); }
};

union tDMAC_STADR {
	struct {
		u32 ADDR : 31;
		u32 reserved1 : 1;
	};
	u32 _u32;

	tDMAC_STADR(u32 val) { _u32 = val; }

	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Stadr: 0x%x", _u32); }
};


struct DMACregisters
{
	tDMAC_CTRL	ctrl;
	u32 padding[3];
	tDMAC_STAT	stat;
	u32 padding1[3];
	tDMAC_PCR	pcr;
	u32 padding2[3];

	tDMAC_SQWC	sqwc;
	u32 padding3[3];
	tDMAC_RBSR	rbsr;
	u32 padding4[3];
	tDMAC_RBOR	rbor;
	u32 padding5[3];
	tDMAC_STADR	stadr;
};

// Currently guesswork.
union tINTC_STAT {
	struct {
		u32 interrupts : 10;
	    u32 placeholder : 22;
	};
	u32 _u32;

	tINTC_STAT(u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Stat: 0x%x", _u32); }
};

union tINTC_MASK {
	struct {
	    u32 int_mask : 10;
	    u32 placeholder:22;
	};
	u32 _u32;

	tINTC_MASK(u32 val) { _u32 = val; }

	bool test(u32 flags) const { return !!(_u32 & flags); }
	void set_flags(u32 flags) { _u32 |= flags; }
	void clear_flags(u32 flags) { _u32 &= ~flags; }
	void reset() { _u32 = 0; }
	wxString desc() const { return wxsFormat(L"Mask: 0x%x", _u32); }
};

struct INTCregisters
{
	tINTC_STAT  stat;
	u32 padding[3];
	tINTC_MASK  mask;
};

#define dmacRegs ((DMACregisters*)(PS2MEM_HW+0xE000))
#define intcRegs ((INTCregisters*)(PS2MEM_HW+0xF000))

static __forceinline void throwBusError(const char *s)
{
    Console.Error("%s BUSERR", s);
    dmacRegs->stat.BEIS = true;
}

static __forceinline void setDmacStat(u32 num)
{
	dmacRegs->stat.set_flags(1 << num);
}

// Note: Dma addresses are guaranteed to be aligned to 16 bytes (128 bits)
static __forceinline tDMA_TAG *SPRdmaGetAddr(u32 addr, bool write)
{
	// if (addr & 0xf) { DMA_LOG("*PCSX2*: DMA address not 128bit aligned: %8.8x", addr); }

	// FIXME: Why??? DMA uses physical addresses
	addr &= 0x1ffffff0;

	if (addr < Ps2MemSize::Base)
	{
		return (tDMA_TAG*)&psM[addr];
	}

	if (addr >= 0x11004000 && addr < 0x11010000)
	{
		//Access for VU Memory
		return (tDMA_TAG*)vtlb_GetPhyPtr(addr & 0x1FFFFFF0);
	}

	if (addr >= Ps2MemSize::Base && addr < 0x10000000)
	{
		return (tDMA_TAG*)(write ? psMHW : psMHR);
	}

	Console.Error( "*PCSX2*: DMA error: %8.8x", addr);
	return NULL;
}

// Note: Dma addresses are guaranteed to be aligned to 16 bytes (128 bits)
static __forceinline tDMA_TAG *dmaGetAddr(u32 addr, bool write)
{
	// if (addr & 0xf) { DMA_LOG("*PCSX2*: DMA address not 128bit aligned: %8.8x", addr); }

	if (DMA_TAG(addr).SPR) return (tDMA_TAG*)&psS[addr & 0x3ff0];

	// FIXME: Why??? DMA uses physical addresses
	addr &= 0x1ffffff0;

	if (addr < Ps2MemSize::Base)
	{
		return (tDMA_TAG*)&psM[addr];
	}

	// Secret scratchpad address for DMA = end of maximum main memory?
	if (addr >= 0x10000000 && addr < 0x10004000)
	{
		//Console.Warning("Writing to the scratchpad without the SPR flag set!");
		return (tDMA_TAG*)&psS[addr & 0x3ff0];
	}

	if (addr >= Ps2MemSize::Base && addr < 0x10000000)
	{
		return (tDMA_TAG*)(write ? psMHW : psMHR);
	}

	Console.Error( "*PCSX2*: DMA error: %8.8x", addr);
	return NULL;
}

void hwIntcIrq(int n);
void hwDmacIrq(int n);

bool hwDmacSrcChainWithStack(DMACh *dma, int id);
bool hwDmacSrcChain(DMACh *dma, int id);

extern void intcInterrupt();
extern void dmacInterrupt();

#endif