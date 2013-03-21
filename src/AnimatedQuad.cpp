#include "stdafx.h"

#include "AnimatedQuad.h"

#include "Game.h"
#include "GameState.h"
#include "Viewport.h"
#include "Mesh.h"
#include "dojomath.h"

using namespace Dojo;

AnimatedQuad::AnimatedQuad( GameState* level, const Vector& pos, bool pp ) :
Renderable( level, pos ),
animation( NULL ),
animationTime( 0 ),
pixelScale( 1,1 ),
animationSpeedMultiplier( 1 ),
pixelPerfect( pp )
{
	cullMode = CM_DISABLED;
	inheritScale = false;
	
	//use the default quad
	mesh = level->getMesh( "texturedQuad" );
	
	DEBUG_ASSERT( mesh, "AnimatedQuad requires a quad mesh called 'texturedQuad' to be loaded (use addPrefabMeshes to load one)" );
	
	animation = new Animation( NULL, 0 );
	
	reset();
}



void AnimatedQuad::reset()
{
	Renderable::reset();
	
	pixelScale.x = pixelScale.y = 1;
	screenSize.x = screenSize.y = 1;
	
	if( animation )
		animation->setup(NULL, 0);
	
	setTexture( NULL );
	mesh = gameState->getMesh( "texturedQuad" );

	DEBUG_ASSERT( mesh, "AnimatedQuad requires a quad mesh called 'texturedQuad' to be loaded (use addPrefabMeshes to load one)" );
}

void AnimatedQuad::immediateAnimation( const String& name, float timePerFrame )
{
	FrameSet* set = gameState->getFrameSet( name );
	
	DEBUG_ASSERT_INFO( set != nullptr, "The required FrameSet was not found", "name = " + name );
	
	immediateAnimation( set, timePerFrame );
}

void AnimatedQuad::onAction( float dt )
{		
    advanceAnim(dt);

	_updateScreenSize();

	if( pixelPerfect )
	{
		scale = screenSize;
		scale.z = 1; //be sure to keep scale = 1 or the transform will be denormalized (no inverse!)
	}

	Renderable::onAction( dt );
}

void AnimatedQuad::_updateScreenSize()
{
	if( pixelPerfect )
	{
		DEBUG_ASSERT( getTexture(), "Pixel perfect AnimatedQuads need a texture to be set" );

		gameState->getViewport()->makeScreenSize( screenSize, getTexture() );
		screenSize.x *= pixelScale.x;
		screenSize.y *= pixelScale.y;
	}
	else
	{
		screenSize.x = mesh->getDimensions().x * scale.x;
		screenSize.y = mesh->getDimensions().y * scale.y;
	}
}


