#include "BackgroundQueue.h"

#include "Platform.h"

using namespace Dojo;

const BackgroundQueue::Callback BackgroundQueue::NOP_CALLBACK = []() {
};

BackgroundQueue::BackgroundQueue(int poolSize /* = -1 */) :
	mRunning(true),
	mCompletedQueue(make_unique<CompletedTaskQueue>()),
	mQueue(make_unique<TaskQueue>()) {
	mMainThreadID = std::this_thread::get_id();

	if (poolSize < 0) {
		poolSize = std::thread::hardware_concurrency();
	}

	//create the thread pool
	for (int i = 0; i < poolSize; ++i) {
		mWorkers.emplace_back(make_unique<Worker>(this));
	}
}

void BackgroundQueue::queueTask(const Task& task, const Callback& callback) {
	//debug sync mode
	if (mWorkers.empty()) {
		task();
		callback();
	}
	else {
		mQueue->enqueue(task, callback);
	}
}

void BackgroundQueue::queueOnMainThread(const Callback& c) {
	if (std::this_thread::get_id() == mMainThreadID) { //is this already the main thread? just execute
		c();
	}
	else {
		mCompletedQueue->enqueue(c);
	}
}

void BackgroundQueue::fireCompletedCallbacks() {
	//now, execute the callbacks on the main thread
	Task callback;

	while (mCompletedQueue->try_dequeue(callback)) {
		callback();
	}
}

BackgroundQueue::Worker::Worker(BackgroundQueue* parent) :
	pParent(parent) {
	DEBUG_ASSERT(pParent, "the parent can't be null");

	thread = std::thread([&]() {
		Platform::singleton().prepareThreadContext();

		for (;;) {
			TaskCallbackPair pair;

			if (!pParent->_waitForTaskOrClose(pair)) { //wait for a new task or close
				break;
			}

			pair.first(); //execute the task

			//push the callback on the completed queue
			pParent->queueOnMainThread(pair.second);
		}
	});
}
