/*
 *  Renderable.cpp
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 7/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */


#include "AnimatedQuad.h"

#include "Game.h"
#include "GameState.h"
#include "Viewport.h"
#include "Mesh.h"
#include "Math.h"

using namespace Dojo;

AnimatedQuad::AnimatedQuad( GameState* level, const Vector& pos ) :
Renderable( level, pos ),
animation( NULL ),
animationTime( 0 ),
pixelScale( 1,1,1 ),
autoAdvancement( true )
{		
	//use the default quad
	mesh = level->getMesh( "texturedQuad" );
	
	DOJO_ASSERT( mesh );
	
	animation = new Animation( NULL, 0 );
	
	reset();
}

void AnimatedQuad::reset()
{
	Renderable::reset();
	
	pixelScale.x = pixelScale.y = 1;
	screenSize.x = screenSize.y = 1;
	
	animation->setup(NULL, 0);
	
	texture = NULL;
	mesh = gameState->getMesh( "texturedQuad" );
}

void AnimatedQuad::immediateAnimation( const std::string& name, float timePerFrame )
{
	DOJO_ASSERT( name.size() );
	
	immediateAnimation( gameState->getFrameSet( name ), timePerFrame );
}

void AnimatedQuad::action( float dt )
{
	Renderable::action( dt );
	
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP ;
	
	if( autoAdvancement )
		advanceAnim(dt);
}

void AnimatedQuad::prepare( const Vector& viewportPixelRatio )
{
	if( texture )
	{		
		//compute the pixel occupied by this texture on the screen				
		scale.x = texture->getWidth() * viewportPixelRatio.x * pixelScale.x;
		scale.y = texture->getHeight() * viewportPixelRatio.y * pixelScale.y;	
	}
	else  //no texture
	{
		scale.x = scale.y = 1;
	}
	
	screenSize.x = scale.x;
	screenSize.y = scale.y;			
}

void AnimatedQuad::_updateScreenSize()
{
	DOJO_ASSERT( texture );
	
	gameState->getViewport()->makeScreenSize( screenSize, texture );
	screenSize.x *= pixelScale.x;
	screenSize.y *= pixelScale.y;
}
bool AnimatedQuad::_canBeRenderedBy( Viewport* v )
{
	return rendered = Math::AABBCollides( position, screenSize, v->position, v->getSize());
}


