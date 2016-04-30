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

namespace Dojo {
	///Objects inheriting from StateInterface are at the same time a State, and a State Machine themselves; most of the gameplay logic of a Dojo game should be implemented overriding onBegin, onLoop and onEnd protected events
	class StateInterface {
	public:

		StateInterface();

		virtual ~StateInterface();

		///sets a new substate either immediately or at the next loop.
		/**
			-Immediately when there's no current state, eg. at begin
			-at the next loop when it replaces a previous state

		Warning, calling setState
			-with a pending state change
			-during a state change (onStateBegin, onTransition, onStateEnd)
			is an error and a failed ASSERT.
		*/
		void setState(int newState);

		///sets a new substate immediately
		/**
		 beware - this can cause MANY unexpected behaviours

		 Warning, calling setState
		 -with a pending state change
		 -during a state change (onStateBegin, onTransition, onStateEnd)
		 is an error and a failed ASSERT.
		 */
		void setStateImmediate(int newState);

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
		void setState(std::shared_ptr<StateInterface> child);

		int getCurrentState() {
			return currentState;
		}

		StateInterface* getChildState() {
			return currentStatePtr.get();
		}

		bool isCurrentState(int state) const {
			return currentState == state;
		}

		bool isNextState(int state) const {
			return nextState == state;
		}

		bool isCurrentState(StateInterface* s) const {
			return currentStatePtr.get() == s;
		}

		bool hasChildState() {
			return currentStatePtr != nullptr;
		}

		bool hasCurrentState() {
			return currentStatePtr != nullptr or currentState != -1;
		}

		bool hasNextState() {
			return nextStatePtr != nullptr or nextState != -1;
		}

		bool hasPendingTransition() {
			return not mTransitionCompleted;
		}

		///begin the execution of this state
		virtual void begin();

		///loop the execution of this state (and its childs)
		virtual void loop(float dt);

		///end the execution of this state (and its childs)
		virtual void end();

	protected:
		int currentState = -1;
		std::shared_ptr<StateInterface> currentStatePtr;

		int nextState = -1;
		std::shared_ptr<StateInterface> nextStatePtr;

	private:

		bool mTransitionCompleted = true, mCanSetNextState = true;

		//------ state events

		///onBegin is called each time *this* State is made current on the parent state.
		/**
		For example, Game's onBegin() is called only when the application starts.
		*/
		virtual void onBegin() {
		}

		///onLoop contains all of the game's "non-event-response code", as it is called once per frame on each object that listens to it
		virtual void onLoop(float dt) {
		}

		///onEnd is called when this State is replaced by another State in the parent, or if the parent itself is destroyed
		virtual void onEnd() {
		}

		//----- immediate substate events

		///onStateBegin is called each time an implicit substate of this State is made current
		/**
		You can check which State has begun using isCurrentState()
		*/
		virtual void onStateBegin() {
		}

		///onStateLoop is called each frame on the current (implicit) state.
		/**
		You can check which State is current using isCurrentState()
		*/
		virtual void onStateLoop(float dt) {
		}

		///onStateBegin is called each time an implicit substate of this State has been replaced, or the parent has been destroyed.
		/**
		You can check which State has ended using isCurrentState()
		*/
		virtual void onStateEnd() {
		}

		///this is called each time a transition will happen.
		/**
		 returning false delays the transition to the next frame (with a new onTransition call)
		*/
		virtual bool onTransition() {
			return true;
		}

		void _subStateBegin();
		void _subStateLoop(float dt);
		void _subStateEnd();

		void _nextState(int& newState);
		void _nextState(std::shared_ptr<StateInterface>& child);
		void _applyNextState();
	};
}
