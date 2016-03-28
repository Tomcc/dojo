#include "TimedEvent.h"

#include "Platform.h"
#include "WorkerPool.h"

namespace Dojo {
	class EventManager {
	public:
		static EventManager instance;

		void registerEvent(TimedEventImpl& event) {
			mEvents.emplace(&event);
		}

		void removeEvent(TimedEventImpl& event) {
			auto elem = mEvents.find(&event);
			DEBUG_ASSERT(elem != mEvents.end(), "Cannot remove an event that wasn't registered");
			mEvents.erase(elem);
		}

		void runTimedEvents(TimePoint now);

	protected:
		SmallSet<TimedEventImpl*> mEvents;
	};

	EventManager EventManager::instance;

	class TimedEventImpl {
	public:
		Duration mInterval;
		TimePoint mNextTime;

		bool mRunning = false;
		AsyncCallback mCallback;
		AsyncTask mTask;
		WorkerPool& mTargetPool;

		TimedEventImpl(Duration interval,
			AsyncTask task,
			AsyncCallback callback,
			WorkerPool& targetPool) :
			mInterval(interval),
			mTargetPool(targetPool),
			mCallback(std::move(callback)) {

			//wrap the task to report that it's finished
			//it would be nice to use std::future for this
			//but checking if a future is obtained is blocking.
			mTask = [this, task]{
				task();
				mRunning = false;
			};

			EventManager::instance.registerEvent(self);
		}

		~TimedEventImpl() {
			EventManager::instance.removeEvent(self);
		}

		bool isReady(TimePoint now) {
			return !mRunning && now >= mNextTime;
		}

		void run() {
			DEBUG_ASSERT(mRunning == false, "Cannot run now");

			mRunning = true;
			mTargetPool.queue(mTask, mCallback);

			mNextTime = std::chrono::high_resolution_clock::now() + mInterval;
		}
	};

	TimedEvent::TimedEvent() {}
	TimedEvent::~TimedEvent() {}

	TimedEvent::TimedEvent(TimedEvent&& rhs) {
		self = std::move(rhs);
	}

	TimedEvent& TimedEvent::operator=(TimedEvent&& rhs) {
		mImpl = std::move(rhs.mImpl);
		return self;
	}

	void TimedEvent::start(std::chrono::high_resolution_clock::duration duration, AsyncTask task, AsyncCallback callback /* =  */, optional_ref<WorkerPool> targetPool /* = */) {
		mImpl = make_unique<TimedEventImpl>(
			duration,
			std::move(task),
			std::move(callback),
			targetPool.is_some() ? targetPool.unwrap() : Platform::singleton().getMainThreadPool()
			);
	}

	void TimedEvent::runTimedEvents(TimePoint now) {
		EventManager::instance.runTimedEvents(now);
	}

	void EventManager::runTimedEvents(TimePoint now) {
		//run all the events ready to run
		for (auto&& event : mEvents) {
			if (event->isReady(now)) {
				event->run();
			}
		}
	}
}