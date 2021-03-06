// FB Alpha Scooter Shooter driver module
// Based on MAME driver by Pierpaolo Prazzoli

#include "tiles_generic.h"
#include "m6809_intf.h"
#include "burn_ym2203.h"

static unsigned char *AllMem;
static unsigned char *RamEnd;
static unsigned char *MemEnd;
static unsigned char *AllRam;
static unsigned char *DrvM6809ROM;
static unsigned char *DrvZ80ROM;
static unsigned char *DrvGfxROM0;
static unsigned char *DrvGfxROM1;
static unsigned char *DrvColPROM;
static unsigned char *DrvSprRAM;
static unsigned char *DrvColRAM;
static unsigned char *DrvVidRAM;
static unsigned char *DrvZ80RAM;

static unsigned int *DrvPalette;
static unsigned int *Palette;
static unsigned char DrvRecalc;

static unsigned char *scroll;

static unsigned char *soundlatch;
static unsigned char *charbank;
static unsigned char *irq_enable;
static unsigned char *flipscreen;
static unsigned char *palbank;

static unsigned char DrvJoy1[8];
static unsigned char DrvJoy2[8];
static unsigned char DrvJoy3[8];
static unsigned char DrvReset;
static unsigned char DrvDips[3];
static unsigned char DrvInputs[3];

static struct BurnInputInfo ScotrshtInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Scotrsht)

static struct BurnDIPInfo ScotrshtDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0x5d, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x10, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x10, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x10, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x10, 0x01, 0x0f, 0x00, "Invalid"		},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x10, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x10, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x10, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x10, 0x01, 0xf0, 0x00, "Invalid"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x11, 0x01, 0x03, 0x03, "2"			},
	{0x11, 0x01, 0x03, 0x02, "3"			},
	{0x11, 0x01, 0x03, 0x01, "4"			},
	{0x11, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x11, 0x01, 0x08, 0x08, "30K 80K"		},
	{0x11, 0x01, 0x08, 0x00, "40K 90K"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x11, 0x01, 0x30, 0x30, "Easy"			},
	{0x11, 0x01, 0x30, 0x20, "Normal"		},
	{0x11, 0x01, 0x30, 0x10, "Difficult"		},
	{0x11, 0x01, 0x30, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x01, 0x01, "Off"			},
	{0x12, 0x01, 0x01, 0x00, "On"			},
};

STDDIPINFO(Scotrsht)

void scotrsht_main_write(unsigned short address, unsigned char data)
{
	switch (address)
	{
		case 0x2043:
			*charbank = data & 1;
		return;

		case 0x2044:
			*irq_enable = data & 2;
			*flipscreen = data & 8;
		return;

		case 0x3000:
			*palbank = (data & 0x70) >> 4;
		return;

		case 0x3100:
			*soundlatch = data;
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x3300:
			// watchdog
		return;
	}

	if ((address & 0xffe0) == 0x2000) {
		scroll[address & 0x1f] = data;
		return;
	}
}

unsigned char scotrsht_main_read(unsigned short address)
{
	switch (address)
	{
		case 0x3100:
			return DrvDips[1];

		case 0x3200:
			return DrvDips[2];

		case 0x3300:
			return DrvInputs[0];

		case 0x3301:
			return DrvInputs[1];

		case 0x3302:
			return DrvInputs[2];

		case 0x3303:
			return DrvDips[0];
	}

	if ((address & 0xffe0) == 0x2000) {
		return scroll[address & 0x1f];
	}

	return 0;
}

unsigned char __fastcall scotrsht_sound_read(unsigned short address)
{
	if (address == 0x8000) {
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		return *soundlatch;
	}

	return 0;
}

void __fastcall scotrsht_sound_out(unsigned short port, unsigned char data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			BurnYM2203Write(0, port & 1, data);
		return;
	}
}

unsigned char __fastcall scotrsht_sound_in(unsigned short port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			return BurnYM2203Read(0, port & 1);
	}

	return 0;
}

inline static int DrvSynchroniseStream(int nSoundRate)
{
	return (long long)ZetTotalCycles() * nSoundRate / 3072000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 3072000.0;
}

static int DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	M6809Open(0);
	M6809Reset();
	M6809Close();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	return 0;
}

static int MemIndex()
{
	unsigned char *Next; Next = AllMem;

	DrvM6809ROM	= Next; Next += 0x010000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x010000;
	DrvGfxROM1	= Next; Next += 0x020000;

	DrvColPROM	= Next; Next += 0x000500;

	DrvPalette	= (unsigned int*)Next; Next += 0x01000 * sizeof(int);
	Palette		= (unsigned int*)Next; Next += 0x01000 * sizeof(int);

	AllRam		= Next;

	scroll		= Next; Next += 0x000020;

	DrvSprRAM	= Next; Next += 0x001000;
	DrvColRAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x000800;

	DrvZ80RAM	= Next; Next += 0x000400;

	soundlatch	= Next; Next += 0x000001;
	charbank	= Next; Next += 0x000001;
	irq_enable	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	palbank		= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (int i = 0; i < 0x100; i++)
	{
		int r = DrvColPROM[i + 0x000] & 0x0f;
		int g = DrvColPROM[i + 0x100] & 0x0f;
		int b = DrvColPROM[i + 0x200] & 0x0f;

		DrvPalette[i] = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | (b << 0);
	}

	for (int i = 0; i < 0x200; i++) {
		for (int j = 0; j < 8; j++) {
			int col = ((~i & 0x100) >> 1) | (j << 4) | (DrvColPROM[i + 0x300] & 0x0f);
			Palette[((i & 0x100) << 3) | (j << 8) | (i & 0xff)] = DrvPalette[col];
		}
	}
}

static int DrvGfxDecode()
{
	int Plane[4]  = { 0x000, 0x001, 0x002, 0x003 };
	int XOffs[16] = { 0x000, 0x004, 0x008, 0x00c, 0x010, 0x014, 0x018, 0x01c,
			  0x100, 0x104, 0x108, 0x10c, 0x110, 0x114, 0x118, 0x11c };
	int YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			  0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	unsigned char *tmp = (unsigned char*)malloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 4,  8,  8, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x10000);

	GfxDecode(0x0200, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM1);

	free (tmp);

	return 0;
}

static int DrvInit()
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (unsigned char *)0;
	if ((AllMem = (unsigned char *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvM6809ROM + 0x08000,  0, 1)) return 1;
		memcpy (DrvM6809ROM + 0x4000, DrvM6809ROM + 0xc000, 0x4000);
		if (BurnLoadRom(DrvM6809ROM + 0x0c000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM   + 0x00000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0  + 0x00000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1  + 0x00000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1  + 0x08000,  5, 1)) return 1;

		if (BurnLoadRom(DrvColPROM  + 0x00000,  6, 1)) return 1;
		if (BurnLoadRom(DrvColPROM  + 0x00100,  7, 1)) return 1;
		if (BurnLoadRom(DrvColPROM  + 0x00200,  8, 1)) return 1;
		if (BurnLoadRom(DrvColPROM  + 0x00300,  9, 1)) return 1;
		if (BurnLoadRom(DrvColPROM  + 0x00400, 10, 1)) return 1;

		DrvPaletteInit();
		DrvGfxDecode();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvColRAM,		0x0000, 0x07ff, M6809_RAM);
	M6809MapMemory(DrvVidRAM,		0x0800, 0x0fff, M6809_RAM);
	M6809MapMemory(DrvSprRAM,		0x1000, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x04000,	0x4000, 0xffff, M6809_ROM);
	M6809SetWriteByteHandler(scotrsht_main_write);
	M6809SetReadByteHandler(scotrsht_main_read);
	M6809Close();

	ZetInit(1);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM);
	ZetMapArea(0x4000, 0x43ff, 0, DrvZ80RAM);
	ZetMapArea(0x4000, 0x43ff, 1, DrvZ80RAM);
	ZetMapArea(0x4000, 0x43ff, 2, DrvZ80RAM);
	ZetSetReadHandler(scotrsht_sound_read);
	ZetSetOutHandler(scotrsht_sound_out);
	ZetSetInHandler(scotrsht_sound_in);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(1, 3072000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(3072000);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static int DrvExit()
{
	GenericTilesExit();

	M6809Exit();
	ZetExit();

	BurnYM2203Exit();

	free (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_layer()
{
	for (int offs = 0x0000; offs < 0x0800; offs++)
	{
		int sx = (offs & 0x3f) << 3;
		int sy = (offs >> 6) << 3;

		if (sx & 0x100) continue;

		sy -= scroll[offs & 0x1f];
		if (sy < -7) sy += 256;

		int attr = DrvColRAM[offs];
		int code = DrvVidRAM[offs] | (*charbank << 9) | ((attr & 0x40) << 2);
		int color= (attr & 0x0f) | (*palbank << 4);
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;

		if (*flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
			sy = 248 - sy;
			sx = 248 - sx;
		}

		sy -= 16;

		if (flipy) {
			if (flipx) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_sprites()
{
	for (int i = 0; i < 0xc0; i += 4)
	{
		int attr  = DrvSprRAM[i + 1];
		int code  = DrvSprRAM[i + 0] + ((attr & 0x40) << 2);
		int sx    = DrvSprRAM[i + 2] - ((attr & 0x80) << 1);
		int sy    = DrvSprRAM[i + 3];
		int color = (attr & 0x0f) | (*palbank << 4);
		int flipx = attr & 0x10;
		int flipy = attr & 0x20;

		if (*flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy -= 16;

		{
			if (flipx) flipx  = 0x0f;
			if (flipy) flipx |= 0xf0;

			color = (color << 4) | 0x800;

			unsigned char *src = DrvGfxROM1 + (code << 8);

			for (int y = 0; y < 16; y++, sy++)
			{
				if (sy < 0 || sy >= nScreenHeight) continue;

				int xx = sx;

				for (int x = 0; x < 16; x++, xx++)
				{
					if (xx < 0 || xx >= nScreenWidth) continue;

					int pxl = src[((y << 4) | x) ^ flipx] | color;
					int trans = DrvPalette[pxl & 0xfff];

					if (trans) {
						pTransDraw[(sy * nScreenWidth) + xx] = pxl;
					}
				}
			}
		}
	}
}

static int DrvDraw()
{
	if (DrvRecalc) {
		for (int i = 0; i < 0x1000; i++) {
			int rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	BurnTransferClear();
	draw_layer();

	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static int DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 3);
		for (int i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
	}

	int nInterleave = 100;
	int nSoundBufferPos = 0;
	int nCyclesTotal[2] =  { 3072000 / 60, 3072000 / 60 };

	M6809Open(0);
	ZetOpen(0);

	for (int i = 0; i < nInterleave; i++) {
		
		M6809Run(nCyclesTotal[0] / nInterleave);

		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));

		if (pBurnSoundOut) {
			int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2203Update(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (*irq_enable) M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		int nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		short* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			BurnYM2203Update(pSoundBuf, nSegmentLength);
		}
	}

	ZetClose();
	M6809Close();
	
	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static int DrvScan(int nAction,int *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029706;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
	}

	return 0;
}


// Scooter Shooter

static struct BurnRomInfo scotrshtRomDesc[] = {
	{ "gx545_g03_12c.bin",	0x8000, 0xb808e0d3, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "gx545_g02_10c.bin",	0x4000, 0xb22c0586, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gx545_g01_8c.bin",	0x4000, 0x46a7cc65, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "gx545_g05_5f.bin",	0x8000, 0x856c349c, 3 | BRF_GRA },           //  3 Characters

	{ "gx545_g06_6f.bin",	0x8000, 0x14ad7601, 4 | BRF_GRA },           //  4 Sprites
	{ "gx545_h04_4f.bin",	0x8000, 0xc06c11a3, 4 | BRF_GRA },           //  5

	{ "gx545_6301_1f.bin",	0x0100, 0xf584586f, 5 | BRF_GRA },           //  6 Color Proms
	{ "gx545_6301_2f.bin",	0x0100, 0xad464db1, 5 | BRF_GRA },           //  7
	{ "gx545_6301_3f.bin",	0x0100, 0xbd475d23, 5 | BRF_GRA },           //  8
	{ "gx545_6301_7f.bin",	0x0100, 0x2b0cd233, 5 | BRF_GRA },           //  9
	{ "gx545_6301_8f.bin",	0x0100, 0xc1c7cf58, 5 | BRF_GRA },           // 10
};

STD_ROM_PICK(scotrsht)
STD_ROM_FN(scotrsht)

struct BurnDriver BurnDrvScotrsht = {
	"scotrsht", NULL, NULL, "1985",
	"Scooter Shooter\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, scotrshtRomInfo, scotrshtRomName, ScotrshtInputInfo, ScotrshtDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 0, NULL, NULL, NULL, &DrvRecalc, 0x1000,
	224, 256, 3, 4
};
