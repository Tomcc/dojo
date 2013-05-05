#ifndef BackgroundQueue_h__
#define BackgroundQueue_h__

#include "dojo_common_header.h"

#include <functional>
#include <queue>
#include <Poco/Semaphore.h>
#include <Poco//Mutex.h>

namespace Dojo
{
	///A BackgroundQueue spawn its own thread to run, the tasks that are assigned to it over the available thread pool //TODO the thread pool is just 1 thread!
	/**
	Dojo always spawns a default BackgroundQueue that can be retrieved with Platform::getBackgroundQueue(), but more can be created if needed
	*/
	class BackgroundQueue : public Poco::Runnable, public Poco::Thread
	{
	public:

		typedef std::function< void() > Task;

		///Creates a new empty BackgroundQueue and starts its thread pool
		BackgroundQueue() :
		mQueueSemaphore( 1, 0xffff )
		{
			mQueueSemaphore.wait(); //POCO doesn't allow for empty queues for some reason

			//TODO start more than one thread?
			start( *this );
		}

		///queues this task for execution in the current thread pool
		/**
		Tasks are void to void lambdas, ie []() { printf( "Hello World\n" ); }
		Task execution parameters can be captured with the closure operator.
		*/
		void queueTask( const Task& task )
		{
			Poco::ScopedLock< Poco::Mutex > lock( mQueueMutex );

			mQueue.push( task );

			mQueueSemaphore.set();
		}

		///Waits until this queue stops itself
		/**
		be sure that no tasks are stalling it!
		*/
		void stop()
		{
			running = false;
			mQueueSemaphore.set(); //wake up so that it can be closed
			Poco::Thread::join();
		}

		virtual void run();

	protected:

		typedef std::queue< Task > TaskQueue;

		bool running;

		TaskQueue mQueue;
		Poco::Semaphore mQueueSemaphore;
		Poco::Mutex mQueueMutex;

	private:
	};
}

#endif // BackgroundQueue_h__
