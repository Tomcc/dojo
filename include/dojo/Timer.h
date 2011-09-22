#ifndef Timer_h__
#define Timer_h__

#include "dojo_common_header.h"

namespace Dojo
{
	class Timer 
	{
	public:
		Timer( double period = 0 )
		{
			//start from now
			reset();

			setLoop(period);
		}

		inline void reset()
		{
			eventTime = currentTime();
		}

		inline void disable()
		{
			eventTime = -1;
		}
		
		double currentTime();

		inline void setLoop( double period )
		{
			mPeriod = (period > 0) ? period : 0;
		}

		inline void setLoopFPS( double FPS )
		{
			mPeriod = (FPS > 0) ? 1.f/FPS : 0;
		}

		inline void disableLoop()
		{
			mPeriod = 0;
		}

		///get the time from the last "reset event"
		inline double getElapsedTime()
		{
			return (eventTime > 0) ? currentTime() - eventTime : 0;
		}

		inline bool isEnabled()
		{
			return eventTime > 0;
		}

		///gets the time from the last reset and then resets the timer
		inline double deltaTime()
		{
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
		inline bool loopElapsed( double* actualPeriod = NULL )
		{
			double t = getElapsedTime();
			if( t >= mPeriod)
				reset();

			//send actual period to the user if needed
			if( actualPeriod )
				*actualPeriod = t;

			return t >= mPeriod;

		}

		inline bool isLooping()
		{
			return mPeriod != 0;
		}

	protected:
		double eventTime, mPeriod;

	};
}

#endif // Timer_h__
