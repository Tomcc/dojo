#ifndef XInputJoystick_h__
#define XInputJoystick_h__

#include "dojo_common_header.h"

#include "InputDevice.h"
#include "dojomath.h"
#include "InputSystem.h"

#include <Xinput.h>

#define XINPUTJOYSTICK_CONNECTION_CHECK_TIMEOUT 3.f

namespace Dojo
{
	class XInputJoystick : public InputDevice
	{
	public:

		XInputJoystick( int n ) : 
		InputDevice( DT_XBOX_JOYSTICK, n, 16, 8 ),
		mConnectionCheckTimer( 0 ),
		mConnected( false )
		{
			//set default dead zones
			mDeadZone[ AI_LX ] = 
			mDeadZone[ AI_LY ] =
			mDeadZone[ AI_RX ] =
			mDeadZone[ AI_RY ] = 0.15f;
		}

		///polls the joystick and launches events - note: XInput pads are actually created at startup, even if Dojo treats them client-side as new objects created on connection!
		virtual void poll( float dt )
		{
			XINPUT_STATE state;
			
			if( !mConnected )
			{
				mConnectionCheckTimer -= dt;

				if( mConnectionCheckTimer > 0 )  //do not spam connection checks - check every 3 s
					return;
				else
					mConnectionCheckTimer = XINPUTJOYSTICK_CONNECTION_CHECK_TIMEOUT;
			}

			HRESULT dwResult = XInputGetState( mID, &state );
			bool connected = (dwResult == ERROR_SUCCESS);

			if( connected )
			{
				if( !mConnected ) //yeeeee we're connected!
					Platform::getSingleton()->getInput()->addDevice( this );

				int buttonMask = state.Gamepad.wButtons; //wButtons is a mask where each bit represents a button state

				int kc = (int)KC_JOYPAD_1;
				for (int b = 0; b < mButtonNumber; ++b )
					_notifyButtonState( (KeyCode)(kc + b), Math::getBit( buttonMask, b ) );

				_notifyAxis( AI_LX, (float)state.Gamepad.sThumbLX * (1.0f / (float)0x7fff));
				_notifyAxis( AI_LY, (float)state.Gamepad.sThumbLY * (1.0f / (float)0x7fff));

				_notifyAxis( AI_RX, (float)state.Gamepad.sThumbRX * (1.0f / (float)0x7fff));
				_notifyAxis( AI_RY, (float)state.Gamepad.sThumbRY * (1.0f / (float)0x7fff));

				_notifyAxis( AI_SLIDER1, (float)state.Gamepad.bLeftTrigger * (1.0f / (float)255));

				_notifyAxis( AI_SLIDER2, (float)state.Gamepad.bRightTrigger * (1.0f / (float)255));
			}
			else if( mConnected )
			{
				//notify disconnection to listeners and to the input system
				_fireDisconnected();
				Platform::getSingleton()->getInput()->removeDevice( this );

				//clear the listeners because dojo's contract is to create a *new* joystick object for each connection
				pListeners.clear();
			}

			mConnected = connected;
		}

	protected:

		bool mConnected;
		float mConnectionCheckTimer;

	private:
	};
}

#endif // XInputJoystick_h__
