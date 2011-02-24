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

void Object::action( float dt )
{
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP;
		
	position.x += speed.x * dt;
	position.y += speed.y * dt;
	
	spriteRotation += rotationSpeed * dt;

	if( parent )  //add parent world transform
	{
		worldPosition = parent->worldPosition + position;
		//TODO use rotation
		worldRotation = parent->worldRotation + spriteRotation;
	}
	else 
	{
		worldPosition = position;
		worldRotation = spriteRotation;
	}	

	if( childs )
	{
		for( uint i = 0; i < childs->size(); ++i )
			childs->at(i)->action(dt);
	}
}
