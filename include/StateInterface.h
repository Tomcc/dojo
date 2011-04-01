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
		currentStatePtr( NULL )
		{
			
		}
		
		virtual ~StateInterface()
		{			
			if( currentStatePtr )
				delete currentStatePtr;
		}
		
		inline void setState( uint newState )		
		{
			_stateEnd();
				
			currentState = newState;
			currentStatePtr = NULL;
				
			_stateBegin();
		}
		
		inline void setState( StateInterface* child )
		{
			DEBUG_ASSERT( child );
			
			_stateEnd();
			
			currentState = -1;
			currentStatePtr = child;
			
			_stateBegin();			
		}
		
		inline uint getCurrentState()					{	return currentState;	}
		inline StateInterface* getChildState()		{	return currentStatePtr;	}
		
		inline bool isCurrentState( uint state )		{	return currentState == state;	}
		inline bool isCurrentState( StateInterface* s )	{	return currentStatePtr == s;	}

		inline bool hasChildState()						{	return currentStatePtr != NULL;	}
		
		//------ eventi dello stato
		virtual void onBegin()
		{
			
		}
		virtual void onLoop( float dt )
		{
			
		}
		virtual void onEnd()
		{
			
		}
		
		//----- eventi per i sottostati immediati
		virtual void onStateBegin()=0;		
		virtual void onStateLoop( float dt )=0;		
		virtual void onStateEnd()=0;
		
	protected:
		int currentState;		
		StateInterface* currentStatePtr;
		
	private:
		
		
		inline void _stateEnd()
		{
			if( currentStatePtr )
			{				
				currentStatePtr->onEnd();
			}
			else {
				onStateEnd();
			}		
		}
		
		inline void _stateBegin()
		{
			if( currentStatePtr )
				currentStatePtr->onBegin();
			else
				onStateBegin();
		}
	};
}

#endif