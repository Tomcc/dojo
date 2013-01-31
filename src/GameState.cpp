#include "stdafx.h"

#include "GameState.h"

#include "Game.h"
#include "Object.h"
#include "Sprite.h"
#include "Viewport.h"
#include "Platform.h"
#include "TouchArea.h"

using namespace Dojo;

GameState::GameState( Game* parentGame ) :
Object( this, Vector::ZERO, Vector::ONE ),
ResourceGroup(),
game( parentGame ),
timeElapsed(0),
camera(NULL)
{
	mNeedsAABB = true;
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
	DEBUG_ASSERT( v );
	
	camera = v;
	
	Platform::getSingleton()->getRender()->setViewport( v );
}

void GameState::touchAreaAtPoint( const Vector& point )
{
	Vector pointer = getViewport()->makeWorldCoordinates( point );

	Dojo::Array< TouchArea* > layer;
	int topMostLayer = INT32_MIN;
	
	for( int i = 0; i < mTouchAreas.size(); ++i )
	{
		TouchArea* t = mTouchAreas[i];
		
		bool c = t->contains( pointer );
		if( t->isActive() && t->getLayer() >= topMostLayer && c )
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
		layer[i]->_incrementTouches();	
}

void GameState::updateClickableState()
{
	if( !childs )
		return;
	
	const InputSystem::TouchList& touches = Platform::getSingleton()->getInput()->getTouchList();
		
	//"touch" all the touchareas active in this frame
	for( int i = 0; i < touches.size(); ++i )
		touchAreaAtPoint( touches[i]->point );
	
	///launch events
	for( int i = 0; i < mTouchAreas.size(); ++i )
		mTouchAreas[i]->_fireOnTouchUsingCurrentTouches();
}
