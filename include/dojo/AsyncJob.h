#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class AsyncJob {
	public:
		enum class Status {
			Scheduled,
			Running,
			Callback,
			NotRunning
		};

		class StatusPtr {
		public:
			StatusPtr() {}

			StatusPtr(std::shared_ptr<Status>& status)
				: mStatusPtr(status) {}

			operator Status() const {
				if (auto ptr = mStatusPtr.lock()) {
					return *ptr;
				}
				return Status::NotRunning;
			}

		private:
			//TODO implement faster
			std::weak_ptr<Status> mStatusPtr;
		};

		AsyncTask task;
		AsyncCallback callback;

		AsyncJob() {}
		AsyncJob(AsyncTask&& task, AsyncCallback&& callback)
			: mStatus(make_shared<Status>(Status::Scheduled))
			, task(std::move(task))
			, callback(std::move(callback)) { //TODO pool the memory because it's a waste to reallocate every time

		}

		operator bool() const {
			return task or callback;
		}

		std::shared_ptr<Status> mStatus;
	private:
	};
}

