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
		//HACK this does not use rotation
		worldUpperBound.x = worldPosition.x + mesh->getMax().x * abs( scale.x );
		worldUpperBound.y = worldPosition.y + mesh->getMax().y * abs( scale.y );
		worldUpperBound.z = worldPosition.z + mesh->getMax().z * abs( scale.z );
				
		worldLowerBound.x = worldPosition.x + mesh->getMin().x * abs( scale.x );
		worldLowerBound.y = worldPosition.y + mesh->getMin().y * abs( scale.y );
		worldLowerBound.z = worldPosition.z + mesh->getMin().z * abs( scale.z );
	}
	
	advanceFade(dt);
}