#include "stdafx.h"

#include "Sprite.h"

#include "Viewport.h"
#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Sprite::Sprite( Object* parent, const Vector& pos, const String& defaultAnimName, float tpf, bool pp ) :
AnimatedQuad( parent, pos ),
mAnimationIdx( -1 )
{	
	pixelPerfect = pp;

	//get rid of default animation
	SAFE_DELETE( animation );
	
	if( defaultAnimName.size() )
	{		
		setAnimation( registerAnimation( defaultAnimName, tpf ) );
		
		//set convenient size to fit the first frame			
		_updateScreenSize();

		setSize( screenSize );	
	}
}

int Sprite::registerAnimation( const String& base, float timePerFrame )
{
	FrameSet* set = gameState->getFrameSet( base );

	DEBUG_ASSERT_INFO( set != nullptr, "The FrameSet to be registered could not be found", "name = " + base );

	return registerAnimation( set, timePerFrame );	
}
