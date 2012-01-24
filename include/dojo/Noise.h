//All credits for Perlin Noise implementation go to the original author
//	Stefan Gustavson (stegu@itn.liu.se) @ http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/


#ifndef dojo_Noiser_h
#define dojo_Noiser_h

#include "dojo_common_header.h"

#include "dojomath.h"

#define FADE(t) ( t * t * t * ( t * ( t * 6 - 15 ) + 10 ) )

#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : ((int)x-1 ) )
#define LERP(t, a, b) ((a) + (t)*((b)-(a)))

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
				perm[i] = i;

			for( int i = 0; i < 512; ++i )
			{
				int i1 = nextInt( 0, 256 );
				int i2 = nextInt( 0, 256 );
				char temp = perm[ i1 ];
				perm[i1] = perm[ i2 ];
				perm[i2] = temp;
			}

			for( int i = 0; i < 256; ++i )
				perm[256+i] = perm[i];
		}
			
		long getUsedSeed() 
		{
			return usedSeed;
		}
		
		bool chance( float d ) 
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
						
		float noise( float x, float y, float scale ) 
		{
			x /= scale*0.5;
			y /= scale*0.5;
			
			int ix0, iy0, ix1, iy1;
			float fx0, fy0, fx1, fy1;
			float s, t, nx0, nx1, n0, n1;

			ix0 = FASTFLOOR( x ); // Integer part of x
			iy0 = FASTFLOOR( y ); // Integer part of y
			fx0 = x - ix0;        // Fractional part of x
			fy0 = y - iy0;        // Fractional part of y
			fx1 = fx0 - 1.0f;
			fy1 = fy0 - 1.0f;
			ix1 = (ix0 + 1) & 0xff;  // Wrap to 0..255
			iy1 = (iy0 + 1) & 0xff;
			ix0 = ix0 & 0xff;
			iy0 = iy0 & 0xff;
    
			t = FADE( fy0 );
			s = FADE( fx0 );

			nx0 = grad(perm[ix0 + perm[iy0]], fx0, fy0);
			nx1 = grad(perm[ix0 + perm[iy1]], fx0, fy1);
			n0 = LERP( t, nx0, nx1 );

			nx0 = grad(perm[ix1 + perm[iy0]], fx1, fy0);
			nx1 = grad(perm[ix1 + perm[iy1]], fx1, fy1);
			n1 = LERP(t, nx0, nx1);

			return ( LERP( s, n0, n1 ) ) * 0.665664f * scale;
		}
				
		float noise( float x, float y, float z, float scale ) 
		{
			DEBUG_TODO;
			return 0;
		}
		
		float filternoise( float x, float y, float z, float scale ) 
		{
			DEBUG_TODO;
			return 0;
		}
		
		float filternoise( float x, float y, float scale ) 
		{
			return (1.f + (noise( x, y, scale )/scale))*0.5f;
		}
				
	protected:
		
		char perm[512];
		unsigned int usedSeed, lastSeed;
		
		float  grad( int hash, float x, float y ) {
			int h = hash & 7;      // Convert low 3 bits of hash code
			float u = h<4 ? x : y;  // into 8 simple gradient directions,
			float v = h<4 ? y : x;  // and compute the dot product with (x,y).
			return ((h&1)? -u : u) + ((h&2)? -2.f*v : 2.f*v);
		}

		float  grad( int hash, float x, float y , float z ) {
			int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
			float u = h<8 ? x : y; // gradient directions, and compute dot product.
			float v = h<4 ? y : h==12||h==14 ? x : z; // Fix repeats at h = 12 to 15
			return ((h&1)? -u : u) + ((h&2)? -v : v);
		}

	};
}

#endif
