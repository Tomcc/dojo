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

namespace Dojo 
{
	class Noise
	{
	public:
		
		Noise( long seed ) :
		usedSeed( seed ),
		lastSeed( usedSeed )
		{			
			for( int i = 0; i < 256; ++i )
				p[i] = nextInt(0, 256);
			
			for (int i = 0; i < 256 ; i++) 
				p[256+i] = p[i];
		}
			
		long getUsedSeed() 
		{
			return usedSeed;
		}
		
		bool chance( double d ) 
		{
			return nextFloat() < d;
		}	
		
		///the "base" random - returns an int raging from 0 to RAND_MAX - and saves the context
		int nextInt()
		{
			//srand( lastSeed );			
			return lastSeed = rand();
		}
		
		int nextInt( int min, int max )
		{
			return min + (int)((float)(max-min)*nextFloat());
		}
		
		float nextFloat()
		{
			return (float)nextInt()/(float)RAND_MAX;
		}
		
		float nextFloat( float min, float max )
		{
			return min + (max-min)*nextFloat();
		}		
		
		double perlinNoise(double x, double y, double z) 
		{
			int X = (int)floor(x) & 255,                  // FIND UNIT CUBE THAT
			Y = (int)floor(y) & 255,                  // CONTAINS POINT.
			Z = (int)floor(z) & 255;
			x -= floor(x);                                // FIND RELATIVE X,Y,Z
			y -= floor(y);                                // OF POINT IN CUBE.
			z -= floor(z);
			
			double u = fade(x),                                // COMPUTE FADE CURVES
			v = fade(y),                                // FOR EACH OF X,Y,Z.
			w = fade(z);
			int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
			B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,
			
			return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
										grad(p[BA  ], x-1.0, y  , z   )), // BLENDED
								lerp(u, grad(p[AB  ], x  , y-1.0, z   ),  // RESULTS
                                     grad(p[BB  ], x-1.0, y-1.0, z   ))),// FROM  8
						lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1.0 ),  // CORNERS
                                     grad(p[BA+1], x-1.0, y  , z-1.0 )), // OF CUBE
                             lerp(u, grad(p[AB+1], x  , y-1.0, z-1.0 ),
								  grad(p[BB+1], x-1.0, y-1.0, z-1.0 ))));
		}   
		
		double noise( double x, double y, double z, double scale ) 
		{
			return 0.5 * scale * perlinNoise( x/scale, y/scale, z/scale );
		}
		
		double noise( double x, double y, double scale ) 
		{
			return noise( x, y, scale, scale );
		}
		
		double filternoise( double x, double y, double z, double scale ) 
		{
			return 0.5 * ( 1.0 + perlinNoise( x/scale, y/scale, z/scale ) );
		}
		
		double filternoise( double x, double y, double scale ) 
		{
			return filternoise( x, y, scale, scale );
		}
				
	protected:		
		
		int p[512];
		unsigned int usedSeed, lastSeed;
		
		double fade(double t) { return t * t * t * (t * (t * 6.0 - 15.0) + 10.0); }
		double lerp(double t, double a, double b) { return a + t * (b - a); }
		double grad(int hash, double x, double y, double z) {
			int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
			double u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
			v = h<4 ? y : h==12||h==14 ? x : z;
			return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
		}
	};
}

#endif
