#include "stdafx.h"

#include "dojo/Renderable.h"

#include "dojo/Game.h"
#include "dojo/Viewport.h"

using namespace Dojo;

void Renderable::onAction( float dt )
{
	Object::onAction( dt );
	
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP ;

	if( mesh )
	{
		worldUpperBound.x = worldPosition.x + mesh->getMax().x * scale.x;
		worldUpperBound.y = worldPosition.y + mesh->getMax().y * scale.y;
		worldUpperBound.z = worldPosition.z + mesh->getMax().z * scale.z;
				
		worldLowerBound.x = worldPosition.x + mesh->getMin().x * scale.x;
		worldLowerBound.y = worldPosition.y + mesh->getMin().y * scale.y;
		worldLowerBound.z = worldPosition.z + mesh->getMin().z * scale.z;
	}
	
	advanceFade(dt);
}