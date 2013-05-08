#include "stdafx.h"

#include "BackgroundQueue.h"

#include "Platform.h"

using namespace Dojo;

const BackgroundQueue::Callback BackgroundQueue::NOP_CALLBACK = [](){};

void BackgroundQueue::run()
{
	Platform::getSingleton()->prepareThreadContext();

	running = true;
	while( 1 )
	{
		mQueueSemaphore.wait(); //wait for a new task to arrive

		if( !running ) //it's been awakened for closing?
			break;

		//pop it from the queue
		mQueueMutex.lock();
		auto taskCallbackPair = mQueue.front();
		mQueue.pop();
		mQueueMutex.unlock();

		taskCallbackPair.first(); //execute the task

		//push the callback on the completed queue
		ScopedLock l1( mCompletedQueueMutex );
		mCompletedQueue.push( taskCallbackPair.second );
	}
}

void BackgroundQueue::fireCompletedCallbacks()
{
	mCompletedQueueMutex.lock();

	auto localQueue = mCompletedQueue; //copy the queue for async access
	while( mCompletedQueue.size() ) //and empty it again
		mCompletedQueue.pop();

	mCompletedQueueMutex.unlock();

	//now, execute the callbacks on the main thread
	while ( localQueue.size() )
	{
		localQueue.front()();
		localQueue.pop();
	}
}