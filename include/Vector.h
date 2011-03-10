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
		
		float x, y;
		
		Vector() 
		{
			x = y = 0;
		}
		
		Vector( float X, float Y ) 
		{
			x = X;
			y = Y;
		}	
		
		inline const Vector& operator = ( const Vector& v )
		{
			x = v.x;
			y = v.y;
			
			return *this;
		}
				
		inline const Vector& operator += ( const Vector& v )
		{
			x += v.x;
			y += v.y;
			
			return *this;
		}
		
		inline const Vector& operator -= ( const Vector& v )
		{
			x -= v.x;
			y -= v.y;
			
			return *this;
		}
		
		inline const Vector& operator *= ( float s )
		{
			x *= s;
			y *= s;
			
			return *this;
		}
		
		inline const Vector& operator /= ( float s )
		{
			x /= s;
			y /= s;
			
			return *this;
		}
		
		inline Vector operator + ( const Vector& v ) const 
		{
			return Vector( x + v.x, y + v.y );
		}

		inline Vector operator - ( const Vector& v ) const 
		{
			return Vector( x - v.x, y - v.y );
		}

		inline Vector operator * ( float s ) const 
		{
			return Vector( x*s, y*s );
		}

		inline Vector operator / ( float s ) const 
		{
			float inv = 1.f/s;
			return Vector( x*inv, y*inv );
		}

		inline float operator * ( const Vector& v ) const 
		{
			return x*v.x + y*v.y;
		}

		inline bool operator == ( const Vector& v ) const
		{
			return x == v.x && y == v.y;
		}

		inline float lenght() const 
		{
			return sqrt( x*x + y*y );
		}

		inline float lenghtSquared()
		{
			return x*x + y*y;
		}
		
		inline const Vector& normalize()
		{
			float s = 1.f / sqrt( x*x + y*y );
			x *= s;
			y *= s;
			
			return *this;
		}	

		inline const Vector& reflect( const Dojo::Vector& normal )
		{
			float dot = 2.f * (x*normal.x + y*normal.y);
			x -= dot * normal.x;
			y -= dot * normal.y;

			return *this;
		}
		
		inline float distance( const Vector& v ) const 
		{
			return sqrt( (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) );
		}
		
		inline float distanceSquared( const Vector& v ) const
		{
			return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y);
		}
		
		inline bool isNear( const Vector& v, float threshold = 0.1 )
		{
			return distanceSquared( v ) < threshold*threshold;
		}
				
	protected:
	};
}

#endif