//
//  Noiser.h
//  dojo
//
//  Created by Tommaso Checchi on 7/4/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef dojo_Noiser_h
#define dojo_Noiser_h

#include "dojo_common_header.h"

#include "dojomath.h"
#include "Random.h"

namespace Dojo 
{
	///Noise is a Perlin Noise utility implementation
	class Noise
	{
	public:
		
		///Creates a Noise object drawing numbers from the given Random generator
		/** 
		this allows to use the same random to obtain the same Perlin distribution */
		Noise( Random& rand )
		{
			seed( rand );
		}

		///Creates a Noise object drawing numbers from a new random number generator
		Noise( unsigned int seedInt )
		{
			Random r( seedInt );
			seed( r );
		}

		///creates a new noise object initialized with the default seeding method
		Noise()
		{
			Random gen;
			seed( gen );
		}

		void seed( Random& rand )
		{
			for( int i = 0; i < 256; ++i )
				p[i] = i;

			//switch numbers around
			for( int i = 0; i < 256; ++i )
			{	
				int d = rand.randInt( 255 );

				int t = p[i];
				p[i] = p[d];
				p[d] = t;
			}

			for (int i = 0; i < 256 ; i++) 
				p[256+i] = p[i];
		}
			
		///returns the Perlin noise at point x,y,z
		float perlinNoise(float x, float y, float z) 
		{
			int X = (int)floor(x) & 255,                  // FIND UNIT CUBE THAT
			Y = (int)floor(y) & 255,                  // CONTAINS POINT.
			Z = (int)floor(z) & 255;
			x -= floor(x);                                // FIND RELATIVE X,Y,Z
			y -= floor(y);                                // OF POINT IN CUBE.
			z -= floor(z);
			
			float u = fade(x),                                // COMPUTE FADE CURVES
			v = fade(y),                                // FOR EACH OF X,Y,Z.
			w = fade(z);
			int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
			B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,
			
			return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
										grad(p[BA  ], x-1.0f, y  , z   )), // BLENDED
								lerp(u, grad(p[AB  ], x  , y-1.0f, z   ),  // RESULTS
									 grad(p[BB  ], x-1.0f, y-1.0f, z   ))),// FROM  8
						lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1.0f ),  // CORNERS
									 grad(p[BA+1], x-1.0f, y  , z-1.0f )), // OF CUBE
							 lerp(u, grad(p[AB+1], x  , y-1.0f, z-1.0f ),
								  grad(p[BB+1], x-1.0f, y-1.0f, z-1.0f ))));
		}   
		
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
		
		float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }
		float lerp(float t, float a, float b) { return a + t * (b - a); }
		float grad(int hash, float x, float y, float z) {
			int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
			float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? y : h==12||h==14 ? x : z;
			return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
		}
	};
}

#endif
