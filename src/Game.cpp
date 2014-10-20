#include "stdafx.h"

#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Sprite.h"
#include "dojomath.h"
#include "Platform.h"

using namespace Dojo;

Game::Game( const String& gamename, int w, int h, Orientation r, float nativedt, float maximumdt ) :
name( gamename ),
nativeWidth( w ),
nativeHeight( h ),
nativeOrientation( r ),
mNativeFrameLength( nativedt ),
mMaxFrameLength( maximumdt )
{	
	DEBUG_ASSERT( name.size(), "A Game must have a non-empty name" );
	DEBUG_ASSERT( w >= 0, "negative width" );
	DEBUG_ASSERT( h >= 0, "negative height"  );
	DEBUG_ASSERT( mNativeFrameLength > 0, "the Native Frame Lenght must be greater than 0 seconds" );
	DEBUG_ASSERT( mMaxFrameLength >= mNativeFrameLength, "the native frame lenght must be less or equal than the max frame length" );

	auto& p = Platform::singleton();

	if( w == 0 )
		nativeWidth = p.getScreenWidth();
	if( h == 0 )
		nativeHeight = p.getScreenHeight();

	Math::seedRandom();
	
	p.addApplicationListener( this ); //always listen to the app
}
	
Game::~Game()
{

}