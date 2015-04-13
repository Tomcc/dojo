#include "stdafx.h"

#include "StateInterface.h"

using namespace Dojo;

StateInterface::StateInterface() {

}

StateInterface::~StateInterface() {

}

void StateInterface::setState(int newState)
{
	DEBUG_ASSERT(mCanSetNextState, "This State Machine is in an active transition and can't change its destination state");

	nextState = newState;

	//start immediately if we have no current state
	if (!hasCurrentState())
		_applyNextState();
}

void StateInterface::setStateImmediate(int newState)
{
	DEBUG_ASSERT(mCanSetNextState, "This State Machine is in an active transition and can't change its destination state");

	nextState = newState;

	_applyNextState();
}

void StateInterface::setState(std::unique_ptr<StateInterface> child) {
	DEBUG_ASSERT(mCanSetNextState, "This State Machine is in an active transition and can't change its destination state");
	DEBUG_ASSERT(!hasNextState(), "this State Machine already has a pending state to be set, cannot set another");

	nextStatePtr = std::move(child);

	//start immediately if we have no current state
	if (!hasCurrentState())
		_applyNextState();
}

void StateInterface::begin()
{
	onBegin();
}

void StateInterface::loop(float dt)
{
	if (mTransitionCompleted)  //do not call a loop if the current state is not "active" (ie-transition in progress)
		_subStateLoop(dt);

	if (hasNextState())
		_applyNextState();

	onLoop(dt);
}

void StateInterface::end()
{
	_subStateEnd();

	onEnd();
}

void StateInterface::_subStateBegin()
{
	if (currentStatePtr)
		currentStatePtr->begin();
	else
		onStateBegin();
}

void StateInterface::_subStateLoop(float dt)
{
	if (currentStatePtr)
		currentStatePtr->loop(dt);
	else
		onStateLoop(dt);
}

void StateInterface::_subStateEnd()
{
	if (currentStatePtr)
		currentStatePtr->end();
	else
		onStateEnd();
}


void StateInterface::_nextState(int newState)
{
	//first try, call substate end
	if (mTransitionCompleted) {
		_subStateEnd();
	}

	//now try the transition
	mTransitionCompleted = onTransition();

	if (mTransitionCompleted)
	{
		currentState = newState;
		currentStatePtr = nullptr;
		nextState = -1;

		_subStateBegin();
	}
}

void StateInterface::_nextState(std::unique_ptr<StateInterface>& child)
{
	DEBUG_ASSERT(child, "nullptr substate passed");

	//first try, call substate end
	if (mTransitionCompleted)
	{
		_subStateEnd();
		currentStatePtr.reset(); //delete the current state before the transition
	}

	//now try the transition
	mTransitionCompleted = onTransition();

	if (mTransitionCompleted)
	{
		currentState = -1;
		currentStatePtr = std::move(child);

		_subStateBegin();
	}
}

void StateInterface::_applyNextState()
{
	DEBUG_ASSERT(hasNextState(), "_applyNextState was called but not next state was defined");

	//they have to be cancelled before, because the state that is beginning
	//could need to set them again
	int temp = nextState;

	mCanSetNextState = false; //disable state setting - it can't be done while changing a state!

	//change state
	if (nextStatePtr)
		_nextState(nextStatePtr);
	else if (temp != -1)
		_nextState(temp);

	if (mTransitionCompleted)
		mCanSetNextState = true;
}