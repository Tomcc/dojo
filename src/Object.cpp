#include "stdafx.h"

#include "Object.h"

#include "Game.h"
#include "Renderable.h"
#include "GameState.h"
#include "Platform.h"

using namespace Dojo;
using namespace glm;

Object::Object( GameState* parentLevel, const Vector& pos, const Vector& bbSize  ): 
position( pos ),
gameState( parentLevel ),
speed(0,0,0),
active( true ),
scale( 1,1,1 ),
childs( NULL ),
parent( NULL ),
dispose( false ),
mNeedsAABB( true )
{
	DEBUG_ASSERT( parentLevel );
	
	setSize( bbSize );
}

void Object::addChild( Object* o )
{    
	DEBUG_ASSERT( o );
	
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
    DEBUG_ASSERT( child );
    
    child->_notifyParent( NULL );
    
	Platform::getSingleton()->getRender()->removeRenderable( (Renderable*)child ); //if existing	
}

void Object::removeChild( int i )
{	
	DEBUG_ASSERT( hasChilds() );
	DEBUG_ASSERT( childs->size() > i );
	
	Object* child = childs->at( i );
	
	_unregisterChild( child );
    
	childs->remove( i );
}

void Object::removeChild( Object* o )
{
	DEBUG_ASSERT( o );
	DEBUG_ASSERT( hasChilds() );
	
	int i = childs->getElementIndex( o );
	
	if( i >= 0 )
		removeChild( i );
}

void Object::destroyChild( int i )
{
	DEBUG_ASSERT( hasChilds() );
	DEBUG_ASSERT( childs->size() > i );
	
	Object* child = childs->at( i );
	
	removeChild( i );
	
	SAFE_DELETE( child );
}

void Object::destroyChild( Object* o )
{
	DEBUG_ASSERT( o );
	DEBUG_ASSERT( hasChilds() );
	
	int i = childs->getElementIndex( o );
	
	if( i >= 0 )
		destroyChild( i );
}


void Object::collectChilds()
{
	if( childs )
	{
		for( int i = 0; i < childs->size(); ++i )
		{
			if( childs->at( i )->dispose )
				destroyChild( i-- );
		}
	}
}


void Object::destroyAllChilds()
{
	if( childs )
	{	
		for( int i = 0; i < childs->size(); ++i )
        {
            //_unregisterChild( childs->at(i) );
            DEBUG_ASSERT( childs->at(i) );
			SAFE_DELETE( childs->at(i) );
        }
		
		SAFE_DELETE( childs );
	}
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

void Object::updateWorldTransform()
{	
    //compute local matrix from position and orientation
    mWorldTransform = parent ? parent->getWorldTransform() : Matrix( 1 );
    
    mWorldTransform = glm::translate( mWorldTransform, position );
    mWorldTransform *= mat4_cast( rotation );
    mWorldTransform = glm::scale( mWorldTransform, scale );
    
    //update AABB if needed
    if( mNeedsAABB )
        _updateWorldAABB( -halfSize, halfSize );    
}

void Object::updateChilds( float dt )
{	
	if( childs )
	{
		collectChilds();
		
		for( int i = 0; i < childs->size(); ++i )
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
