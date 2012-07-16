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

void GameState::touchAreaAtPoint( const Vector& point )
{
	Vector pointer = getViewport()->makeWorldCoordinates( point );
		
    TouchArea* topMost = NULL;
    int topMostLayer = INT32_MIN;
    
	for( int i = 0; i < mTouchAreas.size(); ++i )
	{
        TouchArea* t = mTouchAreas[i];
        
        if( t->isActive() && t->getLayer() > topMostLayer && t->contains( pointer ) )
        {
            topMost = t;
            topMostLayer = t->getLayer();
        }
	}
    
    if( topMost )
        topMost->_incrementTouches();
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
