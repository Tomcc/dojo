/*
 *  dojomath.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/15/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Math_h__
#define Math_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo 
{
	class Math
	{	
	public:
		
		static const float PI;
		static const float EULER_TO_RADIANS;
		static const float RADIANS_TO_EULER;
		
		static inline bool isLittleEndian()
		{
			unsigned short x = 0x0001;
			return *((byte*)&x) != 0;
		}
		
		//seeds the random either using the given seed, or current time if omitted
		static void seedRandom( uint seed = 0 );
		
		static float random();
		static float rangeRandom( float min, float max );
		static Vector randomVector( const Vector& min, const Vector& max );

		static Vector randomVector( float min, float max )
		{
			return randomVector( Vector(min,min,min), Vector(max,max,max) );
		}
		
		static inline float toRadian( float euler )
		{
			return euler * EULER_TO_RADIANS;
		}
		
		static inline float toEuler( float radian )
		{
			return radian * RADIANS_TO_EULER;
		}
		
		static inline float sign( float f )
		{
			if( f > 0 )
				return 1;
			else if( f < 0 )
				return -1;
			else
				return 0;
		}

		static inline float min( float a, float b )
		{
			return (a < b) ? a : b;
		}

		static inline float max( float a, float b )
		{
			return (a > b) ? a : b;
		}

		static inline float clamp( float n, float max, float min )
		{
			DEBUG_ASSERT( max >= min );
			
			if( n > max )	return max;
			if( n < min )	return min;
			return n;
		}
		
		static inline float saturate( float n )
		{
			return clamp( n, 1, 0 );
		}

		static inline uint nextPowerOfTwo( uint val )
		{
			--val;
			val = (val >> 1) | val;
			val = (val >> 2) | val;
			val = (val >> 4) | val;
			val = (val >> 8) | val;
			val = (val >> 16) | val;
			return ++val; 
		}
		
		static inline float lerp( float a, float b, float s )
		{
			return a * s + b * (1.f-s);
		}
		
		static inline bool isNear( float a, float b, float bias )
		{
			return a < b + bias && a > b - bias;
		}
		
		static inline float abs( float a )
		{
			return ( a >= 0 ) ? a : -a;
		}
		
		static inline bool AABBContains( const Vector& max, const Vector& min, const Vector& point )
		{
			DEBUG_TODO;
		}
		
		static inline bool AABBContainsAABB( const Vector& maxA, const Vector& minA, const Vector& maxB, const Vector& minB )
		{
			return AABBContains( maxA, minA, maxB ) && AABBContains( maxA, minA, minB );
		}
		
		static inline bool AABBsCollide( const Vector& maxA, const Vector& minA, const Vector& maxB, const Vector& minB )
		{
			float cx = maxA.x - minB.x;
			float cy = maxA.y - minB.y;
			float cz = maxA.z - minB.z;

			return 
				!(cx > (maxA.x - minA.x) + (maxB.x - minB.x) || cx < 0) &&
				!(cy > (maxA.y - minA.y) + (maxB.y - minB.y) || cy < 0) &&	
				!(cz > (maxA.z - minA.z) + (maxB.z - minB.z) || cz < 0);
		}
				
		static unsigned char rotateLeft(unsigned char n, unsigned char i)
		{  
			return (n << i) | (n >> (8 - i));
		}
		
		static unsigned char rotateRight(unsigned char n, unsigned char i)
		{  
			return (n >> i) | (n << (8 - i));
		}

	};
}

#endif

