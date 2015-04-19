/*
 *  Object.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Vector.h"
#include "SmallSet.h"

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
	class Object {
	public:

		typedef SmallSet<Unique<Object>> ChildList;

		bool inheritScale;

		Vector position, speed, scale;

		///Creates a new Object as a child of the given object at the given relative position, with bbSize size
		/**
		\remark the object still has to be attached!
		*/
		Object(Object* parent, const Vector& pos, const Vector& bbSize = Vector::ONE);

		virtual ~Object();

		virtual void reset();

		//forces an update of the world transform
		void updateWorldTransform();

		///sets a AABB size
		void setSize(const Vector& bbSize) {
			DEBUG_ASSERT( bbSize.x >= 0 && bbSize.y >= 0 && bbSize.z >= 0, "Malformed size (one component was less or equal to 0)" );

			size = bbSize;
			halfSize = size * 0.5f;

		}

		///sets a 2D AABB size
		void setSize(float x, float y) {
			setSize(Vector(x, y));
		}

		///set the orientation quaternion for this object
		void setRotation(const Quaternion& quat) {
			rotation = quat;
		}

		///sets the full orientation using a vector made of radians around x,y,z
		void setRotation(Radians yaw, Radians pitch, Radians roll) {
			setRotation(Quaternion(Vector(yaw, pitch, roll)));
		}

		///sets the rotation around the Z axis (2D rotation) for this object
		void setRoll(Radians r) {
			setRotation(Radians(0.f), Radians(0.f), r);
		}

		///rotates the object starting from the current orientation around the given axis
		void rotate(Degrees r, const Vector& axis = Vector::UNIT_Z) {
			rotation = glm::rotate(rotation, r, axis);
		}

		void setActive(bool a) {
			active = a;
		}

		///tells if this object is a renderable
		virtual bool isRenderable() {
			return false;
		}

		const Vector& getSize() const {
			return size;
		}

		const Vector& getHalfSize() const {
			return halfSize;
		}

		GameState* getGameState() const {
			return gameState;
		}

		///returns the world position of a point in local space
		Vector getWorldPosition(const Vector& localPos) const;

		///returns the world coordinates' position of this Object
		Vector getWorldPosition() const {
			return getWorldPosition(Vector::ZERO);
		}

		///returns the position in local coordinates of the given world position
		Vector getLocalPosition(const Vector& worldPos) const;

		///returns a local direction in world space
		Vector getWorldDirection(const Vector& dir3 = Vector::UNIT_Z);

		Vector getLocalDirection(const Vector& worldDir);

		const Quaternion& getRotation() const {
			return rotation;
		}

		///returns the euclidean "roll" angle, or rotation around Z
		Radians getRoll() const;

		bool isActive() const;

		bool hasChilds() const {
			return childs.size() > 0;
		}

		const Matrix& getWorldTransform() const {
			return mWorldTransform;
		}

		// 		const Vector& getWorldMax() const
		// 		{
		// 			return worldUpperBound;
		// 		}
		// 		
		// 		const Vector& getWorldMin() const
		// 		{
		// 			return worldLowerBound;
		// 		}

		Object* getParent() {
			return parent;
		}

		size_t getChildCount() const {
			return childs.size();
		}

		Matrix getFullTransformRelativeTo(const Matrix& parent) const;

		///adds a non-renderable child
		template <class T>
		T& addChild(Unique<T> o) {
			return (T&)_addChild(std::move(o));
		}

		///adds a renderable child, and attaches it to the given Render layer
		template <class T>
		T& addChild(Unique<T> o, int layer) {
			return (T&)_addChild(std::move(o), layer);
		}

		///removes a child if existing and gives it back to the caller
		Unique<Object> removeChild(Object& o);

		///destroys all the children that have been marked by dispose
		void collectChilds();

		///completely destroys all the children of this object
		void destroyAllChildren();

		void updateChilds(float dt);

		virtual void onAction(float dt);

		virtual void dispose() {
			disposed = true;
		}

		struct AABB {
			Vector min, max;

			AABB grow(float s) const {
				return{
					min - s,
					max + s
				};
			}
		};

		AABB transformAABB(const Vector& min, const Vector& max) const;

	protected:

		GameState* gameState;

		Vector size, halfSize;

		Quaternion rotation;

		Matrix mWorldTransform;

		bool active;
		bool disposed;

		Object* parent;
		ChildList childs;


		Object& _addChild(Unique<Object> o);
		Renderable& _addChild(Unique<Renderable> o, int layer);

		void _unregisterChild(Object& child);

	};
}
