#include "Object.h"

#include "Game.h"
#include "Renderable.h"
#include "GameState.h"
#include "Renderer.h"
#include "Platform.h"

using namespace Dojo;
using namespace glm;

Object::Object(Object& parentObject, const Vector& pos, const Vector& bbSize):
	position(pos),
	gameState(&parentObject.getGameState()),
	speed(0, 0, 0),
	active(true),
	scale(1, 1, 1),
	parent(nullptr),
	disposed(false),
	inheritScale(true) {
	setSize(bbSize);
}

Object::~Object() {
	destroyAllChildren();
}

Object& Object::_addChild(Unique<Object> o) {
	DEBUG_ASSERT(o->parent == nullptr, "The child you want to attach already has a parent");
	DEBUG_ASSERT(!children.contains(o), "Element already in the vector!");

	o->parent = this;

	auto ptr = o.get();

	children.emplace(std::move(o));

	return *ptr;
}

Renderable& Object::_addChild(Unique<Renderable> o, int layer) {
	Platform::singleton().getRenderer().addRenderable(*o, layer);

	return addChild(std::move(o));
}

void Object::_unregisterChild(Object& child) {
	child.parent = nullptr;

	if (child.isRenderable())
		Platform::singleton().getRenderer().removeRenderable((Renderable&)child); //if existing	
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

void Object::collectChilds() {
	for (size_t i = 0; i < children.size(); ++i) {
		if (children[i]->disposed) {
			_unregisterChild(*children[i]);
			children.erase(children[i]);

			i = 0; //a destructor might dispose of other children...
		}
	}
}


void Object::destroyAllChildren() {
	for (auto& c : children)
		_unregisterChild(*c);

	children.clear();
}

AABB Object::transformAABB(const AABB& local) const {
	AABB bb;

	//get the eight world-position corners and transform them
	bb.max = Vector::MIN;
	bb.min = Vector::MAX;

	Vector vertex;

	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 3; ++j)
			vertex[j] = Math::getBit(i, j) ? local.max[j] : local.min[j];

		vertex = getWorldPosition(vertex);

		bb.max = Vector::max(bb.max, vertex);
		bb.min = Vector::min(bb.min, vertex);
	}

	return bb;
}

Vector Object::getWorldPosition(const Vector& localPos) const {
	if (parent) {
		glm::vec4 pos = getWorldTransform() * glm::vec4(localPos.x, localPos.y, localPos.z, 1.0f);
		return Vector(pos.x, pos.y, pos.z);
	}
	else
		return localPos;
}

bool Object::isActive() const {
	return active && (!parent || parent->isActive());
}

void Object::reset() {
	active = true;
	speed.x = speed.y = 0;

	updateWorldTransform();
}

Vector Object::getLocalPosition(const Vector& worldPos) const {
	Matrix inv = glm::inverse(getWorldTransform()); //TODO make faster for gods' sake
	glm::vec4 p(worldPos.x, worldPos.y, worldPos.z, 1);
	p = inv * p;

	return Vector(p.x, p.y, p.z);
}

Vector Object::getWorldDirection(const Vector& dir3 /*= Vector::UNIT_Z */) {
	glm::vec4 dir(dir3, 0);
	dir = getWorldTransform() * dir;

	return Vector(dir.x, dir.y, dir.z);
}

Vector Object::getLocalDirection(const Vector& worldDir) {
	DEBUG_TODO;

	return Vector::ZERO;
}

Radians Object::getRoll() const {
	return (Radians)Degrees(glm::roll(rotation));
}

Matrix Object::getFullTransformRelativeTo(const Matrix& parent) const {
	Matrix m = glm::translate(parent, position) * mat4_cast(rotation);
	return (scale == Vector::ONE) ? m : glm::scale(m, scale);
}

void Object::updateWorldTransform() {
	//compute local matrix from position and orientation
	if (!parent)
		mWorldTransform = Matrix(1);
	else {
		mWorldTransform = parent->getWorldTransform();
		if (!inheritScale)
			mWorldTransform = glm::scale(mWorldTransform, 1.f / parent->scale);
	}

	mWorldTransform = getFullTransformRelativeTo(mWorldTransform);
}

void Object::updateChilds(float dt) {
	if (children.size() > 0) {

		//WARNING: do not use a ranged for loop in this one!
		//a child might remove any other child from the array 
		//so we need to always check with the updated size
		for (size_t i = 0; i < children.size(); ++i) {
			if (children[i]->isActive())
				children[i]->onAction(dt);
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
		if (c->isRenderable()) {
			((Renderable&)*c).setVisible(visible);
		}
		c->setAllChildrenVisibleHACK(visible);
	}
}

void Object::dispose() {
	disposed = true;
}
