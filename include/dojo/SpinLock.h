#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class SpinLock {
	public:
		SpinLock() {
			mLock.clear(std::memory_order_release);
		}

		void lock() {
			while (mLock.test_and_set(std::memory_order_acquire)); //spin
		}

		void unlock() {
			mLock.clear(std::memory_order_release);
		}

	protected:
		std::atomic_flag mLock;
	};
}

