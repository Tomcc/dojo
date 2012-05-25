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
dispose( false )
{
	DEBUG_ASSERT( parentLevel );
	
	setSize( bbSize );
}

void Object::addChild( Object* o )
{
	if( !childs )
		childs = new ChildList(10,10);

	DEBUG_ASSERT( o );

	childs->add( o );

	o->_notifyParent( this );
}

void Object::addChild( Renderable* o, int layer )
{
	addChild( o );

	Platform::getSingleton()->getRender()->addRenderable( o, layer );
}

void Object::removeChild( int i )
{	
	DEBUG_ASSERT( hasChilds() );
	DEBUG_ASSERT( childs->size() > i );
	
	Object* child = childs->at( i );
	
	child->_notifyParent( NULL );
	
	childs->remove( i );
		
	Platform::getSingleton()->getRender()->removeRenderable( (Renderable*)child ); //if existing	
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
			childs->at(i)->dispose = true;
	
		collectChilds();
	
		SAFE_DELETE( childs );
	}
}

void Object::updateWorldTransform()
{	
    //compute local matrix from position and orientation
    mWorldTransform = Matrix(1);
    
    mWorldTransform = glm::translate( mWorldTransform, position );
    mWorldTransform *= mat4_cast( rotation );    
    mWorldTransform = glm::scale( mWorldTransform, scale );
    
    if( parent )  //combine with parent transform
        mWorldTransform *= parent->getWorldTransform();     
     
    //update max and min
    worldUpperBound = getWorldPosition( halfSize );
    worldLowerBound = getWorldPosition( halfSize );
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
	
    //HACK
    //angle += rotationSpeed * dt;

	updateWorldTransform();
	
	updateChilds( dt );
}
