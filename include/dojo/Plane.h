//
//  Plane.h
//  Dice
//
//  Created by Tommaso Checchi on 7/13/11.
//  Copyright 2011 none. All rights reserved.
//

#pragma once

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

		///Creates a plane centered in C and passing from points A and B
		Plane( const Vector& center, const Vector& A, const Vector& B )
		{
			setup( center, A, B );
		}
		
		///Creates a plane with the given normal and distance from the origin
		void setup( const Vector& normal, float distance )
		{
			n = normal;
			d = distance;
		}
		
		///Creates a plane centered in C and passing from points A and B
		void setup( const Vector& center, const Vector& A, const Vector& B )
		{
			n = ((A-center) ^ (B-center)).normalized();
			
			d = -(center * n);
		}

		///Returns the distance of a point from the given plane
		float getDistance( const Vector& point ) const
		{
			return point * n + d;
		}
				
		///tells which side of this plane the AABB lies in. -1 is negative, 0 is on the line, 1 is positive
		int getSide( const Vector& center, const Vector& halfsize ) const
		{
			float dist = getDistance( center );
			
			float maxAbsDist = n.absDot( halfsize );
			
			if (dist < -maxAbsDist)			
				return -1;
			else if (dist > +maxAbsDist)	
				return 1;
			else							
				return 0;
		}

		///returns the vector placed at the intersection between the line passing between these two points and the plane, or false if the line is parallel
		bool intersection( const Vector& origin, const Vector& direction, Vector& out )
		{
			Vector planeOrigin = n * -d;
			float num = (planeOrigin - origin) * n;
			float den = direction * n;

			if( num == 0 && den == 0 ) //no intersections
				return false;

			out = origin + direction * (num/den);
			return true;
		}
	};
}

