#ifndef BackgroundQueue_h__
#define BackgroundQueue_h__

#include "dojo_common_header.h"

#include <functional>
#include <queue>
#include <Poco/Semaphore.h>
#include <Poco/Mutex.h>

namespace Dojo
{
	///A BackgroundQueue queues the tasks that are assigned to it and eventually assigns them to an available thread from its thread pool //TODO the thread pool is just 1 thread!
	/**
	Dojo always spawns a default BackgroundQueue that can be retrieved with Platform::getBackgroundQueue(), but more can be created if needed.
	The BackgroundQueue also fires the "then" callbacks for the tasks that have finished on the main thread.
	*/
	class BackgroundQueue : public Poco::Runnable, public Poco::Thread
	{
	public:

		typedef std::function< void() > Task;
		typedef std::function< void() > Callback;

		static const Callback NOP_CALLBACK;

		///Creates a new empty BackgroundQueue and starts its thread pool
		/**
		\param poolSize the number of threads in the pool size. If -1 is passed, the default size is the available cores number x 2.
		*/
		BackgroundQueue( int poolSize = -1 ) :
		mQueueSemaphore( 1, 0xffff ),
        running( false )
		{
			mQueueSemaphore.wait(); //POCO doesn't allow for semaphores to be created empty for some reason

			//TODO use a threadpool
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
		void fireCompletedCallbacks();

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
