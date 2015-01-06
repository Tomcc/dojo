#include "stdafx.h"

#include "Keyboard.h"

using namespace Dojo;

Keyboard::Keyboard() :
InputDevice(Type::Keyboard, 0, KC_JOYPAD_1, 3) {

}

void Keyboard::addFakeAxis(Axis axis, KeyCode min, KeyCode max) {
	if (!hasAxis(axis))
		mFakeAxes.emplace_back(axis, min, max);
}

void Keyboard::poll(float dt) {
	float accum[Axis::_AI_COUNT] = { 0 };

	for (auto& fakeAxis : mFakeAxes)
	{
		accum[fakeAxis.axis] += isKeyDown(fakeAxis.max) ? 1.f : 0.f;
		accum[fakeAxis.axis] -= isKeyDown(fakeAxis.min) ? 1.f : 0.f;
	}

	for (int x = 0; x < mAxisNumber; ++x)
		_notifyAxis((Axis)x, accum[x]);
}

bool Keyboard::hasAxis(Axis x) const  {
	for (auto& a : mFakeAxes) {
		if (a.axis == x)
			return true;
	}
	
	return false;
}
