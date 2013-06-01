#include "stdafx.h"

#include "BackgroundQueue.h"

#include "Platform.h"

using namespace Dojo;

const BackgroundQueue::Callback BackgroundQueue::NOP_CALLBACK = [](){};

BackgroundQueue::BackgroundQueue( int poolSize /* = -1 */ ) :
	mQueueSemaphore( 0, 0xffff ),
	mRunning( true )
{
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
    {
        Lock lock( mQueueMutex );
        
        mQueue.push( TaskCallbackPair( task, callback ) );
        
        mQueueSemaphore.set();
    }
}

void BackgroundQueue::fireCompletedCallbacks()
{
	mCompletedQueueMutex.lock();

	auto localQueue = mCompletedQueue; //copy the queue for async access
	while( !mCompletedQueue.empty() ) //and empty it again
		mCompletedQueue.pop();

	mCompletedQueueMutex.unlock();

	//now, execute the callbacks on the main thread
	while ( localQueue.size() )
	{
		localQueue.front()();
		localQueue.pop();
	}
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
		pParent->_queueCompletionCallback( pair.second );
	}
}