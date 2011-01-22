/*
 *  Vector.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef Vector_h__
#define Vector_h__

#include "dojo_common_header.h"

namespace Dojo	
{
	class Vector 
	{
	public:
		
		static const Vector ZERO;
		static const Vector UNIT_X;
		static const Vector UNIT_Y;
		static const Vector ONE;
		
		float x, y, z;
		
		Vector() 
		{
			x = y = z = 0;
		}
		
		Vector( float X, float Y, float Z = 0 ) 
		{
			x = X;
			y = Y;
			z = Z;
		}	
		
		inline const Vector& operator = ( const Vector& v )
		{
			x = v.x;
			y = v.y;
			z = v.z;
			
			return *this;
		}
				
		inline const Vector& operator += ( const Vector& v )
		{
			x += v.x;
			y += v.y;
			z += v.z;
			
			return *this;
		}
		
		inline const Vector& operator -= ( const Vector& v )
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			
			return *this;
		}
		
		inline const Vector& operator *= ( float s )
		{
			x *= s;
			y *= s;
			z *= s;
			
			return *this;
		}
		
		inline const Vector& operator /= ( float s )
		{
			x /= s;
			y /= s;
			z /= s;
			
			return *this;
		}
		
		inline Vector operator + ( const Vector& v ) const 
		{
			return Vector( x + v.x, y + v.y, z + v.z );
		}
		
		inline const Vector& normalize()
		{
			float s = 1.f / sqrt( x*x + y*y + z*z );
			x *= s;
			y *= s;
			z *= s;
			
			return *this;
		}	
		
		inline float distance( const Vector& v )
		{
			return sqrt( (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z) );
		}
		
		inline float distanceSquared( const Vector& v )
		{
			return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z);
		}
		
		inline bool isNear( const Vector& v, float threshold = 0.1 )
		{
			return distanceSquared( v ) < threshold*threshold;
		}
				
	protected:
	};
}

#endif