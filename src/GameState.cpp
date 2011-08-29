#include "stdafx.h"

#include "GameState.h"

#include "Game.h"
#include "Object.h"
#include "Sprite.h"
#include "Viewport.h"

using namespace Dojo;

GameState::GameState( Game* parentGame ) :
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
	destroyAll();
		
	//flush resources
	unloadAll();
}


void GameState::addObject( Renderable* s, int layer, bool clickable )		
{
	Platform::getSingleton()->getRender()->addRenderable( s, layer );
	
	addObject( s );
	
	if( clickable )
		addClickable( s );
}

void GameState::addLight( Light* l )
{
	DEBUG_ASSERT( l );
	
	addObject( l );
	
	Platform::getSingleton()->getRender()->addLight( l );
}

void GameState::setViewport( Viewport* v )
{
	DEBUG_ASSERT( v );
	
	camera = v;
	
	Platform::getSingleton()->getRender()->setViewport( v );
}

void GameState::removeSprite( Renderable* s )
{
	removeObject( s );
	removeClickableSprite( s );
	
	Platform::getSingleton()->getRender()->removeRenderable( s );
}

void GameState::removeClickableSprite( Renderable* s )
{
	DEBUG_ASSERT( s );
	
	clickables.remove( s );
}

void GameState::destroyObject( Object* o )
{
	DEBUG_ASSERT(o);
	
	removeObject( o );
	
	delete o;
}

void GameState::destroyObject( Renderable* s )
{
	DEBUG_ASSERT( s );
	
	removeSprite( s );
	
	delete s;
}

void GameState::removeAll()
{	
	clickables.clear();
	objects.clear();
}

void GameState::destroyAll()
{
	for( int i = 0; i < objects.size(); ++i )
		objects[i]->dispose = true;
	
	collectDisposed();
}

Renderable* GameState::getClickableAtPoint( const Vector& point )
{	
	//look for clicked clickables
	Renderable *c = NULL;
	Renderable *clickable = NULL;
	
	//find the pointer position in viewport space
	Vector pointer = camera->makeWorldCoordinates( point );
	
	for( uint i = 0; i < clickables.size(); ++i )
	{
		c = clickables[i];
				
		if( c->isVisible() && c->isActive() && c->contains( pointer ) )
		{
			//pick highest layer
			if( !clickable || (clickable && c->getLayer() > clickable->getLayer() ) ) 
			   clickable = c;
		}
	}
	
	return clickable;
}

void GameState::collectDisposed()
{
	for( uint i = 0; i < objects.size(); ++i )
	{
		Object * o = objects.at(i);
		
		if( o->dispose )
		{
			o->onDestruction();
			
			//could be a clickable
			removeSprite( (Renderable*)o );
			
			--i;
			
			delete o;
		}
	}		
}

void GameState::updateObjects( float dt )
{		
	collectDisposed();
	
	for( uint i = 0; i < objects.size(); ++i )
	{
		if( objects[i]->isActive() )
			objects[i]->onAction( dt );
	}			
}


void GameState::onTouchBegan( const Vector& point )
{
	Renderable* click = getClickableAtPoint( point );

	if( click && click->clickListener )
		click->clickListener->onButtonPressed( click , point );
	else
		onButtonPressed( click, point ); //puo anche essere null!
}

void GameState::onTouchEnd(const Dojo::Vector &point)
{
	Renderable* click = getClickableAtPoint( point );

	if( click && click->clickListener )
		click->clickListener->onButtonReleased( click , point );
	else
		onButtonReleased( click, point ); //puo anche essere null!
}
