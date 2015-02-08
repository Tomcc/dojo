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
disposed( false ),
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
	DEBUG_ASSERT(!childs.contains(o), "Element already in the vector!");

	o->parent = this;

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
	child.parent = nullptr;
	
    if( child.isRenderable() )
        Platform::singleton().getRenderer().removeRenderable( (Renderable&)child ); //if existing	
}

Unique<Object> Object::removeChild( Object& o )
{
	DEBUG_ASSERT( hasChilds(), "This Object has no childs" );
	
	auto elem = ChildList::find(childs, o);
	if (elem != childs.end()) {
		auto child = std::move(*elem);
		_unregisterChild(*child);
		childs.erase(elem);
		return child;
	}
	else {
		DEBUG_FAIL("This object is not a child");
		return nullptr;
	}
}

void Object::collectChilds()
{
	for (size_t i = 0; i < childs.size(); ++i) {
		if (childs[i]->disposed) {
			_unregisterChild(*childs[i]);
			childs.erase(childs[i]);

			i = 0; //a destructor might dispose of other children...
		}
	}
}


void Object::destroyAllChildren()
{
	for (auto& c : childs)
		_unregisterChild(*c);

	childs.clear();
}

Object::AABB Object::transformAABB( const Vector& localMin, const Vector& localMax ) const
{
	AABB bb;

	//get the eight world-position corners and transform them
	bb.max = Vector::MIN;
	bb.min = Vector::MAX;
	
	Vector vertex;    
	
	for( int i = 0; i < 8; ++i )
	{
		for( int j = 0; j < 3; ++j )
			vertex[j] = Math::getBit( i, j ) ? localMax[j] : localMin[j];
		
		vertex = getWorldPosition( vertex );
		
		bb.max = Math::max(bb.max, vertex);
		bb.min = Math::min(bb.min, vertex);
	}

	return bb;
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


Dojo::Vector Dojo::Object::getLocalPosition(const Vector& worldPos) const {
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
}

void Object::updateChilds( float dt )
{
	if (childs.size() > 0) {

		//WARNING: do not use a ranged for loop in this one!
		//a child might remove any other child from the array 
		//so we need to always check with the updated size
		for (size_t i = 0; i < childs.size(); ++i) { 
			if (childs[i]->isActive())
				childs[i]->onAction(dt);
		}

		collectChilds();
	}
}

void Object::onAction( float dt )
{	
	position += speed * dt;	

	updateWorldTransform();
	
	updateChilds( dt );
}
