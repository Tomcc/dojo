#pragma once

#include "dojo_common_header.h"

#include "SPSCQueue.h"
#include "SpinLock.h"

namespace Dojo {
	///A multi-producer single-consumer queue which is actually just a shim+spinlock over SPSCQueue.
	//TODO actually implement without the spinlock?
	template <typename T, size_t MAX_BLOCK_SIZE = 512>
	class MPSCQueue {
	public:
		MPSCQueue() {}

		template <class... Args>
		void enqueue(Args&& ... args) {
			std::lock_guard<SpinLock> lock(mSpinLock);
			mQueue.enqueue(std::forward<Args>(args)...);
		}

		template <typename U>
		bool try_dequeue(U& result) {
			return mQueue.try_dequeue(result);
		}

	private:

		SpinLock mSpinLock;
		SPSCQueue<T, MAX_BLOCK_SIZE> mQueue;
	};
}

