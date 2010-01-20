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

#include "JitCommon/JitBase.h"

#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

#include "PPCSymbolDB.h"

namespace Profiler
{

bool g_ProfileBlocks;
bool g_ProfileInstructions;

struct BlockStat
{
	BlockStat(int bn, u64 c) : blockNum(bn), cost(c) {}
	int blockNum;
	u64 cost;

	bool operator <(const BlockStat &other) const {
		return cost > other.cost;
	}
};

void WriteProfileResults(const char *filename) {
	std::vector<BlockStat> stats;
	stats.reserve(jit->GetBlockCache()->GetNumBlocks());
	u64 cost_sum = 0;
#ifdef _WIN32
	u64 timecost_sum = 0;
	u64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSec);
#endif
	for (int i = 0; i < jit->GetBlockCache()->GetNumBlocks(); i++)
	{
		const JitBlock *block = jit->GetBlockCache()->GetBlock(i);
		u64 cost = (block->originalSize / 4) * block->runCount;		// rough heuristic. mem instructions should cost more.
#ifdef _WIN32
		u64 timecost = block->ticCounter;					// Indeed ;)
#endif
		if (block->runCount >= 1) {  // Todo: tweak.
			stats.push_back(BlockStat(i, cost));
		}
		cost_sum += cost;
#ifdef _WIN32
		timecost_sum += timecost;
#endif
	}

	sort(stats.begin(), stats.end());
	FILE *f = fopen(filename, "w");
	if (!f) {
		PanicAlert("failed to open %s", filename);
		return;
	}
	fprintf(f, "origAddr\tblkName\tcost\ttimeCost\tpercent\ttimePercent\tOvAllinBlkTime(ms)\tblkCodeSize\n");
	for (unsigned int i = 0; i < stats.size(); i++)
	{
		const JitBlock *block = jit->GetBlockCache()->GetBlock(stats[i].blockNum);
		if (block)
		{
			std::string name = g_symbolDB.GetDescription(block->originalAddress);
			double percent = 100.0 * (double)stats[i].cost / (double)cost_sum;
#ifdef _WIN32 
			double timePercent = 100.0 * (double)block->ticCounter / (double)timecost_sum;
			fprintf(f, "%08x\t%s\t%llu\t%llu\t%.2lf\t%llf\t%lf\t%i\n", 
				block->originalAddress, name.c_str(), stats[i].cost, block->ticCounter, percent, timePercent, (double)block->ticCounter*1000.0/(double)countsPerSec, block->codeSize);
#else
			fprintf(f, "%08x\t%s\t%llu\t???\t%.2lf\t???\t???\t%i\n", 
				block->originalAddress, name.c_str(), stats[i].cost,  /*block->ticCounter.QuadPart,*/ percent, /*timePercent, (double)block->ticCounter.QuadPart*1000.0/(double)countsPerSec.QuadPart,*/ block->codeSize);
#endif
		}
	}
	fclose(f);
}

}  // namespace
