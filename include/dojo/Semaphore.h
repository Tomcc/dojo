#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class Semaphore {
	public:
		Semaphore(uint32_t initialCount);
		Semaphore& operator=(const Semaphore&) = delete;

		void wait();
		void notifyOne();

	protected:
		std::mutex mConditionMutex;
		std::atomic<uint32_t> mCount;
		std::condition_variable mCondition;
	};
}