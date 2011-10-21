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
		
		StateInterface( bool autoDelete = true ) :
		currentState(-1),
		currentStatePtr( NULL ),
		autoDelete( autoDelete ),
		mLoopDisabled( false )
		{
			nextState = -1;
			nextStatePtr = NULL;
		}
		
		virtual ~StateInterface()
		{			
			if( currentStatePtr )
				SAFE_DELETE( currentStatePtr );
		}
		
		inline void setState( uint newState )		
		{			
			nextState = newState;
			
			//start immediately if we have no current state
			if( !hasCurrentState() )
				_applyNextState();
		}
		
		inline void setState( StateInterface* child )
		{
			nextStatePtr = child;
						
			//start immediately if we have no current state
			if( !hasCurrentState() )
				_applyNextState();
		}
		
		inline int getCurrentState()					{	return currentState;	}
		inline StateInterface* getChildState()			{	return currentStatePtr;	}
		
		inline bool isCurrentState( int state )			{	return currentState == state;	}
		inline bool isCurrentState( StateInterface* s )	{	return currentStatePtr == s;	}
		inline bool isAutoDeleted()						{	return autoDelete;				}

		inline bool hasChildState()						{	return currentStatePtr != NULL;	}
		
		inline bool hasCurrentState()					{	return currentStatePtr != NULL || currentState != -1; }
		inline bool hasNextState()						{	return nextStatePtr != NULL || nextState != -1; }
		
		///begin the execution of this state
		inline void begin()
		{
			onBegin();
		}
		
		///loop the execution of this state (and its childs)
		inline void loop( float dt )
		{			
			if( !mLoopDisabled )  //do not call a loop if the current state is not "active" (ie-transition in progress)
				_subStateLoop( dt );
		
			onLoop( dt );
			
			if( hasNextState() )
				_applyNextState();
		}
		
		///end the execution of this state (and its childs)
		inline void end()
		{
			_subStateEnd();
			
			onEnd();	
		}
						
	protected:
		int currentState;		
		StateInterface* currentStatePtr;
		
		int nextState;
		StateInterface* nextStatePtr;
		
		bool autoDelete;
		
	private:
		
		bool mLoopDisabled;
				
		//------ state events
		virtual void onBegin()
		{
			
		}
		virtual void onLoop( float dt )
		{
			
		}
		virtual void onEnd()
		{
			
		}
		
		//----- immediate substate events
		virtual void onStateBegin()
		{
			
		}
		
		virtual void onStateLoop( float dt )
		{
			
		}
		
		virtual void onStateEnd()
		{
			
		}
		
		///this is called each time a transition will happen. 
		/**
		 returning false delays the transition to the next frame (with a new onTransition call)
		*/
		virtual bool onTransition()
		{
			return true;	
		}
				
		inline void _subStateBegin()
		{
			if( currentStatePtr )
				currentStatePtr->begin();
			else
				onStateBegin();
		}
		
		inline void _subStateLoop( float dt )
		{
			if( currentStatePtr )
				currentStatePtr->loop( dt );
			else
				onStateLoop( dt );
		}
		
		inline void _subStateEnd()
		{
			if( currentStatePtr )
				currentStatePtr->end();
			else
				onStateEnd();
		}
				
		inline void _nextState( uint newState )		
		{
			_subStateEnd();
			
			if( currentStatePtr && currentStatePtr->isAutoDeleted() )
				SAFE_DELETE( currentStatePtr );
			
			currentState = newState;
			currentStatePtr = NULL;
			
			_subStateBegin();
		}
		
		inline void _nextState( StateInterface* child )
		{
			DEBUG_ASSERT( child );
			
			_subStateEnd();
			
			if( currentStatePtr && currentStatePtr->isAutoDeleted() )
				SAFE_DELETE( currentStatePtr );
			
			currentState = -1;
			currentStatePtr = child;
			
			_subStateBegin();
		}
		
		inline void _applyNextState()
		{
			DEBUG_ASSERT( hasNextState() );
			
			bool transitionAllowed = onTransition();
			mLoopDisabled = !transitionAllowed;  //disable the loop callback until the transition has been made
			
			if( transitionAllowed )
			{
				//they have to be cancelled before, because the state that is beginning
				//could need to set them again
				int temp = nextState;
				StateInterface* tempPtr = nextStatePtr;
			
				nextState = -1;
				nextStatePtr = NULL;
			
				//change state
				if( tempPtr )
					_nextState( tempPtr );
				else if( temp != -1 )
					_nextState( temp );
			}
		}		
	};
}

#endif