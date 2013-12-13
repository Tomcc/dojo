#include "stdafx.h"

#include "BackgroundQueue.h"

#include "Platform.h"

using namespace Dojo;

const BackgroundQueue::Callback BackgroundQueue::NOP_CALLBACK = [](){};

BackgroundQueue::BackgroundQueue( int poolSize /* = -1 */ ) :
	mRunning( true )
{
	mMainThreadID = Poco::Thread::currentTid();

	if( poolSize < 0 )
		poolSize = Platform::getSingleton()->getCPUCoreCount();

	//create the thread pool
	for( int i = 0; i < poolSize; ++i )
		mWorkers.push_back( std::unique_ptr<Worker>(new Worker(this)) );
}

void BackgroundQueue::queueTask( const Task& task, const Callback& callback )
{
    //debug sync mode
    if( mWorkers.empty() )
    {
        task();
        callback();
    }
    else
		mQueue.queue(TaskCallbackPair(task, callback));
}

void BackgroundQueue::queueOnMainThread( const Callback& c )
{
	if( Poco::Thread::currentTid() == mMainThreadID ) //is this already the main thread? just execute
		c();
	else
		mCompletedQueue.queue(c);
}

void BackgroundQueue::fireCompletedCallbacks()
{
	//now, execute the callbacks on the main thread
	Task callback;

	while ( mCompletedQueue.tryPop(callback) )
		callback();
}

void BackgroundQueue::Worker::run()
{
	Platform::getSingleton()->prepareThreadContext();

	while( 1 )
	{
		TaskCallbackPair pair;

		if( !pParent->_waitForTaskOrClose( pair ) ) //wait for a new task or close
			break;

		pair.first(); //execute the task

		//push the callback on the completed queue
		pParent->queueOnMainThread( pair.second );
	}
}