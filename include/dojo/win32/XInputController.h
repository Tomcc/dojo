#pragma once

#include "dojo_common_header.h"

#include "InputDevice.h"

namespace Dojo {
	class XInputController : public InputDevice {
	public:

		XInputController(int n);

		virtual ~XInputController() {
		}

		bool isConnected();

		///polls the joystick and launches events - note: XInput pads are actually created at startup, even if Dojo treats them client-side as new objects created on connection!
		virtual void poll(float dt);

		virtual bool hasAxis(Axis a) const override;

	protected:

		bool mConnected;
		float mConnectionCheckTimer;

	private:
	};
}
