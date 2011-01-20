//
//  Game.mm
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/25/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#include "Game.h"
#include "Object.h"
#include "GameState.h"

#include <ctime>
#include <cstdlib>

#include "Menu.h"
#include "Sprite.h"
#include "Math.h"

using namespace Dojo;

Game::Game( Application* app, Render* r, SoundManager* s ) :
render( r ),
soundManager( s ),
focus( false ),
application( app ),
currentState( NULL )
{
	Math::seedRandom();
}

	
Game::~Game()
{
	delete currentState;
}

void Game::setCurrentState( GameState* state )
{
	if( currentState )
	{
		currentState->clear();
		
		delete currentState;
	}	
	
	currentState = state;
	
	currentState->initialise();
}

void Game::_fireTouchBeginEvent( const Vector& point )
{	
	lastMovePos = point;
	
	//call only the current state, and filter the buttons
	Renderable* s = currentState->getClickableAtPoint( point );
	
	if( s )
	{
		if( s->clickListener ) //delegate to click listener?
			s->clickListener->onButtonPressed( s );
		else
			currentState->onButtonClicked( s );
	}			
	else
	{		
		currentState->onTouchBegan( point );
	}
}

void Game::_fireTouchMoveEvent( const Vector& point )
{	
	lastMovePos -= point;
	
	currentState->onTouchMove(point, lastMovePos );
	
	lastMovePos = point;
}

void Game::_fireTouchEndEvent( const Vector& point )		
{
	//call only the current state, and filter the buttons
	Renderable* s = currentState->getClickableAtPoint( point );
	
	if( s )
	{
		if( s->clickListener ) //delegate to click listener?
			s->clickListener->onButtonReleased( s );
		else
			currentState->onButtonReleased( s );
	}			
	else
	{		
		currentState->onTouchEnd( point );
	}
	
}

void Game::update( float dt )
{	
	if( focus && currentState )
		currentState->update( dt );
}


