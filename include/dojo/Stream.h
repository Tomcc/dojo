#ifndef Stream_h__
#define Stream_h__

#include "dojo_common_header.h"

namespace Dojo
{
	///Stream represents a binary stream that can be either read-only, write-only or read+write
	/**
	it is an interface to be implemented by actual data sources.
	*/
	class Stream
	{
	public:

		///Stream::Access describes the kind of access to the underlying data a Stream offers
		enum Access
		{
			///this Stream was not initialized or cannot find its source
			SA_BAD_FILE,
			///this Stream can only be used to read
			SA_READONLY,
			///this Stream can only be used to write
			SA_WRITEONLY,
			///this Stream can be used both write and to read
			SA_READWRITE
		};

		///reads up to "number" bytes from the stream into buf, returns the number of bytes read
		virtual int read( byte* buf, int number )
		{
			DEBUG_FAIL( "This Stream is not read-enabled" );
			return 0;
		}

		///writes the given buffer to the stream
		virtual void write( byte* buf, int size )
		{
			DEBUG_FAIL( "This stream is not write-enabled" );
		}

		///returns the total bytes in the stream, -1 if this stream has no end
		virtual long getSize()=0;

		///returns the kind of access this stream provides, ie. read-only
		virtual Access getAccess()=0;

		///returns the current reading/writing position
		virtual long getCurrentPosition()=0;

		///goes to the given position
		virtual int seek( long offset, int fromWhere = SEEK_SET )=0;

		///utility method that tells if the stream is open
		bool isOpen()
		{
			return getAccess() != SA_BAD_FILE;
		}

		///tells if the stream can be read
		bool isReadable()
		{
			return getAccess() == SA_READONLY || getAccess() == SA_READWRITE;
		}

		///tells if the stream can be written to
		bool isWriteable()
		{
			return getAccess() == SA_WRITEONLY || getAccess() == SA_READWRITE;
		}

		///ensures that the stream is in sync with the underlying data, ie. buffered data will be written to file
		virtual void flush()
		{

		}

		///tells the Stream to prepare to be read from
		virtual Access open()
		{
			return getAccess();
		}

		///closes the Stream and releases its resources
		virtual void close()
		{
			flush();
		}
	};
}

#endif // Stream_h__
