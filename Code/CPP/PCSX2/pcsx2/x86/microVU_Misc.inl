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

#pragma once

//------------------------------------------------------------------
// Micro VU - Reg Loading/Saving/Shuffling/Unpacking/Merging...
//------------------------------------------------------------------

void mVUunpack_xyzw(int dstreg, int srcreg, int xyzw) {
	switch ( xyzw ) {
		case 0: SSE2_PSHUFD_XMM_to_XMM(dstreg, srcreg, 0x00); break;
		case 1: SSE2_PSHUFD_XMM_to_XMM(dstreg, srcreg, 0x55); break;
		case 2: SSE2_PSHUFD_XMM_to_XMM(dstreg, srcreg, 0xaa); break;
		case 3: SSE2_PSHUFD_XMM_to_XMM(dstreg, srcreg, 0xff); break;
	}
}

void mVUloadReg(int reg, uptr offset, int xyzw) {
	switch( xyzw ) {
		case 8:		SSE_MOVSS_M32_to_XMM(reg, offset);		break; // X
		case 4:		SSE_MOVSS_M32_to_XMM(reg, offset+4);	break; // Y
		case 2:		SSE_MOVSS_M32_to_XMM(reg, offset+8);	break; // Z
		case 1:		SSE_MOVSS_M32_to_XMM(reg, offset+12);	break; // W
		default:	SSE_MOVAPS_M128_to_XMM(reg, offset);	break;
	}
}

void mVUloadReg2(int reg, int gprReg, uptr offset, int xyzw) {
	switch( xyzw ) {
		case 8:		SSE_MOVSS_Rm_to_XMM(reg, gprReg, offset);	 break; // X
		case 4:		SSE_MOVSS_Rm_to_XMM(reg, gprReg, offset+4);  break; // Y
		case 2:		SSE_MOVSS_Rm_to_XMM(reg, gprReg, offset+8);  break; // Z
		case 1:		SSE_MOVSS_Rm_to_XMM(reg, gprReg, offset+12); break; // W
		default:	SSE_MOVAPSRmtoR(reg, gprReg, offset);		 break;
	}
}

void mVUloadIreg(int reg, int xyzw, VURegs* vuRegs) {
	SSE_MOVSS_M32_to_XMM(reg, (uptr)&vuRegs->VI[REG_I].UL);
	if (!_XYZWss(xyzw)) SSE_SHUFPS_XMM_to_XMM(reg, reg, 0);
}

// Modifies the Source Reg!
void mVUsaveReg(int reg, uptr offset, int xyzw, bool modXYZW) {
	/*SSE_MOVAPS_M128_to_XMM(xmmT2, offset);
	if (modXYZW && (xyzw == 8 || xyzw == 4 || xyzw == 2 || xyzw == 1)) {
		mVUunpack_xyzw<vuIndex>(reg, reg, 0);
	}
	mVUmergeRegs(xmmT2, reg, xyzw);

	SSE_MOVAPS_XMM_to_M128(offset, xmmT2);
	return;*/

	switch ( xyzw ) {
		case 5:		if (x86caps.hasStreamingSIMD4Extensions) {
						SSE4_EXTRACTPS_XMM_to_M32(offset+4,  reg, 1);
						SSE4_EXTRACTPS_XMM_to_M32(offset+12, reg, 3);
					}
					else {
						SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xe1); //WZXY
						SSE_MOVSS_XMM_to_M32(offset+4, reg);
						SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xff); //WWWW
						SSE_MOVSS_XMM_to_M32(offset+12, reg);
					}
					break; // YW
		case 6:		SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xc9);
					SSE_MOVLPS_XMM_to_M64(offset+4, reg);
					break; // YZ
		case 7:		SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0x93); //ZYXW
					SSE_MOVHPS_XMM_to_M64(offset+4, reg);
					SSE_MOVSS_XMM_to_M32(offset+12, reg);
					break; // YZW
		case 9:		SSE_MOVSS_XMM_to_M32(offset, reg);
					SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xff); //WWWW
					SSE_MOVSS_XMM_to_M32(offset+12, reg);
					break; // XW
		case 10:	SSE_MOVSS_XMM_to_M32(offset, reg);
					SSE_MOVHLPS_XMM_to_XMM(reg, reg);
					SSE_MOVSS_XMM_to_M32(offset+8, reg);
					break; //XZ
		case 11:	SSE_MOVSS_XMM_to_M32(offset, reg);
					SSE_MOVHPS_XMM_to_M64(offset+8, reg);
					break; //XZW
		case 13:	SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0x4b); //YXZW				
					SSE_MOVHPS_XMM_to_M64(offset, reg);
					SSE_MOVSS_XMM_to_M32(offset+12, reg);
					break; // XYW
		case 14:	SSE_MOVLPS_XMM_to_M64(offset, reg);
					SSE_MOVHLPS_XMM_to_XMM(reg, reg);
					SSE_MOVSS_XMM_to_M32(offset+8, reg);
					break; // XYZ
		case 4:		if (!modXYZW) mVUunpack_xyzw(reg, reg, 1);
					SSE_MOVSS_XMM_to_M32(offset+4, reg);		 
					break; // Y
		case 2:		if (!modXYZW) mVUunpack_xyzw(reg, reg, 2);
					SSE_MOVSS_XMM_to_M32(offset+8, reg);	
					break; // Z
		case 1:		if (!modXYZW) mVUunpack_xyzw(reg, reg, 3);
					SSE_MOVSS_XMM_to_M32(offset+12, reg);	
					break; // W
		case 8:		SSE_MOVSS_XMM_to_M32(offset, reg);		break; // X
		case 12:	SSE_MOVLPS_XMM_to_M64(offset, reg);		break; // XY
		case 3:		SSE_MOVHPS_XMM_to_M64(offset+8, reg);	break; // ZW
		default:	SSE_MOVAPS_XMM_to_M128(offset, reg);	break; // XYZW
	}
}

// Modifies the Source Reg!
void mVUsaveReg2(int reg, int gprReg, u32 offset, int xyzw) {
	/*SSE_MOVAPSRmtoR(xmmT2, gprReg, offset);
	if (xyzw == 8 || xyzw == 4 || xyzw == 2 || xyzw == 1) {
		mVUunpack_xyzw<vuIndex>(reg, reg, 0);
	}
	mVUmergeRegs(xmmT2, reg, xyzw);
	SSE_MOVAPSRtoRm(gprReg, xmmT2, offset);
	return;*/

	switch ( xyzw ) {
		case 5:		SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xe1); //WZXY
					SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+4);
					SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xff); //WWWW
					SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+12);
					break; // YW
		case 6:		SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xc9);
					SSE_MOVLPS_XMM_to_Rm(gprReg, reg, offset+4);
					break; // YZ
		case 7:		SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0x93); //ZYXW
					SSE_MOVHPS_XMM_to_Rm(gprReg, reg, offset+4);
					SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+12);
					break; // YZW
		case 9:		SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset);
					SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0xff); //WWWW
					SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+12);
					break; // XW
		case 10:	SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset);
					SSE_MOVHLPS_XMM_to_XMM(reg, reg);
					SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+8);
					break; //XZ
		case 11:	SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset);
					SSE_MOVHPS_XMM_to_Rm(gprReg, reg, offset+8);
					break; //XZW
		case 13:	SSE2_PSHUFD_XMM_to_XMM(reg, reg, 0x4b); //YXZW				
					SSE_MOVHPS_XMM_to_Rm(gprReg, reg, offset);
					SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+12);
					break; // XYW
		case 14:	SSE_MOVLPS_XMM_to_Rm(gprReg, reg, offset);
					SSE_MOVHLPS_XMM_to_XMM(reg, reg);
					SSE_MOVSS_XMM_to_Rm(gprReg,  reg, offset+8);
					break; // XYZ
		case 8:		SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset);	 break; // X
		case 4:		SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+4);	 break; // Y
		case 2:		SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+8);	 break; // Z
		case 1:		SSE_MOVSS_XMM_to_Rm(gprReg, reg, offset+12); break; // W
		case 12:	SSE_MOVLPS_XMM_to_Rm(gprReg, reg, offset);	 break; // XY
		case 3:		SSE_MOVHPS_XMM_to_Rm(gprReg, reg, offset+8); break; // ZW
		default:	SSE_MOVAPSRtoRm(gprReg, reg, offset);		 break; // XYZW
	}
}

// Modifies the Source Reg! (ToDo: Optimize modXYZW = 1 cases)
void mVUmergeRegs(int dest, int src, int xyzw, bool modXYZW = 0) {
	xyzw &= 0xf;
	if ( (dest != src) && (xyzw != 0) ) {
		if (x86caps.hasStreamingSIMD4Extensions && (xyzw != 0x8) && (xyzw != 0xf)) {
			if (modXYZW) {
				if		(xyzw == 1) { SSE4_INSERTPS_XMM_to_XMM(dest, src, _MM_MK_INSERTPS_NDX(0, 3, 0)); return; }
				else if (xyzw == 2) { SSE4_INSERTPS_XMM_to_XMM(dest, src, _MM_MK_INSERTPS_NDX(0, 2, 0)); return; }
				else if (xyzw == 4) { SSE4_INSERTPS_XMM_to_XMM(dest, src, _MM_MK_INSERTPS_NDX(0, 1, 0)); return; }
			}
			xyzw = ((xyzw & 1) << 3) | ((xyzw & 2) << 1) | ((xyzw & 4) >> 1) | ((xyzw & 8) >> 3); 
			SSE4_BLENDPS_XMM_to_XMM(dest, src, xyzw);
		}
		else {
			switch (xyzw) {
				case 1:  if (modXYZW) mVUunpack_xyzw(src, src, 0);
						 SSE_MOVHLPS_XMM_to_XMM(src, dest);		 // src = Sw Sz Dw Dz
						 SSE_SHUFPS_XMM_to_XMM(dest, src, 0xc4); // 11 00 01 00
						 break;
				case 2:  if (modXYZW) mVUunpack_xyzw(src, src, 0);
						 SSE_MOVHLPS_XMM_to_XMM(src, dest);
						 SSE_SHUFPS_XMM_to_XMM(dest, src, 0x64);
						 break;
				case 3:	 SSE_SHUFPS_XMM_to_XMM(dest, src, 0xe4);
						 break;
				case 4:	 if (modXYZW) mVUunpack_xyzw(src, src, 0);
						 SSE_MOVSS_XMM_to_XMM(src, dest);
						 SSE2_MOVSD_XMM_to_XMM(dest, src);
						 break;
				case 5:	 SSE_SHUFPS_XMM_to_XMM(dest, src, 0xd8);
						 SSE2_PSHUFD_XMM_to_XMM(dest, dest, 0xd8);
						 break;
				case 6:	 SSE_SHUFPS_XMM_to_XMM(dest, src, 0x9c);
						 SSE2_PSHUFD_XMM_to_XMM(dest, dest, 0x78);
						 break;
				case 7:	 SSE_MOVSS_XMM_to_XMM(src, dest);
						 SSE_MOVAPS_XMM_to_XMM(dest, src);
						 break;
				case 8:	 SSE_MOVSS_XMM_to_XMM(dest, src);
						 break;
				case 9:	 SSE_SHUFPS_XMM_to_XMM(dest, src, 0xc9);
						 SSE2_PSHUFD_XMM_to_XMM(dest, dest, 0xd2);
						 break;
				case 10: SSE_SHUFPS_XMM_to_XMM(dest, src, 0x8d);
						 SSE2_PSHUFD_XMM_to_XMM(dest, dest, 0x72);
						 break;
				case 11: SSE_MOVSS_XMM_to_XMM(dest, src);
						 SSE_SHUFPS_XMM_to_XMM(dest, src, 0xe4);
						 break;
				case 12: SSE2_MOVSD_XMM_to_XMM(dest, src);
						 break;
				case 13: SSE_MOVHLPS_XMM_to_XMM(dest, src);
						 SSE_SHUFPS_XMM_to_XMM(src, dest, 0x64);
						 SSE_MOVAPS_XMM_to_XMM(dest, src);
						 break;
				case 14: SSE_MOVHLPS_XMM_to_XMM(dest, src);
						 SSE_SHUFPS_XMM_to_XMM(src, dest, 0xc4);
						 SSE_MOVAPS_XMM_to_XMM(dest, src);
						 break;
				default: SSE_MOVAPS_XMM_to_XMM(dest, src); 
						 break;
			}
		}
	}
}

//------------------------------------------------------------------
// Micro VU - Misc Functions
//------------------------------------------------------------------

// Transforms the Address in gprReg to valid VU0/VU1 Address
_f void mVUaddrFix(mV, int gprReg) {
	if (mVU == &microVU1) {
		AND32ItoR(gprReg, 0x3ff); // wrap around
		SHL32ItoR(gprReg, 4);
	}
	else {
		u8 *jmpA, *jmpB; 
		CMP32ItoR(gprReg, 0x400);
		jmpA = JL8(0); // if addr >= 0x4000, reads VU1's VF regs and VI regs
			AND32ItoR(gprReg, 0x43f); // ToDo: theres a potential problem if VU0 overrides VU1's VF0/VI0 regs!
			jmpB = JMP8(0);
		x86SetJ8(jmpA);
			AND32ItoR(gprReg, 0xff); // if addr < 0x4000, wrap around
		x86SetJ8(jmpB);
		SHL32ItoR(gprReg, 4); // multiply by 16 (shift left by 4)
	}
}

// Backup Volatile Regs (EAX, ECX, EDX, MM0~7, XMM0~7, are all volatile according to 32bit Win/Linux ABI)
_f void mVUbackupRegs(microVU* mVU) {
	mVU->regAlloc->flushAll();
	SSE_MOVAPS_XMM_to_M128((uptr)&mVU->xmmPQb[0], xmmPQ);
}

// Restore Volatile Regs
_f void mVUrestoreRegs(microVU* mVU) {
	SSE_MOVAPS_M128_to_XMM(xmmPQ,  (uptr)&mVU->xmmPQb[0]);
}

//------------------------------------------------------------------
// Micro VU - Custom SSE Instructions
//------------------------------------------------------------------

struct SSEMaskPair { u32 mask1[4], mask2[4]; };

static const __aligned16 SSEMaskPair MIN_MAX = 
{
	{0xffffffff, 0x80000000, 0xffffffff, 0x80000000},
	{0x00000000, 0x40000000, 0x00000000, 0x40000000}
};


// Warning: Modifies t1 and t2
void MIN_MAX_PS(microVU* mVU, int to, int from, int t1, int t2, bool min) {
	bool t1b = 0, t2b = 0;
	if (t1 < 0) { t1 = mVU->regAlloc->allocReg(); t1b = 1; }
	if (t2 < 0) { t2 = mVU->regAlloc->allocReg(); t2b = 1; }

	// ZW
	SSE2_PSHUFD_XMM_to_XMM(t1, to, 0xfa);
	SSE2_PAND_M128_to_XMM (t1, (uptr)MIN_MAX.mask1);
	SSE2_POR_M128_to_XMM  (t1, (uptr)MIN_MAX.mask2);
	SSE2_PSHUFD_XMM_to_XMM(t2, from, 0xfa);
	SSE2_PAND_M128_to_XMM (t2, (uptr)MIN_MAX.mask1);
	SSE2_POR_M128_to_XMM  (t2, (uptr)MIN_MAX.mask2);
	if (min) SSE2_MINPD_XMM_to_XMM(t1, t2);
	else     SSE2_MAXPD_XMM_to_XMM(t1, t2);

	// XY
	SSE2_PSHUFD_XMM_to_XMM(t2, from, 0x50);
	SSE2_PAND_M128_to_XMM (t2, (uptr)MIN_MAX.mask1);
	SSE2_POR_M128_to_XMM  (t2, (uptr)MIN_MAX.mask2);
	SSE2_PSHUFD_XMM_to_XMM(to, to, 0x50);
	SSE2_PAND_M128_to_XMM (to, (uptr)MIN_MAX.mask1);
	SSE2_POR_M128_to_XMM  (to, (uptr)MIN_MAX.mask2);
	if (min) SSE2_MINPD_XMM_to_XMM(to, t2);
	else     SSE2_MAXPD_XMM_to_XMM(to, t2);

	SSE_SHUFPS_XMM_to_XMM(to, t1, 0x88);
	if (t1b) mVU->regAlloc->clearNeeded(t1);
	if (t2b) mVU->regAlloc->clearNeeded(t2);
}

// Warning: Modifies to's upper 3 vectors, and t1
void MIN_MAX_SS(mV, int to, int from, int t1, bool min) {
	bool t1b = 0;
	if (t1 < 0) { t1 = mVU->regAlloc->allocReg(); t1b = 1; }
	SSE_SHUFPS_XMM_to_XMM (to, from, 0);
	SSE2_PAND_M128_to_XMM (to, (uptr)MIN_MAX.mask1);
	SSE2_POR_M128_to_XMM  (to, (uptr)MIN_MAX.mask2);
	SSE2_PSHUFD_XMM_to_XMM(t1, to, 0xee);
	if (min) SSE2_MINPD_XMM_to_XMM(to, t1);
	else	 SSE2_MAXPD_XMM_to_XMM(to, t1);
	if (t1b) mVU->regAlloc->clearNeeded(t1);
}

// Warning: Modifies all vectors in 'to' and 'from', and Modifies xmmT1 and xmmT2
void ADD_SS(microVU* mVU, int to, int from, int t1, int t2) {

	u8 *localptr[8];
	bool t1b = 0, t2b = 0;
	if (t1 < 0) { t1 = mVU->regAlloc->allocReg(); t1b = 1; }
	if (t2 < 0) { t2 = mVU->regAlloc->allocReg(); t2b = 1; }

	SSE_MOVAPS_XMM_to_XMM(t1, to);
	SSE_MOVAPS_XMM_to_XMM(t2, from);
	SSE2_MOVD_XMM_to_R(gprT2, to);
	SHR32ItoR(gprT2, 23); 
	SSE2_MOVD_XMM_to_R(gprT1, from);
	SHR32ItoR(gprT1, 23);
	AND32ItoR(gprT2, 0xff);
	AND32ItoR(gprT1, 0xff); 
	SUB32RtoR(gprT2, gprT1); //gprT2 = exponent difference

	CMP32ItoR(gprT2, 25);
	localptr[0] = JGE8(0);
	CMP32ItoR(gprT2, 0);
	localptr[1] = JG8(0);
	localptr[2] = JE8(0);
	CMP32ItoR(gprT2, -25);
	localptr[3] = JLE8(0);
		NEG32R(gprT2); 
		DEC32R(gprT2);
		MOV32ItoR(gprT1, 0xffffffff);
		SHL32CLtoR(gprT1);
		SSE2_PCMPEQB_XMM_to_XMM(to, to);
		SSE2_MOVD_R_to_XMM(from, gprT1);
		SSE_MOVSS_XMM_to_XMM(to, from);
		SSE2_PCMPEQB_XMM_to_XMM(from, from);
	localptr[4] = JMP8(0);

	x86SetJ8(localptr[0]);
		MOV32ItoR(gprT1, 0x80000000);
		SSE2_PCMPEQB_XMM_to_XMM(from, from);
		SSE2_MOVD_R_to_XMM(to, gprT1);
		SSE_MOVSS_XMM_to_XMM(from, to);
		SSE2_PCMPEQB_XMM_to_XMM(to, to);
	localptr[5] = JMP8(0);

	x86SetJ8(localptr[1]);
		DEC32R(gprT2);
		MOV32ItoR(gprT1, 0xffffffff);
		SHL32CLtoR(gprT1); 
		SSE2_PCMPEQB_XMM_to_XMM(from, from);
		SSE2_MOVD_R_to_XMM(to, gprT1);
		SSE_MOVSS_XMM_to_XMM(from, to);
		SSE2_PCMPEQB_XMM_to_XMM(to, to);
	localptr[6] = JMP8(0);

	x86SetJ8(localptr[3]);
		MOV32ItoR(gprT1, 0x80000000);
		SSE2_PCMPEQB_XMM_to_XMM(to, to);
		SSE2_MOVD_R_to_XMM(from, gprT1);
		SSE_MOVSS_XMM_to_XMM(to, from);
		SSE2_PCMPEQB_XMM_to_XMM(from, from);
	localptr[7] = JMP8(0);

	x86SetJ8(localptr[2]);
	x86SetJ8(localptr[4]);
	x86SetJ8(localptr[5]);
	x86SetJ8(localptr[6]);
	x86SetJ8(localptr[7]);

	SSE_ANDPS_XMM_to_XMM(to,   t1); // to   contains mask
	SSE_ANDPS_XMM_to_XMM(from, t2); // from contains mask
	SSE_ADDSS_XMM_to_XMM(to, from);
	if (t1b) mVU->regAlloc->clearNeeded(t1);
	if (t2b) mVU->regAlloc->clearNeeded(t2);
}

#define clampOp(opX, isPS) {					\
	mVUclamp3(mVU, to,   t1, (isPS)?0xf:0x8);	\
	mVUclamp3(mVU, from, t1, (isPS)?0xf:0x8);	\
	opX(to, from);								\
	mVUclamp4(to, t1, (isPS)?0xf:0x8);			\
}

void SSE_MAXPS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	if (CHECK_VU_MINMAXHACK) { SSE_MAXPS_XMM_to_XMM(to, from); }
	else					 { MIN_MAX_PS(mVU, to, from, t1, t2, 0); }
}
void SSE_MINPS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	if (CHECK_VU_MINMAXHACK) { SSE_MINPS_XMM_to_XMM(to, from); }
	else					 { MIN_MAX_PS(mVU, to, from, t1, t2, 1); }
}
void SSE_MAXSS(mV, int to, int from, int t1 = -1, int t2 = -1) { 
	if (CHECK_VU_MINMAXHACK) { SSE_MAXSS_XMM_to_XMM(to, from); }
	else					 { MIN_MAX_SS(mVU, to, from, t1, 0); }
}
void SSE_MINSS(mV, int to, int from, int t1 = -1, int t2 = -1) { 
	if (CHECK_VU_MINMAXHACK) { SSE_MINSS_XMM_to_XMM(to, from); }
	else					 { MIN_MAX_SS(mVU, to, from, t1, 1); }
}
void SSE_ADD2SS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	if (!CHECK_VUADDSUBHACK) { clampOp(SSE_ADDSS_XMM_to_XMM, 0); }
	else					 { ADD_SS(mVU, to, from, t1, t2); }
}

void SSE_ADD2PS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_ADDPS_XMM_to_XMM, 1);
}
void SSE_ADDPS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_ADDPS_XMM_to_XMM, 1);
}
void SSE_ADDSS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_ADDSS_XMM_to_XMM, 0);
}
void SSE_SUBPS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_SUBPS_XMM_to_XMM, 1);
}
void SSE_SUBSS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_SUBSS_XMM_to_XMM, 0);
}
void SSE_MULPS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_MULPS_XMM_to_XMM, 1);
}
void SSE_MULSS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_MULSS_XMM_to_XMM, 0);
}
void SSE_DIVPS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_DIVPS_XMM_to_XMM, 1);
}
void SSE_DIVSS(mV, int to, int from, int t1 = -1, int t2 = -1) {
	clampOp(SSE_DIVSS_XMM_to_XMM, 0);
}

//------------------------------------------------------------------
// Micro VU - Custom Quick Search
//------------------------------------------------------------------

static __pagealigned u8 mVUsearchXMM[__pagesize];

// Generates a custom optimized block-search function 
// Note: Structs must be 16-byte aligned! (GCC doesn't guarantee this)
void mVUcustomSearch() {
	HostSys::MemProtectStatic(mVUsearchXMM, Protect_ReadWrite, false);
	memset_8<0xcc,__pagesize>(mVUsearchXMM);
	xSetPtr(mVUsearchXMM);

	xMOVAPS  (xmm0, ptr32[ecx]);
	xPCMP.EQD(xmm0, ptr32[edx]);
	xMOVAPS  (xmm1, ptr32[ecx + 0x10]);
	xPCMP.EQD(xmm1, ptr32[edx + 0x10]);
	xPAND	 (xmm0, xmm1);

	xMOVMSKPS(eax, xmm0);
	xCMP	 (eax, 0xf);
	xForwardJL8 exitPoint;

	xMOVAPS  (xmm0, ptr32[ecx + 0x20]);
	xPCMP.EQD(xmm0, ptr32[edx + 0x20]);
	xMOVAPS	 (xmm1, ptr32[ecx + 0x30]);
	xPCMP.EQD(xmm1, ptr32[edx + 0x30]);
	xPAND	 (xmm0, xmm1);

	xMOVAPS  (xmm2, ptr32[ecx + 0x40]);
	xPCMP.EQD(xmm2, ptr32[edx + 0x40]);
	xMOVAPS  (xmm3, ptr32[ecx + 0x50]);
	xPCMP.EQD(xmm3, ptr32[edx + 0x50]);
	xPAND	 (xmm2, xmm3);

	xMOVAPS	 (xmm4, ptr32[ecx + 0x60]);
	xPCMP.EQD(xmm4, ptr32[edx + 0x60]);
	xMOVAPS	 (xmm5, ptr32[ecx + 0x70]);
	xPCMP.EQD(xmm5, ptr32[edx + 0x70]);
	xPAND	 (xmm4, xmm5);

	xMOVAPS  (xmm6, ptr32[ecx + 0x80]);
	xPCMP.EQD(xmm6, ptr32[edx + 0x80]);
	xMOVAPS  (xmm7, ptr32[ecx + 0x90]);
	xPCMP.EQD(xmm7, ptr32[edx + 0x90]);
	xPAND	 (xmm6, xmm7);

	xPAND (xmm0, xmm2);
	xPAND (xmm4, xmm6);
	xPAND (xmm0, xmm4);
	xMOVMSKPS(eax, xmm0);

	exitPoint.SetTarget();
	xRET();
	HostSys::MemProtectStatic(mVUsearchXMM, Protect_ReadOnly, true);
}
