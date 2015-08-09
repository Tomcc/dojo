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
#include "AABB.h"
#include "RenderLayer.h"

namespace Dojo {

	class GameState;
	class Renderable;
	class Component;

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

		Vector position, speed;

		///Creates a new Object as a child of the given object at the given relative position, with bbSize size
		/**
		\remark the object still has to be attached!
		*/
		Object(Object& parent, const Vector& pos, const Vector& bbSize = Vector::One);

		virtual ~Object();

		virtual void reset();

		//forces an update of the world transform
		void updateWorldTransform();

		///sets a AABB size
		void setSize(const Vector& bbSize);

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
			setRotation(0.0_rad, 0.0_rad, r);
		}

		///rotates the object starting from the current orientation around the given axis
		void rotate(Degrees r, const Vector& axis = Vector::UnitZ) {
			rotation = glm::rotate(rotation, r, axis);
		}

		void setActive(bool a) {
			active = a;
		}

		const Vector& getSize() const {
			return size;
		}

		const Vector& getHalfSize() const {
			return halfSize;
		}

		GameState& getGameState() const {
			return *gameState;
		}

		///returns the world position of a point in local space
		Vector getWorldPosition(const Vector& localPos) const;

		///returns the world coordinates' position of this Object
		Vector getWorldPosition() const {
			return getWorldPosition(Vector::Zero);
		}

		///returns the position in local coordinates of the given world position
		Vector getLocalPosition(const Vector& worldPos) const;

		///returns a local direction in world space
		Vector getWorldDirection(const Vector& dir3 = Vector::UnitZ) const;

		Vector getLocalDirection(const Vector& worldDir);

		const Quaternion& getRotation() const {
			return rotation;
		}

		///returns the euclidean "roll" angle, or rotation around Z
		Radians getRoll() const;

		bool isActive() const;

		bool hasChilds() const {
			return children.size() > 0;
		}

		const Matrix& getWorldTransform() const {
			return mWorldTransform;
		}

		Object* getParent() {
			return parent;
		}

		size_t getChildCount() const {
			return children.size();
		}

		Matrix getFullTransformRelativeTo(const Matrix& parent) const;

		///adds a child
		template <class T>
		T& addChild(Unique<T> o) {
			return (T&)_addChild(std::move(o));
		}

		///removes a child if existing and gives it back to the caller
		Unique<Object> removeChild(Object& o);

		template<class T>
		bool has() const {
			return T::ID < components.size() && components[T::ID];
		}

		template<class T>
		T& get() const {
			//this returns a reference and asserts for presence rather than
			//going the full check route because we enforce that it's the
			//calling code that needs to call get() only when a component can be found
			DEBUG_ASSERT(has<T>(), "Component not found");
			return (T&) * components[T::ID];
		}

		///destroys all the children that have been marked by dispose
		void collectChilds();

		///removes all the children of this object, returns ownership to the caller
		ChildList removeAllChildren();

		void updateChilds(float dt);

		Component& _addComponent(Unique<Component> c, int ID);

		template<class T>
		T& addComponent(Unique<T> c) {
			return (T&)_addComponent(std::move(c), T::ID);
		}

		virtual void onAction(float dt);

		///sets all children visible or invisible. //HACK this needs to be removed in favor of an actual scene graph traversal
		void setAllChildrenVisibleHACK(bool visible);

		virtual void dispose();

		///"authorization" method called when disposing. Useful to override when waiting for threaded stuff to finish on destruction
		virtual bool canDestroy() const;

		AABB transformAABB(const AABB& local) const;
	protected:

		GameState* gameState;

		std::vector<Unique<Component>> components; //TODO not a vector pls

		Vector size, halfSize;

		Quaternion rotation;

		Matrix mWorldTransform;

		bool active;
		
		Object* parent;
		ChildList children;

		Object& _addChild(Unique<Object> o);

		void _unregisterChild(Object& child);

	private:
		bool disposed;
	};
}
