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

	/*mesh = new Mesh(NULL);

	mesh->setDynamic(true);
	mesh->setVertexCap(100);
	mesh->setVertexFieldEnabled( Mesh::VF_POSITION3D );
	mesh->setVertexFieldEnabled( Mesh::VF_COLOR );
	mesh->setTriangleMode( Mesh::TM_STRIP );*/

	cullMode = CM_DISABLED;

	mesh = l->getMesh( "texturedQuad" );
}

Trail::~Trail()
{
	//delete mesh;
}

void Trail::addSegment( const Vector& end1, const Vector& end2 )
{
	//is the trail full?
	if( points.size() >= maxSegments*2 )
	{
		//remove the older couple from the head
		points.pop();
		points.pop();

		DEBUG_MESSAGE( this << " pop" );
	}
	else
		DEBUG_MESSAGE( this << " add" );

	//get the current position, create a point couple and push it
	points.add( end1 );
	points.add( end2 );
}

void Trail::retesselate( const Vector& startpoint )
{
	DEBUG_ASSERT( points.size() );

	mesh->begin();

	mesh->vertex(Vector::ZERO);

	for( int i = 0; i < points.size(); ++i )
	{
		mesh->vertex( points.at(i) );
		mesh->color( Color::WHITE );
	}

	mesh->end();
}

void Trail::onAction( float dt )
{
	Renderable::onAction(dt);

	if( autoUpdate )
	{
		//HACK
		/*if( elapsedTime >= updateInterval )
		{
			addSegment( 
					getWorldPosition( offset ),
					getWorldPosition( offset*-1 ) );

			elapsedTime = 0;
		}

		//show?
		setVisible( points.size() > 0 );

		if( isVisible() )
			retesselate( getWorldPosition() );*/
	}

	//force world position
	worldPosition = worldRotation = Vector::ZERO;

	/*scale.x = 1.f/parent->scale.x;
	scale.y = 1.f/parent->scale.y;
	scale.z = 1.f/parent->scale.z;*/

	elapsedTime += dt;
}