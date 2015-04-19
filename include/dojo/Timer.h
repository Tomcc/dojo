#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	///A Timer is an utility class to work with real (system) time
	class Timer {
	public:

		///returns the current system high-precision time
		static double currentTime();

		///Creates a new (optionally looping) Timer, using current time
		Timer(double period = 0) {
			//start from now
			reset();

			setLoop(period);
		}

		///resets the Timer to the current time
		void reset() {
			eventTime = currentTime();
		}

		void disable() {
			eventTime = -1;
		}

		///sets the timer as looping - to be used with loopElapsed()
		void setLoop(double period) {
			mPeriod = (period > 0) ? period : 0;
		}

		///removes the loop
		void disableLoop() {
			mPeriod = 0;
		}

		///get the time from the last "reset event"
		double getElapsedTime() {
			return (eventTime > 0) ? currentTime() - eventTime : 0;
		}

		bool isEnabled() {
			return eventTime > 0;
		}

		///gets the time from the last reset and then resets the timer
		double deltaTime() {
			double t = getElapsedTime();
			reset();
			return t;
		}

		///method that returns > 0 each "period" seconds
		/**
		\return
		When the method is called and period has passed, it returns the real cycle time
		\remark If loop is disabled, this method always returns true.
		*/
		bool loopElapsed(double* actualPeriod = NULL) {
			double t = getElapsedTime();
			if (t >= mPeriod)
				reset();

			//send actual period to the user if needed
			if (actualPeriod)
				*actualPeriod = t;

			return t >= mPeriod;

		}

		bool isLooping() {
			return mPeriod != 0;
		}

	protected:
		double eventTime, mPeriod;

	};
}
