#include "Semaphore.h"

using namespace Dojo;

Semaphore::Semaphore(uint32_t initialCount) : mCount(initialCount) {}

void Semaphore::notifyOne() {
	++mCount;
	mCondition.notify_one();
}

void Semaphore::wait() {
	auto lock = std::unique_lock<std::mutex>(mConditionMutex);

	while (mCount <= 0) {
		mCondition.wait(lock);
	}

	--mCount;
}


