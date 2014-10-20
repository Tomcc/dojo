#include "stdafx.h"

#include "GameState.h"

#include "Game.h"
#include "Object.h"
#include "Sprite.h"
#include "Viewport.h"
#include "Platform.h"
#include "TouchArea.h"
#include "InputSystem.h"

using namespace Dojo;

GameState::GameState( Game* parentGame ) :
Object( this, Vector::ZERO, Vector::ONE ),
ResourceGroup(),
game( parentGame ),
timeElapsed(0),
camera(nullptr)
{
	mNeedsAABB = true;

	gameState = this; //useful to pass a GameState around as an Object
}

GameState::~GameState()
{
    clear();
}

void GameState::clear()
{		
	destroyAllChilds();

	//flush resources
	unloadResources( false );
}

void GameState::setViewport( Viewport* v )
{
	DEBUG_ASSERT( v, "can't set a null viewport" );
	
	camera = v;
	
	Platform::singleton().getRender()->addViewport( v );
}

void GameState::touchAreaAtPoint( Touch* touch )
{
	Vector pointer = getViewport()->makeWorldCoordinates( touch->point );

	Dojo::Array< TouchArea* > layer;
	int topMostLayer = INT32_MIN;
	
	for( auto t : mTouchAreas )
	{		
		if( t->isActive() && t->getLayer() >= topMostLayer && t->contains2D( pointer ) )
		{
			//new highest layer - discard lowest layers found
			if( t->getLayer() > topMostLayer )
				layer.clear();

			layer.add( t );

			topMostLayer = t->getLayer();
		}
	}

	//trigger all the areas overlapping in the topmost layer 
	for( int i = 0; i < layer.size(); ++i )
		layer[i]->_incrementTouches( *touch );	
}

void GameState::updateClickableState()
{
	if( !childs )
		return;

	//clear all the touchareas
	for( auto ta : mTouchAreas )
		ta->_clearTouches();
	
	const InputSystem::TouchList& touchList = Platform::singleton().getInput()->getTouchList();
		
	//"touch" all the touchareas active in this frame
	for( auto touch : touchList )
		touchAreaAtPoint( touch );
	
	///launch events
	for( auto ta : mTouchAreas )
		ta->_fireOnTouchUsingCurrentTouches();
}
