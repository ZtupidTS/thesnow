/*====================================================================

   project:      GameCube DSP Tool (gcdsp)
   created:      2005.03.04
   mail:		  duddie@walla.com

   Copyright (c) 2005 Duddie

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   ====================================================================*/

#ifndef _DSP_ASSEMBLE_H
#define _DSP_ASSEMBLE_H

#include <string>
#include <map>

#include "Common.h"
#include "disassemble.h"
#include "DSPTables.h"
#include "LabelMap.h"

enum err_t
{
	ERR_OK = 0,
	ERR_UNKNOWN,
	ERR_UNKNOWN_OPCODE,
	ERR_NOT_ENOUGH_PARAMETERS,
	ERR_TOO_MANY_PARAMETERS,
	ERR_WRONG_PARAMETER,
	ERR_EXPECTED_PARAM_STR,
	ERR_EXPECTED_PARAM_VAL,
	ERR_EXPECTED_PARAM_REG,
	ERR_EXPECTED_PARAM_MEM,
	ERR_EXPECTED_PARAM_IMM,
	ERR_INCORRECT_BIN,
	ERR_INCORRECT_HEX,
	ERR_INCORRECT_DEC,
	ERR_LABEL_EXISTS,
	ERR_UNKNOWN_LABEL,
	ERR_NO_MATCHING_BRACKETS,
	ERR_EXT_CANT_EXTEND_OPCODE,
	ERR_EXT_PAR_NOT_EXT,
	ERR_WRONG_PARAMETER_ACC,
	ERR_WRONG_PARAMETER_MID_ACC,
	ERR_INVALID_REGISTER,
	ERR_OUT_RANGE_NUMBER
};


// Unless you want labels to carry over between files, you probably
// want to create a new DSPAssembler for every file you assemble.
class DSPAssembler
{
public:
	DSPAssembler(const AssemblerSettings &settings);
	~DSPAssembler();

	// line_numbers is optional (and not yet implemented). It'll receieve a list of ints,
	// one for each word of code, indicating the source assembler code line number it came from.

	// If returns false, call GetErrorString to get some text to present to the user.
	bool Assemble(const char *text, std::vector<u16> &code, std::vector<int> *line_numbers = NULL);

	std::string GetErrorString() const { return last_error_str; }
	err_t GetError() const { return last_error; }

private:
	struct param_t
	{
		u32			val;
		partype_t	type;
		char		*str;
	};

	enum segment_t
	{
		SEGMENT_CODE = 0,
		SEGMENT_DATA,
		SEGMENT_OVERLAY,
		SEGMENT_MAX
	};

	// Utility functions
	s32 ParseValue(const char *str);
	u32 ParseExpression(const char *ptr);

	u32 GetParams(char *parstr, param_t *par);

	void InitPass(int pass);
	bool AssembleFile(const char *fname, int pass);

	void ShowError(err_t err_code, const char *extra_info = NULL);
	// void ShowWarning(err_t err_code, const char *extra_info = NULL);

	char *FindBrackets(char *src, char *dst);
	const opc_t *FindOpcode(const char *opcode, u32 par_count, const opc_t * const opcod, int opcod_size);
	bool VerifyParams(const opc_t *opc, param_t *par, int count, bool ext = false);
	void BuildCode(const opc_t *opc, param_t *par, u32 par_count, u16 *outbuf);

	char *gdg_buffer;

	std::string include_dir;
	std::string cur_line;

	u32 m_cur_addr;
	int m_totalSize;
	u8  m_cur_pass;
	
	LabelMap labels;

	u32	code_line;
	bool failed;
	std::string last_error_str;
	err_t last_error;

	typedef std::map<std::string, std::string> AliasMap;
	AliasMap aliases;

	segment_t cur_segment;
	u32 segment_addr[SEGMENT_MAX];
	int m_current_param;
	const AssemblerSettings settings_;
};

#endif  // _DSP_ASSEMBLE_H
