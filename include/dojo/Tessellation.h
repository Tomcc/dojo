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

		///tessellates the countour mesh producing a triangle mesh
		/**
		\param clearInputs auto-clears the input vectors
		*/
		void tessellate( bool clearInputs = true );
	};
}

#endif // Tesselation_h__
