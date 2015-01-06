#pragma once

#include "dojo_common_header.h"

#include "Array.h"
#include "LogEntry.h"

namespace Dojo
{
	class LogListener;

	///The Log class manages dojo's debug output and can redirect it to file, or it can be read from a console
	class Log
	{
	public:

		typedef std::vector< LogEntry > LogQueue;

		Log( int maxLines = 1024 ) :
			mMaxLines( maxLines )
		{
			DEBUG_ASSERT( mMaxLines > 0, "Cannot create a Log with 0 or less lines" );
		}

		~Log()
		{

		}

		///appends another message to the log, with an optional severity level
		void append( const String& message, LogEntry::Level level = LogEntry::EL_WARNING );

		///adds a listener that will receive events from this Log
		void addListener( LogListener* l )
		{
			DEBUG_ASSERT( l, "Cannot add a null listener" );

			pListeners.add( l );
		}

		void removeListener( LogListener* l )
		{
			pListeners.remove( l );
		}

		///returns the last appended entry
		const LogEntry& getLastMessage()
		{
			return mOutput.back();
		}

		const LogQueue::iterator begin()
		{
			return mOutput.begin();
		}

		const LogQueue::iterator end()
		{
			return mOutput.end();
		}

	protected:

		Array< LogListener* > pListeners;
		LogQueue mOutput;
		int mMaxLines;

		void _append( const String& message, LogEntry::Level level );

		void _fireOnLogUpdated( const LogEntry& e );
	};
}

