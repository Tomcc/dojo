#include "WorkerPool.h"

#include "BackgroundWorker.h"

using namespace Dojo;

WorkerPool::WorkerPool(uint32_t workerCount) {
	DEBUG_ASSERT(workerCount > 0, "Invalid worker count");

	while(mWorkers.size() < workerCount) {
		mWorkers.emplace_back(make_unique<BackgroundWorker>());
	}
}

WorkerPool::~WorkerPool() {
	//empty to instantiate ~BackgroundWorker here
}

void WorkerPool::queue(AsyncTask&& task, AsyncCallback&& callback /* = */ ) {
	//round robin between the workers
	//TODO use a sp-mc queue?

	mWorkers[mNextWorker]->queueJob({ std::move(task), std::move(callback) });
}

bool WorkerPool::runOneCallback() {
	//check if any queue has any job and run it
	for(auto& w : mWorkers) {
		if(w->_runOneCallback()) {
			return true;
		}
	}
	return false;
}

