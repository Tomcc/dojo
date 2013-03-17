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

		struct FakeAxis
		{
			Axis axis;

			KeyCode min, max;

			FakeAxis( Axis toEmulate, KeyCode minKey, KeyCode maxKey ) :
			axis( toEmulate ),
			min( minKey ),
			max( maxKey )
			{

			}
		};

		typedef std::vector< FakeAxis > FakeAxes;

		//a keyboard has n buttons (KC_JOYPAD_1 comes right after the KB button defs, and 2 fake axes, LX and LY
		Keyboard() :
		InputDevice( "keyboard", 0, KC_JOYPAD_1, 3 )
		{

		}

		void addFakeAxis( const FakeAxis& a )
		{
			mFakeAxes.push_back( a );
		}

		virtual void poll( float dt )
		{
			for( auto& fakeAxis : mFakeAxes )
			{
				float l = 0;
				l += isKeyDown( fakeAxis.max ) ? 1.f : 0.f;
				l -= isKeyDown( fakeAxis.min ) ? 1.f : 0.f;

				_notifyAxis( fakeAxis.axis, l );
			}
		}

	protected:

		FakeAxes mFakeAxes;
	private:
	};
}

#endif // Keyboard_h__
