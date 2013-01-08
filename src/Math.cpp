#include "stdafx.h"

#include "dojomath.h"

using namespace Dojo;

const float Math::PI = 3.14159265f;
const float Math::TAU = 6.2831853071796f;
const float Math::EULER_TO_RADIANS = 0.01745329251994329f;
const float Math::RADIANS_TO_EULER = 57.295779513082325f;

void Math::seedRandom( uint seed )
{
	if( !seed )
		seed = (uint)time( NULL );

	srand( seed );
}

float Math::random()
{
	return (float)rand()/(float)RAND_MAX;
}

float Math::rangeRandom( float min, float max )
{
	return ((float)rand()/(float)RAND_MAX)*(max-min) + min;
}

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

Vector Math::randomVector2D( const Vector& min, const Vector& max, float z )
{
	DEBUG_ASSERT( min.x < max.x );
	DEBUG_ASSERT( min.y < max.y );

	return Vector(
		rangeRandom( min.x,max.x ), 
		rangeRandom( min.y,max.y ), 
		z );
}