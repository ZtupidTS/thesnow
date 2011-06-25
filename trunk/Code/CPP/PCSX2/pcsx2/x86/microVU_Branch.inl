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

extern bool  doEarlyExit (microVU& mVU);
extern void  mVUincCycles(microVU& mVU, int x);
extern void* mVUcompile  (microVU& mVU, u32 startPC, uptr pState);

__fi int getLastFlagInst(microRegInfo& pState, int* xFlag, int flagType, int isEbit) {
	if (isEbit) return findFlagInst(xFlag, 0x7fffffff);
	if (pState.needExactMatch & (1<<flagType)) return 3;
	return (((pState.flagInfo >> (2*flagType+2)) & 3) - 1) & 3;
}

void mVU0clearlpStateJIT() { if (!microVU0.prog.cleared) memzero(microVU0.prog.lpState); }
void mVU1clearlpStateJIT() { if (!microVU1.prog.cleared) memzero(microVU1.prog.lpState); }

void mVUendProgram(mV, microFlagCycles* mFC, int isEbit) {

	int fStatus = getLastFlagInst(mVUpBlock->pState, mFC->xStatus, 0, isEbit);
	int fMac	= getLastFlagInst(mVUpBlock->pState, mFC->xMac,    1, isEbit);
	int fClip	= getLastFlagInst(mVUpBlock->pState, mFC->xClip,   2, isEbit);
	int qInst	= 0;
	int pInst	= 0;
	mVU.regAlloc->flushAll();

	if (isEbit) {
		memzero(mVUinfo);
		memzero(mVUregsTemp);
		mVUincCycles(mVU, 100); // Ensures Valid P/Q instances (And sets all cycle data to 0)
		mVUcycles -= 100;
		qInst = mVU.q;
		pInst = mVU.p;
		if (mVUinfo.doDivFlag) {
			sFLAG.doFlag = 1;
			sFLAG.write  = fStatus;
			mVUdivSet(mVU);
		}
		if (mVUinfo.doXGKICK) { mVU_XGKICK_DELAY(mVU, 1); }
		if (doEarlyExit(mVU)) {
			if (!isVU1) xCALL(mVU0clearlpStateJIT);
			else		xCALL(mVU1clearlpStateJIT);
		}
	}

	// Save P/Q Regs
	if (qInst) { xPSHUF.D(xmmPQ, xmmPQ, 0xe5); }
	xMOVSS(ptr32[&mVU.regs().VI[REG_Q].UL], xmmPQ);
	if (isVU1) {
		xPSHUF.D(xmmPQ, xmmPQ, pInst ? 3 : 2);
		xMOVSS(ptr32[&mVU.regs().VI[REG_P].UL], xmmPQ);
	}

	// Save Flag Instances
#if 1 // CHECK_MACROVU0 - Always on now
	xMOV(ptr32[&mVU.regs().VI[REG_STATUS_FLAG].UL],	getFlagReg(fStatus));
#else
	mVUallocSFLAGc(gprT1, gprT2, fStatus);
	xMOV(ptr32[&mVU.regs().VI[REG_STATUS_FLAG].UL],	gprT1);
#endif
	mVUallocMFLAGa(mVU, gprT1, fMac);
	mVUallocCFLAGa(mVU, gprT2, fClip);
	xMOV(ptr32[&mVU.regs().VI[REG_MAC_FLAG].UL],	gprT1);
	xMOV(ptr32[&mVU.regs().VI[REG_CLIP_FLAG].UL],	gprT2);

	if (isEbit || isVU1) { // Clear 'is busy' Flags
		xAND(ptr32[&VU0.VI[REG_VPU_STAT].UL], (isVU1 ? ~0x100 : ~0x001)); // VBS0/VBS1 flag
		xAND(ptr32[&mVU.getVifRegs().stat], ~VIF1_STAT_VEW); // Clear VU 'is busy' signal for vif
	}

	if (isEbit != 2) { // Save PC, and Jump to Exit Point
		xMOV(ptr32[&mVU.regs().VI[REG_TPC].UL], xPC);
		xJMP(mVU.exitFunct);
	}
}

// Recompiles Code for Proper Flags and Q/P regs on Block Linkings
void mVUsetupBranch(mV, microFlagCycles& mFC) {
	
	mVU.regAlloc->flushAll();	// Flush Allocated Regs
	mVUsetupFlags(mVU, mFC);	// Shuffle Flag Instances

	// Shuffle P/Q regs since every block starts at instance #0
	if (mVU.p || mVU.q) { xPSHUF.D(xmmPQ, xmmPQ, shufflePQ); }
}

void normBranchCompile(microVU& mVU, u32 branchPC) {
	microBlock* pBlock;
	blockCreate(branchPC/8);
	pBlock = mVUblocks[branchPC/8]->search((microRegInfo*)&mVUregs);
	if (pBlock)	{ xJMP(pBlock->x86ptrStart); }
	else		{ mVUcompile(mVU, branchPC, (uptr)&mVUregs); }
}

void normJumpCompile(mV, microFlagCycles& mFC, bool isEvilJump) {
	memcpy_const(&mVUpBlock->pStateEnd, &mVUregs, sizeof(microRegInfo));
	mVUsetupBranch(mVU, mFC);
	mVUbackupRegs(mVU);

	if(!mVUpBlock->jumpCache) { // Create the jump cache for this block
		mVUpBlock->jumpCache = new microJumpCache[mProgSize/2];
	}

	if (isEvilJump)		xMOV(gprT2, ptr32[&mVU.evilBranch]);
	else				xMOV(gprT2, ptr32[&mVU.branch]);
	if (doJumpCaching)	xMOV(gprT3, (uptr)mVUpBlock);
	else				xMOV(gprT3, (uptr)&mVUpBlock->pStateEnd);

	if (!mVU.index) xCALL(mVUcompileJIT<0>); //(u32 startPC, uptr pState)
	else			xCALL(mVUcompileJIT<1>);

	mVUrestoreRegs(mVU);
	xJMP(gprT1);  // Jump to rec-code address
}

void normBranch(mV, microFlagCycles& mFC) {

	// E-bit Branch
	if (mVUup.eBit) { iPC = branchAddr/4; mVUendProgram(mVU, &mFC, 1); return; }
	
	// Normal Branch
	mVUsetupBranch(mVU, mFC);
	normBranchCompile(mVU, branchAddr);
}

void condBranch(mV, microFlagCycles& mFC, int JMPcc) {
	mVUsetupBranch(mVU, mFC);
	xCMP(ptr16[&mVU.branch], 0);
	incPC(3);
	if (mVUup.eBit) { // Conditional Branch With E-Bit Set
		mVUendProgram(mVU, &mFC, 2);
		xForwardJump8 eJMP((JccComparisonType)JMPcc);
			incPC(1); // Set PC to First instruction of Non-Taken Side
			xMOV(ptr32[&mVU.regs().VI[REG_TPC].UL], xPC);
			xJMP(mVU.exitFunct);
		eJMP.SetTarget();
		incPC(-4); // Go Back to Branch Opcode to get branchAddr
		iPC = branchAddr/4;
		xMOV(ptr32[&mVU.regs().VI[REG_TPC].UL], xPC);
		xJMP(mVU.exitFunct);
		return;
	}
	else { // Normal Conditional Branch
		microBlock* bBlock;
		incPC2(1); // Check if Branch Non-Taken Side has already been recompiled
		blockCreate(iPC/2);
		bBlock = mVUblocks[iPC/2]->search((microRegInfo*)&mVUregs);
		incPC2(-1);
		if (bBlock)	{ // Branch non-taken has already been compiled
			xJcc(xInvertCond((JccComparisonType)JMPcc), bBlock->x86ptrStart);
			incPC(-3); // Go back to branch opcode (to get branch imm addr)
			normBranchCompile(mVU, branchAddr);
		}
		else { 
			s32* ajmp = xJcc32((JccComparisonType)JMPcc); 
			u32 bPC = iPC; // mVUcompile can modify iPC, mVUpBlock, and mVUregs so back them up
			microBlock* pBlock = mVUpBlock;
			memcpy_const(&pBlock->pStateEnd, &mVUregs, sizeof(microRegInfo));

			incPC2(1);  // Get PC for branch not-taken
			mVUcompile(mVU, xPC, (uptr)&mVUregs);

			iPC = bPC;
			incPC(-3); // Go back to branch opcode (to get branch imm addr)
			uptr jumpAddr = (uptr)mVUblockFetch(mVU, branchAddr, (uptr)&pBlock->pStateEnd);
			*ajmp = (jumpAddr - ((uptr)ajmp + 4));
		}
	}
}

void normJump(mV, microFlagCycles& mFC) {
	if (mVUlow.constJump.isValid) { // Jump Address is Constant
		if (mVUup.eBit) { // E-bit Jump
			iPC = (mVUlow.constJump.regValue*2) & (mVU.progMemMask);
			mVUendProgram(mVU, &mFC, 1);
			return;
		}
		int jumpAddr = (mVUlow.constJump.regValue*8)&(mVU.microMemSize-8);
		mVUsetupBranch(mVU, mFC);
		normBranchCompile(mVU, jumpAddr);
		return;
	}

	if (mVUup.eBit) { // E-bit Jump
		mVUendProgram(mVU, &mFC, 2);
		xMOV(gprT1, ptr32[&mVU.branch]);
		xMOV(ptr32[&mVU.regs().VI[REG_TPC].UL], gprT1);
		xJMP(mVU.exitFunct);
	}
	else normJumpCompile(mVU, mFC, 0);
}
