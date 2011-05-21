#include "stdafx.h"

#include "AnimatedQuad.h"

#include "Game.h"
#include "GameState.h"
#include "Viewport.h"
#include "Mesh.h"
#include "math.h"

using namespace Dojo;

AnimatedQuad::AnimatedQuad( GameState* level, const Vector& pos, bool pp ) :
Renderable( level, pos ),
animation( NULL ),
animationTime( 0 ),
pixelScale( 1,1 ),
autoAdvancement( true ),
pixelPerfect( pp )
{
	cullMode = CM_DISABLED;
	
	//use the default quad
	mesh = level->getMesh( "texturedQuad" );
	
	DEBUG_ASSERT( mesh );
	
	animation = new Animation( NULL, 0 );
	
	reset();
}



void AnimatedQuad::reset()
{
	Renderable::reset();
	
	pixelScale.x = pixelScale.y = 1;
	screenSize.x = screenSize.y = 1;
	
	animation->setup(NULL, 0);
	
	setTexture( NULL );
	mesh = gameState->getMesh( "texturedQuad" );
}

void AnimatedQuad::immediateAnimation( const std::string& name, float timePerFrame )
{
	DEBUG_ASSERT( name.size() );
	
	immediateAnimation( gameState->getFrameSet( name ), timePerFrame );
}

void AnimatedQuad::onAction( float dt )
{
	Renderable::onAction( dt );
	
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP ;
	
	if( autoAdvancement )
		advanceAnim(dt);
}

void AnimatedQuad::prepare( const Vector& viewportPixelRatio )
{
	Renderable::prepare(viewportPixelRatio);

	if( getTexture(0) && pixelPerfect )
	{		
		//compute the pixel occupied by the first texture on the screen				
		scale.x = getTexture()->getWidth() * viewportPixelRatio.x * pixelScale.x;
		scale.y = getTexture()->getHeight() * viewportPixelRatio.y * pixelScale.y;	

		screenSize.x = scale.x;
		screenSize.y = scale.y;	
	}
	else
	{
		screenSize.x = mesh->getDimensions().x * scale.x;
		screenSize.y = mesh->getDimensions().y * scale.y;
	}
}

void AnimatedQuad::_updateScreenSize()
{
	DEBUG_ASSERT( getTextureNumber() );
	
	gameState->getViewport()->makeScreenSize( screenSize, getTexture() );
	screenSize.x *= pixelScale.x;
	screenSize.y *= pixelScale.y;
}


