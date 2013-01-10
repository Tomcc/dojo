#ifndef Joystick_h__
#define Joystick_h__

#include "dojo_common_header.h"

#include "Array.h"

namespace Dojo
{
	///a generic interface over actual joypad implementations
	class Joystick
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

		class Listener
		{
		public:
			virtual void onButtonPressed( Dojo::Joystick* j, int button )	{};
			virtual void onButtonReleased( Dojo::Joystick* j, int button )	{};

			virtual void onAxisMoved( Dojo::Joystick* j, Dojo::Joystick::Axis a, float state, float change )	{};

			///this event is fired just before the joystick is disconnected and the Joystick object deleted
			virtual void onDisconnected( Dojo::Joystick* j ) {};
		};

		Joystick( int ID ) :
		mID( ID )
		{
			memset( mAxis, 0, sizeof( mAxis ) );
			memset( mButton, 0, sizeof( mButton ) );
		}

		virtual bool isKeyDown( int keyNumber )
		{
			DEBUG_ASSERT( keyNumber > 0 );
			DEBUG_ASSERT( keyNumber < BUTTON_MAX );

			return mButton[ keyNumber ];
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

		inline void _notifyButtonState( int button, bool pressed )
		{
			if( mButton[button] != pressed )
			{
				mButton[button] = pressed; //buffer state

				if( pressed ) for( Listener* l : pListeners )
					l->onButtonPressed( this, button );

				else for( Listener* l : pListeners )
					l->onButtonReleased( this, button );
			}
		}

		inline void _notifyAxis( Axis a, float state)
		{
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

	protected:

		typedef Dojo::Array< Listener* > ListenerList;

		ListenerList pListeners;

		bool mButton[ BUTTON_MAX ];
		float mAxis[ _AI_COUNT ];

		int mID;
	};
}
#endif // Joystick_h__
