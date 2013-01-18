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

		//a keyboard has n buttons (KC_JOYPAD_1 comes right after the KB button defs, and 2 fake axes, LX and LY
		Keyboard() :
		InputDevice( DT_KEYBOARD, 0, KC_JOYPAD_1, 2 )
		{

		}

		virtual void poll( float dt )
		{
			float lx = 0;
			lx += isKeyDown( KC_RIGHT ) ? 1.f : 0.f;
			lx -= isKeyDown( KC_LEFT ) ? 1.f : 0.f;

			float ly = 0;
			ly += isKeyDown( KC_UP ) ? 1.f : 0.f;
			ly -= isKeyDown( KC_DOWN ) ? 1.f : 0.f;

			_notifyAxis( AI_LX, lx );
			_notifyAxis( AI_LY, ly );
		}

	protected:
	private:
	};
}

#endif // Keyboard_h__
