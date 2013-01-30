#include "stdafx.h"

#include "Game.h"

#include "Object.h"
#include "GameState.h"
#include "Sprite.h"
#include "dojomath.h"
#include "Platform.h"

using namespace Dojo;

Game::Game( const String& gamename, uint w, uint h, Orientation r, float nativedt, float maximumdt ) :
name( gamename ),
nativeWidth( w ),
nativeHeight( h ),
nativeOrientation( r ),
mNativeFrameLength( nativedt ),
mMaxFrameLength( maximumdt )
{	
	DEBUG_ASSERT( name.size() );
	DEBUG_ASSERT( w >= 0 );
	DEBUG_ASSERT( h >= 0 );
	DEBUG_ASSERT( mNativeFrameLength > 0 );
	DEBUG_ASSERT( mMaxFrameLength >= mNativeFrameLength );

	Platform* p = Platform::getSingleton();

	if( w == 0 )
		nativeWidth = p->getScreenWidth();
	if( h == 0 )
		nativeHeight = p->getScreenHeight();

	Math::seedRandom();
	
	p->addApplicationListener( this ); //always listen to the app
}
	
Game::~Game()
{

}

