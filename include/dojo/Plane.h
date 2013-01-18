//
//  Plane.h
//  Dice
//
//  Created by Tommaso Checchi on 7/13/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef Dice_Plane_h
#define Dice_Plane_h

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{	
	///A geometric Plane implementation
	class Plane
	{
	public:
		
		Vector n;
		float d;
		
		///Empty (invalid) plane constructor
		Plane() :
		d( 0 )
		{
			
		}
		
		///Creates a plane with the given normal and distance
		Plane( const Vector& normal, float distance )
		{
			setup( normal, distance );	
		}
		
		///Creates a plane with the given normal and distance
		inline void setup( const Vector& normal, float distance )
		{
			n = normal;
			d = distance;
		}
		
		///Creates a plane centered in C and passing from points A and B
		inline void setup( const Vector& center, const Vector& A, const Vector& B )
		{
			n = ((A-center) ^ (B-center)).normalized();
			
			d = -( center * n);
		}
		
		///returns the distance of the plane from center
		inline float distance( const Vector& center )
		{
			return center.distanceFromPlane( n, d );
		}
				
		///tells which side of this plane the AABB lies in. -1 is negative, 0 is both, 1 is positive
		inline int getSide( const Vector& center, const Vector& halfsize )
		{
			float dist = center.distanceFromPlane( n, d );
			
			float maxAbsDist = n.absDot( halfsize );
			
			if (dist < -maxAbsDist)			
				return -1;
			else if (dist > +maxAbsDist)	
				return 1;
			else							
				return 0;
		}
	};
}

#endif
