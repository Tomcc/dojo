#pragma once

#include "dojo_common_header.h"

#include "Stream.h"

namespace Dojo {
	///MemoryInputStream is a wrapper to read and stream from a preallocated memory area
	class MemoryInputStream : public Stream {
	public:

		MemoryInputStream(byte* mem, int size);

		///reads up to "number" bytes from the stream into buf, returns the number of bytes read
		virtual int read(byte* buf, int number);

		///returns the total bytes in the stream, -1 if this stream has no end
		virtual long getSize();

		///returns the kind of access this stream provides, ie. read-only
		virtual Access getAccess();

		///returns the current reading/writing position
		virtual long getCurrentPosition();

		///goes to the given position
		virtual int seek(long offset, int fromWhere = SEEK_SET);

	protected:

		byte* pMem;
		int mSize;

		int mPosition;

	private:
	};
}
