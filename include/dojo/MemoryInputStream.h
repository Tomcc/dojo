#pragma once

#include "dojo_common_header.h"

#include "Stream.h"

namespace Dojo
{
	///MemoryInputStream is a wrapper to read and stream from a preallocated memory area
	class MemoryInputStream : public Stream
	{
	public:

		MemoryInputStream( byte* mem, int size ) :
		pMem( mem ),
		mSize( size ),
		mPosition( 0 )
		{
			DEBUG_ASSERT( mem != nullptr, "the memory must not be null" );
			DEBUG_ASSERT( size > 0, "the size must be a valid" );
		}

		///reads up to "number" bytes from the stream into buf, returns the number of bytes read
		virtual int read( byte* buf, int number )
		{
			if( mPosition < 0 || mPosition >= mSize ) //invalid position, cannot read
				return 0;

			int toRead = std::min( number, mSize - mPosition );
			
			memcpy( buf, pMem + mPosition, toRead );
			mPosition += toRead;
			return toRead;
		}

		///returns the total bytes in the stream, -1 if this stream has no end
		virtual long getSize()
		{
			return mSize;
		}

		///returns the kind of access this stream provides, ie. read-only
		virtual Access getAccess()
		{
			return SA_READONLY;
		}

		///returns the current reading/writing position
		virtual long getCurrentPosition()
		{
			return mPosition;
		}

		///goes to the given position
		virtual int seek( long offset, int fromWhere = SEEK_SET )
		{
			if( fromWhere == SEEK_SET )
				mPosition = offset;
			else if( fromWhere == SEEK_CUR )
				mPosition += offset;
			else if( fromWhere == SEEK_END )
				mPosition = mSize - offset;
			else
				DEBUG_FAIL( "invalid seek origin" );

			return 0;
		}

	protected:

		byte* pMem;
		int mSize;

		int mPosition;

	private:
	};
}


