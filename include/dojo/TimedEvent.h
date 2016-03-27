#pragma once

namespace Dojo {
	class WorkerPool;
	class TimedEventImpl;

	class TimedEvent {
		friend class EventManager;
	public:
		using TimePoint = std::chrono::high_resolution_clock::time_point;
		using Interval = std::chrono::high_resolution_clock::duration;

		static void runTimedEvents(TimePoint now);
		
		TimedEvent();
		~TimedEvent();

		TimedEvent(TimedEvent&&);
		TimedEvent& operator=(TimedEvent&&);
		
		TimedEvent(const TimedEvent&) = delete;
		TimedEvent& operator=(const TimedEvent&) = delete;

		void start(
			Interval interval,
			AsyncTask task, 
			AsyncCallback callback = {},
			optional_ref<WorkerPool> targetPool = {}
		);

	protected:
		
		Unique<TimedEventImpl> mImpl;
	};
}

