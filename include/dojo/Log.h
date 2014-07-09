#ifndef Log_h__
#define Log_h__

#include "dojo_common_header.h"

#include "Array.h"

#include <list>

namespace Dojo
{
	///The Log class manages dojo's debug output and can redirect it to file, or it can be read from a console
	class Log
	{
	public:

		///An individual entry of the log, complete with timestamp and importance
		struct Entry
		{
			enum Level
			{
				EL_INFO,
				EL_WARNING,
				EL_ERROR,
				ER_FATAL
			};

			time_t timestamp;
			String text;
			Level level;

			Entry( const String& msg, Level lvl ) :
				text( msg ),
				level( lvl )
			{
				timestamp = time(NULL);
			}
		};

		typedef std::list< Entry > LogQueue;

		///A Log::Listener receives events fired by Logs
		class Listener
		{
		public:
			///the LogUpdate event is fired each time a new string is appended to a Log
			virtual void onLogUpdated( Log* l, const Entry& message )	{}
		};

		///A StdoutListener just echoes the log on the standard output
		class Stdout : public Listener
		{
		public:
			virtual void onLogUpdated( Log* l, const Entry& message )
			{
				std::cout << message.text.ASCII() << std::endl;
			}
		};

		Log( int maxLines = 1024 ) :
			mMaxLines( maxLines )
		{
			DEBUG_ASSERT( mMaxLines > 0, "Cannot create a Log with 0 or less lines" );
		}

		~Log()
		{

		}

		///appends another message to the log, with an optional severity level
		void append( const String& message, Entry::Level level = Entry::EL_WARNING );

		///adds a listener that will receive events from this Log
		void addListener( Listener* l )
		{
			DEBUG_ASSERT( l, "Cannot add a null listener" );

			pListeners.add( l );
		}

		void removeListener( Listener* l )
		{
			pListeners.remove( l );
		}

		///returns the last appended entry
		const Entry& getLastMessage()
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

		Dojo::Array< Listener* > pListeners;
		LogQueue mOutput;
		int mMaxLines;

		void _append( const String& message, Entry::Level level );

		void _fireOnLogUpdated( const Entry& e )
		{
			for( auto listener : pListeners )
				listener->onLogUpdated( this, getLastMessage() );
		}
	};
}

#endif // Log_h__
