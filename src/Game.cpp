#include "stdafx.h"

#include "dojo/Game.h"

#include "dojo/Object.h"
#include "dojo/GameState.h"
#include "dojo/Sprite.h"
#include "dojo/math.h"

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

