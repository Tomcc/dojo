#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class SpinLock {
	public:
		SpinLock() : mLocked(false) {}

		void lock() {
			while (mLocked); //spin
			mLocked = true; //lock
		}

		void unlock() {
			DEBUG_ASSERT(mLocked, "Cannot unlock now");
			mLocked = false;
		}

	protected:
		std::atomic<bool> mLocked;
	};
}

