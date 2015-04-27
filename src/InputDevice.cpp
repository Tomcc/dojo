#include "InputDevice.h"
#include "InputDeviceListener.h"

using namespace Dojo;

InputDevice::InputDevice(Type type, int ID, int buttonNumber, int axisNumber) :
	mID(ID),
	mButtonNumber(buttonNumber),
	mAxisNumber(axisNumber),
	type(type) {
	for (int i = 0; i < mAxisNumber; ++i) {
		mAxis.push_back(0);
		mDeadZone.push_back(0);
	}
}

void InputDevice::addListener(InputDeviceListener& l) {
	pListeners.emplace(&l);	
}

void InputDevice::removeListener(InputDeviceListener& l) {
	DEBUG_ASSERT(pListeners.find(&l) != pListeners.end(), "The listened to be removed is not registered");

	pListeners.erase(&l);
}

bool InputDevice::isKeyDown(KeyCode key) {
	KeyPressedMap::iterator elem = mButton.find(key);
	return elem != mButton.end() ? elem->second : false;
}

bool InputDevice::isKeyDown(int action) {
	//check all the keys bound to this one
	for (auto&& binding : mBindings) {
		if (binding.action == action && isKeyDown(binding.key))
			return true;
	}
	return false;
}

bool InputDevice::hasBinding(int action, KeyCode key) const {
	for (auto&& b : mBindings) {
		if (action == b.action && key == b.key)
			return true;
	}
	return false;
}

void InputDevice::addBinding(int action, KeyCode key) {
	if (!hasBinding(action,key))
		mBindings.emplace(action, key);
}

float InputDevice::getAxis(Axis axis) {
	return mAxis[axis];
}

void InputDevice::poll(float dt) {

}

void InputDevice::_notifyButtonState(KeyCode key, bool pressed) {
	if (isKeyDown(key) != pressed) {
		mButton[key] = pressed; //buffer state

		//notify once for every action connected to this key
		for (auto&& binding : mBindings) {
			if (binding.key == key) {
				if (pressed) {
					for (size_t i = 0; i < pListeners.size(); ++i) //do not use ranges! listeners can be added and removed in the inner loop
						pListeners[i]->onButtonPressed(*this, binding.action);
				}

				else {
					for (size_t i = 0; i < pListeners.size(); ++i) //do not use ranges! listeners can be added and removed in the inner loop
						pListeners[i]->onButtonReleased(*this, binding.action);
				}
			}
		}
	}
}

void InputDevice::_notifyAxis(Axis a, float state) {
	//apply the dead zone
	if (abs(state) < mDeadZone[a])
		state = 0;

	if (mAxis[a] != state) {
		float change = mAxis[a] - state;
		mAxis[a] = state;

		for (InputDeviceListener* l : pListeners)
			l->onAxisMoved(*this, a, state, change);
	}
}

void InputDevice::_fireDisconnected() {
	for (InputDeviceListener* l : pListeners)
		l->onDisconnected(*this);
}
