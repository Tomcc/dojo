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
	Render* render = Platform::getSingleton()->getRender();
	
	for( int i = render->getLastLayerID()-1; i >= render->getFirstLayerID(); --i )
	{
		Render::Layer* l = render->getLayer( i );
		
		for( int j = 0; j < l->size(); ++j )
		{
			Renderable* r = l->at(j);
			
			if( r->isClickable() && r->isVisible() && r->isActive() && r->contains( pointer ) )
				return r;
		}
	}
	
	return NULL;
}

void GameState::onTouchBegan( const InputSystem::Touch& touch )
{
	Renderable* click = getClickableAtPoint( touch.point );

	if( click )
		click->clickListener->onButtonPressed( click , touch.point );
	else
		onButtonPressed( NULL, touch.point );
}

void GameState::onTouchEnd( const InputSystem::Touch& touch )
{
	Renderable* click = getClickableAtPoint( touch.point );

	if( click )
		click->clickListener->onButtonReleased( click , touch.point );
	else
		onButtonReleased( NULL, touch.point );
}
