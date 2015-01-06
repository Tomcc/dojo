#include "stdafx.h"

#include "Object.h"

#include "Game.h"
#include "Renderable.h"
#include "GameState.h"
#include "Platform.h"

using namespace Dojo;
using namespace glm;

Object::Object( Object* parentObject, const Vector& pos, const Vector& bbSize  ): 
position( pos ),
gameState( parentObject->getGameState() ),
speed(0,0,0),
active( true ),
scale( 1,1,1 ),
parent( nullptr ),
dispose( false ),
inheritScale( true )
{
	setSize( bbSize );
}

Object::~Object()
{
	destroyAllChildren();
}

Object& Object::_addChild( Unique<Object> o )
{    
	DEBUG_ASSERT(o->parent == nullptr, "The child you want to attach already has a parent");

	o->_notifyParent(this);

	auto ptr = o.get();

	childs.emplace( std::move(o) );

	return *ptr;
}

Renderable& Object::_addChild( Unique<Renderable> o, int layer )
{
	Platform::singleton().getRenderer().addRenderable(*o, layer);

	return addChild( std::move(o) );
}

void Object::_unregisterChild( Object& child )
{
	child._notifyParent( NULL );
	
    if( child.isRenderable() )
        Platform::singleton().getRenderer().removeRenderable( (Renderable&)child ); //if existing	
}

Unique<Object> Object::removeChild( Object& o )
{
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );
	
	auto elem = ChildList::find(childs, o);
	if (elem != childs.end()) {
		auto child = std::move(*elem);
		childs.erase(elem);
		_unregisterChild(*child);
		return child;
	}
	else {
		DEBUG_FAIL("This object is not a child");
		return nullptr;
	}
}

void Object::collectChilds()
{
	auto itr = childs.begin();
	for (; itr != childs.end();) {
		if ((*itr)->dispose)
			itr = childs.erase(itr);
		else
			++itr;
	}
}


void Object::destroyAllChildren()
{
	for (auto& c : childs)
		_unregisterChild(*c);

	childs.clear();
}

void Object::_updateWorldAABB( const Vector& localMin, const Vector& localMax )
{
	//get the eight world-position corners and transform them
	worldUpperBound = Vector::MIN;
	worldLowerBound = Vector::MAX;
	
	Vector vertex;    
	
	for( int i = 0; i < 8; ++i )
	{
		for( int j = 0; j < 3; ++j )
			vertex[j] = Math::getBit( i, j ) ? localMax[j] : localMin[j];
		
		vertex = getWorldPosition( vertex );
		
		worldUpperBound = Math::max( worldUpperBound, vertex );
		worldLowerBound = Math::min( worldLowerBound, vertex );
	}
}

Vector Object::getWorldPosition(const Vector& localPos) const {
	if (parent)
	{
		glm::vec4 pos = getWorldTransform() * glm::vec4(localPos.x, localPos.y, localPos.z, 1.0f);
		return Vector(pos.x, pos.y, pos.z);
	}
	else
		return localPos;
}

void Object::reset() {
	active = true;
	speed.x = speed.y = 0;

	updateWorldTransform();
}

void Object::setRoll(float r) {
	setRotation(Vector(0, 0, r));
}


Vector Object::getLocalPosition(const Vector& worldPos) {
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

float Object::getRoll() const {
	return glm::roll(rotation) * Math::EULER_TO_RADIANS;
}

bool Object::contains(const Vector& p) {
	return
		p.x <= worldUpperBound.x &&
		p.x >= worldLowerBound.x &&
		p.y <= worldUpperBound.y &&
		p.y >= worldLowerBound.y &&
		p.z <= worldUpperBound.z &&
		p.z >= worldLowerBound.z;
}

bool Object::contains2D(const Vector& p) {
	return
		p.x <= worldUpperBound.x &&
		p.x >= worldLowerBound.x &&
		p.y <= worldUpperBound.y &&
		p.y >= worldLowerBound.y;
}

bool Object::collidesWith(const Vector& MAX, const Vector& MIN) {
	return Math::AABBsCollide(getWorldMax(), getWorldMin(), MAX, MIN);
}

bool Object::collidesWith(Object * t) {
	DEBUG_ASSERT(t, "collidesWith: colliding Object is NULL");
	return collidesWith(t->getWorldMax(), t->getWorldMin());
}

Matrix Object::getFullTransformRelativeTo(const Matrix & parent) const
{
	Matrix m = glm::translate(parent, position) * mat4_cast(rotation);
	return (scale == Vector::ONE) ? m : glm::scale(m, scale);
}

void Object::updateWorldTransform()
{	
	auto oldTransform = mWorldTransform;

	//compute local matrix from position and orientation
	if (!parent)
		mWorldTransform = Matrix(1);
	else 
	{
		mWorldTransform = parent->getWorldTransform();
		if (!inheritScale)
			mWorldTransform = glm::scale(mWorldTransform, 1.f / parent->scale);
	}

	mWorldTransform = getFullTransformRelativeTo(mWorldTransform);

	if (oldTransform != mWorldTransform)
		_updateWorldAABB( -halfSize, halfSize );
}

void Object::updateChilds( float dt )
{
	if (childs.size() > 0) {

		for (auto& child : childs) {
			if (child->isActive())
				child->onAction(dt);
		}

	}
}

void Object::onAction( float dt )
{	
	position += speed * dt;	

	updateWorldTransform();
	
	updateChilds( dt );
}
