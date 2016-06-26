#include "BackgroundWorker.h"

#include "Platform.h"

using namespace Dojo;

BackgroundWorker::BackgroundWorker(bool async, bool allowMultipleProducers) :
	mRunning(false),
	mCompletedQueue(make_unique<SPSCQueue<AsyncJob>>()),
	mQueue(make_unique<SPSCQueue<AsyncJob>>()),
	isAsync(async),
	allowMultipleProducers(allowMultipleProducers),
	mAvailableTasksSemaphore(0) {

	if (isAsync) {
		startAsync();
	}
	else {
		mRunning = true;
	}
}

BackgroundWorker::~BackgroundWorker() {
	if (mRunning and isAsync) {
		stop();
	}
}

AsyncJob BackgroundWorker::_waitForNextTask() {
	AsyncJob job;

	if (isAsync) {
		//TODO try work-stealing here
	// 	if(not mQueue->try_dequeue(job)) {
	//      teek jerbs
	// 	}

		do {
			mAvailableTasksSemaphore.wait();

			//the thread was killed while sleeping, return nothing
			if (not mRunning) {
				return{};
			}

		} while (not mQueue->try_dequeue(job)); //it's not sure that a job will be there, condition_variable has spurious wakeups

		return job;
	}
	else if(mQueue->try_dequeue(job)) { //just try to get one and return
		return job;
	}

	return{};
}

bool BackgroundWorker::runNextTask() {
	if (auto job = _waitForNextTask()) {
		auto& status = *job.mStatus;
		status = AsyncJob::Status::Running;
		job.task();

		if (job.callback) {
			status = AsyncJob::Status::Callback;
			mCompletedQueue->enqueue(std::move(job));
		}
		return true;
	}
	return false;
}

void BackgroundWorker::startAsync() {
	DEBUG_ASSERT(mRunning == false and not mThread.joinable(), "Already running");

	mRunning = true;
	mThread = std::thread([this] {
		while(mRunning) {
			runNextTask();

			std::this_thread::yield(); //go easy on the CPU, these tasks aren't priority anyway
		}
	});
}

void BackgroundWorker::queueJob(AsyncJob&& job) {
	if (not allowMultipleProducers) {
		mQueueLock.lock();
	}
	mQueue->enqueue(std::move(job));
	mAvailableTasksSemaphore.notifyOne();
	
	if(not allowMultipleProducers) {
		mQueueLock.unlock();
	}
}

void BackgroundWorker::stop() {
	DEBUG_ASSERT(isAsync, "Cannot stop a synchronous worker");

	if (mRunning) {
		DEBUG_ASSERT(mThread.joinable(), "The thread is not running even if the Worker thinks it is");

		mRunning = false;

		//wake up the thread so it can kill itself (muahah)
		mAvailableTasksSemaphore.notifyOne();

		mThread.join();
	}
}

bool BackgroundWorker::_runOneCallback() {
	AsyncJob job;
	if(mCompletedQueue->try_dequeue(job)){
		job.callback();
		return true;
	}
	return false;
}

bool BackgroundWorker::_runAllCallbacks() {
	int runTasks = 0;
	while(true) {
		if(_runOneCallback()) {
			++runTasks;
		}
		else {
			break;
		}
	}
	return runTasks > 0;
}

void BackgroundWorker::sync() {
	DEBUG_ASSERT(isAsync, "TODO implement for sync queues");
	if (mRunning) {
		_runAllCallbacks(); //run all callbacks because they might have tasks which queue new jobs here

		//repeat until all the jobs have been pushed
		do {
			Semaphore sync(0);
			queueJob(AsyncJob{ [this, &sync]() {
				sync.notifyOne();
			},
			AsyncCallback() });
			sync.wait();
		} while (_runOneCallback());
	}
}
