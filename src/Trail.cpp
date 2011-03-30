#include "stdafx.h"

#include "Trail.h"

#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Trail::Trail( GameState* l, const Vector& pos, const std::string& texName /* = */ ) :
Renderable( l, pos ),
offset( Vector::UNIT_Y ),
updateInterval( 1 )
{
	if( !texName.empty() )
		setTexture( l->getFrameSet(texName)->getFrame(0) );
}

void Trail::action( float dt )
{
	Renderable::action(dt);

	if( elapsedTime >= updateInterval )
	{
		//is the trail full?
		if( points.size() == maxSegments )
		{
			//remove the older couple from the head
			points.pop();
			points.pop();
		}

		//get the current position, create a point couple and push it
		points.push( getWorldPosition( offset ) );
		points.push( getWorldPosition( offset*-1 ) );
	}

	elapsedTime += dt;
}