#include "BackgroundWorker.h"

#include "Platform.h"

using namespace Dojo;

BackgroundWorker::BackgroundWorker() :
	mRunning(false),
	mCompletedQueue(make_unique<SPSCQueue<AsyncCallback>>()),
	mQueue(make_unique<SPSCQueue<AsyncJob>>()) {
	start();
}

BackgroundWorker::~BackgroundWorker() {
	if (mRunning) {
		stop();
	}
}

AsyncJob BackgroundWorker::_waitForNextTask() {
	AsyncJob job;

	//TODO try work-stealing here
// 	if(!mQueue->try_dequeue(job)) {
//      teek jerbs
// 	}

	do {
		std::unique_lock<std::mutex> lock(mTasksAvailableMutex);
		mTasksAvailable.wait(lock);

		//the thread was killed while sleeping, return nothing
		if(!mRunning) {
			return{};
		}

	} while (!mQueue->try_dequeue(job)); //it's not sure that a job will be there, condition_variable has spurious wakeups

	return job;
}

void BackgroundWorker::_processNextTask() {
	if (auto job = _waitForNextTask()) {
		job.task();

		if (job.callback) {
			mCompletedQueue->enqueue(std::move(job.callback));
		}
	}
}

void BackgroundWorker::start() {
	DEBUG_ASSERT(mRunning == false && !mThread.joinable(), "Already running");

	mRunning = true;
	mThread = std::thread([this] {
		while(mRunning) {
			_processNextTask();

			std::this_thread::yield(); //go easy on the CPU, these tasks aren't priority anyway
		}
	});
}

void BackgroundWorker::queueJob(AsyncJob&& job) {
	mQueue->enqueue(std::move(job));
	mTasksAvailable.notify_one();
}

void BackgroundWorker::stop() {
	if (mRunning) {
		mRunning = false;

		//wake up the thread so it can kill itself (muahah)
		mTasksAvailable.notify_one();

		mThread.join();
	}
}

bool BackgroundWorker::_runOneCallback() {
	AsyncCallback callback;
	if(mCompletedQueue->try_dequeue(callback)){
		callback();
		return true;
	}
	return false;
}
