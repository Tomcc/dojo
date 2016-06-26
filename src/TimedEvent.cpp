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

		void registerSimpleTask(TimePoint t, AsyncTask task) {
			mSimpleTasks.emplace(t, std::move(task));
		}

		void removeEvent(TimedEventImpl& event) {
			auto elem = mEvents.find(&event);
			DEBUG_ASSERT(elem != mEvents.end(), "Cannot remove an event that wasn't registered");
			mEvents.erase(elem);
		}

		void runTimedEvents(TimePoint now);

	private:
		SmallSet<TimedEventImpl*> mEvents;
		std::multimap<TimePoint, AsyncTask> mSimpleTasks;
	};

	EventManager EventManager::instance;

	class TimedEventImpl {
	public:
		Duration mInterval;
		TimePoint mNextTime;

		AsyncJob::StatusPtr mStatus;
		AsyncCallback mCallback;
		AsyncTask mTask;
		WorkerPool& mTargetPool;

		TimedEventImpl(Duration interval,
			AsyncTask task,
			AsyncCallback callback,
			WorkerPool& targetPool) :
			mInterval(interval),
			mTargetPool(targetPool),
			mTask(std::move(task)),
			mCallback(std::move(callback)) {

			EventManager::instance.registerEvent(self);
		}

		~TimedEventImpl() {
			EventManager::instance.removeEvent(self);

			//stall until the task is done
			while (mStatus != AsyncJob::Status::NotRunning and mTargetPool.runOneCallback());
		}

		bool isReady(TimePoint now) {
			return mStatus == AsyncJob::Status::NotRunning and now >= mNextTime;
		}

		void run() {
			DEBUG_ASSERT(mStatus == AsyncJob::Status::NotRunning, "Cannot run now");

			mStatus = mTargetPool.queue(mTask, mCallback);

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

	void TimedEvent::delay(TimePoint t, AsyncTask task) {
		EventManager::instance.registerSimpleTask(t, std::move(task));
	}

	void EventManager::runTimedEvents(TimePoint now) {
		//run all the events ready to run
		for (auto&& event : mEvents) {
			if (event->isReady(now)) {
				event->run();
			}
		}

		if (mSimpleTasks.size()) {
			auto top = mSimpleTasks.begin();
			if (now >= top->first) {
				top->second();
				mSimpleTasks.erase(top);
			}
		}
	}
}