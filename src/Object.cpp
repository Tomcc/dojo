#include "stdafx.h"

#include "Object.h"

#include "Game.h"
#include "Renderable.h"
#include "GameState.h"

using namespace Dojo;

Object::Object( GameState* parentLevel, const Vector& pos, const Vector& bbSize  ): 
position( pos ),
gameState( parentLevel ),
speed(0,0,0),
active( true ),
angle( 0,0,0 ),
rotationSpeed( 0,0,0 ),
scale( 1,1,1 ),
childs( NULL ),
parent( NULL ),
dispose( false ),
inheritAngle( true ),
customMatrix( NULL )
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

void Object::addChild( Renderable* o, uint layer, bool clickable )
{
	addChild( o );

	Platform::getSingleton()->getRender()->addRenderable( o, layer );
	
	//compatibility utility - either set listener to NULL if not clickable, or to gameState if clickable AND not defined
	if( !clickable )
		o->clickListener = NULL;
	else if( clickable && !o->clickListener )
		o->clickListener = gameState;
}

void Object::removeChild( Object* o )
{
	DEBUG_ASSERT( o );

	o->_notifyParent( NULL );

	childs->remove( o );

	Platform::getSingleton()->getRender()->removeRenderable( (Renderable*)o ); //if existing
}

void Object::destroyAllChilds()
{
	if( childs )
	{	
		for( uint i = 0; i < childs->size(); ++i )
			childs->at(i)->dispose = true;
	
		collectChilds();
	
		delete childs;
	
		childs = NULL;
	}
}

void Object::updateWorldPosition()
{
	if( parent )  //add parent world transform
	{
		if( inheritAngle )
		{		
			worldPosition = parent->getWorldPosition( position );
			
			//TODO also rotate angle
			worldRotation = angle;
		}
		else
		{
			worldPosition = parent->getWorldPosition() + position;
			worldRotation = angle;
		}
	}
	else 
	{
		worldPosition = position;
		worldRotation = angle;
	}	
	
	//update max and min TODO - real transforms
	max = worldPosition + halfSize;
	min = worldPosition - halfSize;
}

void Object::collectChilds()
{
	if( childs )
	{
		for( uint i = 0; i < childs->size(); ++i )
		{
			Object* o = childs->at( i );
			
			if( o->dispose )
			{
				removeChild(o);
				
				--i;
				
				delete o;
			}
		}
	}
}

void Object::updateChilds( float dt )
{
	
	if( childs )
	{
		collectChilds();
		
		for( uint i = 0; i < childs->size(); ++i )
		{
			if( childs->at(i )->isActive() )
				childs->at(i)->onAction(dt);
		}
	}
}

void Object::onAction( float dt )
{	
	position += speed * dt;	
	angle += rotationSpeed * dt;

	updateWorldPosition();
	
	updateChilds( dt );
}
