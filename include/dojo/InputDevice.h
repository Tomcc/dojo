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

		///A Device Listener receives events about buttons and axis changes on the device it listens to
		class Listener
		{
		public:
			///ButtonPressed events are sent when the button bound to "action" is pressed on the device j
			virtual void onButtonPressed( Dojo::InputDevice* j, int action )	{}
			///ButtonReleased events are sent when the button bound to "action" is released on the device j
			virtual void onButtonReleased( Dojo::InputDevice* j, int action )	{}

			///AxisMoved events are sent when the axis a is changed on the device j, with a current state of "state" and with the reported relative speed
			virtual void onAxisMoved( Dojo::InputDevice* j, Dojo::InputDevice::Axis a, float state, float speed )	{}

			///this event is fired just before the device is disconnected and the InputDevice object deleted
			virtual void onDisconnected( Dojo::InputDevice* j ) {}
		};

		///Creates a new InputDevice of the given type, bound to the ID slot, supporting "buttonNumber" buttons and "axisNumber" axes
		InputDevice( String name, int ID, int buttonNumber, int axisNumber ) :
		mID( ID ),
		mButtonNumber( buttonNumber ),
		mAxisNumber( axisNumber ),
		mType( name )
		{
			for( int i = 0; i < mAxisNumber; ++i )
			{
				mAxis.add( 0 );
				mDeadZone.add( 0 );
			}
		}

		///returns if the given action is pressed
		virtual bool isKeyDown( KeyCode key )
		{
			KeyPressedMap::iterator elem = mButton.find( key );
			return elem != mButton.end() ? elem->second : false;
		}

		///returns the instant state of this axis
		virtual float getAxis( Axis axis )
		{
			return mAxis[ axis ];
		}

		///returns the bound slot for this Device
		/*
		for example, 0..3 for XBox controllers, or 0..n for each control method mapped to the keyboard
		*/
		inline int getID()
		{
			return mID;
		}

		inline void addListener( Listener* l )
		{
			DEBUG_ASSERT( l, "Adding a null listener" );
			DEBUG_ASSERT( !pListeners.exists( l ), "The listener is already registered" );

			pListeners.add( l );
		}

		inline void removeListener( Listener* l )
		{
			DEBUG_ASSERT( l, "The passed listener is NULL");
			DEBUG_ASSERT( pListeners.exists( l ), "The listened to be removed is not registered" );

			pListeners.remove( l );
		}

		///Adds an "Action Binding" to this device
		/**
		InputDevice::Listener will only receive events in terms of Actions;
		the same action can be bound to more than one KeyCode to allow for easy input configuration.
		\remark the default Action for a key is the key number itself
		*/
		inline void addBinding( int action, KeyCode key )
		{
			mBindings[ key ] = action;
		}

		///returns the action bound to this KeyCode
		/** 
		/remark the default action for unassigned keys is the key number itself */
		inline int getActionForKey( KeyCode key )
		{
			KeyActionMap::iterator elem = mBindings.find( key );
			return elem != mBindings.end() ? elem->second : key;
		}

		inline const String& getType()
		{
			return mType;
		}

		///each device can be polled each frame if needed
		virtual void poll( float dt )
		{

		}

		///internal
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

		///internal
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

		///internal
		inline void _fireDisconnected()
		{
			for( Listener* l : pListeners )
				l->onDisconnected( this );
		}

	protected:

		typedef Dojo::Array< Listener* > ListenerList;
		typedef std::unordered_map< KeyCode, int, std::hash<int> > KeyActionMap;
		typedef std::unordered_map< KeyCode, bool, std::hash<int> > KeyPressedMap;
		typedef Dojo::Array< float > FloatList;

		String mType;
		int mAxisNumber, mButtonNumber;

		ListenerList pListeners;

		KeyPressedMap mButton;
		FloatList mAxis, mDeadZone;

		KeyActionMap mBindings;

		int mID;
	};
}
#endif // Joystick_h__
