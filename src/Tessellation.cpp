#include "stdafx.h"

#include "Tessellation.h"
#include "dojomath.h"
#include "Timer.h"

#undef REAL

//the triangle library is included inline here
extern "C"
{
	//credits and documentation from http://www.cs.cmu.edu/~quake/triangle.html
	#include "triangle.c"
};

using namespace Dojo;

void Tessellation::tessellate( bool clearInputs /* = true */ )
{
	Timer timer;

	DEBUG_ASSERT( !positions.empty() && !indices.empty(), "Cannot tesselate an empty contour" );

	struct triangulateio in, out;

	std::vector< REAL > pointlist;
	std::vector< int > segmentmarkerlist;

	memset( &out, 0, sizeof(out) );

	in.numberofpointattributes = 0;
	in.pointmarkerlist = nullptr;
	in.numberofholes = 0;
	in.numberofregions = 0;
	
	//fill the points
	in.numberofpoints = positions.size();
	in.pointlist = (REAL*) malloc( in.numberofpoints * 2 * sizeof( REAL ) );
	for( int i = 0; i < in.numberofpoints; ++i )
	{
		in.pointlist[i*2] = positions[i].x;
		in.pointlist[i*2+1] = positions[i].y;
	}

	//fill in the edges
	in.numberofsegments = indices.size()/2;
	in.segmentlist = (int*) malloc( indices.size() * sizeof( int ) );
	in.segmentmarkerlist = nullptr;

	memcpy( in.segmentlist, indices.data(), indices.size() * sizeof( int ) );

	/*for( int i = 0; i < in.numberofsegments; ++i ) //all the segments we're adding are boundary segments (1)
		segmentmarkerlist.push_back( 1 );

	in.segmentmarkerlist = segmentmarkerlist.data();*/
	//out

	//z - indices numbered from 0
	//Q - no printf
	//p - triangulates "in"
	//c - enclose the convex hull with segments
	//V - verbose

	triangulate( "pzQ", &in, &out, nullptr );

	//grab positions
	for( int i = 0; i < out.numberofpoints; ++i )
		outPositions.push_back( Vector( out.pointlist[i*2], out.pointlist[i*2+1] ) );

	//grab triangle indices, inverting those as they are counter-clockwise 
	DEBUG_ASSERT( out.numberofcorners == 3, "The tesselated triangles don't have 3 corners, deal with it");
	for( int i = 0; i < out.numberoftriangles*3; ++i )
		outIndices.push_back( out.trianglelist[i] );

	DEBUG_MESSAGE( "Tesselated a character in " << timer.deltaTime() * 1000. << " ms" );

	if( clearInputs )
	{
		positions.clear();
		indices.clear();
	}
}
