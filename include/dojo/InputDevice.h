#ifndef InputDevice_h__
#define InputDevice_h__

#include "dojo_common_header.h"

#include "Array.h"

namespace Dojo
{
	///a generic interface over the actual device implementations
	class InputDevice
	{
	public:

		const static int BUTTON_MAX = 16;

		enum Type
		{
			DT_KEYBOARD,
			DT_XBOX_JOYSTICK,
			DT_DINPUT_JOYSTICK,
			DT_MAC_JOYSTICK,
			DT_LINUX_JOYSTICK,
			DT_OUYA_JOYSTICK
		};

		//! each pad has a number of analog inputs; each one is a float
		enum Axis
		{
			AI_LX,
			AI_LY,
			AI_LZ,

			AI_RX,
			AI_RY,
			AI_RZ,

			AI_SLIDER1,
			AI_SLIDER2,

			_AI_COUNT
		};

		class Listener
		{
		public:
			virtual void onButtonPressed( Dojo::InputDevice* j, int action )	{};
			virtual void onButtonReleased( Dojo::InputDevice* j, int action )	{};

			virtual void onAxisMoved( Dojo::InputDevice* j, Dojo::InputDevice::Axis a, float state, float change )	{};

			///this event is fired just before the device is disconnected and the InputDevice object deleted
			virtual void onDisconnected( Dojo::InputDevice* j ) {};
		};

		InputDevice( Type type, int ID, int buttonNumber, int axisNumber ) :
		mID( ID ),
		mButtonNumber( buttonNumber ),
		mAxisNumber( axisNumber )
		{
			for( int i = 0; i < mAxisNumber; ++i )
			{
				mAxis.add( 0 );
				mDeadZone.add( 0 );
			}
		}

		virtual bool isKeyDown( KeyCode key )
		{
			KeyPressedMap::iterator elem = mButton.find( key );
			return elem != mButton.end() ? elem->second : false;
		}

		virtual float getAxis( Axis axis )
		{
			return mAxis[ axis ];
		}

		inline int getID()
		{
			return mID;
		}

		inline void addListener( Listener* l )
		{
			DEBUG_ASSERT( l );
			DEBUG_ASSERT( !pListeners.exists( l ) );

			pListeners.add( l );
		}

		inline void removeListener( Listener* l )
		{
			DEBUG_ASSERT( l );
			DEBUG_ASSERT( pListeners.exists( l ) );

			pListeners.remove( l );
		}

		inline void addBinding( int action, KeyCode key )
		{
			mBindings[ key ] = action;
		}

		///returns the set binding for this action
		/** /remark the default action for unassigned keys is the key itself */
		inline int getActionForKey( KeyCode key )
		{
			KeyActionMap::iterator elem = mBindings.find( key );
			return elem != mBindings.end() ? elem->second : key;
		}

		inline Type getType()
		{
			return mType;
		}

		inline void _notifyButtonState( KeyCode key, bool pressed )
		{
			if( isKeyDown( key ) != pressed )
			{
				mButton[key] = pressed; //buffer state

				int action = getActionForKey( key );

				if( pressed ) for( Listener* l : pListeners )
					l->onButtonPressed( this, action );

				else for( Listener* l : pListeners )
					l->onButtonReleased( this, action );
			}
		}

		inline void _notifyAxis( Axis a, float state)
		{
			//apply the dead zone
			if( abs(state) < mDeadZone[a] )
				state = 0;

			if( mAxis[a] != state )
			{
				float change = mAxis[ a ] - state;
				mAxis[ a ] = state;

				for( Listener* l : pListeners )
					l->onAxisMoved( this, a, state, change );
			}
		}

		inline void _fireDisconnected()
		{
			for( Listener* l : pListeners )
				l->onDisconnected( this );
		}

		///each device can be polled each frame if needed
		virtual void poll( float dt )
		{

		}

	protected:

		typedef Dojo::Array< Listener* > ListenerList;
		typedef unordered_map< KeyCode, int > KeyActionMap;
		typedef unordered_map< KeyCode, bool > KeyPressedMap;
		typedef Dojo::Array< float > FloatList;

		Type mType;
		int mAxisNumber, mButtonNumber;

		ListenerList pListeners;

		KeyPressedMap mButton;
		FloatList mAxis, mDeadZone;

		KeyActionMap mBindings;

		int mID;
	};
}
#endif // Joystick_h__
