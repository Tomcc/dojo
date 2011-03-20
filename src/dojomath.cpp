#include "stdafx.h"

#include "dojomath.h"

using namespace Dojo;

const float Math::PI = 3.14159265f;
const float Math::EULER_TO_RADIANS = 0.01745329251994329f;
const float Math::RADIANS_TO_EULER = 57.295779513082325f;

Vector Math::randomVector( const Vector& min, const Vector& max )
{
	DEBUG_ASSERT( min.x < max.x );
	DEBUG_ASSERT( min.y < max.y );
	DEBUG_ASSERT( min.z < max.z );

	return Vector(
		rangeRandom( min.x,max.x ), 
		rangeRandom( min.y,max.y ), 
		rangeRandom( min.z,max.z ) );
}