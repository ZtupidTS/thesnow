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
#include "Threading.h"
#include "TlsVariable.inl"

using namespace Threading;

// thread-local console indentation setting.
static DeclareTls(int) conlog_Indent( 0 );

// thread-local console color storage.
static DeclareTls(ConsoleColors) conlog_Color( DefaultConsoleColor );


static wxString	m_buffer;		// used by ConsoleBuffer
static Mutex	m_bufferlock;	// used by ConsoleBuffer

// This function re-assigns the console log writer(s) to the specified target.  It makes sure
// to flush any contents from the buffered console log (which typically accumulates due to
// log suspension during log file/window re-init operations) into the new log.
//
// Important!  Only Assert and Null console loggers are allowed during C++ startup init (when
// the program or DLL first loads).  Other log targets rely on the static buffer and a
// threaded mutex lock, which are only valid after C++ initialization has finished.
void Console_SetActiveHandler( const IConsoleWriter& writer, FILE* flushfp )
{
	pxAssertDev(
		(writer.DoWrite != NULL)	&& (writer.DoWriteLn != NULL) &&
		(writer.Newline != NULL)	&& (writer.SetTitle != NULL) &&
		(writer.DoSetColor != NULL),
		"Invalid IConsoleWriter object!  All function pointer interfaces must be implemented."
	);

	if( &writer != &ConsoleWriter_Buffered )
	{
		ScopedLock lock( m_bufferlock );
		if( !ConsoleBuffer_Get().IsEmpty() )
			writer.DoWriteLn( ConsoleBuffer_Get() );
	}

	Console	= writer;

#ifdef PCSX2_DEVBUILD
	DevCon	= writer;
#endif

#ifdef PCSX2_DEBUG
	DbgCon	= writer;
#endif
}

// --------------------------------------------------------------------------------------
//  ConsoleNull
// --------------------------------------------------------------------------------------

static void __concall ConsoleNull_SetTitle( const wxString& title ) {}
static void __concall ConsoleNull_DoSetColor( ConsoleColors color ) {}
static void __concall ConsoleNull_Newline() {}
static void __concall ConsoleNull_DoWrite( const wxString& fmt ) {}
static void __concall ConsoleNull_DoWriteLn( const wxString& fmt ) {}

const IConsoleWriter ConsoleWriter_Null =
{
	ConsoleNull_DoWrite,
	ConsoleNull_DoWriteLn,
	ConsoleNull_DoSetColor,

	ConsoleNull_DoWrite,
	ConsoleNull_Newline,
	ConsoleNull_SetTitle,

	0,		// instance-level indentation (should always be 0)
};

// --------------------------------------------------------------------------------------
//  Console_Stdout
// --------------------------------------------------------------------------------------

#ifdef __LINUX__
static __forceinline const wxChar* GetLinuxConsoleColor(ConsoleColors color)
{
    switch(color)
    {
        case Color_Black: return L"\033[30m";
        case Color_StrongBlack: return L"\033[30m\033[1m";

        case Color_Red: return L"\033[31m";
        case Color_StrongRed: return L"\033[31m\033[1m";

        case Color_Green: return L"\033[32m";
        case Color_StrongGreen: return L"\033[32m\033[1m";

        case Color_Yellow: return L"\033[33m";
        case Color_StrongYellow: return L"\033[33m\033[1m";

        case Color_Blue: return L"\033[34m";
        case Color_StrongBlue: return L"\033[34m\033[1m";

        // No orange, so use magenta.
        case Color_Orange:
        case Color_Magenta: return L"\033[35m";
        case Color_StrongOrange:
        case Color_StrongMagenta: return L"\033[35m\033[1m";

        case Color_Cyan: return L"\033[36m";
        case Color_StrongCyan: return L"\033[36m\033[1m";

        // Use 'white' instead of grey.
        case Color_Gray:
        case Color_White: return L"\033[37m";
        case Color_StrongGray:
        case Color_StrongWhite: return L"\033[37m\033[1m";

        // On some other value being passed, clear any formatting.
        case Color_Default:
        default: return L"\033[0m";
    }
}
#endif

// One possible default write action at startup and shutdown is to use the stdout.
static void __concall ConsoleStdout_DoWrite( const wxString& fmt )
{
	wxPrintf( fmt );
}

// Default write action at startup and shutdown is to use the stdout.
static void __concall ConsoleStdout_DoWriteLn( const wxString& fmt )
{
	wxPrintf( fmt + L"\n" );
}

static void __concall ConsoleStdout_Newline()
{
	wxPrintf( L"\n" );
}

static void __concall ConsoleStdout_DoSetColor( ConsoleColors color )
{
#ifdef __LINUX__
	wxPrintf(L"\033[0m");
    wxPrintf(GetLinuxConsoleColor(color));
#endif
}

static void __concall ConsoleStdout_SetTitle( const wxString& title )
{
#ifdef __LINUX__
	wxPrintf(L"\033]0;" + title + L"\007");
#endif
}

const IConsoleWriter ConsoleWriter_Stdout =
{
	ConsoleStdout_DoWrite,			// Writes without newlines go to buffer to avoid error log spam.
	ConsoleStdout_DoWriteLn,
	ConsoleStdout_DoSetColor,

	ConsoleNull_DoWrite,			// writes from stdout are ignored here, lest we create infinite loop hell >_<
	ConsoleStdout_Newline,
	ConsoleStdout_SetTitle,

	0,		// instance-level indentation (should always be 0)
};

// --------------------------------------------------------------------------------------
//  ConsoleAssert
// --------------------------------------------------------------------------------------

static void __concall ConsoleAssert_DoWrite( const wxString& fmt )
{
	pxFail( L"Console class has not been initialized; Message written:\n\t" + fmt );
}

static void __concall ConsoleAssert_DoWriteLn( const wxString& fmt )
{
	pxFail( L"Console class has not been initialized; Message written:\n\t" + fmt );
}

const IConsoleWriter ConsoleWriter_Assert =
{
	ConsoleAssert_DoWrite,
	ConsoleAssert_DoWriteLn,
	ConsoleNull_DoSetColor,

	ConsoleNull_DoWrite,
	ConsoleNull_Newline,
	ConsoleNull_SetTitle,

	0,		// instance-level indentation (should always be 0)
};

// --------------------------------------------------------------------------------------
//  ConsoleBuffer
// --------------------------------------------------------------------------------------

const wxString& ConsoleBuffer_Get()
{
	return m_buffer;
}

void ConsoleBuffer_Clear()
{
	ScopedLock lock( m_bufferlock );
	m_buffer.Clear();
}

// Flushes the contents of the ConsoleBuffer to the specified destination file stream, and
// clears the buffer contents to 0.
void ConsoleBuffer_FlushToFile( FILE *fp )
{
	ScopedLock lock( m_bufferlock );
	if( fp == NULL || m_buffer.IsEmpty() ) return;
	px_fputs( fp, m_buffer.ToUTF8() );
	m_buffer.Clear();
}

static void __concall ConsoleBuffer_DoWrite( const wxString& fmt )
{
	ScopedLock lock( m_bufferlock );
	m_buffer += fmt;
}

static void __concall ConsoleBuffer_DoWriteLn( const wxString& fmt )
{
	ScopedLock lock( m_bufferlock );
	m_buffer += fmt + L"\n";
}

const IConsoleWriter ConsoleWriter_Buffered =
{
	ConsoleBuffer_DoWrite,			// Writes without newlines go to buffer to avoid assertion spam.
	ConsoleBuffer_DoWriteLn,
	ConsoleNull_DoSetColor,

	ConsoleBuffer_DoWrite,
	ConsoleNull_Newline,
	ConsoleNull_SetTitle,

	0,		// instance-level indentation (should always be 0)
};

// --------------------------------------------------------------------------------------
//  Console_wxLogError
// --------------------------------------------------------------------------------------

static void __concall Console_wxLogError_DoWriteLn( const wxString& fmt )
{
	if( !m_buffer.IsEmpty() )
	{
		wxLogError( m_buffer );
		m_buffer.Clear();
	}
	wxLogError( fmt );
}

const IConsoleWriter ConsoleWriter_wxError =
{
	ConsoleBuffer_DoWrite,			// Writes without newlines go to buffer to avoid error log spam.
	Console_wxLogError_DoWriteLn,
	ConsoleNull_DoSetColor,

	ConsoleBuffer_DoWrite,
	ConsoleNull_Newline,
	ConsoleNull_SetTitle,

	0,		// instance-level indentation (should always be 0)
};

// Sanity check: truncate strings if they exceed 512k in length.  Anything like that
// is either a bug or really horrible code that needs to be stopped before it causes
// system deadlock.
static const int MaxFormattedStringLength = 0x80000;

template< typename CharType >
class FormatBuffer : public Mutex
{
public:
	bool&				clearbit;
	SafeArray<CharType>	buffer;

	FormatBuffer( bool& bit_to_clear_on_destruction ) :
		clearbit( bit_to_clear_on_destruction )
	,	buffer( 4096, wxsFormat( L"%s Format Buffer", (sizeof(CharType)==1) ? "Ascii" : "Unicode" ) )
	{
	}

	virtual ~FormatBuffer() throw()
	{
		clearbit = true;
		Wait();		// lock the mutex, just in case.
	}

};

static bool ascii_buffer_is_deleted = false;
static bool unicode_buffer_is_deleted = false;

static FormatBuffer<char>	ascii_buffer( ascii_buffer_is_deleted );
static FormatBuffer<wxChar>	unicode_buffer( unicode_buffer_is_deleted );

static void format_that_ascii_mess( SafeArray<char>& buffer, const char* fmt, va_list argptr )
{

	while( true )
	{
		int size = buffer.GetLength();
		int len = vsnprintf(buffer.GetPtr(), size, fmt, argptr);

		// some implementations of vsnprintf() don't NUL terminate
		// the string if there is not enough space for it so
		// always do it manually
		buffer[size-1] = '\0';

		if( size >= MaxFormattedStringLength ) break;

		// vsnprintf() may return either -1 (traditional Unix behavior) or the
		// total number of characters which would have been written if the
		// buffer were large enough (newer standards such as Unix98)

		if ( len < 0 )
			len = size + (size/4);

		if ( len < size ) break;
		buffer.ExactAlloc( len + 1 );
	};

	// performing an assertion or log of a truncated string is unsafe, so let's not; even
	// though it'd be kinda nice if we did.
}

static void format_that_unicode_mess( SafeArray<wxChar>& buffer, const wxChar* fmt, va_list argptr)
{
	while( true )
	{
		int size = buffer.GetLength();
		int len = wxVsnprintf(buffer.GetPtr(), size, fmt, argptr);

		// some implementations of vsnprintf() don't NUL terminate
		// the string if there is not enough space for it so
		// always do it manually
		buffer[size-1] = L'\0';

		if( size >= MaxFormattedStringLength ) break;

		// vsnprintf() may return either -1 (traditional Unix behavior) or the
		// total number of characters which would have been written if the
		// buffer were large enough (newer standards such as Unix98)

		if ( len < 0 )
			len = size + (size/4);

		if ( len < size ) break;
		buffer.ExactAlloc( len + 1 );
	};

	// performing an assertion or log of a truncated string is unsafe, so let's not; even
	// though it'd be kinda nice if we did.
}

static wxString ascii_format_string(const char* fmt, va_list argptr)
{
	if( ascii_buffer_is_deleted )
	{
		SafeArray<char>	localbuf( 4096, L"Temporary Ascii Formatting Buffer" );
		format_that_ascii_mess( localbuf, fmt, argptr );
		return fromUTF8( localbuf.GetPtr() );
	}
	else
	{
		ScopedLock locker( ascii_buffer );
		format_that_ascii_mess( ascii_buffer.buffer, fmt, argptr );
		return fromUTF8( ascii_buffer.buffer.GetPtr() );
	}
}


static wxString unicode_format_string(const wxChar* fmt, va_list argptr)
{
	if( unicode_buffer_is_deleted )
	{
		SafeArray<wxChar> localbuf( 4096, L"Temporary Unicode Formatting Buffer" );
		format_that_unicode_mess( localbuf, fmt, argptr );
		return localbuf.GetPtr();
	}
	else
	{
		ScopedLock locker( unicode_buffer );
		format_that_unicode_mess( unicode_buffer.buffer, fmt, argptr );
		return unicode_buffer.buffer.GetPtr();
	}
}

// =====================================================================================================
//  IConsole Interfaces
// =====================================================================================================
// (all non-virtual members that do common work and then pass the result through DoWrite
//  or DoWriteLn)

// Parameters:
//   glob_indent - this parameter is used to specify a global indentation setting.  It is used by
//      WriteLn function, but defaults to 0 for Notice and Error calls.  Local indentation always
//      applies to all writes.
wxString IConsoleWriter::_addIndentation( const wxString& src, int glob_indent=0 ) const
{
	const int indent = glob_indent + _imm_indentation;
	if( indent == 0 ) return src;

	wxArrayString pieces;
	SplitString( pieces, src, L'\n' );
	const wxString indentStr( L'\t', indent );
	wxString result;
	result.reserve( src.Length() + 24 );
	JoinString( result, pieces, L'\n' + indentStr );
	return indentStr + result;
}

// Sets the indentation to be applied to all WriteLn's.  The indentation is added to the
// primary write, and to any newlines specified within the write.  Note that this applies
// to calls to WriteLn *only* -- calls to Write bypass the indentation parser.
const IConsoleWriter& IConsoleWriter::SetIndent( int tabcount ) const
{
	conlog_Indent += tabcount;
	pxAssert( conlog_Indent >= 0 );
	return *this;
}

IConsoleWriter IConsoleWriter::Indent( int tabcount ) const
{
	IConsoleWriter retval = *this;
	retval._imm_indentation = tabcount;
	return retval;
}

// Changes the active console color.
// This color will be unset by calls to colored text methods
// such as ErrorMsg and Notice.
const IConsoleWriter& IConsoleWriter::SetColor( ConsoleColors color ) const
{
	pxAssertMsg( color >= Color_Current && color < ConsoleColors_Count, "Invalid ConsoleColor specified." );

	if( conlog_Color != color )
		DoSetColor( conlog_Color = color );

	return *this;
}

ConsoleColors IConsoleWriter::GetColor() const
{
	return conlog_Color;
}

// Restores the console color to default (usually black, or low-intensity white if the console uses a black background)
const IConsoleWriter& IConsoleWriter::ClearColor() const
{
	if( conlog_Color != DefaultConsoleColor )
		DoSetColor( conlog_Color = DefaultConsoleColor );

	return *this;
}

// --------------------------------------------------------------------------------------
//  ASCII/UTF8 (char*)
// --------------------------------------------------------------------------------------

bool IConsoleWriter::Write( const char* fmt, ... ) const
{
	if( DoWrite == ConsoleNull_DoWrite ) return false;

	va_list args;
	va_start(args,fmt);
	DoWrite( ascii_format_string(fmt, args) );
	va_end(args);

	return false;
}

bool IConsoleWriter::Write( ConsoleColors color, const char* fmt, ... ) const
{
	if( DoWrite == ConsoleNull_DoWrite ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( color );
	DoWrite( ascii_format_string(fmt, args) );
	va_end(args);

	return false;
}

bool IConsoleWriter::WriteLn( const char* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	DoWriteLn( _addIndentation( ascii_format_string(fmt, args), conlog_Indent ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::WriteLn( ConsoleColors color, const char* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;
	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( color );
	DoWriteLn( _addIndentation( ascii_format_string(fmt, args), conlog_Indent ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::Error( const char* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( Color_StrongRed );
	DoWriteLn( _addIndentation( ascii_format_string(fmt, args) ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::Warning( const char* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( Color_StrongOrange );
	DoWriteLn( _addIndentation( ascii_format_string(fmt, args) ) );
	va_end(args);

	return false;
}

// --------------------------------------------------------------------------------------
//  FmtWrite Variants - Unicode/UTF16 style
// --------------------------------------------------------------------------------------

bool IConsoleWriter::Write( const wxChar* fmt, ... ) const
{
	if( DoWrite == ConsoleNull_DoWrite ) return false;

	va_list args;
	va_start(args,fmt);
	DoWrite( unicode_format_string( fmt, args ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::Write( ConsoleColors color, const wxChar* fmt, ... ) const
{
	if( DoWrite == ConsoleNull_DoWrite ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( color );
	DoWrite( unicode_format_string( fmt, args ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::WriteLn( const wxChar* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	DoWriteLn( _addIndentation( unicode_format_string( fmt, args ), conlog_Indent ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::WriteLn( ConsoleColors color, const wxChar* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( color );
	DoWriteLn( _addIndentation( unicode_format_string( fmt, args ), conlog_Indent ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::Error( const wxChar* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( Color_StrongRed );
	DoWriteLn( _addIndentation( unicode_format_string( fmt, args ) ) );
	va_end(args);

	return false;
}

bool IConsoleWriter::Warning( const wxChar* fmt, ... ) const
{
	if( DoWriteLn == ConsoleNull_DoWriteLn ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( Color_StrongOrange );
	DoWriteLn( _addIndentation( unicode_format_string( fmt, args ) ) );
	va_end(args);

	return false;
}

// --------------------------------------------------------------------------------------
//  
// --------------------------------------------------------------------------------------

bool IConsoleWriter::WriteFromStdout( const char* fmt, ... ) const
{
	if( DoWriteFromStdout == ConsoleNull_DoWrite ) return false;

	va_list args;
	va_start(args,fmt);
	DoWrite( ascii_format_string(fmt, args) );
	va_end(args);

	return false;
}

bool IConsoleWriter::WriteFromStdout( ConsoleColors color, const char* fmt, ... ) const
{
	if( DoWriteFromStdout == ConsoleNull_DoWrite ) return false;

	va_list args;
	va_start(args,fmt);
	ConsoleColorScope cs( color );
	DoWrite( ascii_format_string(fmt, args) );
	va_end(args);

	return false;
}


// --------------------------------------------------------------------------------------
//  Default Writer for C++ init / startup:
// --------------------------------------------------------------------------------------
// In GUI modes under Windows I default to Assert, because windows lacks a qualified universal
// program console.  In console mode I use Stdio instead, since the program is pretty well
// promised a valid console in any platform (except maybe Macs, which probably consider consoles
// a fundamental design flaw or something).

#if wxUSE_GUI && defined(__WXMSW__)
#	define _DefaultWriter_	ConsoleWriter_Assert
#else
#	define _DefaultWriter_	ConsoleWriter_Stdout
#endif

// Important!  Only Assert and Null console loggers are allowed for initial console targeting.
// Other log targets rely on the static buffer and a threaded mutex lock, which are only valid
// after C++ initialization has finished.

IConsoleWriter	Console	= _DefaultWriter_;

#ifdef PCSX2_DEVBUILD
	IConsoleWriter	DevConWriter= _DefaultWriter_;
#endif

#ifdef PCSX2_DEBUG
	IConsoleWriter	DbgConWriter= _DefaultWriter_;
#endif

