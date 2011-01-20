/*
 *  Sprite.mm
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/23/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sprite.h"

#include "Viewport.h"

using namespace Dojo;

Sprite::Sprite( GameState* level, const Vector& pos, const std::string& defaultAnimName, float tpf ) :
AnimatedQuad( level, pos )
{			
	//do not use immediate animation
	delete animation;
	animation = NULL;
	
	if( defaultAnimName.size() )
	{		
		FrameSet* s = level->getFrameSet( defaultAnimName );
		
		setAnimation( registerAnimation( s, tpf ) );
		
		//set convenient size to fit the first frame			
		setSize( screenSize );	
		_updateScreenSize();
	}
}

uint Sprite::registerAnimation( const std::string& base, float timePerFrame )
{
	FrameSet* s = gameState->getFrameSet( base );
	
	DOJO_ASSERT( s );
	DOJO_ASSERT( timePerFrame >= 0 );
	
	return registerAnimation( s, timePerFrame );	
}
