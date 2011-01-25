#include "stdafx.h"

#include "Object.h"

#include "Game.h"

using namespace Dojo;

void Object::action( float dt )
{
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP;
		
	position.x += speed.x * dt;
	position.y += speed.y * dt;
}
