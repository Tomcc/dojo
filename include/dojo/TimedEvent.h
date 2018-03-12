#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	class WorkerPool;
	class TimedEventImpl;

	class TimedEvent {
		friend class EventManager;
	public:
		static void runTimedEvents(TimePoint now);

		static void delay(TimePoint t, AsyncTask task);
		
		TimedEvent();
		~TimedEvent();

		TimedEvent(TimedEvent&&);
		TimedEvent& operator=(TimedEvent&&);
		
		TimedEvent(const TimedEvent&) = delete;
		TimedEvent& operator=(const TimedEvent&) = delete;

		void start(
			Duration interval,
			AsyncTask task, 
			AsyncCallback callback = {},
			optional_ref<WorkerPool> targetPool = {}
		);

	private:
		
		std::unique_ptr<TimedEventImpl> mImpl;
	};
}

