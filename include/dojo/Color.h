/*
 *  Color.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Color_h__
#define Color_h__

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo
{
	class Color	
	{
	public:
		
		typedef unsigned int RGBAPixel;
		
		static const Color RED, GREEN, BLUE, BLACK, WHITE, GRAY, YELLOW, NIL;
				
		float r,g,b,a;
		
		Color() :
		r(0),
		g(0),
		b(0),
		a(0)
		{
			
		}
		
		Color( float r, float g, float b, float a )
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		Color( const Vector& v, float a ) 
		{
			this->r = v.x;
			this->g = v.y;
			this->b = v.z;
			this->a = a;
		}
		
		void setRGBA( byte r, byte g, byte b, byte a = 255 )
		{
			this->r = (float)r/255.f;
			this->g = (float)g/255.f;
			this->b = (float)b/255.f;
			this->a = (float)a/255.f;
		}
		
		void setRGBA( RGBAPixel pixel )
		{
			byte* ch = (byte*)&pixel;
			setRGBA( ch[0], ch[1], ch[2], ch[3] );
		}
				
		inline RGBAPixel toRGBA()
		{
			RGBAPixel p;
			byte* ch = (byte*)&p;
			
			ch[0] = (byte)(r * 255);
			ch[1] = (byte)(g * 255);
			ch[2] = (byte)(b * 255);
			ch[3] = (byte)(a * 255);
			
			return p;
		}

		///scales the color
		/** ALPHA IS UNAFFECTED */
		inline Color operator * ( float s ) const 
		{			
			return Color( r * s, g * s, b * s, a );
		}
		
		inline Color operator * ( const Color& c ) const 
		{			
			return Color( r * c.r, g * c.g, b * c.b, a * c.a );
		}

		inline Color operator + ( float s ) const
		{
			return Color( r + s, g + s, b + s, a + s );
		}

		inline Color lerp( float s, const Color& c ) const
		{
			float invs = 1.f-s;
			return Color( 
				r*invs + c.r*s, 
				g*invs + c.g*s, 
				b*invs + c.b*s, 
				a*invs + c.a*s );
		}
		
		inline void operator *= ( float s )
		{
			r *= s;
			g *= s;
			b *= s;
		}
						
	protected:
	};
}

#endif