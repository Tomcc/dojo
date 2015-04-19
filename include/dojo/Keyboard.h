#pragma once

#include "dojo_common_header.h"

#include "InputDevice.h"

namespace Dojo {
	///A keyboard represents a single key mapping abstracted on the keyboard
	/**\remark multiple Keyboards can exist at the same time, allowing for same-keyboard multiplayer*/
	class Keyboard : public InputDevice {
	public:

		struct FakeAxis {
			Axis axis;
			KeyCode min, max;

			FakeAxis(Axis axis, KeyCode min, KeyCode max) :
				axis(axis),
				min(min),
				max(max) {

			}

			bool operator ==(const FakeAxis& other) const {
				return axis == other.axis && min == other.min && max == other.max;
			}
		};

		typedef std::vector<FakeAxis> FakeAxes;

		//a keyboard has n buttons (KC_JOYPAD_1 comes right after the KB button defs, and 2 fake axes, LX and LY
		Keyboard();

		virtual ~Keyboard() {
		}

		void addFakeAxis(Axis axis, KeyCode min, KeyCode max);

		virtual void poll(float dt) override;

		virtual bool hasAxis(Axis a) const override;

	protected:

		FakeAxes mFakeAxes;
	private:
	};
}
