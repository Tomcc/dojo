#ifndef Tessellation_h__
#define Tessellation_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{
	
	///A Tessellation is a 2D triangle mesh created by the tessellation of an area enclosed by edges using Constrained Delaunay Triangulation
	/**
	The tessellation class can tessellate any given countour mesh that is initialized in its "positions" and "indices" arrays.

	\remark if a contour is not closed or one or more contours intersect, the results of the tessellation are undefined
	*/
	class Tessellation
	{
	public:

		struct Position
		{
			double x, y;

			Position( const Vector& p ) :
				x( p.x ),
				y( p.y )
			{

			}

			Position( double X, double Y ) : 
				x( X ),
				y( Y )
			{

			}

			Vector toVec()
			{
				return Vector( (float)x, (float)y );
			}
		};

		struct Segment
		{
			int i1, i2;

			Segment( int a, int b ) :
				i1( a ),
				i2( b )
			{
				DEBUG_ASSERT( a >= 0, "Invalid negative index" );
				DEBUG_ASSERT( b >= 0, "Invalid negative index" );
				DEBUG_ASSERT( a != b, "A segment can't start and end at the same vertex" );
			}
		};

		///a Loop defines a closed circuit of segments using their start and end index-indices
		struct Contour
		{
			std::vector< int > indices;

			int parity;
			bool closed;

			Contour( ) :
				parity( -1 ),
				closed( false )
			{

			}

			///adds a segment to this contour, marks it as closed if end == start
			void _addSegment( int start, int end )
			{
				indices.push_back( start );
				indices.push_back( end );
				
				closed = (end == indices.front());
			}
		};

		typedef std::vector< Contour > ContourList;

		//in
		std::vector< Position > positions;
		std::vector< Segment > segments;
		std::vector< unsigned int > colors; //HACK

		//mid
		ContourList contours;
		std::vector< int > contourForSegment;
		std::vector< Position > holes;

		//out
		std::vector< int > outIndices;

		///Creates an empty 2D Tesselation object
		Tessellation()
		{

		}

		///Adds a 2D point to the tessellation contour
		void addPoint( const Vector& p )
		{
			positions.push_back( p );
			colors.push_back( 0xffff0000 ); //HACK
		}

		///adds a point and the indices to construct a single segment starting from the last point
		void addSegment( const Vector& p )
		{
			int idx = positions.size()-1;
			addPoint( p );

			//add indices to the point
			segments.push_back( Segment( idx, idx+1 ) );
		}

		///adds a quadratic bezier curve (single control point) starting from the last point
		void addQuadradratic( const Vector& B, const Vector& C, float pointsPerUnitLength )
		{
			Vector U, V, A = positions.back().toVec();

			//TODO actually add points evaluating the "curvyness" of the path
			float length = A.distance( B ) + B.distance( C ); //compute a rough length of this arc
			int subdivs = (int)(length * pointsPerUnitLength + 1);

			for(int i = 1; i <= subdivs; i++)
			{
				float t = (float)i / subdivs;
				
				U = A.lerpTo( t, B );
				V = B.lerpTo( t, C );

				addSegment( U.lerpTo( t, V ) );
			}
		}

		///adds a cubic bezier curve (double control point) starting from the last point
		void addCubic( const Vector& B, const Vector& C, const Vector& D, float pointsPerUnitLength )
		{
			Vector U,V,W,M,N, A = positions.back().toVec();
			
			//TODO actually add points evaluating the "curvyness" of the path
			float length = A.distance( B ) + B.distance( C ) + C.distance( D ); //compute a rough length of this arc
			int subdivs = (int)(length * pointsPerUnitLength + 1);

			for( int i = 0; i <= subdivs; i++)
			{
				float t = (float)i / subdivs;

				U = (1.0f - t) * A + t * B;
				V = (1.0f - t) * B + t * C;
				W = (1.0f - t) * C + t * D;

				U = A.lerpTo( t, B );
				V = B.lerpTo( t, C );
				W = C.lerpTo( t, D );

				M = U.lerpTo( t, V );
				N = V.lerpTo( t, W );

				addSegment( M.lerpTo( t, N ) );
			}
		}
		
		///removes i2 from the point list and rearranges all the indices to point to i1
		void mergePoints( int i1, int i2 );

		///merges all the points that share the same position
		/**
		this method will be automatically run by tessellate() as the triangulation algorithm doesn't allow for duplicate points
		*/
		void mergeDuplicatePoints();

		///builds the internal "loops" structure, representing all the contours of this tessellation
		/**
		each loop contains a copy of all of its segments
		*/
		void findContours();

		///tessellates the countour mesh producing a triangle mesh
		/**
		\param clearInputs auto-clears the input vectors
		*/
		void tessellate( bool clearInputs = true );

	protected:

		bool _raycastSegmentAlongX( const Segment& segment, const Position& startPosition );

		int _assignToIncompleteContour( int start, int end );
	};
}

#endif // Tesselation_h__
