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
parent( parentObject ),
dispose( false ),
mNeedsAABB( true ),
inheritScale( true )
{
	DEBUG_ASSERT( parent, "Null parent provided: any Object needs to have a non-null parent, except GameState" );
	
	setSize( bbSize );
}

Object::~Object()
{
	destroyAllChilds();
}

void Object::addChild( Object* o )
{    
	DEBUG_ASSERT( o, "Child to add is null" );
	
	if( !childs )
		childs = new ChildList(10,10);

	childs->add( o );

	o->_notifyParent( this );
}

void Object::addChild( Renderable* o, int layer )
{
	addChild( o );

	Platform::getSingleton()->getRender()->addRenderable( o, layer );
}

void Object::_unregisterChild( Object* child )
{
	DEBUG_ASSERT( child, "Child to remove is null" );
	
	child->_notifyParent( NULL );
	
    if( child->isRenderable() )
        Platform::getSingleton()->getRender()->removeRenderable( (Renderable*)child ); //if existing	
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
