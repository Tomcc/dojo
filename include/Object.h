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

namespace Dojo {
	
	class GameState;
	class Renderable;
	
	class Object : public BaseObject
	{
	public:
		
		typedef Dojo::Array< Object* > ChildList;
		
		bool absoluteTimeSpeed;
		
		Vector position, speed;		
		float spriteRotation, rotationSpeed;	
				
		Object( GameState* parentLevel, const Vector& pos, const Vector& bbSize  ): 
		position( pos ),
		gameState( parentLevel ),
		speed(0,0),
		active( true ),
		absoluteTimeSpeed( false ),
		spriteRotation( 0 ),
		rotationSpeed( 0 ),
		childs( NULL ),
		parent( NULL )
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
			DEBUG_ASSERT( bbSize.x >= 0 && bbSize.y >= 0 );
			
			size = bbSize;
			halfSize.x = size.x * 0.5f;
			halfSize.y = size.y * 0.5f;
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
		
		inline float getMinX()				{	return position.x - halfSize.x;	}
		inline float getMaxX()				{	return position.x + halfSize.x;	}
		inline float getMinY()				{	return position.y - halfSize.y;	}
		inline float getMaxY()				{	return position.y + halfSize.y;	}
		
		void addChild( Object* o );
		void addChild( Renderable* o, uint layer, bool clickable );

		inline bool contains( const Vector& p )
		{
			return 
				p.x < worldPosition.x + halfSize.x && 
				p.x > worldPosition.x - halfSize.x && 
				p.y < worldPosition.y + halfSize.y && 
				p.y > worldPosition.y - halfSize.y;
		}
		
		inline bool collidesWith( const Vector& BBPos, const Vector& BBSize )
		{			
			DEBUG_ASSERT( BBSize.x > 0 );
			DEBUG_ASSERT( BBSize.y > 0 );
			
			float cx = position.x + halfSize.x - BBPos.x + BBSize.x*0.5f;
			float cy = position.y + halfSize.y - BBPos.y + BBSize.y*0.5f;
			
			return !(cx > size.x + BBSize.x || cx < 0) && !(cy > size.y + BBSize.y || cy < 0);		
		}
		
		inline bool collidesWith( Object * t )
		{			
			DEBUG_ASSERT( t );
			
			float cx = position.x + halfSize.x - t->position.x + t->halfSize.x;
			float cy = position.y + halfSize.y - t->position.y + t->halfSize.y;
			
			return !(cx > size.x + t->size.x || cx < 0) && !(cy > size.y + t->size.y || cy < 0);
		}
		
		virtual void action( float dt );

		void _notifyParent( Object* p )
		{
			DEBUG_ASSERT( p );
			DEBUG_ASSERT( !parent );

			parent = p;
		}
		
	protected:		
		
		GameState* gameState;
		
		Vector size, halfSize;
		
		Vector worldPosition;
		float worldRotation;
				
		bool active;

		Object* parent;
		ChildList* childs;
	};
}

#endif