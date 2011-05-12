/*
 *  TouchListener.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/26/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef TouchListener_h__
#define TouchListener_h__

#include "dojo_common_header.h"

#include "Array.h"

#include "Vector.h"

namespace Dojo 
{
	class Renderable;
	
	class TouchSource
	{
	public:
				
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
					source->removeTouchListener( this );
			}
			
			inline TouchSource* getSource()	{	return source;	}
			
			virtual void onTouchBegan( const Vector& point )=0;
			virtual void onTouchMove( const Vector& point, const Vector& trans )=0;
			virtual void onTouchEnd( const Vector& point )=0;	
			
			virtual void onShake()
			{
				
			}
			
			virtual void onAcceleration( float x, float y, float z, float roll )
			{
				
			}

			virtual void onKeyPressed( uint character, uint keyID )
			{

			}	

			virtual void onKeyReleased( uint character, uint keyID )
			{

			}

			inline void _notifySource( TouchSource* src )	{	source = src;	}
			
		protected:
			
			TouchSource* source;
		};	
				
		typedef Array<Listener*> ListenerList;
		
		TouchSource( bool enable = true ) :
		enabled(enable)
		{
			
		}
		
		inline void addTouchListener( Listener* l )
		{
			DEBUG_ASSERT( l );
			
			if( l->getSource() == NULL )
			{
				listeners.add( l );				
				l->_notifySource( this );
			}
		}
		
		inline void removeTouchListener( Listener* l )
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
		
		virtual void _fireTouchBeginEvent( const Vector& point )
		{
			if( enabled )
			{
				lastMovePos = point;
				
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onTouchBegan( point );
			}
		}
		
		virtual void _fireTouchMoveEvent( const Vector& point )
		{
			if( enabled )
			{
				lastMovePos -= point;
				
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onTouchMove( point, lastMovePos );
				
				lastMovePos = point;
			}
		}
		
		virtual void _fireTouchEndEvent( const Vector& point )
		{
			if( enabled )
			{
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onTouchEnd( point );
			}
		}	
		
		virtual void _fireShakeEvent()
		{
			if( enabled )
			{				
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onShake();
			}
		}
		
		virtual void _fireAccelerationEvent( float x, float y, float z, float roll )
		{
			if( enabled )
			{				
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onAcceleration( x,y,z,roll );
			}
		}

		virtual void _fireKeyPressedEvent(Dojo::uint character, Dojo::uint keyID )
		{
			if( enabled )
			{				
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onKeyPressed( character, keyID );
			}
		}

		virtual void _fireKeyReleasedEvent(Dojo::uint character, Dojo::uint keyID )
		{
			if( enabled )
			{				
				for( uint i = 0; i < listeners.size(); ++i )
					listeners.at(i)->onKeyReleased( character, keyID );
			}
		}
	
	protected:
		
		bool enabled;
		
		ListenerList listeners;		
		
		Vector lastMovePos;
	};
}

#endif