/*
 *  StateInterface.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

namespace Dojo
{
	///Objects inheriting from StateInterface are at the same time a State, and a State Machine themselves; most of the gameplay logic of a Dojo game should be implemented overriding onBegin, onLoop and onEnd protected events
	class StateInterface
	{
	public:
		
		StateInterface( bool autoDelete = true ) :
		currentState(-1),
		currentStatePtr( NULL ),
		mTransitionCompleted( true ),
		mCanSetNextState( true ),
		mAutoDeleteState( false )
		{
			nextState = -1;
			nextStatePtr = NULL;
		}
		
		virtual ~StateInterface()
		{			
			if( currentStatePtr && mAutoDeleteState )
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
		void setState( int newState )		
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
		void setStateImmediate( int newState )		
		{			
			DEBUG_ASSERT( mCanSetNextState, "This State Machine is in an active transition and can't change its destination state" );
			
			nextState = newState;
			
			_applyNextState();
		}
		
		///sets a new substate either immediately or at the next loop.
		/**
		 -Immediately when there's no current state, eg. at begin
		 -at the next loop when it replaces a previous state
		 
		 \param autoDelete the child state is destroyed when replaced or when the parent (this) state is destroyed

		 \remark calling setState
		 -with a pending state change
		 -during a state change (onStateBegin, onTransition, onStateEnd)
		 is an error and a failed ASSERT.
		 */
		void setState( StateInterface* child, bool autoDelete = true )
		{
			DEBUG_ASSERT( mCanSetNextState, "This State Machine is in an active transition and can't change its destination state" );
 			DEBUG_ASSERT( !hasNextState(), "this State Machine already has a pending state to be set, cannot set another" );
			
			nextStatePtr = child;
			mAutoDeleteState = autoDelete;
						
			//start immediately if we have no current state
			if( !hasCurrentState() )
				_applyNextState();
		}
		
		int getCurrentState()					{	return currentState;	}
		StateInterface* getChildState()			{	return currentStatePtr;	}
		
		bool isCurrentState( int state ) const			{	return currentState == state;	}
		bool isCurrentState( StateInterface* s ) const 	{	return currentStatePtr == s;	}

		bool hasChildState()						{	return currentStatePtr != NULL;	}
		
		bool hasCurrentState()					{	return currentStatePtr != NULL || currentState != -1; }
		bool hasNextState()						{	return nextStatePtr != NULL || nextState != -1; }
		
		bool hasPendingTransition()				{	return !mTransitionCompleted;	}
		
		///begin the execution of this state
		void begin()
		{
			onBegin();
		}
		
		///loop the execution of this state (and its childs)
		void loop( float dt )
		{			
			if( mTransitionCompleted )  //do not call a loop if the current state is not "active" (ie-transition in progress)
				_subStateLoop( dt );

			if( hasNextState() )
				_applyNextState();

			onLoop( dt );
		}
		
		///end the execution of this state (and its childs)
		void end()
		{
			_subStateEnd();
			
			onEnd();	
		}
						
	protected:
		int currentState;		
		StateInterface* currentStatePtr;
		
		int nextState;
		StateInterface* nextStatePtr;
		
	private:
		
		bool mTransitionCompleted, mCanSetNextState, mAutoDeleteState;
				
		//------ state events

		///onBegin is called each time *this* State is made current on the parent state.
		/**
		For example, Game's onBegin() is called only when the application starts.
		*/
		virtual void onBegin()
		{
			
		}

		///onLoop contains all of the game's "non-event-response code", as it is called once per frame on each object that listens to it
		virtual void onLoop( float dt )
		{
			
		}

		///onEnd is called when this State is replaced by another State in the parent, or if the parent itself is destroyed
		virtual void onEnd()
		{
			
		}
		
		//----- immediate substate events

		///onStateBegin is called each time an implicit substate of this State is made current
		/**
		You can check which State has begun using isCurrentState()
		*/
		virtual void onStateBegin()
		{
			
		}
		
		///onStateLoop is called each frame on the current (implicit) state.
		/**
		You can check which State is current using isCurrentState()
		*/
		virtual void onStateLoop( float dt )
		{
			
		}
		
		///onStateBegin is called each time an implicit substate of this State has been replaced, or the parent has been destroyed.
		/**
		You can check which State has ended using isCurrentState()
		*/
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
				
		void _subStateBegin()
		{
			if( currentStatePtr )
				currentStatePtr->begin();
			else
				onStateBegin();
		}
		
		void _subStateLoop( float dt )
		{
			if( currentStatePtr )
				currentStatePtr->loop( dt );
			else
				onStateLoop( dt );
		}
		
		void _subStateEnd()
		{
			if( currentStatePtr )
				currentStatePtr->end();
			else
				onStateEnd();
		}
				
		void _nextState( int newState )		
		{
			//first try, call substate end
			if( mTransitionCompleted )
			{
				_subStateEnd();
			
				if( currentStatePtr && mAutoDeleteState )
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
		
		void _nextState( StateInterface* child )
		{
			DEBUG_ASSERT( child != nullptr, "null substate passed" );
			
			//first try, call substate end
			if( mTransitionCompleted )
			{
				_subStateEnd();
			
				if( currentStatePtr && mAutoDeleteState )
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
		
		void _applyNextState()
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

