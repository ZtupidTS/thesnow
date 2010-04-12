// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#ifndef _PPCANALYST_H
#define _PPCANALYST_H

#include <vector>
#include <map>

#include <string>

#include "Common.h"
#include "Gekko.h"

class PPCSymbolDB;
struct Symbol;

namespace PPCAnalyst
{

struct CodeOp //16B
{
	UGeckoInstruction inst;
	u32 address;
	u32 branchTo; //if 0, not a branch
	int branchToIndex; //index of target block
	s8 regsOut[2];
	s8 regsIn[3];
	s8 fregOut;
	s8 fregsIn[3];
	bool isBranchTarget;
	bool wantsCR0;
	bool wantsCR1;
	bool wantsPS1;
	bool outputCR0;
	bool outputCR1;
	bool outputPS1;
	bool skip;  // followed BL-s for example
};

struct BlockStats
{
	bool isFirstBlockOfFunction;
	bool isLastBlockOfFunction;
	int numCycles;
};

struct BlockRegStats
{
	short firstRead[32];
	short firstWrite[32];
	short lastRead[32];
	short lastWrite[32];
	short numReads[32];
	short numWrites[32];

	bool any;
	bool anyTimer;

	int GetTotalNumAccesses(int reg) {return numReads[reg] + numWrites[reg];}
	int GetUseRange(int reg) {
		return max(lastRead[reg], lastWrite[reg]) - 
			   min(firstRead[reg], firstWrite[reg]);}
};


class CodeBuffer
{
	int size_;
public:
	CodeBuffer(int size);
	~CodeBuffer();

	int GetSize() const { return size_; }

	PPCAnalyst::CodeOp *codebuffer;


};

u32 Flatten(u32 address, int *realsize, BlockStats *st, BlockRegStats *gpa, BlockRegStats *fpa, CodeBuffer *buffer, int blockSize);
void LogFunctionCall(u32 addr);
void FindFunctions(u32 startAddr, u32 endAddr, PPCSymbolDB *func_db);
bool AnalyzeFunction(u32 startAddr, Symbol &func, int max_size = 0);

}  // namespace

#endif

