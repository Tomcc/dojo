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

#include <cmath>

using namespace std;

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
		
		inline bool operator == ( const Vector& v ) const
		{
			return x == v.x && y == v.y && z == v.z;
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

		inline Vector operator ^ ( const Vector& v ) const 
		{
			return Vector( 
				y * v.z - z * v.y,
				z * v.x - x * v.z,
				x * v.y - y * v.x );	
		}

		inline const Vector& set( float X, float Y, float Z )
		{
			x = X;
			y = Y;
			z = Z;

			return *this;
		}

		inline float lenght() const 
		{
			return sqrtf( lenghtSquared() );
		}

		inline float lenghtSquared() const 
		{
			return x*x+y*y+z*z;
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

		inline const Vector& scale( const Vector& scale )
		{
			x *= scale.x;
			y *= scale.y;
			z *= scale.z;

			return *this;
		}

		///rotate the vector around side axis
		/**
		http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
		*/
		inline const Vector& pitch( float angle )
		{
			float c = cos( angle * 0.0174f );
			float s = sin( angle * 0.0174f ); 
			//Vector cross( 0, -z, y )
			
			//x is not varied
			set( x, y*c - z*s, z*c + y*s);
			
			return *this;
		}

		//rotate the vector around up axis
		inline const Vector& yaw( float angle )
		{
			float c = cos( angle * 0.0174f );
			float s = sin( angle * 0.0174f ); 
			//vector cross( z, 0, -x )

			set( x*c + z*s, y, z*c - x*s );
			
			return *this;
		}

		///rotation along the forward axis
		inline const Vector& roll( float angle )
		{
			float c = cos( angle * 0.0174f );
			float s = sin( angle * 0.0174f ); 
			//vector cross( -y, x, 0 )

			set( x*c - y*s, y*c + x*s, z );

			return *this;
		}
		
		inline const Vector& rotate( const Vector& angles )
		{
			pitch( angles.x );
			yaw( angles.y );

			return *this;
		}

		///uses the current vector as three euler angles and returns their direction
		inline Vector directionFromAngle()
		{
			Vector forward( 0,0,1 );

			return forward.rotate( *this );
		}

		///uses the current vector to return three (two, really) euler angles that give its rotation
		/**
		Rotation is relative to the "forward" vector (0,0,1)
		*/
		inline Vector angleFromDirection()
		{
			return Vector(
					57.47f * atan2(x, y),
					57.47f * atan2(z,sqrt((x*x) + (y*y)) ),
					0 );
		}

		inline float distance( const Vector& v ) const 
		{
			return sqrt( distanceSquared(v) );
		}
		
		inline float distanceSquared( const Vector& v ) const
		{
			return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z);
		}
		
		inline float distanceFromPlane( const Vector& normal, float d ) const 
		{
			return *this * normal + d;
		}

		inline bool isNear( const Vector& v, float threshold = 0.1 )
		{
			return distanceSquared( v ) < threshold*threshold;
		}

		inline float absDot( const Vector& v ) const 
		{
			float a = x * v.x;
			a = (a < 0) ? -a : a;

			float b = y * v.y;
			b = (b < 0) ? -b : b;

			float c = z * v.z;
			c = (c < 0) ? -c : c;

			return a + b + c;
		}
				
	protected:
	};
}

#endif