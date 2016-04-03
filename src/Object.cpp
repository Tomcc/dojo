#include "Object.h"

#include "Game.h"
#include "Renderable.h"
#include "GameState.h"
#include "Renderer.h"
#include "Platform.h"
#include "range.h"

using namespace Dojo;
using namespace glm;

Object::Object(Object& parentObject, const Vector& pos, const Vector& bbSize):
	position(pos),
	active(true),
	disposed(false) {
	if (auto gs = parentObject.gameState.to_ref()) {
		gameState = gs.get();
	}
	setSize(bbSize);
}

Object::~Object() {
	//allow each component to grab its own ownership, eg. for threaded destruction
	for(auto&& c : components) {
		if(c) {
			auto& ref = *c;
			ref.onDestroy(std::move(c));
		}
	}

	removeAllChildren();
}

Object& Object::_addChild(Unique<Object> o) {
	DEBUG_ASSERT(o->parent.is_none(), "The child you want to attach already has a parent");
	DEBUG_ASSERT(!children.contains(o), "Element already in the vector!");

	o->parent = self;

	auto ptr = o.get();

	o->updateWorldTransform();
	children.emplace(std::move(o));

	//call onAttach on all of the children components
	for (auto&& c : ptr->components) {
		if (c) {
			c->onAttach();
		}
	}

	return *ptr;
}

void Object::_unregisterChild(Object& child) {
	//call onAttach on all of the children components
	for (auto&& c : child.components) {
		if (c) {
			c->onDetach();
		}
	}

	child.parent = {};
}

Unique<Object> Object::removeChild(Object& o) {
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );

	auto elem = ChildList::find(children, o);

	DEBUG_ASSERT(elem != children.end(), "This object is not a child");

	auto child = std::move(*elem);
	_unregisterChild(*child);
	children.erase(elem);
	return child;
}

bool Object::canDestroy() const {
	for (auto&& component : components) {
		if (component && !component->canDestroy()) {
			return false; //this one can't be destroyed, wait
		}
	}

	return true;
}

void Object::collectChilds() {
	for (size_t i = 0; i < children.size(); ++i) {
		auto& child = children[i];

		if (child->disposed && child->canDestroy()) {
			_unregisterChild(*child);
			children.erase(child);

			i = 0; //a destructor might dispose of other children...
		}
	}
}

Object::ChildList Object::removeAllChildren() {
	for (auto&& c : children) {
		_unregisterChild(*c);
	}

	return std::move(children);
}

AABB Object::transformAABB(const AABB& local) const {
	AABB bb;

	//get the eight world-position corners and transform them
	//TODO this is massively slow, pls optimize
	bb.max = Vector::Min;
	bb.min = Vector::Max;

	Vector vertex;

	for (byte i = 0; i < 8; ++i) {
		for (byte j = 0; j < 3; ++j) {
			vertex[j] = Math::getBit(i, j) ? local.max[j] : local.min[j];
		}

		vertex = getWorldPosition(vertex);

		bb.max = Vector::max(bb.max, vertex);
		bb.min = Vector::min(bb.min, vertex);
	}

	return bb;
}

Vector Object::getWorldPosition(const Vector& localPos) const {
	if (parent.is_some()) {
		glm::vec4 pos = getWorldTransform() * glm::vec4(localPos.x, localPos.y, localPos.z, 1.0f);
		return (Vector&)pos;
	}
	else {
		return localPos;
	}
}

bool Object::isActive() const {
	return active && !disposed && (parent.is_none() || parent.unwrap().isActive());
}

void Object::reset() {
	active = true;
	speed.x = speed.y = 0;

	updateWorldTransform();
}

Vector Object::getLocalPosition(const Vector& worldPos) const {
	//TODO make faster for gods' sake
	glm::vec4 p(worldPos, 1);
	p = glm::inverse(getWorldTransform()) * p;

	return (Vector&)p;
}

Vector Object::getWorldDirection(const Vector& dir3 /*= Vector::UNIT_Z*/) const {
	glm::vec4 dir(dir3, 0);
	dir = getWorldTransform() * dir;

	return (Vector&)dir;
}

Vector Object::getLocalDirection(const Vector& worldDir) {
	DEBUG_TODO;

	return Vector::Zero;
}

Radians Object::getRoll() const {
	return Radians(glm::roll(rotation));
}

Matrix Object::getFullTransformRelativeTo(const Matrix& parent) const {
	return glm::translate(parent, position) * mat4_cast(rotation);
}

Matrix Object::getParentWorldTransform() const {
	//compute local matrix from position and orientation
	if (auto p = parent.to_ref()) {
		return p.get().getWorldTransform();
	}
	return Matrix{ 1 };
}

void Object::updateWorldTransform() {
	mWorldTransform = getFullTransformRelativeTo(getParentWorldTransform());
}

void Object::updateChilds(float dt) {
	if (children.size() > 0) {

		//WARNING: do not use a ranged for loop in this one!
		//a child might remove any other child from the array
		//so we need to always check with the updated size
		for (size_t i = 0; i < children.size(); ++i) {
			if (children[i]->isActive()) {
				children[i]->onAction(dt);
			}
		}

		collectChilds();
	}
}

void Object::onAction(float dt) {
	position += speed * dt;

	updateWorldTransform();

	updateChilds(dt);
}

void Object::setAllChildrenVisibleHACK(bool visible) {
	for (auto&& c : children) {
		if (c->has<Renderable>()) {
			c->get<Renderable>().setVisible(visible);
		}

		c->setAllChildrenVisibleHACK(visible);
	}
}

void Object::dispose() {
	DEBUG_ASSERT(!disposed, "Already disposed");

	disposed = true;

	onDispose();
	for (auto&& c : components) {
		if (c) {
			c->onDispose();
		}
	}
}

void Object::setSize(const Vector& bbSize) {
	DEBUG_ASSERT(bbSize.x >= 0 && bbSize.y >= 0 && bbSize.z >= 0, "Malformed size (one component was less or equal to 0)");

	size = bbSize;
	halfSize = size * 0.5f;
}

Component& Object::_addComponent(Unique<Component> c, int ID) {
	DEBUG_ASSERT(parent.is_none(), "The object has been already added to the scene");

	if (ID >= (int)components.size()) {
		components.resize(ID + 1); //TODO this is shitty very much, need a better O(1) method of storage
	}

	components[ID] = std::move(c);
	return *components[ID];
}
