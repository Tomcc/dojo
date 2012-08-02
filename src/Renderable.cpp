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
	bool previousAABBSetting = mNeedsAABB;
	mNeedsAABB = false; //override the setting
		
	Object::onAction( dt );
	
	if( mesh && (mNeedsAABB = previousAABBSetting) )
		_updateWorldAABB( mesh->getMin(), mesh->getMax() );
	
	advanceFade(dt);
}