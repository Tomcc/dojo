#include "stdafx.h"

#include "XInputController.h"

#include "dojomath.h"
#include "InputSystem.h"
#include "Platform.h"

#include <Xinput.h>

#define XINPUTCONTROLLER_CONNECTION_CHECK_TIMEOUT 3.f

using namespace Dojo;

XInputController::XInputController(int n) :
InputDevice(InputDevice::Type::Xbox, n, 16, 8),
mConnectionCheckTimer(0),
mConnected(false) {
	//set default dead zones
	mDeadZone[AI_LX] =
		mDeadZone[AI_LY] =
		mDeadZone[AI_RX] =
		mDeadZone[AI_RY] = 0.15f;
}

bool XInputController::isConnected() {
	return mConnected;
}

bool XInputController::hasAxis(Axis a) const  {
	return a < Axis::_AI_COUNT; //TODO change if we add axes
}

void XInputController::poll(float dt) {
	XINPUT_STATE state;

	if (!mConnected)
	{
		mConnectionCheckTimer -= dt;

		if (mConnectionCheckTimer > 0)  //do not spam connection checks - check every 3 s
			return;
		else
			mConnectionCheckTimer = XINPUTCONTROLLER_CONNECTION_CHECK_TIMEOUT;
	}

	HRESULT dwResult = XInputGetState(mID, &state);
	bool connected = (dwResult == ERROR_SUCCESS);

	if (connected)
	{
		if (!mConnected) //yeeeee we're connected!
			Platform::singleton().getInput().addDevice(this);

		int buttonMask = state.Gamepad.wButtons; //wButtons is a mask where each bit represents a button state

		int kc = (int)KC_JOYPAD_1;
		for (int b = 0; b < mButtonNumber; ++b)
			_notifyButtonState((KeyCode)(kc + b), Math::getBit(buttonMask, b));

		_notifyAxis(AI_LX, (float)state.Gamepad.sThumbLX * (1.0f / (float)0x7fff));
		_notifyAxis(AI_LY, (float)state.Gamepad.sThumbLY * (1.0f / (float)0x7fff));

		_notifyAxis(AI_RX, (float)state.Gamepad.sThumbRX * (1.0f / (float)0x7fff));
		_notifyAxis(AI_RY, (float)state.Gamepad.sThumbRY * (1.0f / (float)0x7fff));

		float leftTrigger = (float)state.Gamepad.bLeftTrigger * (1.0f / (float)255);
		_notifyAxis(AI_SLIDER1, leftTrigger);

		float rightTrigger = (float)state.Gamepad.bRightTrigger * (1.0f / (float)255);
		_notifyAxis(AI_SLIDER2, rightTrigger);

		//emulate two buttons with the triggers, if the user doesn't need the full range
		_notifyButtonState(KC_XBOX_LT, leftTrigger > 0.9f);
		_notifyButtonState(KC_XBOX_RT, rightTrigger > 0.9f);
	}
	else if (mConnected)
	{
		//notify disconnection to listeners and to the input system
		_fireDisconnected();
		Platform::singleton().getInput().removeDevice(this);

		//clear the listeners because dojo's contract is to create a *new* joystick object for each connection
		pListeners.clear();
	}

	mConnected = connected;
}
