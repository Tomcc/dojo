#include "stdafx.h"

#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Sprite.h"
#include "Math.h"

using namespace Dojo;

const float Game::UPDATE_INTERVAL_CAP = 1.f/60.f;

Game::Game( Platform* p ) :
platform( p ),
focus( false )
{
	DEBUG_ASSERT( platform );
	
	Math::seedRandom();
}
	
Game::~Game()
{

}

