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
		
		static const Color RED, GREEN, BLUE, BLACK, WHITE, GRAY, NIL;
				
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
		
		void setRGBA8( byte r, byte g, byte b, byte a = 255 )
		{
			this->r = (float)r/255.f;
			this->g = (float)g/255.f;
			this->b = (float)b/255.f;
			this->a = (float)a/255.f;
		}

		///scales the color
		/** ALPHA IS UNAFFECTED */
		inline Color operator * ( float s ) const 
		{			
			return Color( r * s, g * s, b * s, a );
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
				
	protected:
	};
}

#endif