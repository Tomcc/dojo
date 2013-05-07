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
		typedef std::function< void() > Callback;

		static const Callback NOP_CALLBACK;

		///Creates a new empty BackgroundQueue and starts its thread pool
		BackgroundQueue() :
		mQueueSemaphore( 1, 0xffff ),
        running( false )
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
		void queueTask( const Task& task, const Callback& callback = NOP_CALLBACK )
		{
			ScopedLock lock( mQueueMutex );

			mQueue.push( TaskCallbackPair( task, callback ) );

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

		///-internal- causes the queue to fire the completion listeners on the main thread
		void _fireCompletedCallbacks();

	protected:

		typedef std::pair< Task, Callback > TaskCallbackPair;
		typedef std::queue< TaskCallbackPair > TaskQueue;
		typedef std::queue< Task > CompletedTaskQueue;

		bool running;

		TaskQueue mQueue;
		Poco::Semaphore mQueueSemaphore;
		Poco::Mutex mQueueMutex;

		CompletedTaskQueue mCompletedQueue;
		Poco::Mutex mCompletedQueueMutex;

	private:
	};
}

#endif // BackgroundQueue_h__
