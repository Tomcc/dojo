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

void Tessellation::mergePoints( int i1, int i2 )
{
	//remove i2 from the list
	positions.erase( positions.begin() + i2 );

	//replace all the occurrences of i2 with i1; move all the indices > i2 down by one
	for( auto& idx : indices )
	{
		if( idx == i2 )
			idx = i1;
		else if( idx > i2 )
			--idx;
	}
}

void Tessellation::mergeDuplicatePoints()
{
	//TODO: make faster? it is already a quite specific/fast NN implementation but takes 4x Delaunay
	std::unordered_map< Vector, int > positionToIndexMap;

	//iterate over all of the points
	for( int i = 0; i < positions.size(); ++i )
	{
		auto& p = positions[i];
		//try to find the new position in the map
		auto elem = positionToIndexMap.find( p );

		//if it isn't in the set, add it with its current index
		if( elem == positionToIndexMap.end() )
			positionToIndexMap[ p ] = i;

		else //discard this point and map all the existing segments to its replacement
			mergePoints( elem->second, i-- );
	}
}

Tessellation::LoopList Tessellation::findLoops()
{
	//a loop is defined by a list of segments which start and end at the same index
	LoopList loops;

	//build a "streaks" list containing where "streaks" of concatenated segments start and end
	struct Streak
	{
		int start, end, startIdx, endIdx;
	};

	std::vector< Streak > streaks;

	Streak currentStreak;
	currentStreak.start = 0;
	currentStreak.startIdx = indices[ currentStreak.start ];
	
	int prevEnd = indices[1];
	for( int i = 2; i < indices.size(); i += 2 )
	{
		int start = indices[i];

		if( start != prevEnd ) //streak ended!
		{
			currentStreak.end = i-1;
			currentStreak.endIdx = prevEnd;

			streaks.push_back( currentStreak ); 
			
			//init the streak again
			currentStreak.start = i; 
			currentStreak.startIdx = start;
		}

		prevEnd = indices[ i+1 ]; 
	}

	//try to concatenate streaks and get loops

	return loops;
}

void Tessellation::tessellate( bool clearInputs /* = true */ )
{
	Timer timer;

	DEBUG_ASSERT( !positions.empty() && !indices.empty(), "Cannot tesselate an empty contour" );

	//remove duplicate points
	mergeDuplicatePoints();

//	LoopList loops = findLoops();

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
