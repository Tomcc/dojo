#include "stdafx.h"

#include "Object.h"

#include "Game.h"
#include "Renderable.h"
#include "GameState.h"

using namespace Dojo;


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

	if( clickable )
		gameState->addClickable( o );
}

void Object::removeChild( Object* o )
{
	DEBUG_ASSERT( o );

	o->_notifyParent( NULL );

	childs->remove( o );

	Platform::getSingleton()->getRender()->removeRenderable( (Renderable*)o ); //if existing

	gameState->removeClickableSprite( (Renderable*)o ); //if existing
}

void Object::action( float dt )
{
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP;
	
	position += speed * dt;	
	angle += rotationSpeed * dt;

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

	if( childs )
	{
		for( uint i = 0; i < childs->size(); ++i )
		{
			if( childs->at(i)->dispose )
			{
				removeChild( childs->at(i) );
				gameState->removeObject( childs->at(i) );
			}
			else
				childs->at(i)->action(dt);
		}
	}
}
