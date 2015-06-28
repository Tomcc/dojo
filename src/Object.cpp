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
	active(true),
	parent(nullptr),
	disposed(false) {
	setSize(bbSize);
}

Object::~Object() {
	removeAllChildren();
}

Object& Object::_addChild(Unique<Object> o, RenderLayer::ID layer) {
	DEBUG_ASSERT(o->parent == nullptr, "The child you want to attach already has a parent");
	DEBUG_ASSERT(!children.contains(o), "Element already in the vector!");

	if (o->getRenderable()) {
		//TODO make the layer a property of renderable, this is dumb
		DEBUG_ASSERT(layer != RenderLayer::InvalidID, "Please specify a valid layer when making a object visible");

		Platform::singleton().getRenderer().addRenderable(*o->getRenderable(), layer);
	}

	o->parent = this;

	auto ptr = o.get();

	o->updateWorldTransform();
	children.emplace(std::move(o));

	return *ptr;
}

void Object::_unregisterChild(Object& child) {
	child.parent = nullptr;

	//TODO it doesn't make much sense to keep this here
	if (auto graphics = child.getRenderable())
		Platform::singleton().getRenderer().removeRenderable(*graphics); //if existing	
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
	for (auto& component : components) {
		if (!component->canDestroy()) {
			return false; //this one can't be destroyed, wait
		}
	}
	return true;
}

void Object::collectChilds() {
	for (auto i : range(children.size())) {
		auto& child = children[i];
		if (child->disposed && child->canDestroy()) {
			_unregisterChild(*child);
			children.erase(child);

			i = 0; //a destructor might dispose of other children...
		}
	}
}

Object::ChildList Object::removeAllChildren() {
	for (auto& c : children) {
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
		for (byte j = 0; j < 3; ++j)
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
	//TODO make faster for gods' sake
	glm::vec4 p(worldPos, 1);
	p = glm::inverse(getWorldTransform()) * p;

	return Vector(p.x, p.y, p.z);
}

Vector Object::getWorldDirection(const Vector& dir3 /*= Vector::UNIT_Z*/) const {
	glm::vec4 dir(dir3, 0);
	dir = getWorldTransform() * dir;

	return Vector(dir.x, dir.y, dir.z);
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

void Object::updateWorldTransform() {
	//compute local matrix from position and orientation
	mWorldTransform = getFullTransformRelativeTo(
		parent ? parent->getWorldTransform() : Matrix(1));
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
		if (auto graphics = c->getRenderable()) {
			graphics->setVisible(visible);
		}
		c->setAllChildrenVisibleHACK(visible);
	}
}

void Object::dispose() {
	DEBUG_ASSERT(!disposed, "Already disposed");

	disposed = true;
	for (auto& c : components) {
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

Renderable* Object::getRenderable() {
	static_assert(Renderable::ID == 0, "ops, this function is a hack :)");
	return (Renderable*)(components.empty() ? nullptr : components[0].get());
}

Component& Dojo::Object::_addComponent(Unique<Component> c, int ID) {
	if (ID >= (int)components.size()) {
		components.resize(ID + 1); //TODO this is shitty very much, need a better O(1) method of storage
	}
	components[ID] = std::move(c);
	return *components[ID];
}
