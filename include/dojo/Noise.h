//
//  Noiser.h
//  dojo
//
//  Created by Tommaso Checchi on 7/4/11.
//  Copyright 2011 none. All rights reserved.
//

#pragma once

#include "dojo_common_header.h"

#include "dojomath.h"

namespace Dojo 
{
	class Random;

	///Noise is a Perlin Noise utility implementation
	class Noise
	{
	public:
		
		///Creates a Noise object drawing numbers from the given Random generator
		/** 
		this allows to use the same random to obtain the same Perlin distribution */
		Noise( Random& rand );

		///Creates a Noise object drawing numbers from a new random number generator
		Noise( size_t seedInt );

		///creates a new noise object initialized with the default seeding method
		Noise();

		void seed( Random& rand );
			
		///returns the Perlin noise at point x,y,z
		float perlinNoise(float x, float y, float z);   
		
		///returns the Perlin noise at position x,y,z with an octave given by "scale"
		/** 
		a bigger scale will generate bigger Perlin features */
		float noise( float x, float y, float z, float scale ) 
		{
			return 0.5f * scale * perlinNoise( x/scale, y/scale, z/scale );
		}
		
		///returns the Perlin noise at position x,y with an octave given by "scale"
		/** 
		a bigger scale will generate bigger Perlin features;
		z is used to return different "planes" at different scales, generating completely different 2D slices at each scale.*/
		float noise( float x, float y, float scale ) 
		{
			return noise( x, y, scale, scale );
		}
		
		///returns the Perlin noise at x,y,z, scaled down to 0..1 range
		float filternoise( float x, float y, float z, float scale ) 
		{
			return 0.5f * ( 1.0f + perlinNoise( x/scale, y/scale, z/scale ) );
		}

		///returns the Perlin noise at x,y, scaled down to 0..1 range
		float filternoise( float x, float y, float scale ) 
		{
			return filternoise( x, y, scale, scale );
		}

		///returns the Perlin noise at x, scaled down to 0..1 range
		float filternoise( float x, float scale ) 
		{
			return filternoise( x, x, x, scale );
		}
				
	protected:
		
		int p[512];
		
		float fade(float t);
		float lerp(float t, float a, float b);
		float grad(int hash, float x, float y, float z);
	};
}

