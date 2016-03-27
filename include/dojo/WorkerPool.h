#pragma once

namespace Dojo {
	class BackgroundWorker;

	///a pool of worker that can execute tasks and sends back callbacks
	class WorkerPool {
	public:
		WorkerPool(uint32_t workerCount);
		~WorkerPool();

		void queue(AsyncTask&& task, AsyncCallback&& callback = {});

		bool runOneCallback();

	protected:
		uint32_t mNextWorker = 0;
		std::vector<Unique<BackgroundWorker>> mWorkers;
	private:
	};
}

