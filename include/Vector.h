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
		static const Vector UNIT_Z;
		static const Vector ONE;
		
		float x, y, z;
		
		Vector() 
		{
			x = y = z = 0;
		}
		
		Vector( float X, float Y ) 
		{
			x = X;
			y = Y;
			z = 0;
		}

		///still available in 2D for compatibility
		Vector( float X, float Y, float Z ) 
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

		inline Vector operator - ( const Vector& v ) const 
		{
			return Vector( x - v.x, y - v.y, z - v.z );
		}

		inline Vector operator * ( float s ) const 
		{			
			return Vector( x * s, y * s, z * s );
		}

		inline Vector operator / ( float s ) const 
		{
			return Vector( x/s, y/s, z/s );
		}

		inline float operator * ( const Vector& v ) const 
		{
			return x*v.x + y*v.y + z*v.z;
		}

		inline float lenght() const 
		{
			return sqrt( lenghtSquared() );
		}

		inline float lenghtSquared() const 
		{
			return (*this)*(*this);
		}
		
		inline const Vector& normalize()
		{
			float s = 1.f / lenght();
			x *= s;
			y *= s;
			z *= s;

			return *this;
		}	

		inline const Vector& reflect( const Dojo::Vector& normal )
		{
			float dot = 2.f * ( *this * normal );
			x -= dot * normal.x;
			y -= dot * normal.y;
			z -= dot * normal.z;

			return *this;
		}
		
		inline float distance( const Vector& v ) const 
		{
			return sqrt( distanceSquared(v) );
		}
		
		inline float distanceSquared( const Vector& v ) const
		{
			return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z);
		}
		
		inline bool isNear( const Vector& v, float threshold = 0.1 )
		{
			return distanceSquared( v ) < threshold*threshold;
		}

		inline std::ostream& writeToStream( std::ostream& str )
		{
			str << x << ' ' << y << ' ' << z << ' ';

			return str;
		}

		inline void readFromStream( std::istream& str )
		{
			str >> x;
			str >> y;
			str >> z;
		}
				
	protected:
	};
}

#endif