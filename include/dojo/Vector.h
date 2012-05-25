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

using namespace std;

namespace Dojo	
{
	class Vector : public glm::vec3
	{
	public:
		
		static const Vector ZERO;
		static const Vector UNIT_X;
		static const Vector NEGATIVE_UNIT_X;
		static const Vector UNIT_Y;
		static const Vector NEGATIVE_UNIT_Y;
		static const Vector UNIT_Z;
		static const Vector NEGATIVE_UNIT_Z;
		static const Vector ONE;
		static const Vector MAX, MIN;
		
		Vector( float f = 0 ) : glm::vec3(f)
		{
            
		}
        		
		Vector( float X, float Y ) : glm::vec3( X, Y, 0 )
		{
            
		}
        
        Vector( float x, float y, float z ) : glm::vec3( x,y,z )
        {
            
        }
        
        Vector( const glm::vec3& v ) : glm::vec3( v )
        {
            
        }
		
		/*inline const Vector& operator = ( const Vector& v )
		{
			x = v.x;
			y = v.y;
			z = v.z;
			
			return *this;
		}*/
				
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
		
		/*inline const Vector& operator *= ( float s )
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
		}*/

		inline Vector operator - ( const Vector& v ) const 
		{
			return Vector( x - v.x, y - v.y, z - v.z );
		}

		inline Vector operator * ( float s ) const 
		{			
			return Vector( x * s, y * s, z * s );
		}

		/*inline Vector operator / ( float s ) const 
		{
			return Vector( x/s, y/s, z/s );
		}*/

		inline float operator * ( const Vector& v ) const 
		{
			return glm::dot( (const glm::vec3&)(*this), (const glm::vec3&)v );
		}

		inline Vector operator ^ ( const Vector& v ) const 
		{
			return glm::cross( *this, v );
		}
		
		inline const float normalize()
		{
			float l = length();
			float s = 1.f / l;
			x *= s;
			y *= s;
			z *= s;

			return l;
		}	
		
		inline Vector normalized() const
		{
			Vector v = *this;
			v.normalize();
			
			return v;
		}
		
		inline Vector lerp( float s, const Dojo::Vector& v )
		{			
			float invs = 1.f - s;			
			return Vector( x*s + invs*v.x, y*s + invs*v.y, z*s + invs*v.z);
		}

		/*inline const Vector& reflect( const Dojo::Vector& normal )
		{
			float dot = 2.f * ( *this * normal );
			x -= dot * normal.x;
			y -= dot * normal.y;
			z -= dot * normal.z;

			return *this;
		}*/

		inline const Vector& scale( const Vector& scale )
		{
			x *= scale.x;
			y *= scale.y;
			z *= scale.z;

			return *this;
		}
        
        ///returns a vector with abs componenents
        inline Vector absComponents()
        {
            return Vector( abs(x), abs(y), abs(z) );
        }
        
        ///multiplies component by component
        inline Vector mulComponents( const Vector& v )
        {
            return Vector( x*v.x, y*v.y, z*v.z );
        }

		inline float distance( const Vector& v ) const 
		{
			return sqrt( distanceSquared(v) );
		}
		
		inline float distanceSquared( const Vector& v ) const
		{
			return (x-v.x)*(x-v.x) + (y-v.y)*(y-v.y) + (z-v.z)*(z-v.z);
		}
		
		///returns the distance of this vector from the given plane
		inline float distanceFromPlane( const Vector& normal, float d ) const 
		{
			return glm::dot((const glm::vec3&)(*this), (const glm::vec3&)normal) + d;
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