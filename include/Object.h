/*
 *  Object.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Object_h__
#define Object_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "BaseObject.h"
#include "Array.h"
#include "dojomath.h"

namespace Dojo {
	
	class GameState;
	class Renderable;
	
	class Object : public BaseObject
	{
	public:
		
		typedef Dojo::Array< Object* > ChildList;
		
		bool absoluteTimeSpeed;

		bool dispose;
		
		Vector position, speed;		
		float spriteRotation, rotationSpeed;

		Vector axis;
				
		Object( GameState* parentLevel, const Vector& pos, const Vector& bbSize  ): 
		position( pos ),
		gameState( parentLevel ),
		speed(0,0,0),
		active( true ),
		absoluteTimeSpeed( false ),
		spriteRotation( 0 ),
		rotationSpeed( 0 ),
		childs( NULL ),
		parent( NULL ),
		dispose( false ),
		axis( Vector::UNIT_Z )
		{
			DEBUG_ASSERT( parentLevel );
			
			setSize( bbSize );
		}
		
		virtual ~Object()
		{
			
		}
		
		virtual void reset()
		{
			active = true;
			speed.x = speed.y = 0;
			
			spriteRotation = 0;			
			rotationSpeed = 0;
		}
		
		inline void setSize( float x, float y )
		{						
			DEBUG_ASSERT( x >= 0 && y >= 0 );
						
			size.x = x;
			size.y = y;
			halfSize.x = size.x * 0.5f;
			halfSize.y = size.y * 0.5f;
		}
		
		inline void setSize( const Vector& bbSize )
		{
			setSize( bbSize.x, bbSize.y );
		}
				
		inline void setActive( bool a )		{	active = a;	}
		
		inline void scale( float f )		
		{
			setSize( size.x * f, size.y * f );
		}
		
		inline const Vector& getSize()		{	return size;	}			
		inline const Vector& getHalfSize()	{	return halfSize;}	
		
		inline GameState* getGameState()	{	return gameState;	}

		inline const Vector& getWorldPosition()
		{
			return worldPosition;
		}

		inline float getWorldRotation()
		{
			return worldRotation;
		}
						
		inline bool isActive()				{	return active;	}
		
		//TODO use real transforms
		inline const Vector& getWorldMax()				{	return max;	}
		inline const Vector& getWorldMin()				{	return min;	}

		void addChild( Object* o );
		void addChild( Renderable* o, uint layer, bool clickable );

		void removeChild( Object* o );
		
		inline bool collidesWith( const Vector& MAX, const Vector& MIN )
		{			
			return Math::AABBsCollide( getWorldMax(), getWorldMin(), MAX, MIN );
		}
		
		inline bool collidesWith( Object * t )
		{			
			DEBUG_ASSERT( t );

			return collidesWith( t->getWorldMax(), t->getWorldMin() );
		}
		
		virtual void action( float dt );

		virtual void onDestruction()
		{

		}

		void _notifyParent( Object* p )
		{
			DEBUG_ASSERT( p != parent );

			parent = p;
		}
		
	protected:		
		
		GameState* gameState;
		
		Vector size, halfSize, max, min;
		
		Vector worldPosition;
		float worldRotation;
				
		bool active;

		Object* parent;
		ChildList* childs;
	};
}

#endif