#ifndef FileStreamImpl_h__
#define FileStreamImpl_h__

#include "dojo_common_header.h"

#include "FileStream.h"

namespace Dojo
{
	///File is a real filesystem file
	class File : public FileStream
	{
	public:

		File( const String & path, bool write = false ) :
		FileStream( path, write ),
		mAccess( SA_BAD_FILE ),
		mFile( nullptr ),
		mSize( 0 )
		{

		}

		virtual Access open()
		{
			DEBUG_ASSERT( !isOpen(), "The file was already open" );

			//open the file
			const char* openmode = mWrite ? "wb" : "rb";
			mFile = fopen( mPath.UTF8().c_str(), openmode );

			if( mFile )
			{
				mAccess = mWrite ? SA_WRITEONLY : SA_READONLY;
				_updateSize();
			}
			else
				mAccess = SA_BAD_FILE;

			return getAccess();
		}

		virtual long getSize()
		{
			DEBUG_ASSERT( isReadable(), "The file must be readable to get its size" );
			DEBUG_ASSERT( mFile, "The C file is invalid" );

			return mSize;
		}

		virtual Access getAccess()
		{
			return mAccess;
		}

		virtual int read(  byte* buf, int number )
		{
			DEBUG_ASSERT( isReadable(), "The file must be open and readable" );
			DEBUG_ASSERT( mFile, "The C file is invalid" );

			return fread( buf, 1, number, mFile );
		}

		virtual void write( byte* buf, int size )
		{
			DEBUG_ASSERT( isWriteable(), "The file must be open and write enabled" );
			DEBUG_ASSERT( mFile, "The C file is invalid" );

			fwrite( buf, 1, size, mFile );
		}

		virtual void close()
		{
			DEBUG_ASSERT( isOpen(), "Tried to close a file which wasn't open" );
			DEBUG_ASSERT( mFile, "The C file is invalid" );

			fclose( mFile );
			mFile = nullptr;
			mAccess = SA_BAD_FILE;
		}

	protected:

		FILE* mFile;
		long mSize;

		Access mAccess;

		void _updateSize()
		{
			DEBUG_ASSERT( isReadable(), "The file must be readable to get its size" );
			DEBUG_ASSERT( mFile, "The C file is invalid" );

			long curptr = ftell( mFile );
			fseek( mFile, 0, SEEK_END );
			mSize = ftell( mFile );
			fseek( mFile, curptr, SEEK_SET );
		}
	private:
	};
}
#endif // File_h__
