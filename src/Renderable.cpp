#include "stdafx.h"

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"
#include "Mesh.h"
#include "GameState.h"

using namespace Dojo;

Renderable::Renderable( GameState* level, const Vector& pos, Mesh* m ) :
	Object( level, pos, Vector::ONE ),
	visible( true ),
	layer(0),
	renderingOrder(0),
	currentFadeTime(0),
	mCulled( false )
{
	reset();

	mesh = m;
}

Renderable::Renderable( GameState* level, const Vector& pos, const String& meshName ) :
	Object( level, pos, Vector::ONE ),
	visible( true ),
	layer(0),
	renderingOrder(0),
	currentFadeTime(0),
	mCulled( false )
{
	reset();

	if( meshName.size() )
		setMesh( level->getMesh( meshName ) );
}

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