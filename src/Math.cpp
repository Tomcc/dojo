#include "stdafx.h"

#include "dojomath.h"

using namespace Dojo;

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
