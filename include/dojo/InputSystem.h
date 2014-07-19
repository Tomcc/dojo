/*
 *  TouchListener.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/26/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef InputSystem_h__
#define InputSystem_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "Vector.h"
#include "ApplicationListener.h"
#include "Touch.h"

namespace Dojo 
{
	class Renderable;
	class InputDevice;
	
	///InputSystem manages the input at the lower level in Dojo
	/**
	Listening to the InputSystem allows to be updated about 
	-Touches on the touch screen, 
	-Accelerations on the gyroscope, 
	-mouse position and clicks
	-InputDevice connection and disconnection
	*/
	class InputSystem : public ApplicationListener
	{
	public:
		
		typedef Array< Touch* > TouchList;
		typedef Array< InputDevice* > DeviceList;
			
		class Listener
		{
		public:
			
			Listener() :
			source( NULL )
			{
				
			}		
			
			virtual ~Listener()
			{
				if( source )
					source->removeListener( this );
			}
			
			InputSystem* getSource()	{	return source;	}
			
			virtual void onTouchBegan( const Touch& touch ) {}
			virtual void onTouchMove( const Touch& touch )  {}
			virtual void onTouchEnd( const Touch& touch )   {}
			
			virtual void onMouseMove( const Dojo::Vector& curPos, const Dojo::Vector& oldPos )	{}
			virtual void onScrollWheel( float scroll )	{}
			
			virtual void onShake()	{}
			
			virtual void onAcceleration( const Dojo::Vector& accel, float roll )	{}

			virtual void onDeviceConnected( Dojo::InputDevice* j ) {}
			virtual void onDeviceDisconnected( Dojo::InputDevice* j ) {}

			void _notifySource( InputSystem* src )	{	source = src;	}
			
		protected:
			
			InputSystem* source;
		};	
				
		typedef Array<Listener*> ListenerList;
		
		InputSystem( bool enable = true );
		
		virtual ~InputSystem()
		{
			
		}

		///registers a new device to this InputSystem
		/** 
		and sends an event about its connection to the listeners */
		void addDevice( InputDevice* device )
		{
			DEBUG_ASSERT( !mDeviceList.exists( device ), "addDevice: this device was already registered" );

			mDeviceList.add( device );

			_fireDeviceConnected( device );
		}

		///unregisters a new device to this InputSystem
		/** 
		and sends an event about its disconnection to the listeners */
		void removeDevice( InputDevice* device )
		{
			DEBUG_ASSERT( mDeviceList.exists( device ), "removeDevice: this device was not registered" );

			_fireDeviceDisconnected( device );

			mDeviceList.remove( device );			
		}
		
		void addListener( Listener* l )
		{
			DEBUG_ASSERT( l != nullptr, "addListener: null listener passed " );
			
			if( l->getSource() == NULL )
			{
				listeners.add( l );				
				l->_notifySource( this );
			}
		}
		
		void removeListener( Listener* l )
		{
			DEBUG_ASSERT( l != nullptr, "removeListener: null listener passed " );
			
			if( l->getSource() == this )
			{
				listeners.remove( l );
				l->_notifySource(NULL);
			}
		}
        
        ///polls all the registered devices
        void poll( float dt );
        
		///enables or disables the whole input
		void setEnabled( bool e )
		{
			enabled = e;
		}
		
		///returns a list of the touches that existed in the last frame (that either began, or were kept still)
		const TouchList& getTouchList()
		{
			return mTouchList;
		}

		///returns a list of the already connected devices
		const DeviceList& getDeviceList() const
		{
			return mDeviceList;
		}

		virtual void onApplicationFocusGained()
		{
			//buffered states such as key presses etc could have changed while the app was out of focus!
			//TODO
		}

		void _fireTouchBeginEvent(const Vector& point, Touch::Type type);
		void _fireTouchMoveEvent(const Vector& currentPos, const Vector& prevPos, Touch::Type type);
		void _fireTouchEndEvent(const Vector& point, Touch::Type type);
		
		void _fireMouseMoveEvent( const Vector& currentPos, const Vector& prevPos );		
		void _fireScrollWheelEvent( float scroll );		
		
		void _fireShakeEvent();		
		void _fireAccelerationEvent( const Dojo::Vector& accel, float roll );

	protected:
		
		bool enabled;
		
		ListenerList listeners;
		
		TouchList mTouchList;        
		int mAssignedTouches;

		DeviceList mDeviceList;
				
		Touch* _registertouch( const Vector& point, Touch::Type type )
		{
			Touch* t = new Touch( mAssignedTouches++, point, type );
			
			mTouchList.add( t );
			
			return t;
		}
		
		int _getExistingTouchID(const Vector& point, Touch::Type type)
		{
			//find the nearest touch to this position
			float minDist = FLT_MAX;
			int nearest = -1;
			
			for( int i = 0; i < mTouchList.size(); ++i )
			{
				if ( type != type )
					continue;

				float d = mTouchList[i]->point.distance( point );
				
				if( d < minDist )
				{
					nearest = i;
					minDist = d;
				}
			}
			
			return nearest;
		}
		
		Touch* _getExistingTouch(const Vector& point, Touch::Type type)
		{
			return mTouchList[ _getExistingTouchID( point, type ) ];
		}
		
		Touch* _popExistingTouch(const Vector& point, Touch::Type type)
		{
			int idx = _getExistingTouchID( point, type );

			DEBUG_ASSERT( idx >= 0, "Needed to remove an existing touch but it was not found" );

			Touch* t = mTouchList[idx];
			
			mTouchList.remove( idx );
			
			return t;
		}

		void _fireDeviceConnected( Dojo::InputDevice* j );
		void _fireDeviceDisconnected( Dojo::InputDevice* j );

	};
}

#endif
