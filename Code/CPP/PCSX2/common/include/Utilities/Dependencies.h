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

// Dependencies.h : Contains classes required by all Utilities headers.

// --------------------------------------------------------------------------------------
//  Forward Declarations Section
// --------------------------------------------------------------------------------------

class wxOutputStream;
class wxInputStream;


// This should prove useful....
#define wxsFormat wxString::Format

// --------------------------------------------------------------------------------------
//  ImplementEnumOperators  (macro)
// --------------------------------------------------------------------------------------
// This macro implements ++/-- operators for any conforming enumeration.  In order for an
// enum to conform, it must have _FIRST and _COUNT members defined, and must have a full
// compliment of sequential members (no custom assignments) --- looking like so:
//
// enum Dummy {
//    Dummy_FIRST,
//    Dummy_Item = Dummy_FIRST,
//    Dummy_Crap,
//    Dummy_COUNT
// };
//
// The macro also defines utility functions for bounds checking enumerations:
//   EnumIsValid(value);   // returns TRUE if the enum value is between FIRST and COUNT.
//   EnumAssert(value);
//
// It also defines a *prototype* for converting the enumeration to a string.  Note that this
// method is not implemented!  You must implement it yourself if you want to use it:
//   EnumToString(value);
//
#define ImplementEnumOperators( enumName ) \
	static __forceinline enumName& operator++	( enumName& src ) { src = (enumName)((int)src+1); return src; } \
	static __forceinline enumName& operator--	( enumName& src ) { src = (enumName)((int)src-1); return src; } \
	static __forceinline enumName operator++	( enumName& src, int ) { enumName orig = src; src = (enumName)((int)src+1); return orig; } \
	static __forceinline enumName operator--	( enumName& src, int ) { enumName orig = src; src = (enumName)((int)src-1); return orig; } \
 \
	static __forceinline bool operator<	( const enumName& left, const pxEnumEnd_t& )	{ return (int)left < enumName##_COUNT; } \
	static __forceinline bool operator!=( const enumName& left, const pxEnumEnd_t& )	{ return (int)left != enumName##_COUNT; } \
	static __forceinline bool operator==( const enumName& left, const pxEnumEnd_t& )	{ return (int)left == enumName##_COUNT; } \
 \
	static __forceinline bool EnumIsValid( enumName id ) { \
		return ((int)id >= enumName##_FIRST) && ((int)id < enumName##_COUNT); } \
	static __forceinline bool EnumAssert( enumName id ) { \
		return pxAssert( EnumIsValid(id) ); } \
	static __forceinline void EnumAssume( enumName id ) { \
		pxAssume( EnumIsValid(id) ); } \
 \
	extern const wxChar* EnumToString( enumName id )

class pxEnumEnd_t { };
static const pxEnumEnd_t pxEnumEnd = {};

// --------------------------------------------------------------------------------------
//  DeclareNoncopyableObject
// --------------------------------------------------------------------------------------
// This macro provides an easy and clean method for ensuring objects are not copyable.
// Simply add the macro to the head or tail of your class declaration, and attempts to
// copy the class will give you a moderately obtuse compiler error that will have you
// scratching your head for 20 minutes.
//
// (... but that's probably better than having a weird invalid object copy having you
//  scratch your head for a day).
//
// Programmer's notes:
//  * We intentionally do NOT provide implementations for these methods, which should
//    never be referenced anyway.

//  * I've opted for macro form over multi-inherited class form (Boost style), because
//    the errors generated by the macro are considerably less voodoo.  The Boost-style
//    The macro reports the exact class that causes the copy failure, while Boost's class
//    approach just reports an error in whatever "NoncopyableObject" is inherited.
//
//  * This macro is the same as wxWidgets' DECLARE_NO_COPY_CLASS macro.  This one is free
//    of wx dependencies though, and has a nicer typeset. :)
//
#ifndef DeclareNoncopyableObject
#	define DeclareNoncopyableObject(classname)	\
	private:									\
	explicit classname(const classname&);		\
	classname& operator=(const classname&)
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// macro provided for tagging translation strings, without actually running them through the
// translator (which the _() does automatically, and sometimes we don't want that).  This is
// a shorthand replacement for wxTRANSLATE.
//
#define wxLt(a)		(a)

#include <wx/string.h>
#include <wx/gdicmn.h>		// for wxPoint/wxRect stuff
#include <wx/intl.h>
#include <wx/log.h>

#include "Pcsx2Defs.h"

#include <stdexcept>
#include <algorithm>
#include <string>
#include <cstring>		// string.h under c++
#include <cstdio>		// stdio.h under c++
#include <cstdlib>
#include <vector>
#include <list>

#include "Utilities/Assertions.h"
