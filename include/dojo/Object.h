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
		
		bool dispose,
			inheritScale;
		
		Vector position, speed, scale;
		
		///Creates a new Object as a child of the given object at the given relative position, with bbSize size
		/**
		\remark the object still has to be attached!
		*/
		Object( Object* parent, const Vector& pos, const Vector& bbSize  );

		virtual ~Object();
		
		virtual void reset()
		{
			active = true;
			speed.x = speed.y = 0;
						
			updateWorldTransform();
		}
		
		//forces an update of the world transform
		void updateWorldTransform();

		///sets a AABB size
		inline void setSize( const Vector& bbSize )
		{
			DEBUG_ASSERT( bbSize.x >= 0 && bbSize.y >= 0 && bbSize.z >= 0 );

			size = bbSize;
			halfSize = size * 0.5f;

		}
		///sets a 2D AABB size
		inline void setSize( float x, float y )
		{						
			setSize( Vector( x,y) );
		}
		
		///set the orientation quaternion for this object
		inline void setRotation( const Quaternion& quat )
		{
			rotation = quat;
		}

		///rotates the object starting from the current orientation around the given axis
		inline void rotate( float r, const Vector& axis = Vector::UNIT_Z )
		{
			rotation = glm::rotate( rotation, r, axis );
		}
		
		///sets the full orientation using a vector made of euler angles around x,y,z
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
			Matrix inv = glm::inverse( getWorldTransform() ); //TODO make faster for gods' sake
			glm::vec4 p( worldPos.x, worldPos.y, worldPos.z, 1 );
			p = inv * p;

			return Vector( p.x, p.y, p.z );
		}
		
		///returns a local direction in world space
		inline Vector getWorldDirection( const Vector& dir3 = Vector::UNIT_Z )
		{
			glm::vec4 dir( dir3, 0 );
			dir = getWorldTransform() * dir;
			
			return Vector( dir.x, dir.y, dir.z );
		}

		inline Vector getLocalDirection( const Vector& worldDir )
		{
			DEBUG_TODO;

			return Vector::ZERO;
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
			
			return 
			p.x <= worldUpperBound.x && 
			p.x >= worldLowerBound.x && 
			p.y <= worldUpperBound.y && 
			p.y >= worldLowerBound.y &&
			p.z <= worldUpperBound.z && 
			p.z >= worldLowerBound.z;
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