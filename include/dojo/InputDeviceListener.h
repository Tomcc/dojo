#pragma once

#include "dojo_common_header.h"
#include "InputDevice.h"

namespace Dojo {

	///A Device Listener receives events about buttons and axis changes on the device it listens to
	class InputDeviceListener
	{
	public:
		///ButtonPressed events are sent when the button bound to "action" is pressed on the device j
		virtual void onButtonPressed(InputDevice* j, int action)	{}
		///ButtonReleased events are sent when the button bound to "action" is released on the device j
		virtual void onButtonReleased(InputDevice* j, int action)	{}

		///AxisMoved events are sent when the axis a is changed on the device j, with a current state of "state" and with the reported relative speed
		virtual void onAxisMoved(InputDevice* j, InputDevice::Axis a, float state, float speed)	{}

		///this event is fired just before the device is disconnected and the InputDevice object deleted
		virtual void onDisconnected(InputDevice* j) {}
	};
}



