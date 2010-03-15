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
#include "R5900OpcodeTables.h"
#include "iR5900LoadStore.h"
#include "iR5900.h"

// Implemented at the bottom of the module:
void SetFastMemory(int bSetFast);

namespace R5900 { 
namespace Dynarec { 
namespace OpcodeImpl {

/*********************************************************
* Load and store for GPR                                 *
* Format:  OP rt, offset(base)                           *
*********************************************************/
#ifndef LOADSTORE_RECOMPILE

namespace Interp = R5900::Interpreter::OpcodeImpl;

REC_FUNC_DEL(LB, _Rt_);
REC_FUNC_DEL(LBU, _Rt_);
REC_FUNC_DEL(LH, _Rt_);
REC_FUNC_DEL(LHU, _Rt_);
REC_FUNC_DEL(LW, _Rt_);
REC_FUNC_DEL(LWU, _Rt_);
REC_FUNC_DEL(LWL, _Rt_);
REC_FUNC_DEL(LWR, _Rt_);
REC_FUNC_DEL(LD, _Rt_);
REC_FUNC_DEL(LDR, _Rt_);
REC_FUNC_DEL(LDL, _Rt_);
REC_FUNC_DEL(LQ, _Rt_);
REC_FUNC(SB);
REC_FUNC(SH);
REC_FUNC(SW);
REC_FUNC(SWL);
REC_FUNC(SWR);
REC_FUNC(SD);
REC_FUNC(SDL);
REC_FUNC(SDR);
REC_FUNC(SQ);
REC_FUNC(LWC1);
REC_FUNC(SWC1);
REC_FUNC(LQC2);
REC_FUNC(SQC2);

#else

__aligned16 u64 retValues[2];

void _eeOnLoadWrite(int reg)
{
	int regt;

	if( !reg ) return;

	_eeOnWriteReg(reg, 1);
	regt = _checkXMMreg(XMMTYPE_GPRREG, reg, MODE_READ);

	if( regt >= 0 ) {
		if( xmmregs[regt].mode & MODE_WRITE ) {
			if( reg != _Rs_ ) {
				SSE2_PUNPCKHQDQ_XMM_to_XMM(regt, regt);
				SSE2_MOVQ_XMM_to_M64((u32)&cpuRegs.GPR.r[reg].UL[2], regt);
			}
			else SSE_MOVHPS_XMM_to_M64((u32)&cpuRegs.GPR.r[reg].UL[2], regt);
		}
		xmmregs[regt].inuse = 0;
	}
}

using namespace Interpreter::OpcodeImpl;

__aligned16 u32 dummyValue[4];

void SetFastMemory(int bSetFast)
{
	// nothing
}

//////////////////////////////////////////////////////////////////////////////////////////
//
void recLoad64( u32 bits, bool sign )
{
	jASSUME( bits == 64 || bits == 128 );

	//no int 3? i love to get my hands dirty ;p - Raz
	//write8(0xCC);

	// Load EDX with the destination.
	// 64/128 bit modes load the result directly into the cpuRegs.GPR struct.

	if( _Rt_ )
		MOV32ItoR(EDX, (uptr)&cpuRegs.GPR.r[ _Rt_ ].UL[ 0 ] );
	else
		MOV32ItoR(EDX, (uptr)&dummyValue[0] );

	if( GPR_IS_CONST1( _Rs_ ) )
	{
		_eeOnLoadWrite(_Rt_);
		_deleteEEreg(_Rt_, 0);
		u32 srcadr = g_cpuConstRegs[_Rs_].UL[0] + _Imm_;
		if( bits == 128 ) srcadr &= ~0x0f;
		vtlb_DynGenRead64_Const( bits, srcadr );
	}
	else
	{
		// Load ECX with the source memory address that we're reading from.
		_eeMoveGPRtoR(ECX, _Rs_);
		if ( _Imm_ != 0 )
			ADD32ItoR( ECX, _Imm_ );
		if( bits == 128 )		// force 16 byte alignment on 128 bit reads
			AND32ItoR(ECX,~0x0F);	// emitter automatically encodes this as an 8-bit sign-extended imm8

		_eeOnLoadWrite(_Rt_);
		_deleteEEreg(_Rt_, 0);

		vtlb_DynGenRead64(bits);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//
void recLoad32( u32 bits, bool sign )
{
	jASSUME( bits <= 32 );

	//no int 3? i love to get my hands dirty ;p - Raz
	//write8(0xCC);

	// 8/16/32 bit modes return the loaded value in EAX.

	if( GPR_IS_CONST1( _Rs_ ) )
	{
		_eeOnLoadWrite(_Rt_);
		_deleteEEreg(_Rt_, 0);

		u32 srcadr = g_cpuConstRegs[_Rs_].UL[0] + _Imm_;
		vtlb_DynGenRead32_Const( bits, sign, srcadr );
	}
	else
	{
		// Load ECX with the source memory address that we're reading from.
		_eeMoveGPRtoR(ECX, _Rs_);
		if ( _Imm_ != 0 )
			ADD32ItoR( ECX, _Imm_ );

		_eeOnLoadWrite(_Rt_);
		_deleteEEreg(_Rt_, 0);
		vtlb_DynGenRead32(bits, sign);
	}

	if( _Rt_ )
	{
		// EAX holds the loaded value, so sign extend as needed:
		if (sign)
			CDQ();
		else
			XOR32RtoR(EDX,EDX);

		MOV32RtoM( (int)&cpuRegs.GPR.r[ _Rt_ ].UL[ 0 ], EAX );
		MOV32RtoM( (int)&cpuRegs.GPR.r[ _Rt_ ].UL[ 1 ], EDX );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//

// edxAlreadyAssigned - set to true if edx already holds the value being written (used by SWL/SWR)
void recStore(u32 sz, bool edxAlreadyAssigned=false)
{
	// Performance note: Const prop for the store address is good, always.
	// Constprop for the value being stored is not really worthwhile (better to use register
	// allocation -- simpler code and just as fast)

	// Load EDX first with the value being written, or the address of the value
	// being written (64/128 bit modes).  TODO: use register allocation, if the
	// value is allocated to a register.

	if( !edxAlreadyAssigned )
	{
		if( sz < 64 )
		{
			_eeMoveGPRtoR(EDX, _Rt_);
		}
		else if (sz==128 || sz==64)
		{
			_deleteEEreg(_Rt_, 1);		// flush register to mem
			MOV32ItoR(EDX,(int)&cpuRegs.GPR.r[ _Rt_ ].UL[ 0 ]);
		}
	}

	// Load ECX with the destination address, or issue a direct optimized write
	// if the address is a constant propagation.

	if( GPR_IS_CONST1( _Rs_ ) )
	{
		u32 dstadr = g_cpuConstRegs[_Rs_].UL[0] + _Imm_;
		if( sz == 128 ) dstadr &= ~0x0f;
		vtlb_DynGenWrite_Const( sz, dstadr );
	}
	else
	{
		_eeMoveGPRtoR(ECX, _Rs_);

		if ( _Imm_ != 0 )
			ADD32ItoR(ECX, _Imm_);
		if (sz==128)
			AND32ItoR(ECX,~0x0F);

		vtlb_DynGenWrite(sz);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//
void recLB( void )  { recLoad32(8,true); }
void recLBU( void ) { recLoad32(8,false); }
void recLH( void )  { recLoad32(16,true); }
void recLHU( void ) { recLoad32(16,false); }
void recLW( void )  { recLoad32(32,true); }
void recLWU( void ) { recLoad32(32,false); }
void recLD( void )  { recLoad64(64,false); }
void recLQ( void )  { recLoad64(128,false); }

void recSB( void )  { recStore(8); }
void recSH( void )  { recStore(16); }
void recSW( void )  { recStore(32); }
void recSQ( void )  { recStore(128); }
void recSD( void )  { recStore(64); }

//////////////////////////////////////////////////////////////////////////////////////////
// Non-recompiled Implementations Start Here -->
// (LWL/SWL, LWR/SWR, etc)

////////////////////////////////////////////////////
void recLWL( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_eeOnLoadWrite(_Rt_);
	_deleteEEreg(_Rt_, 1);

	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)LWL );
}

////////////////////////////////////////////////////
void recLWR( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_eeOnLoadWrite(_Rt_);
	_deleteEEreg(_Rt_, 1);

	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)LWR );   
}

static const u32 SWL_MASK[4] = { 0xffffff00, 0xffff0000, 0xff000000, 0x00000000 };
static const u32 SWR_MASK[4] = { 0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };

static const u8 SWR_SHIFT[4] = { 0, 8, 16, 24 };
static const u8 SWL_SHIFT[4] = { 24, 16, 8, 0 };

////////////////////////////////////////////////////
void recSWL( void ) 
{
	// Perform a translated memory read, followed by a translated memory write
	// of the "merged" result.
	
	// NOTE: Code incomplete. I'll fix/finish it soon. --air
	if( 0 ) //GPR_IS_CONST1( _Rs_ ) )
	{
		_eeOnLoadWrite(_Rt_);
		//_deleteEEreg(_Rt_, 0);

		u32 addr = g_cpuConstRegs[_Rs_].UL[0] + _Imm_;
		u32 shift = addr & 3;
		vtlb_DynGenRead32_Const( 32, false, addr & 3 );

		// Prep eax/edx for producing the writeback result:
		// equiv to:  (cpuRegs.GPR.r[_Rt_].UL[0] >> SWL_SHIFT[shift]) | (mem & SWL_MASK[shift])

		//_deleteEEreg(_Rt_, 1);
		//MOV32MtoR( EDX, (int)&cpuRegs.GPR.r[ _Rt_ ].UL[ 0 ] );

		_eeMoveGPRtoR(EDX, _Rt_);
		AND32ItoR( EAX, SWL_MASK[shift] );
		SHR32ItoR( EDX, SWL_SHIFT[shift] );
		OR32RtoR( EDX, EAX );

		recStore( 32, true );
	}
	else
	{
		_deleteEEreg(_Rs_, 1);
		_deleteEEreg(_Rt_, 1);
		MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
		//MOV32ItoM( (int)&cpuRegs.pc, pc );	// pc's not needed by SWL
		CALLFunc( (int)SWL );
	}
}

////////////////////////////////////////////////////
void recSWR( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_deleteEEreg(_Rt_, 1);
	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)SWR );
}

////////////////////////////////////////////////////
void recLDL( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_eeOnLoadWrite(_Rt_);
	_deleteEEreg(_Rt_, 1);
	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)LDL );
}

////////////////////////////////////////////////////
void recLDR( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_eeOnLoadWrite(_Rt_);
	_deleteEEreg(_Rt_, 1);
	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)LDR );
}

////////////////////////////////////////////////////
void recSDL( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_deleteEEreg(_Rt_, 1);
	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)SDL );
}

////////////////////////////////////////////////////
void recSDR( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_deleteEEreg(_Rt_, 1);
	MOV32ItoM( (int)&cpuRegs.code, cpuRegs.code );
	//MOV32ItoM( (int)&cpuRegs.pc, pc );
	CALLFunc( (int)SDR );
}

//////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************
* Load and store for COP1                                *
* Format:  OP rt, offset(base)                           *
*********************************************************/

////////////////////////////////////////////////////
void recLWC1( void )
{
	_deleteEEreg(_Rs_, 1);
	_deleteFPtoXMMreg(_Rt_, 2);

	MOV32MtoR( ECX, (int)&cpuRegs.GPR.r[ _Rs_ ].UL[ 0 ] );
	if ( _Imm_ != 0 )	
		ADD32ItoR( ECX, _Imm_ );

	vtlb_DynGenRead32(32, false);
	MOV32RtoM( (int)&fpuRegs.fpr[ _Rt_ ].UL, EAX );
}

////////////////////////////////////////////////////
void recSWC1( void )
{
	_deleteEEreg(_Rs_, 1);
	_deleteFPtoXMMreg(_Rt_, 0);

	MOV32MtoR( ECX, (int)&cpuRegs.GPR.r[ _Rs_ ].UL[ 0 ] );
	if ( _Imm_ != 0 )	
		ADD32ItoR( ECX, _Imm_ );

	MOV32MtoR(EDX, (int)&fpuRegs.fpr[ _Rt_ ].UL );
	vtlb_DynGenWrite(32);
}

////////////////////////////////////////////////////

/*********************************************************
* Load and store for COP2 (VU0 unit)                     *
* Format:  OP rt, offset(base)                           *
*********************************************************/

#define _Ft_ _Rt_
#define _Fs_ _Rd_
#define _Fd_ _Sa_

void recLQC2( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_deleteVFtoXMMreg(_Ft_, 0, 2);

	MOV32MtoR( ECX, (int)&cpuRegs.GPR.r[ _Rs_ ].UL[ 0 ] );
	if ( _Imm_ != 0 )
		ADD32ItoR( ECX, _Imm_);

	if ( _Rt_ )
		MOV32ItoR(EDX, (int)&VU0.VF[_Ft_].UD[0] );
	else
		MOV32ItoR(EDX, (int)&dummyValue[0] );

	vtlb_DynGenRead64(128);
}

////////////////////////////////////////////////////
void recSQC2( void ) 
{
	_deleteEEreg(_Rs_, 1);
	_deleteVFtoXMMreg(_Ft_, 0, 0);

	MOV32MtoR( ECX, (int)&cpuRegs.GPR.r[ _Rs_ ].UL[ 0 ] );
	if ( _Imm_ != 0 )
		ADD32ItoR( ECX, _Imm_ );

	MOV32ItoR(EDX, (int)&VU0.VF[_Ft_].UD[0] );
	vtlb_DynGenWrite(128);
}

#endif

} } }	// end namespace R5900::Dynarec::OpcodeImpl

using namespace R5900::Dynarec;
using namespace R5900::Dynarec::OpcodeImpl;

void SetFastMemory(int bSetFast) {}
