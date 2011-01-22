/*
 *  Math.mm
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/15/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "dojomath.h"

#include <cstdlib>

using namespace Dojo;

void Math::seedRandom( uint seed )
{
	if( !seed )
		seed = time( NULL );
	
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

