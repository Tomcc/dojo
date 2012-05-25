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
#include "Array.h"

#include "dojomath.h"

namespace Dojo {
	
	class GameState;
	class Renderable;
	
	class Object 
	{
	public:
		
		typedef Dojo::Array< Object* > ChildList;
		
		bool dispose;
		
		Vector position, speed, angle, rotationSpeed, scale;
		
		Object( GameState* parentLevel, const Vector& pos, const Vector& bbSize  );
		
		virtual ~Object()
		{
            destroyAllChilds();
		}
		
		virtual void reset()
		{
			active = true;
			speed.x = speed.y = 0;
			
			angle = Vector::ZERO;
			rotationSpeed = Vector::ZERO;
			
			updateWorldPose();
		}
		
		void updateWorldPose();
		
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
				
		inline const Vector& getSize()		{	return size;	}			
		inline const Vector& getHalfSize()	{	return halfSize;}	
		
		inline GameState* getGameState()	{	return gameState;	}

		inline const Vector& getWorldPosition()
		{
			return worldPosition;
		}

		///returns the world position of the given local point
		inline Vector getWorldPosition( const Vector& localPos )
		{
            DEBUG_TODO;

			return localPos;
		}

		inline Vector getLocalPosition( const Vector& worldPos )
		{
            DEBUG_TODO;            

			return worldPos;
		}

		inline const Vector& getWorldRotation()
		{
			return worldRotation;
		}

		inline Vector getWorldDirection()
		{
			DEBUG_TODO;
		}
						
		inline bool isActive()				{	return active;	}
		
		inline bool hasChilds()
		{
			return childs != NULL && childs->size() > 0;
		}
				
		//TODO use real transforms
		inline const Vector& getWorldMax()	{	return worldUpperBound;	}
		inline const Vector& getWorldMin()	{	return worldLowerBound;	}

		inline Object* getChild( int i )				
		{
			DEBUG_ASSERT( hasChilds() );
			DEBUG_ASSERT( childs->size() > i );
			
			return childs->at( i );
		}
        
        inline Object* getParent()
        {
            return parent;
        }
		
		inline int getChildNumber()
		{
			return (childs) ? childs->size() : 0;
		}
		
		void addChild( Object* o );
		void addChild( Renderable* o, int layer );

		void removeChild( int idx );
		void removeChild( Object* o );
		
		void collectChilds();
		
		void destroyChild( int idx );
		void destroyChild( Object* o );
		
		///completely destroys all the childs of this object
		void destroyAllChilds();
		
		void updateChilds( float dt );
        
        inline bool contains( const Vector& p )
		{
			return 
            p.x < worldUpperBound.x && 
            p.x > worldLowerBound.x && 
            p.y < worldUpperBound.y && 
            p.y > worldLowerBound.y;
		}
		
		inline bool collidesWith( const Vector& MAX, const Vector& MIN )
		{			
			return Math::AABBsCollide( getWorldMax(), getWorldMin(), MAX, MIN );
		}
		
		inline bool collidesWith( Object * t )
		{			
			DEBUG_ASSERT( t );

			return collidesWith( t->getWorldMax(), t->getWorldMin() );
		}
		
		virtual void onAction( float dt );

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
		
		Vector size, halfSize;
		
		Vector worldPosition;
		Vector worldRotation;
                
		Vector worldUpperBound, worldLowerBound;
				
		bool active;

		Object* parent;
		ChildList* childs;
	};
}

#endif