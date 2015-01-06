/*
 *  dojomath.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/15/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo 
{
	class Math
	{	
	public:
		
		static const float PI; ///well, pi
		static const float TAU; ///2*pi
		static const float EULER_TO_RADIANS;  ///magic number which multiplied to an euler value converts it to radians
		static const float RADIANS_TO_EULER;  ///magic number which multiplied to a radian value converts it to euler
		
		///returns if this machine is little endian
		static bool isLittleEndian()
		{
			unsigned short x = 0x0001;
			return *((byte*)&x) != 0;
		}
		
		///seeds the random either using the given seed, or current time if omitted
		static void seedRandom( unsigned int seed = 0 );
		
		///quick and dirty random (see Random class for a more reliable implementation)
		static float random();

		static bool oneEvery(int n); //returns true one time in n

		///returns a random float in the range [min...max[
		static float rangeRandom( float min, float max );

		///returns a random point in the given space rectangle
		static Vector randomVector( const Vector& min, const Vector& max );
		
		///returns a random point in the given space rectangle (2D)
		static Vector randomVector2D( const Vector& min, const Vector& max, float z = 0 );

		///returns a random unit vector in 2D
		static Vector randomUnit2D();

		static float toRadian( float euler )
		{
			return euler * EULER_TO_RADIANS;
		}
		
		static float toEuler( float radian )
		{
			return radian * RADIANS_TO_EULER;
		}
		
		static float sign( float f )
		{
			if( f > 0 )
				return 1;
			else if( f < 0 )
				return -1;
			else
				return 0;
		}

		static float min( float a, float b )
		{
			return (a < b) ? a : b;
		}

		static float max( float a, float b )
		{
			return (a > b) ? a : b;
		}

		static int min( int a, int b )
		{
			return (a < b) ? a : b;
		}

		static int max( int a, int b )
		{
			return (a > b) ? a : b;
		}
		
		///returns a vector which components are the component-wise max of a and b
		static Vector max( const Vector& a, const Vector& b ) 
		{
			return Vector( max( a.x,b.x ), max( a.y,b.y ), max( a.z, b.z ) );
		}
		
		///returns a vector which components are the component-wise min of a and b
		static Vector min( const Vector& a, const Vector& b )
		{
			return Vector( min( a.x,b.x ), min( a.y,b.y ), min( a.z, b.z ) );
		}

		///clamps n between max and min
		static float clamp( float n, float max, float min )
		{
			DEBUG_ASSERT( max >= min, "clamp: max must be >= than min" );
			
			if( n > max )	return max;
			if( n < min )	return min;
			return n;
		}
		
		static Vector clamp(const Vector& val, const Vector& max, const Vector& min)
		{
			return Vector(
				clamp(val.x, max.x, min.x),
				clamp(val.y, max.y, min.y),
				clamp(val.z, max.z, min.z));
		}


		///gets the nth bit in the mask i
		static bool getBit( int i, byte n )
		{
			DEBUG_ASSERT( n < 32, "getBit: byte number was more than 32" );
			
			return (i >> n) & 0x1;
		}
		
		///sets the nth bit of the mask i
		static int setBit( int i, int n, bool state )
		{
			DEBUG_ASSERT( n < 32, "setBit: byte number was more than 32" );
			
			if( state )
				return i | ( 0x1 << n );
			else
				return i & (~( 0x1 << n ));
		}

		///clamps in [0..1[
		static float saturate( float n )
		{
			return clamp( n, 1, 0 );
		}

		///returns the smallest power of two greater than val
		static unsigned int nextPowerOfTwo( unsigned int val )
		{
			--val;
			val = (val >> 1) | val;
			val = (val >> 2) | val;
			val = (val >> 4) | val;
			val = (val >> 8) | val;
			val = (val >> 16) | val;
			return ++val; 
		}
		
		static float lerp( float to, float from, float s )
		{
			return to * s + from * (1.f-s);
		}

		static Vector lerp(const Vector& to, const Vector& from, float s) {
			return Vector(
				lerp(to.x, from.x, s),
				lerp(to.y, from.y, s),
				lerp(to.z, from.z, s));
		}
		
		///returns if a is inside ]b-bias, b+bias[
		static bool isNear( float a, float b, float bias )
		{
			return a < b + bias && a > b - bias;
		}
		
		static float abs( float a )
		{
			return ( a >= 0 ) ? a : -a;
		}
		
		static bool AABBContains( const Vector& max, const Vector& min, const Vector& point )
		{
			return max.x >= point.x && max.y >= point.y && max.z >= point.z && min.x <= point.x && min.y <= point.y && min.z <= point.z;
		}
		
		static bool AABBContainsAABB( const Vector& maxA, const Vector& minA, const Vector& maxB, const Vector& minB )
		{
			return AABBContains( maxA, minA, maxB ) && AABBContains( maxA, minA, minB );
		}

		static bool segmentsOverlap(float maxA, float minA, float maxB, float minB) {
			float c = maxA - minB;
			return !(c > (maxA - minA) + (maxB - minB) || c < 0);
		}
		
		static bool AABBsCollide( const Vector& maxA, const Vector& minA, const Vector& maxB, const Vector& minB )
		{
			return
				segmentsOverlap(maxA.x, minA.x, maxB.x, minB.x) &&
				segmentsOverlap(maxA.y, minA.y, maxB.y, minB.y) &&
				segmentsOverlap(maxA.z, minA.z, maxB.z, minB.z);
		}

		static bool AABBsCollide2D(const Vector& maxA, const Vector& minA, const Vector& maxB, const Vector& minB)
		{
			return
				segmentsOverlap(maxA.x, minA.x, maxB.x, minB.x) &&
				segmentsOverlap(maxA.y, minA.y, maxB.y, minB.y);
		}
		
		///does a bitwise shift-with-rotation left on the byte n
		static unsigned char rotateLeft(unsigned char n, unsigned char i)
		{  
			return (n << i) | (n >> (8 - i));
		}

		///does a bitwise shift-with-rotation right on the byte n
		static unsigned char rotateRight(unsigned char n, unsigned char i)
		{  
			return (n >> i) | (n << (8 - i));
		}

	};
}

