#include "stdafx.h"

#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Menu.h"
#include "Sprite.h"
#include "Math.h"

using namespace Dojo;

Game::Game( Application* app, Render* r, SoundManager* s, TouchSource* t  ) :
render( r ),
soundManager( s ),
touchSource( t ),
focus( false ),
application( app )
{
	DEBUG_ASSERT( r && s && t );
	
	Math::seedRandom();
}

	
Game::~Game()
{

}

