#include "stdafx.h"

#include "Trail.h"

#include "FrameSet.h"
#include "GameState.h"

using namespace Dojo;

Trail::Trail( GameState* l, const Vector& pos, const std::string& texName /* = */ ) :
Renderable( l, pos ),
offset( Vector::UNIT_Y ),
updateInterval( 1 ),
autoUpdate( true ),
elapsedTime(0),
maxSegments(10)
{
	if( !texName.empty() )
		setTexture( l->getFrameSet(texName)->getFrame(0) );

	mesh = new Mesh(NULL);

	mesh->setDynamic(true);
	mesh->setVertexCap(100);
	mesh->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	mesh->setTriangleMode( Mesh::TM_STRIP );

	cullMode = CM_DISABLED;
}


void Trail::addSegment( const Vector& end1, const Vector& end2 )
{
	//is the trail full?
	if( points.size() >= maxSegments*2 )
	{
		//remove the older couple from the head
		points.pop();
		points.pop();
	}

	//get the current position, create a point couple and push it
	points.add( end1 );
	points.add( end2 );
}

void Trail::retesselate( const Vector& startpoint )
{
	DEBUG_ASSERT( points.size() );

	mesh->begin();

	mesh->vertex(startpoint);

	for( int i = 0; i < points.size(); ++i )
	{
		mesh->vertex( points.at(i) );
	}

	mesh->end();
}

void Trail::action( float dt )
{
if( isVisible() )
{
	worldPosition = realWorldPos;
	worldRotation = realWorldRot;
}

	Renderable::action(dt);

	realWorldPos = worldPosition;
	realWorldRot = worldRotation;

	if( autoUpdate )
	{
		if( elapsedTime >= updateInterval )
		{
			addSegment( 
					getWorldPosition( offset ),
					getWorldPosition( offset*-1 ) );

			elapsedTime = 0;
		}

		//show?
		setVisible( points.size() > 0 );

		if( isVisible() )
			retesselate( getWorldPosition() );
	}

	//force world position
	worldPosition = worldRotation = Vector::ZERO;

	elapsedTime += dt;
}