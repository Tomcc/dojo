#ifndef Tesselation_h__
#define Tesselation_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{
	
	///A Tesselation is a 2D triangle mesh created by the tesselation of an area enclosed by edges using Constrained Delaunay Triangulation
	/**
	The tesselation class can tesselate any given countour mesh that is initialized in its "positions" and "indices" arrays.
	*/
	class Tessellation
	{
	public:
		std::vector< Vector > positions, outPositions;
		std::vector< int > indices, outIndices;

		///Creates an empty 2D Tesselation object
		Tessellation()
		{

		}

		void addPoint( const Vector& p )
		{
			positions.push_back( p );
		}

		///adds a point and the indices to construct a single segment starting from the last point
		void addSegment( const Vector& p )
		{
			int idx = positions.size()-1;
			positions.push_back( p );

			//add indices to the point
			indices.push_back( idx );
			indices.push_back( idx+1 );
		}

		///adds a quadratic bezier curve (single control point) starting from the last point
		void addQuadradratic( const Vector& B, const Vector& C, float pointsPerUnitLength )
		{
			Vector U, V, A = positions.back();

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
			Vector U,V,W,M,N, A = positions.back();
			
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

		///tessellates the countour mesh producing a triangle mesh
		/**
		\param clearInputs auto-clears the input vectors
		*/
		void tessellate( bool clearInputs = true );
	};
}

#endif // Tesselation_h__
