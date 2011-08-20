#include "stdafx.h"

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"

using namespace Dojo;

Renderable::~Renderable()
{
	//be sure that it is removed
	gameState->removeSprite( this );
}

void Renderable::onAction( float dt )
{
	Object::onAction( dt );
	
	if( mesh )
	{
		worldUpperBound.x = worldPosition.x + mesh->getMax().x * abs( scale.x );
		worldUpperBound.y = worldPosition.y + mesh->getMax().y * abs( scale.y );
		worldUpperBound.z = worldPosition.z + mesh->getMax().z * abs( scale.z );
				
		worldLowerBound.x = worldPosition.x + mesh->getMin().x * abs( scale.x );
		worldLowerBound.y = worldPosition.y + mesh->getMin().y * abs( scale.y );
		worldLowerBound.z = worldPosition.z + mesh->getMin().z * abs( scale.z );
	}
	
	advanceFade(dt);
}