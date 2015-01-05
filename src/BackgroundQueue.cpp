#include "stdafx.h"

#include "BackgroundQueue.h"

#include "Platform.h"

using namespace Dojo;

const BackgroundQueue::Callback BackgroundQueue::NOP_CALLBACK = [](){};

BackgroundQueue::BackgroundQueue( int poolSize /* = -1 */ ) :
	mRunning( true ),
	mCompletedQueue( new CompletedTaskQueue ),
	mQueue( new TaskQueue )
{
	mMainThreadID = std::this_thread::get_id();

	if (poolSize < 0)
		poolSize = std::thread::hardware_concurrency();

	//create the thread pool
	for( int i = 0; i < poolSize; ++i )
		mWorkers.push_back( Unique<Worker>(new Worker(this)) );
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
		mQueue->queue(TaskCallbackPair(task, callback));
}

void BackgroundQueue::queueOnMainThread( const Callback& c )
{
	if( std::this_thread::get_id() == mMainThreadID ) //is this already the main thread? just execute
		c();
	else
		mCompletedQueue->queue(c);
}

void BackgroundQueue::fireCompletedCallbacks()
{
	//now, execute the callbacks on the main thread
	Task callback;

	while ( mCompletedQueue->tryPop(callback) )
		callback();
}

BackgroundQueue::Worker::Worker(BackgroundQueue* parent) :
pParent(parent)
{
	DEBUG_ASSERT(pParent, "the parent can't be null");

	thread = std::thread([&]()
	{
		Platform::singleton().prepareThreadContext();

		while( 1 )
		{
			TaskCallbackPair pair;

			if( !pParent->_waitForTaskOrClose( pair ) ) //wait for a new task or close
				break;

			pair.first(); //execute the task

			//push the callback on the completed queue
			pParent->queueOnMainThread( pair.second );
		}
	});
}
	