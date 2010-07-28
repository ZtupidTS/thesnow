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

#include "MemcpyFast.h"

extern void* __fastcall pcsx2_aligned_malloc(size_t size, size_t align);
extern void* __fastcall pcsx2_aligned_realloc(void* handle, size_t size, size_t align);
extern void pcsx2_aligned_free(void* pmem);

// aligned_malloc: Implement/declare linux equivalents here!
#if !defined(_MSC_VER) && !defined(HAVE_ALIGNED_MALLOC)
#	define _aligned_malloc pcsx2_aligned_malloc
#	define _aligned_free pcsx2_aligned_free
# 	define _aligned_realloc pcsx2_aligned_realloc
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Safe deallocation macros -- checks pointer validity (non-null) when needed, and sets
// pointer to null after deallocation.

#define safe_delete( ptr ) \
	((void) (delete (ptr)), (ptr) = NULL)

#define safe_delete_array( ptr ) \
	((void) (delete[] (ptr)), (ptr) = NULL)

// No checks for NULL -- wxWidgets says it's safe to skip NULL checks and it runs on 
// just about every compiler and libc implementation of any recentness.
#define safe_free( ptr ) \
	( (void) (free( ptr ), !!0), (ptr) = NULL )
	//((void) (( ( (ptr) != NULL ) && (free( ptr ), !!0) ), (ptr) = NULL))

#define safe_fclose( ptr ) \
	((void) (( ( (ptr) != NULL ) && (fclose( ptr ), !!0) ), (ptr) = NULL))

// Implementation note: all known implementations of _aligned_free check the pointer for
// NULL status (our implementation under GCC, and microsoft's under MSVC), so no need to
// do it here.
#define safe_aligned_free( ptr ) \
	((void) ( _aligned_free( ptr ), (ptr) = NULL ))

#define SafeSysMunmap( ptr, size ) \
	((void) ( HostSys::Munmap( (uptr)(ptr), size ), (ptr) = NULL ))

// Microsoft Windows only macro, useful for freeing out COM objects:
#define safe_release( ptr ) \
	((void) (( ( (ptr) != NULL ) && ((ptr)->Release(), !!0) ), (ptr) = NULL))

//////////////////////////////////////////////////////////////////////////////////////////
// Handy little class for allocating a resizable memory block, complete with
// exception-based error handling and automatic cleanup.
//
template< typename T >
class SafeArray
{
	DeclareNoncopyableObject(SafeArray);

public:
	static const int DefaultChunkSize = 0x1000 * sizeof(T);

public:
	wxString	Name;			// user-assigned block name
	int			ChunkSize;

protected:
	T*			m_ptr;
	int			m_size;			// size of the allocation of memory

protected:
	// Internal constructor for use by derived classes.  This allows a derived class to
	// use its own memory allocation (with an aligned memory, for example).
	// Throws:
	//   Exception::OutOfMemory if the allocated_mem pointer is NULL.
	explicit SafeArray( const wxChar* name, T* allocated_mem, int initSize )
		: Name( name )
	{
		ChunkSize	= DefaultChunkSize;
		m_ptr		= allocated_mem;
		m_size		= initSize;

		if( m_ptr == NULL )
			throw Exception::OutOfMemory(name + wxsFormat(L" (SafeArray::constructor) [size=%d]", initSize));
	}

	virtual T* _virtual_realloc( int newsize )
	{
		T* retval = (T*)((m_ptr == NULL) ?
			malloc( newsize * sizeof(T) ) :
			realloc( m_ptr, newsize * sizeof(T) )
		);
		
		if( IsDebugBuild )
		{
			// Zero everything out to 0xbaadf00d, so that its obviously uncleared
			// to a debuggee

			u32* fill = (u32*)&retval[m_size];
			const u32* end = (u32*)((((uptr)&retval[newsize-1])-3) & ~0x3);
			for( ; fill<end; ++fill ) *fill = 0xbaadf00d;
		}
		
		return retval;
	}

public:
	virtual ~SafeArray()
	{
		safe_free( m_ptr );
	}

	explicit SafeArray( const wxChar* name=L"Unnamed" )
		: Name( name )
	{
		ChunkSize	= DefaultChunkSize;
		m_ptr		= NULL;
		m_size		= 0;
	}

	explicit SafeArray( int initialSize, const wxChar* name=L"Unnamed" )
		: Name( name )
	{
		ChunkSize	= DefaultChunkSize;
		m_ptr		= (initialSize==0) ? NULL : (T*)malloc( initialSize * sizeof(T) );
		m_size		= initialSize;

		if( (initialSize != 0) && (m_ptr == NULL) )
			throw Exception::OutOfMemory(name + wxsFormat(L" (SafeArray::constructor) [size=%d]", initialSize));
	}

	// Clears the contents of the array to zero, and frees all memory allocations.
	void Dispose()
	{
		m_size = 0;
		safe_free( m_ptr );
	}

	bool IsDisposed() const { return (m_ptr==NULL); }

	// Returns the size of the memory allocation, as according to the array type.
	int GetLength() const { return m_size; }
	// Returns the size of the memory allocation in bytes.
	int GetSizeInBytes() const { return m_size * sizeof(T); }

	// reallocates the array to the explicit size.  Can be used to shrink or grow an
	// array, and bypasses the internal threshold growth indicators.
	void ExactAlloc( int newsize )
	{
		if( newsize == m_size ) return;

		m_ptr = _virtual_realloc( newsize );
		if( m_ptr == NULL )
			throw Exception::OutOfMemory(Name +
				wxsFormat(L" (SafeArray::ExactAlloc) [oldsize=%d] [newsize=%d]", m_size, newsize)
			);

		m_size = newsize;
	}

	// Ensures that the allocation is large enough to fit data of the
	// amount requested.  The memory allocation is not resized smaller.
	void MakeRoomFor( int newsize )
	{
		if( newsize > m_size )
			ExactAlloc( newsize );
	}

	// Extends the containment area of the array.  Extensions are performed
	// in chunks.
	void GrowBy( int items )
	{
		MakeRoomFor( m_size + ChunkSize + items + 1 );
	}

	// Gets a pointer to the requested allocation index.
	// DevBuilds : Generates assertion if the index is invalid.
	T *GetPtr( uint idx=0 ) { return _getPtr( idx ); }
	const T *GetPtr( uint idx=0 ) const { return _getPtr( idx ); }

	// Gets an element of this memory allocation much as if it were an array.
	// DevBuilds : Generates assertion if the index is invalid.
	T& operator[]( int idx ) { return *_getPtr( (uint)idx ); }
	const T& operator[]( int idx ) const { return *_getPtr( (uint)idx ); }

	virtual SafeArray<T>* Clone() const
	{
		SafeArray<T>* retval = new SafeArray<T>( m_size );
		memcpy_fast( retval->GetPtr(), m_ptr, sizeof(T) * m_size );
		return retval;
	}

protected:
	// A safe array index fetcher.  Throws an exception if the array index
	// is outside the bounds of the array.
	// Performance Considerations: This function adds quite a bit of overhead
	// to array indexing and thus should be done infrequently if used in
	// time-critical situations.  Instead of using it from inside loops, cache
	// the pointer into a local variable and use std (unsafe) C indexes.
	T* _getPtr( uint i ) const
	{
		IndexBoundsCheckDev( Name.c_str(), i, m_size );
		return &m_ptr[i];
	}
};

//////////////////////////////////////////////////////////////////////////////////////////
// SafeList - Simple growable container without all the mess or hassle of std containers.
//
// This container is intended for reasonably simple class types only.  Things which this
// container does not handle with desired robustness:
//
//  * Classes with non-trivial constructors (such that construction creates much overhead)
//  * Classes with copy constructors (copying is done using performance memcpy)
//  * Classes with destructors (they're not called, sorry!)
//
template< typename T >
class SafeList
{
	DeclareNoncopyableObject(SafeList);

public:
	static const int DefaultChunkSize = 0x80 * sizeof(T);

public:
	wxString	Name;			// user-assigned block name
	int			ChunkSize;		// assigned DefaultChunkSize on init, reconfigurable at any time.

protected:
	T*		m_ptr;
	int		m_allocsize;		// size of the allocation of memory
	uint	m_length;			// length of the array (active items, not buffer allocation)

protected:
	virtual T* _virtual_realloc( int newsize )
	{
		return (T*)realloc( m_ptr, newsize * sizeof(T) );
	}

public:
	virtual ~SafeList() throw()
	{
		safe_free( m_ptr );
	}

	explicit SafeList( const wxChar* name=L"Unnamed" )
		: Name( name )
	{
		ChunkSize	= DefaultChunkSize;
		m_ptr		= NULL;
		m_allocsize	= 0;
		m_length	= 0;
	}

	explicit SafeList( int initialSize, const wxChar* name=L"Unnamed" )
		: Name( name )
	{
		ChunkSize	= DefaultChunkSize;
		m_allocsize	= initialSize;
		m_length	= 0;
		m_ptr		= (T*)malloc( initialSize * sizeof(T) );

		if( m_ptr == NULL )
			throw Exception::OutOfMemory(Name +
				wxsFormat(L" (SafeList::Constructor) [length=%d]", m_length)
			);

		for( int i=0; i<m_allocsize; ++i )
		{
			new (&m_ptr[i]) T();
		}

	}

	// Returns the size of the list, as according to the array type.  This includes
	// mapped items only.  The actual size of the allocation may differ.
	int GetLength() const { return m_length; }

	// Returns the size of the list, in bytes.  This includes mapped items only.
	// The actual size of the allocation may differ.
	int GetSizeInBytes() const { return m_length * sizeof(T); }

	void MatchLengthToAllocatedSize()
	{
		m_length = m_allocsize;
	}

	// Ensures that the allocation is large enough to fit data of the
	// amount requested.  The memory allocation is not resized smaller.
	void MakeRoomFor( int blockSize )
	{
		if( blockSize > m_allocsize )
		{
			const int newalloc = blockSize + ChunkSize;
			m_ptr = _virtual_realloc( newalloc );
			if( m_ptr == NULL )
				throw Exception::OutOfMemory(Name +
					wxsFormat(L" (SafeList::MakeRoomFor) [oldlen=%d] [newlen=%d]", m_length, blockSize)
				);

			for( ; m_allocsize<newalloc; ++m_allocsize )
			{
				new (&m_ptr[m_allocsize]) T();
			}
		}
	}

	void GrowBy( int items )
	{
		MakeRoomFor( m_length + ChunkSize + items + 1 );
	}

	// Sets the item length to zero.  Does not free memory allocations.
	void Clear()
	{
		m_length = 0;
	}

	// Appends an item to the end of the list and returns a handle to it.
	T& New()
	{
		_MakeRoomFor_threshold( m_length + 1 );
		return m_ptr[m_length++];
	}

	// Gets an element of this memory allocation much as if it were an array.
	// DevBuilds : Generates assertion if the index is invalid.
	T& operator[]( int idx )				{ return *_getPtr( (uint)idx ); }
	const T& operator[]( int idx ) const	{ return *_getPtr( (uint)idx ); }

	T* GetPtr()				{ return m_ptr; }
	const T* GetPtr() const	{ return m_ptr; }

	T& GetLast()			{ return m_ptr[m_length-1]; }
	const T& GetLast() const{ return m_ptr[m_length-1]; }

	int Add( const T& src )
	{
		_MakeRoomFor_threshold( m_length + 1 );
		m_ptr[m_length] = src;
		return m_length++;
	}

	// Same as Add, but returns the handle of the new object instead of it's array index.
	T& AddNew( const T& src )
	{
		_MakeRoomFor_threshold( m_length + 1 );
		m_ptr[m_length] = src;
		return m_ptr[m_length];
	}

	// Performs a standard array-copy removal of the given item.  All items past the
	// given item are copied over.
	// DevBuilds : Generates assertion if the index is invalid.
	void Remove( int index )
	{
		IndexBoundsCheckDev( Name.c_str(), index, m_length );

		int copylen = m_length - index;
		if( copylen > 0 )
			memcpy_fast( &m_ptr[index], &m_ptr[index+1], copylen );
	}

	virtual SafeList<T>* Clone() const
	{
		SafeList<T>* retval = new SafeList<T>( m_length );
		memcpy_fast( retval->m_ptr, m_ptr, sizeof(T) * m_length );
		return retval;
	}

protected:

	void _MakeRoomFor_threshold( int newsize )
	{
		MakeRoomFor( newsize + ChunkSize );
	}

	// A safe array index fetcher.  Throws an exception if the array index
	// is outside the bounds of the array.
	// Performance Considerations: This function adds quite a bit of overhead
	// to array indexing and thus should be done infrequently if used in
	// time-critical situations.  Instead of using it from inside loops, cache
	// the pointer into a local variable and use std (unsafe) C indexes.
	T* _getPtr( uint i ) const
	{
		IndexBoundsCheckDev( Name.c_str(), i, m_length );
		return &m_ptr[i];
	}
};

// --------------------------------------------------------------------------------------
//  SafeAlignedArray class
// --------------------------------------------------------------------------------------
// Handy little class for allocating a resizable memory block, complete with
// exception-based error handling and automatic cleanup.
// This one supports aligned data allocations too!

template< typename T, uint Alignment >
class SafeAlignedArray : public SafeArray<T>
{
protected:
	T* _virtual_realloc( int newsize )
	{
		return (T*)( ( this->m_ptr == NULL ) ?
			_aligned_malloc( newsize * sizeof(T), Alignment ) :
			_aligned_realloc( this->m_ptr, newsize * sizeof(T), Alignment )
		);
	}

	// Appends "(align: xx)" to the name of the allocation in devel builds.
	// Maybe useful,maybe not... no harm in attaching it. :D

public:
	virtual ~SafeAlignedArray()
	{
		safe_aligned_free( this->m_ptr );
		// mptr is set to null, so the parent class's destructor won't re-free it.
	}

	explicit SafeAlignedArray( const wxChar* name=L"Unnamed" ) :
		SafeArray<T>::SafeArray( name )
	{
	}

	explicit SafeAlignedArray( int initialSize, const wxChar* name=L"Unnamed" ) :
		SafeArray<T>::SafeArray(
			name,
			(T*)_aligned_malloc( initialSize * sizeof(T), Alignment ),
			initialSize
		)
	{
	}

	virtual SafeAlignedArray<T,Alignment>* Clone() const
	{
		SafeAlignedArray<T,Alignment>* retval = new SafeAlignedArray<T,Alignment>( this->m_size );
		memcpy_fast( retval->GetPtr(), this->m_ptr, sizeof(T) * this->m_size );
		return retval;
	}
};


// For lack of a better place for now (they depend on SafeList so they can't go in StringUtil)
extern void SplitString( SafeList<wxString>& dest, const wxString& src, const wxString& delims );
extern wxString JoinString( const SafeList<wxString>& src, const wxString& separator );
