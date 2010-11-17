/*
 *  Timer.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/31/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Timer_h__
#define Timer_h__

#include "dojo_config.h"

namespace Dojo {
	
	class Timer
	{
	public:
			
		
		Timer() :
		looping( false )
		{
			reset();
		}
		
		inline void reset()
		{
			timeElapsed = 0;
			eventTime = 0;
		}
		
		inline void advance( float dt )
		{
			timeElapsed += dt;
		}
		
		inline void setCountdown( float t, bool loop = false )
		{
			reset();
			eventTime = t;
			
			looping = loop;
		}
		
		inline float getTimeElapsed()		{	return timeElapsed;		}
		inline float getCountdownLenght()	{	return eventTime;		}
		
		inline bool countdownElapsed()
		{
			if( eventTime && timeElapsed > eventTime )
			{
				reset();
				
				if( looping ) //reschedule
					setCountdown( eventTime, true );
				
				return true;
			}
			
			return false;
		}
		
	protected:
		
		float timeElapsed;
		float eventTime;
		
		bool looping;		
	};
}

#endif