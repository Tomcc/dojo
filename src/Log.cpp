#include "stdafx.h"

#include "Log.h"
#include "Platform.h"
#include "BackgroundQueue.h"
#include "LogListener.h"

using namespace Dojo;

void Log::_append( const String& message, LogEntry::Level level )
{
	mOutput.push_back( LogEntry( message, level ) );

	if( mOutput.size() == mMaxLines )
		mOutput.erase( mOutput.begin() );

	_fireOnLogUpdated( getLastMessage() );
}

///appends another message to the log, with an optional severity level
void Log::append( const String& message, LogEntry::Level level )
{
	//execute the appending & notifying on the main thread!
	auto q = Platform::singleton().getBackgroundQueue();
	
	//this hack is needed before we need Log to exist ASAP, even before a BackgroundQueue is created
	//and anyway, when there's no BQ there should be no sync problems
	if( q )
		q->queueOnMainThread(
		[&]()
		{
			_append(message,level);
		} );
	else
		_append(message,level);
}

void Log::_fireOnLogUpdated(const LogEntry& e) {
	for (auto listener : pListeners)
		listener->onLogUpdated(this, getLastMessage());
}
