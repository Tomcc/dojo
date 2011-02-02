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
	game->getRender()->addRenderable( s, layer );
	
	addObject( s );
	
	if( clickable )
		addClickable( s );
}

void GameState::removeSprite( Renderable* s )
{
	removeObject( s );
	removeClickableSprite( s );
	
	game->getRender()->removeRenderable( s );
}

void GameState::removeClickableSprite( Renderable* s )
{
	clickables.removeElement( s );
	s->clickListener = NULL;
}


void GameState::removeAll()
{
	for( uint i = 0; i < clickables.size(); ++i )
		clickables.at(i)->clickListener = NULL;
	
	clickables.clear();
	objects.clear();
}


SoundManager* GameState::getSoundManager()
{	
	return game->getSoundManager();	
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
				
				objects.removeElement( i );
				
				--i;
			}
		}
	}			
}	
