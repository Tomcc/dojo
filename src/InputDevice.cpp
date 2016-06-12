#include "InputDevice.h"
#include "InputDeviceListener.h"

using namespace Dojo;

InputDevice::InputDevice(Type type, int ID, uint8_t buttonNumber, uint8_t axisNumber) :
	mID(ID),
	mButtonNumber(buttonNumber),
	mAxisNumber(axisNumber),
	type(type) {
	for (uint8_t i = 0; i < mAxisNumber; ++i) {
		mAxis.emplace_back(0.f);
		mDeadZone.emplace_back(0.f);
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
		if (binding.action == action and isKeyDown(binding.key)) {
			return true;
		}
	}

	return false;
}

bool InputDevice::hasBinding(int action, KeyCode key) const {
	for (auto&& b : mBindings) {
		if (action == b.action and key == b.key) {
			return true;
		}
	}

	return false;
}

void InputDevice::addBinding(int action, KeyCode key) {
	if (not hasBinding(action, key)) {
		mBindings.emplace(action, key);
	}
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
					for (size_t i = 0; i < pListeners.size(); ++i) { //do not use ranges! listeners can be added and removed in the inner loop
						pListeners[i]->onButtonPressed(self, binding.action);
					}
				}

				else {
					for (size_t i = 0; i < pListeners.size(); ++i) { //do not use ranges! listeners can be added and removed in the inner loop
						pListeners[i]->onButtonReleased(self, binding.action);
					}
				}
			}
		}
	}
}

void InputDevice::_notifyAxis(Axis a, float state) {
	//apply the dead zone
	if (std::abs(state) < mDeadZone[a]) {
		state = 0;
	}

	if (mAxis[a] != state) {
		float change = mAxis[a] - state;
		mAxis[a] = state;

		for (InputDeviceListener* l : pListeners) {
			l->onAxisMoved(self, a, state, change);
		}
	}
}

void InputDevice::_fireDisconnected() {
	for (InputDeviceListener* l : pListeners) {
		l->onDisconnected(self);
	}
}

void InputDevice::clearBindings() {
	mBindings.clear();
}
