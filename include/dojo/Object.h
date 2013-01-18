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
	
	///Object is the base class of any object that can be placed and moved in a GameState
	/** 
	it has a position, a speed, a rotation and a scale that are used to determine its world transform.
	When other Objects are attached to a single root Object as children, they share their parent's
	world transform and move in its local space

	Objects automatically listen to the "action" event, that is called each frame.
	
	Objects are automatically collected when the dispose flag is set to true on them, or on one of its parents.
	*/
	class Object 
	{
	public:
		
		typedef Dojo::Array< Object* > ChildList;
		
		bool dispose;
		
		Vector position, speed, scale;
		
		///Creates a new Object in the given GameState at the given position, with bbSize size
		Object( GameState* parentLevel, const Vector& pos, const Vector& bbSize  );
		
		virtual ~Object()
		{
			destroyAllChilds();
		}
		
		virtual void reset()
		{
			active = true;
			speed.x = speed.y = 0;
						
			updateWorldTransform();
		}
		
		//forces an update of the world transform
		void updateWorldTransform();
		
		///sets a 2D AABB size
		inline void setSize( float x, float y )
		{						
			DEBUG_ASSERT( x >= 0 && y >= 0 );
						
			size.x = x;
			size.y = y;
			halfSize.x = size.x * 0.5f;
			halfSize.y = size.y * 0.5f;
		}
		
		///sets a 2D AABB size
		inline void setSize( const Vector& bbSize )
		{
			setSize( bbSize.x, bbSize.y );
		}
		
		inline void setRotation( const Quaternion& quat )
		{
			rotation = quat;
		}
		
		inline void setRotation( const Vector& eulerAngles )
		{
			setRotation( Quaternion( eulerAngles ) );
		}
				
		inline void setActive( bool a )		{	active = a;	}
				
		inline const Vector& getSize()		{	return size;	}			
		inline const Vector& getHalfSize()	{	return halfSize;}	
		
		inline GameState* getGameState()	{	return gameState;	}

		///returns the world position of a point in local space
		inline Vector getWorldPosition( const Vector& localPos )
		{
			if( parent )
			{
				glm::vec4 pos = getWorldTransform() * glm::vec4(localPos.x, localPos.y, localPos.z, 1.0f );
				return Vector( pos.x , pos.y, pos.z );                
			}
			else
				return localPos;
		}
				
		///returns the world coordinates' position of this Object
		inline const Vector getWorldPosition()
		{
			return getWorldPosition( position );
		}

		///returns the position in local coordinates of the given world position
		inline Vector getLocalPosition( const Vector& worldPos )
		{
			DEBUG_TODO;            

			return worldPos;
		}
		
		///returns a local direction in world space
		inline Vector getWorldDirection( const Vector& dir3 = Vector::UNIT_Z )
		{
			glm::vec4 dir( dir3, 0 );
			dir = getWorldTransform() * dir;
			
			return Vector( dir.x, dir.y, dir.z );
		}
		
		inline const Quaternion getRotation()
		{
			return rotation;
		}
						
		inline bool isActive()				{	return active;	}
		
		inline bool hasChilds()
		{
			return childs != NULL && childs->size() > 0;
		}

		inline const Matrix& getWorldTransform()    
		{  
			return mWorldTransform; 
		}

		inline Object* getChild( int i )				
		{
			DEBUG_ASSERT( hasChilds() );
			DEBUG_ASSERT( childs->size() > i );
			
			return childs->at( i );
		}
		
		inline Vector& getWorldMax()
		{
			DEBUG_ASSERT( mNeedsAABB );
			
			return worldUpperBound;
		}
		
		inline Vector& getWorldMin()
		{
			DEBUG_ASSERT( mNeedsAABB );
			
			return worldLowerBound;
		}
		
		inline Object* getParent()
		{
			return parent;
		}
		
		inline int getChildNumber()
		{
			return (childs) ? childs->size() : 0;
		}
		
		///adds a non-renderable child
		void addChild( Object* o );
		///adds a renderable child, and attaches it to the given Render layer
		void addChild( Renderable* o, int layer );

		///removes a child if existing
		void removeChild( int idx );
		///removes a child if existing
		void removeChild( Object* o );
		
		///destroys all the children that have been marked by dispose
		void collectChilds();
		
		void destroyChild( int idx );
		void destroyChild( Object* o );
		
		///completely destroys all the children of this object
		void destroyAllChilds();
		
		void updateChilds( float dt );
		
		inline bool contains( const Vector& p )
		{
			DEBUG_ASSERT( mNeedsAABB );
			
			///HACK only works in 2D!
			return 
			p.x < worldUpperBound.x && 
			p.x > worldLowerBound.x && 
			p.y < worldUpperBound.y && 
			p.y > worldLowerBound.y;
		}
		
		inline bool collidesWith( const Vector& MAX, const Vector& MIN )
		{		
			DEBUG_ASSERT( mNeedsAABB );
			
			return Math::AABBsCollide( getWorldMax(), getWorldMin(), MAX, MIN );
		}
		
		inline bool collidesWith( Object * t )
		{			
			DEBUG_ASSERT( t );
			DEBUG_ASSERT( mNeedsAABB );            

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
		
		Quaternion rotation;
		
		Matrix mWorldTransform;
				
		Vector worldUpperBound, worldLowerBound;
				
		bool active, mNeedsAABB;

		Object* parent;
		ChildList* childs;
		
		void _updateWorldAABB( const Vector& min, const Vector& max );
		
		void _unregisterChild( Object* child );
	};
}

#endif