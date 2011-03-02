#include "stdafx.h"

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"

using namespace Dojo;

void Renderable::action( float dt )
{
	Object::action( dt );
	
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP ;

	worldUpperBound = worldPosition + mesh->getMax() - mesh->getCenter();
	worldLowerBound = worldPosition + mesh->getMin() - mesh->getCenter();
	
	advanceFade(dt);
}