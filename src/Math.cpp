#include "stdafx.h"

#include "dojomath.h"
#include "Random.h"

using namespace Dojo;

const float Math::PI = 3.14159265f;
const float Math::TAU = 6.2831853071796f;
const float Math::EULER_TO_RADIANS = 0.01745329251994329f;
const float Math::RADIANS_TO_EULER = 57.295779513082325f;

static Random randomImpl;

void Math::seedRandom(unsigned int seed)
{
	if( !seed )
		seed = (unsigned int) time(NULL);

	randomImpl = Random(seed);
}

float Math::random()
{
	return (float)randomImpl.rand();
}

bool Math::oneEvery(int n) 
{
	return randomImpl.randInt(n) == 0;
}

float Math::rangeRandom( float min, float max )
{
	DEBUG_ASSERT( min <= max, "The min end of a random range must be less or equal than the max end" );

	return (float)randomImpl.rand(min, max);
}

Vector Math::randomVector( const Vector& min, const Vector& max )
{
	return Vector(
		rangeRandom( min.x,max.x ), 
		rangeRandom( min.y,max.y ), 
		rangeRandom( min.z,max.z ) );
}

Vector Math::randomVector2D( const Vector& min, const Vector& max, float z )
{
	return Vector(
		rangeRandom( min.x,max.x ), 
		rangeRandom( min.y,max.y ), 
		z );
}

Vector Math::randomUnit2D() {
	auto a = Math::rangeRandom(0, Math::TAU);

	return Vector(cosf(a), sinf(a));
}