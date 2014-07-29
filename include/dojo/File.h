#pragma once

#include "dojo_common_header.h"

#include "FileStream.h"

namespace Dojo
{
	///File is a real filesystem file
	class File : public FileStream
	{
	public:

		File( const String & path, bool write = false );

        virtual ~File();
        
		virtual Access open();

		virtual long getSize();

		virtual Access getAccess();

		virtual long getCurrentPosition();

		virtual int seek( long offset, int fromWhere = SEEK_SET );

		virtual int read(  byte* buf, int number );

		virtual void write( byte* buf, int size );

		virtual void close();
        
        ///creates a new File* to read the underlying file. Does not copy reading status
        virtual Stream* copy();

	protected:

		FILE* mFile;
		long mSize;

		Access mAccess;

		void _updateSize();
	private:
	};
}

