#include "stdafx.h"

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"
#include "Mesh.h"

using namespace Dojo;

Renderable::~Renderable()
{
	
}

void Renderable::onAction( float dt )
{
	Object::onAction( dt );
	
	if( mesh )
	{
        worldUpperBound = getWorldPosition( mesh->getMax() );
        worldLowerBound = getWorldPosition( mesh->getMin() );
	}
	
	advanceFade(dt);
}