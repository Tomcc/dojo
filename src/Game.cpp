#include "stdafx.h"

#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Sprite.h"
#include "dojomath.h"

using namespace Dojo;

const float Game::UPDATE_INTERVAL_CAP = 1.f/60.f;

Game::Game( const String& gamename, uint w, uint h, Render::RenderOrientation r ) :
name( gamename ),
nativeWidth( w ),
nativeHeight( h ),
nativeOrientation( r ),
focus( false )
{	
	DEBUG_ASSERT( name.size() );
	DEBUG_ASSERT( w > 0 );
	DEBUG_ASSERT( h > 0 );
	DEBUG_ASSERT( r <= Render::RO_LANDSCAPE_RIGHT );

	Math::seedRandom();
}
	
Game::~Game()
{

}

