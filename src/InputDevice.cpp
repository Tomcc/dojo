#include "stdafx.h"

#include "InputDevice.h"

using namespace Dojo;

bool Dojo::InputDevice::isKeyDown(int action)
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

void Dojo::InputDevice::addBinding(int action, KeyCode key)
{
	mBindings[key] = action;
	mInverseBindings.emplace(action, key);
}
