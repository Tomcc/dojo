#ifndef Keyboard_h__
#define Keyboard_h__

#include "dojo_common_header.h"

#include "InputDevice.h"

namespace Dojo
{
	///A keyboard represents a single key mapping abstracted on the keyboard
	/**\remark multiple Keyboards can exist at the same time, allowing for same-keyboard multiplayer*/
	class Keyboard : public InputDevice
	{
	public:

		struct FakeAxis {
			Axis axis;
			KeyCode min, max;
			FakeAxis(Axis axis, KeyCode min, KeyCode max) :
				axis(axis),
				min(min),
				max(max) {

			}
		};
		typedef std::vector< FakeAxis > FakeAxes;

		//a keyboard has n buttons (KC_JOYPAD_1 comes right after the KB button defs, and 2 fake axes, LX and LY
		Keyboard() :
		InputDevice( "keyboard", 0, KC_JOYPAD_1, 3 )
		{

		}

		void addFakeAxis( Axis axis, KeyCode min, KeyCode max )
		{
			mFakeAxes.emplace_back(axis, min, max);
		}

		virtual void poll( float dt )
		{
			float accum[Axis::_AI_COUNT] = { 0 };

			for( auto& fakeAxis : mFakeAxes )
			{
				accum[fakeAxis.axis] += isKeyDown( fakeAxis.max ) ? 1.f : 0.f;
				accum[fakeAxis.axis] -= isKeyDown( fakeAxis.min ) ? 1.f : 0.f;
			}

			for (int x = 0; x < mAxisNumber; ++x)
				_notifyAxis((Axis)x, accum[x]);
		}

	protected:

		FakeAxes mFakeAxes;
	private:
	};
}

#endif // Keyboard_h__
