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
#include "Radians.h"

namespace Dojo 
{
	class Math
	{	
	public:
		
		static const Radians PI; ///well, pi
		static const Radians TAU; ///2*pi

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
		
		///clamps n between max and min
		template<typename T>
		static T clamp( T n, T max, T min )
		{
			DEBUG_ASSERT( max >= min, "clamp: max must be >= than min" );
			
			return std::min(max, std::max(min, n));
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
			return clamp( n, 1.f, 0.f );
		}

		///returns the sign of val
		static float sign(float val) {
			if (val < 0)
				return -1.f;
			else if (val > 0)
				return 1.f;
			else
				return 0;
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

		static bool AABBContains2D(const Vector& max, const Vector& min, const Vector& point)
		{
			return max.x >= point.x && max.y >= point.y && min.x <= point.x && min.y <= point.y;
		}
		
		static bool AABBContainsAABB( const Vector& maxA, const Vector& minA, const Vector& maxB, const Vector& minB )
		{
			return AABBContains( maxA, minA, maxB ) && AABBContains( maxA, minA, minB );
		}

		static bool segmentsOverlap(float x2, float x1, float y2, float y1) {
			return x2 >= y1 && y2 >= x1;
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

		static Radians atan2(float y, float x) {
			return Radians(::atan2f(y, x));
		}
	};
}

