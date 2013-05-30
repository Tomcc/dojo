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
	for( auto& segment: segments )
	{
		if( segment.i1 == i2 )			segment.i1 = i1;
		else if( segment.i1 > i2 )		--segment.i1;

		if( segment.i2 == i2 )			segment.i2 = i1;
		else if( segment.i2 > i2 )		--segment.i2;
	}
}

void Tessellation::mergeDuplicatePoints()
{
	//TODO: make faster? it is already a quite specific/fast NN implementation but takes 4x Delaunay
	std::unordered_map< Vector, int > positionToIndexMap;

	//iterate over all of the points
	for( int i = 0; i < positions.size(); ++i )
	{
		auto p = positions[i].toVec();
		//try to find the new position in the map
		auto elem = positionToIndexMap.find( p );

		//if it isn't in the set, add it with its current index
		if( elem == positionToIndexMap.end() )
			positionToIndexMap[ p ] = i;

		else //discard this point and map all the existing segments to its replacement
			mergePoints( elem->second, i-- );
	}
}

int Tessellation::_assignToIncompleteContour( int start, int end )
{
	//look for an incomplete contour (still open) that ends with start
	for( int i = 0; i < contours.size(); ++i )
	{
		auto& cont = contours[i];
		if( !cont.closed && cont.indices.back() == start )
		{
			cont._addSegment( start, end );
			return i;
		}
	}

	//no existing contour was found, create a new one
	contours.resize( contours.size()+1 );
	contours.back()._addSegment( start, end );

	return contours.size()-1;
}

bool Tessellation::_raycastSegmentAlongX( const Segment& segment, const Position& startPosition )
{
	auto& start = positions[ segment.i1 ];
	auto& end = positions[ segment.i2 ];

	Vector max( std::max( start.x, end.x ), std::max( start.y, end.y ) );
	Vector min( std::min( start.x, end.x ), std::min( start.y, end.y ) );

	//early out: different y, or the segment is on the left of the start point, or parallel
	if( max.y == min.y || startPosition.y <= min.y || startPosition.y > max.y || startPosition.x > max.x )
		return false;

	//do the actual line-line test and find the distance to the starting point
	float x = ((startPosition.y - start.y) * (end.x - start.x)) / (end.y - start.x) + start.x - startPosition.x;

	return x > 0;
}

void Tessellation::findContours()
{
	//TODO sort segments? this might break if they are added in a unexpected manner?

	//rearrange all the indices in continuous contours	
	for( auto& segment : segments ) 
	{
		//look for a contour that ends with the index this one starts with
		//also assign the index of the contour to a backmap from segment to contour
		contourForSegment.push_back( _assignToIncompleteContour( segment.i1, segment.i2 ) );
	}

	//trim still incomplete contours, they're just useless as everything is "out" of them
	for( int i = 0; i < contours.size(); ++i )
	{
		if( !contours[i].closed )
			contours.erase( contours.begin() + i-- );
	}

	if( contours.size() == 1 )
		contours.begin()->parity = 0; //obviously parity 0
	else
	{
		//compute the parity of each contour
		for( int i = 0; i < contours.size(); ++i )
		{
			//choose a random point in the contour and start going right
			//count the number of intersections with the contour's segments to compute parity
			int intersections = 0;
			auto& contour = contours[i];
			auto& startPos = positions[ contour.indices[0] ];
			for( int j = 0; j < segments.size(); ++j )
			{
				if( _raycastSegmentAlongX( segments[j], startPos ) ) //has hit segment i, check to which contour it belongs
				{
					//HACK set in BLUE the vertices of the segment that has been hit
					colors[ segments[j].i1 ] = colors[ segments[j].i2 ] = 0xff0000ff;

					if( contourForSegment[j] != i ) //didn't hit the contour we're tracing for
						++intersections;
				}
			}

			contour.parity = intersections % 2;

			if( contour.parity == 1 )  //odd contour, add an hole to the right or left of the startpos using a slight delta
			{
				auto& endPos = positions[ contour.indices[1] ];
				Vector d = Vector( endPos.y - startPos.y, endPos.x - startPos.x ).normalized() * 0.001;

				holes.push_back( Position( startPos.x + d.x, startPos.y + d.y ) );
			}
		}
	}
}

void Tessellation::tessellate( bool clearInputs /* = true */ )
{
	DEBUG_ASSERT( !positions.empty() && !segments.empty(), "Cannot tesselate an empty contour" );

	//remove duplicate points
	mergeDuplicatePoints();

	findContours();

	static struct triangulateio in, out;

	memset( &out, 0, sizeof(out) );

	in.numberofpointattributes = 0;
	in.pointmarkerlist = nullptr;
	in.numberofholes = 0;
	in.numberofregions = 0;
	
	//fill the points
	in.numberofpoints = positions.size();
	in.pointlist = (REAL*)positions.data();

	//fill in the edges
	in.numberofsegments = segments.size();
	in.segmentlist = (int*)segments.data();
	in.segmentmarkerlist = nullptr;

	//fill in the holes
	in.numberofholes = holes.size();
	in.holelist = (REAL*)holes.data();

	//resize the indices to a "reasonable" max size //TODO find what is "reasonable"!
	outIndices.resize( 1000 );
	out.trianglelist = outIndices.data();

	//p - triangulates "in"
	//z - indices numbered from 0
	//Q - no printf
	//N - no memory is allocated for out.point* structures (we keep the same input points anyway)
	//B - no boundary markers (read: no out.segmentmarkerlist)
	//P - no out.segmentlist (we're not interested thanks)

	triangulate( "pzQNBP", &in, &out, nullptr );

	DEBUG_ASSERT( outIndices.size() >= out.numberoftriangles * 3, "didn't allocate enough space for the indices" );

	//resize to fit exactly the produced triangles
	outIndices.resize( out.numberoftriangles*3 );

	if( clearInputs )
	{
		positions.clear();
		segments.clear();
	}
}
