/*
 *  Math.h
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
		
		//seeds the random either using the given seed, or current time if omitted
		static void seedRandom( uint seed = 0 );
		
		static float random();
		static float rangeRandom( float min, float max );
		
		static inline float sign( float f )
		{
			if( f > 0 )
				return 1;
			else if( f < 0 )
				return -1;
			else
				return 0;
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
		
		static inline bool AABBCollides( const Vector& centerA, const Vector& sizeA, const Vector& centerB, const Vector& sizeB )
		{
			float cx = centerA.x + sizeA.x*0.5f - centerB.x + sizeB.x*0.5;
			float cy = centerA.y + sizeA.y*0.5f - centerB.y + sizeB.y*0.5;
			
			return !(cx > sizeA.x + sizeB.x || cx < 0) && !(cy > sizeA.y + sizeB.y || cy < 0);		
		}
	};
}

#endif

