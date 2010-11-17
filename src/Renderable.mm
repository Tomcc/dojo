/*
 *  Renderable.mm
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 8/14/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"

using namespace Dojo;

void Renderable::action( float dt )
{
	Object::action( dt );
	
	if( absoluteTimeSpeed )  //correct time speed
		dt = Game::UPDATE_INTERVAL_CAP ;
	
	advanceFade(dt);
}

bool Renderable::_canBeRenderedBy( Viewport* v )
{
	return rendered = collidesWith( v );
}

