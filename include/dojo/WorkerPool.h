#pragma once

#include "AsyncJob.h"

namespace Dojo {
	class BackgroundWorker;

	///a pool of worker that can execute tasks and sends back callbacks
	class WorkerPool {
	public:
		const bool isAsync;

		explicit WorkerPool(uint32_t workerCount, bool async = true, bool allowMultipleProducers = false);
		~WorkerPool();

		AsyncJob::StatusPtr queue(AsyncTask task, AsyncCallback callback = {});

		void sync();

		bool runOneCallback();
	protected:
		uint32_t mNextWorker = 0;
		std::vector<Unique<BackgroundWorker>> mWorkers;
	private:
	};
}

