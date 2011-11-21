#include "stdafx.h"

#include "GameState.h"

#include "Game.h"
#include "Object.h"
#include "Sprite.h"
#include "Viewport.h"
#include "Platform.h"

using namespace Dojo;

GameState::GameState( Game* parentGame ) :
Object( this, Vector::ZERO, Vector::ONE ),
ResourceGroup(),
game( parentGame ),
timeElapsed(0),
camera(NULL)
{
	
}

GameState::~GameState()
{	
	clear();
}

void GameState::clear()
{	
	//unregister objects
	destroyAllChilds();
		
	//flush resources
	unloadAll();
}

void GameState::setViewport( Viewport* v )
{
	DEBUG_ASSERT( v );
	
	camera = v;
	
	Platform::getSingleton()->getRender()->setViewport( v );
}

Renderable* GameState::getClickableAtPoint( const Vector& point )
{	
	Vector pointer = getViewport()->makeWorldCoordinates( point );
	
	//look into layers
	Render* r = Platform::getSingleton()->getRender();
	
	for( int i = r->getLastLayerID()-1; i >= r->getFirstLayerID(); --i )
	{
		Render::Layer* l = r->getLayer( i );
		
		for( int j = 0; j < l->size(); ++j )
		{
			Renderable* r = l->at(j);
			
			if( r->isClickable() && r->isVisible() && r->isActive() && r->contains( pointer ) )
				return r;
		}
	}
	
	return NULL;
}

void GameState::onTouchBegan( const Vector& point )
{
	Renderable* click = getClickableAtPoint( point );

	if( click )
		click->clickListener->onButtonPressed( click , point );
	else
		onButtonPressed( NULL, point );
}

void GameState::onTouchEnd(const Dojo::Vector &point)
{
	Renderable* click = getClickableAtPoint( point );

	if( click )
		click->clickListener->onButtonReleased( click , point );
	else
		onButtonReleased( NULL, point );
}
