#include "stdafx.h"

#include "InputDevice.h"
#include "InputDeviceListener.h"

using namespace Dojo;

InputDevice::InputDevice(String name, int ID, int buttonNumber, int axisNumber) :
mID(ID),
mButtonNumber(buttonNumber),
mAxisNumber(axisNumber),
mType(name) {
	for (int i = 0; i < mAxisNumber; ++i)
	{
		mAxis.add(0);
		mDeadZone.add(0);
	}
}

void InputDevice::addListener(InputDeviceListener* l) {
	DEBUG_ASSERT(l, "Adding a null listener");
	DEBUG_ASSERT(!pListeners.exists(l), "The listener is already registered");

	pListeners.add(l);
}

void InputDevice::removeListener(InputDeviceListener* l) {
	DEBUG_ASSERT(l, "The passed listener is NULL");
	DEBUG_ASSERT(pListeners.exists(l), "The listened to be removed is not registered");

	pListeners.remove(l);
}

bool InputDevice::isKeyDown(KeyCode key) {
	KeyPressedMap::iterator elem = mButton.find(key);
	return elem != mButton.end() ? elem->second : false;
}

bool InputDevice::isKeyDown(int action)
{
	//check all the keys bound to this one
	auto range = mInverseBindings.equal_range(action);

	for (; range.first != range.second; ++range.first)
	{
		if (isKeyDown(range.first->second))
			return true;
	}
	return false;
}

void InputDevice::addBinding(int action, KeyCode key)
{
	mBindings[key] = action;
	mInverseBindings.emplace(action, key);
}

float InputDevice::getAxis(Axis axis) {
	return mAxis[axis];
}

int InputDevice::getActionForKey(KeyCode key) {
	KeyActionMap::iterator elem = mBindings.find(key);
	return elem != mBindings.end() ? elem->second : key;
}

void InputDevice::poll(float dt) {

}

void InputDevice::_notifyButtonState(KeyCode key, bool pressed) {
	if (isKeyDown(key) != pressed)
	{
		mButton[key] = pressed; //buffer state

		int action = getActionForKey(key);

		if (pressed) for (InputDeviceListener* l : pListeners)
			l->onButtonPressed(this, action);

		else for (InputDeviceListener* l : pListeners)
			l->onButtonReleased(this, action);
	}
}

void InputDevice::_notifyAxis(Axis a, float state) {
	//apply the dead zone
	if (abs(state) < mDeadZone[a])
		state = 0;

	if (mAxis[a] != state)
	{
		float change = mAxis[a] - state;
		mAxis[a] = state;

		for (InputDeviceListener* l : pListeners)
			l->onAxisMoved(this, a, state, change);
	}
}

void InputDevice::_fireDisconnected() {
	for (InputDeviceListener* l : pListeners)
		l->onDisconnected(this);
}
