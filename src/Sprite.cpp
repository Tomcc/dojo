#include "stdafx.h"

#include "Sprite.h"

#include "Viewport.h"
#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Sprite::Sprite( GameState* level, const Vector& pos, const String& defaultAnimName, float tpf, bool pp ) :
AnimatedQuad( level, pos, pp )
{	
	//get rid of default animation
	SAFE_DELETE( animation );
	
	if( defaultAnimName.size() )
	{		
		FrameSet* s = level->getFrameSet( defaultAnimName );
		
		setAnimation( registerAnimation( s, tpf ) );
		
		//set convenient size to fit the first frame			
		_updateScreenSize();

		setSize( screenSize );	
	}
}

int Sprite::registerAnimation( const String& base, float timePerFrame )
{
	FrameSet* s = gameState->getFrameSet( base );
	
	DEBUG_ASSERT( s );
	DEBUG_ASSERT( timePerFrame >= 0 );
	
	return registerAnimation( s, timePerFrame );	
}
