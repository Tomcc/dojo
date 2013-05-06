 #include "stdafx.h"

#include "BackgroundQueue.h"

#include "Platform.h"

using namespace Dojo;

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
		Task t = mQueue.front();
		mQueue.pop();
		mQueueMutex.unlock();

		t(); //execute
	}
}