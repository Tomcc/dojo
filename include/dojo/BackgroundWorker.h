#pragma once

#include "dojo_common_header.h"

#include "SPSCQueue.h"

namespace Dojo {
	class AsyncJob {
	public:
		AsyncTask task;
		AsyncCallback callback;

		operator bool() const {
			return task || callback;
		}
	};
	///A BackgroundQueue queues the tasks that are assigned to it and eventually assigns them to an available thread from its thread pool
	/**
	Dojo always spawns a default BackgroundQueue that can be retrieved with Platform::getBackgroundQueue(), but more can be created if needed.
	The BackgroundQueue also fires the "then" callbacks for the tasks that have finished on the main thread.
	*/
	class BackgroundWorker {
	public:
		const bool isAsync;

		///Creates a new BackgroundWorker
		explicit BackgroundWorker(bool async);
		virtual ~BackgroundWorker();

		///Start the thread and begin running tasks
		void startAsync();

		///queues this task for execution in the current thread pool
		/**
		Tasks are void to void lambdas, ie []() { printf( "Hello World\n" ); }
		AsyncTask execution parameters can be captured with the closure operator.
		*/
		void queueJob(AsyncJob&& job);

		///Waits until this queue stops itself
		/**
		be sure that no tasks are stalling it!
		*/
		void stop();

		///waits for a task, returns true if any were run
		bool runNextTask();

		bool _runOneCallback();
	protected:

		std::atomic<bool> mRunning;
		std::thread mThread;
		std::condition_variable mTasksAvailable;
		std::mutex mTasksAvailableMutex;

		Unique<SPSCQueue<AsyncJob>> mQueue;
		Unique<SPSCQueue<AsyncCallback>> mCompletedQueue;

		AsyncJob _waitForNextTask();

	private:
	};
}
