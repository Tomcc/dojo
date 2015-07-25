#include "InputSystem.h"

#include "GameState.h"
#include "Platform.h"
#include "InputDevice.h"
#include "InputSystemListener.h"

using namespace Dojo;

InputSystem::InputSystem(bool enable) :
	enabled(enable) {
	Platform::singleton().addApplicationListener(*this);
}

InputSystem::~InputSystem() {

}


void InputSystem::addDevice(InputDevice& device) {
	mDeviceList.emplace(&device);

	_fireDeviceConnected(device);
}

void InputSystem::removeDevice(InputDevice& device) {
	DEBUG_ASSERT(mDeviceList.find(&device) != mDeviceList.end(), "removeDevice: this device was not registered");

	_fireDeviceDisconnected(device);

	mDeviceList.erase(&device);
}

void InputSystem::addListener(InputSystemListener& l) {
	if (l.getSource() == nullptr) {
		listeners.emplace(&l);
		l._notifySource(this);
	}
}

void InputSystem::removeListener(InputSystemListener& l) {
	if (l.getSource() == this) {
		listeners.erase(&l);
		l._notifySource(nullptr);
	}
}

Touch& InputSystem::_registertouch(const Vector& point, Touch::Type type) {
	mTouchList.emplace_back(
		make_unique<Touch>(mAssignedTouches++, point, type));

	return *mTouchList.back();
}

int InputSystem::_getExistingTouchID(const Vector& point, Touch::Type type) {
	//find the nearest touch to this position
	float minDist = FLT_MAX;
	int nearest = -1;

	for (size_t i = 0; i < mTouchList.size(); ++i) {
		if (type != type) {
			continue;
		}

		float d = mTouchList[i]->point.distance(point);

		if (d < minDist) {
			nearest = i;
			minDist = d;
		}
	}

	return nearest;
}

Touch& InputSystem::_getExistingTouch(const Vector& point, Touch::Type type) {
	return *mTouchList[_getExistingTouchID(point, type)];
}

std::unique_ptr<Touch> InputSystem::_popExistingTouch(const Vector& point, Touch::Type type) {
	int idx = _getExistingTouchID(point, type);

	DEBUG_ASSERT(idx >= 0, "Needed to remove an existing touch but it was not found");

	auto t = std::move(mTouchList[idx]);
	mTouchList.erase(mTouchList.begin() + idx);

	return t;
}

void InputSystem::poll(float dt) {
	//update all the touches
	for (auto && touch : mTouchList) {
		touch->_update();
	}

	for (auto && device : mDeviceList) {
		device->poll(dt);
	}
};

void InputSystem::_fireTouchBeginEvent(const Vector& point, Touch::Type type) {
	if (enabled) {
		//create a new Touch
		auto& t = _registertouch(point, type);

		for (auto && listener : listeners) {
			listener->onTouchBegan(t);
		}
	}
}

void InputSystem::_fireTouchMoveEvent(const Vector& currentPos, const Vector& prevPos, Touch::Type type) {
	if (enabled) {
		auto& t = _getExistingTouch(prevPos, type);

		t.point = currentPos;
		t.speed = prevPos - currentPos; //get translation

		for (auto && listener : listeners) {
			listener->onTouchMove(t);
		}
	}
}

void InputSystem::_fireTouchEndEvent(const Vector& point, Touch::Type type) {
	if (enabled) {
		auto t = _popExistingTouch(point, type);
		t->point = point;

		for (auto && listener : listeners) {
			listener->onTouchEnd(*t);
		}
	}
}

void InputSystem::_fireMouseMoveEvent(const Vector& currentPos, const Vector& prevPos) {
	if (enabled) {
		for (auto && listener : listeners) {
			listener->onMouseMove(currentPos, prevPos);
		}
	}
}

void InputSystem::_fireScrollWheelEvent(float scroll) {
	if (!enabled) {
		return;
	}

	for (auto && listener : listeners) {
		listener->onScrollWheel(scroll);
	}
}

void InputSystem::_fireShakeEvent() {
	if (enabled)
		for (auto && listener : listeners) {
			listener->onShake();
		}
}

void InputSystem::_fireAccelerationEvent(const Vector& accel, float roll) {
	if (enabled) {
		for (auto && listener : listeners) {
			listener->onAcceleration(accel, roll);
		}
	}
}

void InputSystem::_fireDeviceConnected(InputDevice& j) {
	//notify listeners
	for (auto && l : listeners) {
		l->onDeviceConnected(j);
	}

	DEBUG_MESSAGE("Connected a new input device!");
}

void InputSystem::_fireDeviceDisconnected(InputDevice& j) {
	DEBUG_ASSERT( mDeviceList.find(&j) != mDeviceList.end(), "Tried to disconnect a non existing device" );

	//first notify this to all the listeners
	for (auto && l : listeners) {
		l->onDeviceDisconnected(j);
	}
}
