#pragma once

#include "dojo_common_header.h"

#include "Pipe.h"

namespace Dojo
{
	///A BackgroundQueue queues the tasks that are assigned to it and eventually assigns them to an available thread from its thread pool
	/**
	Dojo always spawns a default BackgroundQueue that can be retrieved with Platform::getBackgroundQueue(), but more can be created if needed.
	The BackgroundQueue also fires the "then" callbacks for the tasks that have finished on the main thread.
	*/
	class BackgroundQueue
	{
	public:

		typedef std::function< void() > Task;
		typedef std::function< void() > Callback;

		static const Callback NOP_CALLBACK;

		///Creates a new empty BackgroundQueue and starts its thread pool
		/**
		\param poolSize the number of threads in the pool size. If -1 is passed, the default size is the available cores number x 2.
		*/
		BackgroundQueue( int poolSize = -1 );

		virtual ~BackgroundQueue()
		{
			if( mRunning )
				stop();
		}

		///queues this task for execution in the current thread pool
		/**
		Tasks are void to void lambdas, ie []() { printf( "Hello World\n" ); }
		Task execution parameters can be captured with the closure operator.
		*/
		void queueTask( const Task& task, const Callback& callback = NOP_CALLBACK );

		///queues a function to be executed on the main thread
		void queueOnMainThread( const Callback& c );

		///Waits until this queue stops itself
		/**
		be sure that no tasks are stalling it!
		*/
		void stop()
		{
			if (mRunning) {
				mRunning = false;

				for( auto& w : mWorkers )
					w->join();
			}
		}

		///causes the queue to fire the completion listeners on the main thread.
		/**
		\remark remember to call this on any custom BackgroundQueue!
		*/
		void fireCompletedCallbacks();

	protected:

		class Worker
		{
		public:

			Worker(BackgroundQueue* parent);

			void join() {
				thread.join();
			}

		protected:
			BackgroundQueue* pParent;
			std::thread thread;
		};

		typedef std::pair< Task, Callback > TaskCallbackPair;
		typedef Pipe< TaskCallbackPair > TaskQueue;
		typedef Pipe< Task > CompletedTaskQueue;
		typedef std::vector< Unique< Worker > > WorkerList;

		std::atomic<bool> mRunning;

		Unique<TaskQueue> mQueue;
		Unique<CompletedTaskQueue> mCompletedQueue;

		WorkerList mWorkers;

		std::thread::id mMainThreadID;

		///waits for a task, returns false if the thread has to close
		bool _waitForTaskOrClose( TaskCallbackPair& out )
		{
			if( mRunning ) //fetch a new task from the queue
				return mQueue->tryPop(out);

			else return false;
		}

	private:
	};
}
