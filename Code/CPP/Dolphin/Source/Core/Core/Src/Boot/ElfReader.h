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

#ifndef _ELFREADER_H
#define _ELFREADER_H

#include "ElfTypes.h"

enum KnownElfTypes
{
	KNOWNELF_PSP = 0,
	KNOWNELF_DS = 1,
	KNOWNELF_GBA = 2,
	KNOWNELF_GC = 3,
};

typedef int SectionID;

class ElfReader
{
	char *base;
	u32 *base32;

	Elf32_Ehdr *header;
	Elf32_Phdr *segments;
	Elf32_Shdr *sections;
	
	u32 *sectionOffsets;
	u32 *sectionAddrs;
	bool bRelocate;
	u32 entryPoint;

public:	
	ElfReader(void *ptr);
	~ElfReader() { }
	
	u32 Read32(int off) const { return base32[off>>2]; }

	// Quick accessors
	ElfType GetType() const { return (ElfType)(header->e_type); }
	ElfMachine GetMachine() const { return (ElfMachine)(header->e_machine); }
	u32 GetEntryPoint() const { return entryPoint; }
	u32 GetFlags() const { return (u32)(header->e_flags); }

	int GetNumSegments() const { return (int)(header->e_phnum); }
	int GetNumSections() const { return (int)(header->e_shnum); }
	const u8 *GetPtr(int offset) const { return (u8*)base + offset; }
	const char *GetSectionName(int section) const;
	const u8 *GetSectionDataPtr(int section) const
	{
		if (section < 0 || section >= header->e_shnum)
			return 0;
		if (sections[section].sh_type != SHT_NOBITS)
			return GetPtr(sections[section].sh_offset);
		else
			return 0;
	}
	const u8 *GetSegmentPtr(int segment)
	{
		return GetPtr(segments[segment].p_offset);
	}
	u32 GetSectionAddr(SectionID section) const { return sectionAddrs[section]; }
	int GetSectionSize(SectionID section) const { return sections[section].sh_size;	}
	SectionID GetSectionByName(const char *name, int firstSection = 0) const; //-1 for not found

	bool DidRelocate() {
		return bRelocate;
	}
	// More indepth stuff:)
	bool LoadInto(u32 vaddr);
	bool LoadSymbols();
};

#endif

