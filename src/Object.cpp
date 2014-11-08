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
childs( NULL ),
parent( nullptr ),
dispose( false ),
mNeedsAABB( true ),
inheritScale( true )
{
	setSize( bbSize );
}

Object::~Object()
{
	destroyAllChilds();
}

void Object::addChild( Object* o )
{    
	DEBUG_ASSERT( o, "Child to add is null" );
	DEBUG_ASSERT(o->parent == nullptr, "The child you want to attach already has a parent");


	if( !childs )
		childs = new ChildList(10,10);

	childs->add( o );

	o->_notifyParent( this );
}

void Object::addChild( Renderable* o, int layer )
{
	addChild( o );

	Platform::singleton().getRenderer().addRenderable( o, layer );
}

void Object::_unregisterChild( Object* child )
{
	DEBUG_ASSERT( child, "Child to remove is null" );
	
	child->_notifyParent( NULL );
	
    if( child->isRenderable() )
        Platform::singleton().getRenderer().removeRenderable( (Renderable*)child ); //if existing	
}

void Object::removeChild( int i )
{	
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );
	DEBUG_ASSERT( childs->size() > i && i >= 0, "Child index is OOB" );
	
	Object* child = childs->at( i );
	
	_unregisterChild( child );
	
	childs->remove( i );

	//if that was the last child, remove the list itself
	if( childs->size() == 0 )
		SAFE_DELETE( childs );
}

void Object::removeChild( Object* o )
{
	DEBUG_ASSERT( o, "Child to remove is null" );
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );
	
	int i = childs->getElementIndex( o );
	
	if( i >= 0 )
		removeChild( i );
}

void Object::destroyChild( int i )
{
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );
	DEBUG_ASSERT( childs->size() > i && i >= 0, "Child index is OOB" );

	Object* child = childs->at( i );

	child->onDestruction();
	
	removeChild( i );
	
	SAFE_DELETE( child );
}

void Object::destroyChild( Object* o )
{
	DEBUG_ASSERT( o, "Child to destroy is null" );
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );
	
	int i = childs->getElementIndex( o );
	
	if( i >= 0 )
		destroyChild( i );
}


void Object::collectChilds()
{
	if( childs )
	{
		for( int i = 0; childs && i < childs->size(); ++i )
		{
			if( childs->at( i )->dispose )
				destroyChild( i-- );
		}
	}
}


void Object::destroyAllChilds()
{
	//just set all to dispose and then collect them
	if( childs )
		for( auto child : *childs )
			child->dispose = true;

	collectChilds();
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

Object* Object::getChild(int i) {
	DEBUG_ASSERT(hasChilds(), "Tried to retrieve a child from an object with no childs");
	DEBUG_ASSERT(childs->size() > i || i < 0, "Tried to retrieve an out-of-bounds child");

	return childs->at(i);
}

bool Object::contains(const Vector& p) {
	DEBUG_ASSERT(mNeedsAABB, "contains: this Object has no AABB");

	return
		p.x <= worldUpperBound.x &&
		p.x >= worldLowerBound.x &&
		p.y <= worldUpperBound.y &&
		p.y >= worldLowerBound.y &&
		p.z <= worldUpperBound.z &&
		p.z >= worldLowerBound.z;
}

bool Object::contains2D(const Vector& p) {
	DEBUG_ASSERT(mNeedsAABB, "contains: this Object has no AABB");

	return
		p.x <= worldUpperBound.x &&
		p.x >= worldLowerBound.x &&
		p.y <= worldUpperBound.y &&
		p.y >= worldLowerBound.y;
}

bool Object::collidesWith(const Vector& MAX, const Vector& MIN) {
	DEBUG_ASSERT(mNeedsAABB, "collides: this Object has no AABB");

	return Math::AABBsCollide(getWorldMax(), getWorldMin(), MAX, MIN);
}

bool Object::collidesWith(Object * t) {
	DEBUG_ASSERT(t, "collidesWith: colliding Object is NULL");
	DEBUG_ASSERT(mNeedsAABB, "collidesWith: this Object has no AABB");

	return collidesWith(t->getWorldMax(), t->getWorldMin());
}

Matrix Object::getFullTransformRelativeTo(const Matrix & parent) const
{
	Matrix m = glm::translate(parent, position) * mat4_cast(rotation);
	return (scale == Vector::ONE) ? m : glm::scale(m, scale);
}

void Object::updateWorldTransform()
{	
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

	//update AABB if needed
	if( mNeedsAABB )
		_updateWorldAABB( -halfSize, halfSize );
}

void Object::updateChilds( float dt )
{	
	if( childs )
	{
		collectChilds();
		
		for( int i = 0; childs && i < childs->size(); ++i )
		{
			if( childs->at(i )->isActive() )
				childs->at(i)->onAction(dt);
		}
	}
}

void Object::onAction( float dt )
{	
	position += speed * dt;	

	updateWorldTransform();
	
	updateChilds( dt );
}
