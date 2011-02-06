#ifndef Timer_h__
#define Timer_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo
{
	class Timer : public BaseObject
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
		
		double currentTime()
		{
			double d;
#ifdef WIN32
			__int64 freq, gTime;
			QueryPerformanceCounter((LARGE_INTEGER *)&gTime);  // Get current count
			QueryPerformanceFrequency((LARGE_INTEGER *)&freq); // Get processor freq
			d = (double)(gTime)/(double)freq;
#else
			struct timeval tv;
			gettimeofday(&tv, NULL);
			d = tv.tv_usec/1000000.f	+ tv.tv_sec;
#endif
			return d;
		}

		inline void setLoop( float period )
		{
			mPeriod = (period > 0) ? period : 0;
		}

		inline void setLoopFPS( float FPS )
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
		inline bool loopElapsed( float* actualPeriod = NULL )
		{
			float t = getElapsedTime();
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
