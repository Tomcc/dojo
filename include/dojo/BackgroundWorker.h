#pragma once

#include "dojo_common_header.h"

#include "SPSCQueue.h"
#include "AsyncJob.h"
#include "Semaphore.h"
#include "SpinLock.h"

namespace Dojo {
	
	///A BackgroundQueue queues the tasks that are assigned to it and eventually assigns them to an available thread from its thread pool
	/**
	Dojo always spawns a default BackgroundQueue that can be retrieved with Platform::getBackgroundQueue(), but more can be created if needed.
	The BackgroundQueue also fires the "then" callbacks for the tasks that have finished on the main thread.
	*/
	class BackgroundWorker {
	public:
		const bool isAsync;
		const bool allowMultipleProducers;

		///Creates a new BackgroundWorker
		explicit BackgroundWorker(bool async, bool allowMultipleProducers);
		virtual ~BackgroundWorker();

		///Start the thread and begin running tasks
		void startAsync();

		///queues this task for execution in the current thread pool
		/**
		Tasks are void to void lambdas, ie []() { printf( "Hello World\n" ); }
		AsyncTask execution parameters can be captured with the closure operator.
		*/
		void queueJob(AsyncJob&& job);

		///wait until all tasks are completed
		void sync();

		///Waits until this queue stops itself
		/**
		be sure that no tasks are stalling it!
		*/
		void stop();

		///waits for a task, returns true if any were run
		bool runNextTask();

		bool _runOneCallback();
		bool _runAllCallbacks();
	private:

		std::atomic<bool> mRunning;
		std::thread mThread;
		Semaphore mAvailableTasksSemaphore;

		SpinLock mQueueLock;
		Unique<SPSCQueue<AsyncJob>> mQueue;
		Unique<SPSCQueue<AsyncJob>> mCompletedQueue;

		AsyncJob _waitForNextTask();

	private:
	};
}
