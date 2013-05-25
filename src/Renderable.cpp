#include "stdafx.h"

#include "Renderable.h"

#include "Game.h"
#include "Viewport.h"
#include "Mesh.h"
#include "GameState.h"

using namespace Dojo;

Renderable::Renderable( Object* parent, const Vector& pos, Mesh* m ) :
	Object( parent, pos, Vector::ONE ),
	visible( true ),
	layer(0),
	renderingOrder(0),
	currentFadeTime(0),
	mCulled( false )
{
	reset();

	mesh = m;
}

Renderable::Renderable( Object* parent, const Vector& pos, const String& meshName ) :
	Object( parent, pos, Vector::ONE ),
	visible( true ),
	layer(0),
	renderingOrder(0),
	currentFadeTime(0),
	mCulled( false )
{
	reset();

	if( meshName.size() )
	{
		setMesh( getGameState()->getMesh( meshName ) );
		
		DEBUG_ASSERT_INFO( getMesh(), "Tried to create a Renderable but the mesh wasn't found", "name = " + meshName );
	}
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