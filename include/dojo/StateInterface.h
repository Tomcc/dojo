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
	///StateInterface is the State and State machine class
	class StateInterface
	{
	public:
		
		StateInterface( bool autoDelete = true ) :
		currentState(-1),
		currentStatePtr( NULL ),
		autoDelete( autoDelete ),
		mTransitionCompleted( true ),
		mCanSetNextState( true )
		{
			nextState = -1;
			nextStatePtr = NULL;
		}
		
		virtual ~StateInterface()
		{			
			if( currentStatePtr )
				SAFE_DELETE( currentStatePtr );
		}
		
		///sets a new substate either immediately or at the next loop.
		/**
			-Immediately when there's no current state, eg. at begin
			-at the next loop when it replaces a previous state
		 
		Warning, calling setState
			-with a pending state change
			-during a state change (onStateBegin, onTransition, onStateEnd)
			is an error and a failed ASSERT.
		*/
		inline void setState( int newState )		
		{			
			DEBUG_ASSERT( mCanSetNextState, "This State Machine is in an active transition and can't change its destination state" );
			
			nextState = newState;
			
			//start immediately if we have no current state
			if( !hasCurrentState() )
				_applyNextState();
		}
		
		///sets a new substate immediately
		/**
		 beware - this can cause MANY unexpected behaviours
		 
		 Warning, calling setState
		 -with a pending state change
		 -during a state change (onStateBegin, onTransition, onStateEnd)
		 is an error and a failed ASSERT.
		 */
		inline void setStateImmediate( int newState )		
		{			
			DEBUG_ASSERT( mCanSetNextState, "This State Machine is in an active transition and can't change its destination state" );
			
			nextState = newState;
			
			_applyNextState();
		}
		
		///sets a new substate either immediately or at the next loop.
		/**
		 -Immediately when there's no current state, eg. at begin
		 -at the next loop when it replaces a previous state
		 
		 Warning, calling setState
		 -with a pending state change
		 -during a state change (onStateBegin, onTransition, onStateEnd)
		 is an error and a failed ASSERT.
		 */
		inline void setState( StateInterface* child )
		{
			DEBUG_ASSERT( mCanSetNextState, "This State Machine is in an active transition and can't change its destination state" );
			DEBUG_ASSERT( !hasNextState(), "this State Machine already has a pending state to be set, cannot set another" );
			
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
		
		inline bool hasPendingTransition()				{	return !mTransitionCompleted;	}
		
		///begin the execution of this state
		inline void begin()
		{
			onBegin();
		}
		
		///loop the execution of this state (and its childs)
		inline void loop( float dt )
		{			
			if( mTransitionCompleted )  //do not call a loop if the current state is not "active" (ie-transition in progress)
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
		
		bool mTransitionCompleted, mCanSetNextState;
				
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
			//first try, call substate end
			if( mTransitionCompleted )
			{
				_subStateEnd();
			
				if( currentStatePtr && currentStatePtr->isAutoDeleted() )
					SAFE_DELETE( currentStatePtr );
			}
			
			//now try the transition
			mTransitionCompleted = onTransition();
		
			if( mTransitionCompleted )
			{
				currentState = newState;
				currentStatePtr = NULL;
			
				_subStateBegin();
			}
		}
		
		inline void _nextState( StateInterface* child )
		{
			DEBUG_ASSERT( child != nullptr, "null substate passed" );
			
			//first try, call substate end
			if( mTransitionCompleted )
			{
				_subStateEnd();
			
				if( currentStatePtr && currentStatePtr->isAutoDeleted() )
					SAFE_DELETE( currentStatePtr );
			}
			
			//now try the transition
			mTransitionCompleted = onTransition();
			
			if( mTransitionCompleted )
			{				
				currentState = -1;
				currentStatePtr = child;
				
				_subStateBegin();	
			}			
		}
		
		inline void _applyNextState()
		{
			DEBUG_ASSERT( hasNextState(), "_applyNextState was called but not next state was defined" );
						
			//they have to be cancelled before, because the state that is beginning
			//could need to set them again
			int temp = nextState;
			StateInterface* tempPtr = nextStatePtr;
			
			mCanSetNextState = false; //disable state setting - it can't be done while changing a state!
				
			//change state
			if( tempPtr )
				_nextState( tempPtr );
			else if( temp != -1 )
				_nextState( temp );

			if( mTransitionCompleted )
			{
				nextState = -1;
				nextStatePtr = NULL;
				
				mCanSetNextState = true;
			}
		}		
	};
}

#endif