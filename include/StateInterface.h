/*
 *  StateInterface.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef StateInterface_h__
#define StateInterface_h__

#include "dojo_common_header.h"

namespace Dojo
{
	class StateInterface
	{
	public:
		
		StateInterface() :
		currentState(-1),
		previousState(-1)
		{
			
		}
		
		inline void setState( uint newState )		
		{
			if( newState != currentState )
			{
				onStateEnd();
				
				previousState = currentState;				
				currentState = newState;
				
				onStateBegin();
			}
		}
		
		//increments state by one - utility method
		inline void nextState()
		{
			setState( currentState+1 );
		}
		
		inline uint getCurrentState()				{	return currentState;			}
		inline uint getPreviousState()				{	return previousState;			}
		
		inline bool isCurrentState( uint state )	{	return currentState == state;	}
		inline bool isPreviousState( uint state )	{	return previousState == state;	}
		
		virtual void onStateBegin()=0;		
		virtual void onStateLoop( float dt )=0;		
		virtual void onStateEnd()=0;
		
	private:
		
		int currentState;
		int previousState;
	};
}

#endif