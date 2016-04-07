#include "WorkerPool.h"

#include "BackgroundWorker.h"

using namespace Dojo;

WorkerPool::WorkerPool(uint32_t workerCount, bool async) :
isAsync(async) {
	DEBUG_ASSERT(workerCount > 0, "Invalid worker count");
	DEBUG_ASSERT(async || workerCount == 1, "Either the pool is async, or it should only have one queue");

	while(mWorkers.size() < workerCount) {
		mWorkers.emplace_back(make_unique<BackgroundWorker>(isAsync));
	}
}

WorkerPool::~WorkerPool() {
	//stop the workers
	if (isAsync) {
		for (auto&& w : mWorkers) {
			w->sync();
			w->stop();
		}
	}
}

AsyncJob::StatusPtr WorkerPool::queue(AsyncTask task, AsyncCallback callback /* = */ ) {
	//round robin between the workers
	//TODO use a sp-mc queue?
	auto job = AsyncJob{ std::move(task), std::move(callback) };
	AsyncJob::StatusPtr ptr = job.mStatus;

	mWorkers[mNextWorker]->queueJob(std::move(job));

	mNextWorker = (mNextWorker + 1) % mWorkers.size();

	return ptr;
}

bool WorkerPool::runOneCallback() {
	//check if any queue has any job and run it
	for(auto& w : mWorkers) {
		if(w->_runOneCallback()) {
			return true;
		}
	}

	if(!isAsync) { //also try to run one task if tasks must be run on the main thread
		for (auto& w : mWorkers) {
			if(w->runNextTask()) {
				return true;
			}
		}
	}

	return false;
}
