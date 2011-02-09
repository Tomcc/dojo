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
	delete camera;
	
	//unregister objects
	removeAll();
	
	for( uint i = 0; i < objects.size(); ++i )
		delete objects.at(i);	
	
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

void GameState::removeSprite( Renderable* s )
{
	removeObject( s );
	removeClickableSprite( s );
	
	Platform::getSingleton()->getRender()->removeRenderable( s );
}

void GameState::removeClickableSprite( Renderable* s )
{
	clickables.remove( s );
	s->clickListener = NULL;
}


void GameState::removeAll()
{
	for( uint i = 0; i < clickables.size(); ++i )
		clickables.at(i)->clickListener = NULL;
	
	clickables.clear();
	objects.clear();
}

Renderable* GameState::getClickableAtPoint( const Vector& point )
{	
	//look for clicked clickables
	Renderable *c = NULL;
	Renderable *clickable = NULL;
	
	//find the pointer position in viewport space
	Vector pointer = camera->makeWorldCoordinates( point );
	
	for( uint i = 0; i < clickables.size() && !clickable; ++i )
	{
		c = clickables.at(i);
		if( c->isVisible() && c->isActive() && c->contains( pointer ) )
		{
			//pick highest layer
			if( !clickable || (clickable && c->getLayer() > clickable->getLayer() ) ) 
			   clickable = c;
		}
	}
	
	return clickable;
}

void GameState::updateObjects( float dt )
{			
	Object* o;
	for( uint i = 0; i < objects.size(); ++i )
	{
		o = objects.at(i);
		
		if( o->isActive()	)
		{
			o->action( dt );
			
			if( o->dispose )
			{
				delete o;
				
				objects.remove( i );
				
				--i;
			}
		}
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