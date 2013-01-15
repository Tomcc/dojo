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

namespace Dojo 
{
	class Renderable;
	class InputDevice;
	
	///keycodes, thanks to OIS Library
	class InputSystem : public ApplicationListener
	{
	public:
		
		class Touch
		{
		public:
			
			int ID;
			Vector point, speed;
			
			Touch( int _ID, const Vector& _point ) :
			ID( _ID ),
			point( _point ),
			speed( Vector::ZERO )
			{
				
			}
		};
		
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
			
			inline InputSystem* getSource()	{	return source;	}
			
			virtual void onTouchBegan( const Touch& touch ) {}
			virtual void onTouchMove( const Touch& touch )  {}
			virtual void onTouchEnd( const Touch& touch )   {}
			
			virtual void onMouseMove( const Dojo::Vector& curPos, const Dojo::Vector& oldPos )	{}
			virtual void onScrollWheel( float scroll )	{}
			
			virtual void onShake()	{}
			
			virtual void onAcceleration( const Dojo::Vector& accel, float roll )	{}

			virtual void onDeviceConnected( Dojo::InputDevice* j ) {};
			virtual void onDeviceDisconnected( Dojo::InputDevice* j ) {};

			inline void _notifySource( InputSystem* src )	{	source = src;	}
			
		protected:
			
			InputSystem* source;
		};	
				
		typedef Array<Listener*> ListenerList;
		
		InputSystem( bool enable = true );
		
		virtual ~InputSystem()
		{
			
		}

		inline void addDevice( InputDevice* device )
		{
			DEBUG_ASSERT( !mDeviceList.exists( device ) );

			mDeviceList.add( device );
		}

		inline void removeDevice( InputDevice* device )
		{
			DEBUG_ASSERT( mDeviceList.exists( device ) );

			mDeviceList.remove( device );
		}
		
		inline void addListener( Listener* l )
		{
			DEBUG_ASSERT( l );
			
			if( l->getSource() == NULL )
			{
				listeners.add( l );				
				l->_notifySource( this );
			}
		}
		
		inline void removeListener( Listener* l )
		{
			DEBUG_ASSERT( l );
			
			if( l->getSource() == this )
			{
				listeners.remove( l );
				l->_notifySource(NULL);
			}
		}
		
		inline void setEnabled( bool e )
		{
			enabled = e;
		}
		
		inline const TouchList& getTouchList()
		{
			return mTouchList;
		}

		inline const DeviceList& getDeviceList() const
		{
			return mDeviceList;
		}

		virtual void onApplicationFocusLost()
		{
			//flush buffered state
			//TODO
		}

		virtual void onApplicationFocusGained()
		{
			//fill in buffered state
			//TODO
		}

		void _fireTouchBeginEvent( const Vector& point );		
		void _fireTouchMoveEvent( const Vector& currentPos, const Vector& prevPos );		
		void _fireTouchEndEvent( const Vector& point );		
		
		void _fireMouseMoveEvent( const Vector& currentPos, const Vector& prevPos );		
		void _fireScrollWheelEvent( float scroll );		
		
		void _fireShakeEvent();		
		void _fireAccelerationEvent( const Dojo::Vector& accel, float roll );

		void _fireDeviceConnected( Dojo::InputDevice* j );
		void _removeDevice( Dojo::InputDevice* j );

	protected:
		
		bool enabled;
		
		ListenerList listeners;
		
		TouchList mTouchList;        
		int mAssignedTouches;

		DeviceList mDeviceList;
				
		inline Touch* _registertouch( const Vector& point )
		{
			Touch* t = new Touch( mAssignedTouches++, point );
			
			mTouchList.add( t );
			
			return t;
		}
		
		inline int _getExistingTouchID( const Vector& point )
		{
			//find the nearest touch to this position
			float minDist = FLT_MAX;
			int nearest = -1;
			
			for( int i = 0; i < mTouchList.size(); ++i )
			{
				float d = mTouchList[i]->point.distance( point );
				
				if( d < minDist )
				{
					nearest = i;
					minDist = d;
				}
			}
			
			return nearest;
		}
		
		inline Touch* _getExistingTouch( const Vector& point )
		{
			return mTouchList[ _getExistingTouchID( point ) ];
		}
		
		inline Touch* _popExistingTouch( const Vector& point )
		{
			int idx = _getExistingTouchID( point );

			DEBUG_ASSERT( idx >= 0 );

			Touch* t = mTouchList[idx];
			
			mTouchList.remove( idx );
			
			return t;
		}

	};
}

#endif
