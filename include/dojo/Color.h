/*
 *  Color.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "Vector.h"
#include "dojomath.h"

namespace Dojo {
	class Color {
	public:

		typedef unsigned int RGBAPixel;

		static const Color Red, Green, Blue, Black, White, Gray, Yellow, Cyan, Purple, None;

		static Color fromARGB(uint32_t pixel) {
			byte* ch = (byte*)&pixel;
			return{
				ch[2] / 255.f,
				ch[1] / 255.f,
				ch[0] / 255.f,
				ch[3] / 255.f
			};
		}

		static Color fromRGB(uint32_t RGB) {
			byte* ch = (byte*)&RGB;
			return{
				ch[2] / 255.f,
				ch[1] / 255.f,
				ch[0] / 255.f,
				1.f
			};
		}

		float r, g, b, a;

		Color() :
			r(0),
			g(0),
			b(0),
			a(0) {

		}

		Color(float r, float g, float b, float a = 1.f) {
			self.r = r;
			self.g = g;
			self.b = b;
			self.a = a;
		}

		explicit Color(const Vector& v, float a = 1.f) {
			self.r = v.x;
			self.g = v.y;
			self.b = v.z;
			self.a = a;
		}

		///creates from a single A8R8G8B8 32-bit value

		void set(byte r, byte g, byte b, byte a = 255) {
			self.r = (float)r / 255.f;
			self.g = (float)g / 255.f;
			self.b = (float)b / 255.f;
			self.a = (float)a / 255.f;
		}

		bool isNormal() const {
			return
				r >= 0 && r <= 1.f &&
				g >= 0 && g <= 1.f &&
				b >= 0 && b <= 1.f &&
				a >= 0 && a <= 1.f;
		}

		///creates a single 32-bit hex value representing the color (will degrade precision in HDR colors)
		RGBAPixel toRGBA() const {
			DEBUG_ASSERT(isNormal(), "Cannot convert this color to a 32-bit int as it's out of range.");

			RGBAPixel p;
			byte* ch = (byte*)&p;

			ch[0] = Math::packNormalized<byte>(r);
			ch[1] = Math::packNormalized<byte>(g);
			ch[2] = Math::packNormalized<byte>(b);
			ch[3] = Math::packNormalized<byte>(a);

			return p;
		}

		///scales each color's component except alpha
		Color operator *(float s) const {
			return Color(r * s, g * s, b * s, a);
		}

		///multiplies two colors component-wise
		Color operator *(const Color& c) const {
			return Color(r * c.r, g * c.g, b * c.b, a * c.a);
		}

		///sums two colors component-wise
		Color operator +(float s) const {
			return Color(r + s, g + s, b + s, a + s);
		}

		Color operator +(const Color& c) const {
			return{c.r + r, c.g + g, c.b + b, c.a + a};
		}

		///linearly interpolates two colors
		Color lerp(float s, const Color& c) const {
			float invs = 1.f - s;
			return Color(
						r * invs + c.r * s,
						g * invs + c.g * s,
						b * invs + c.b * s,
						a * invs + c.a * s);
		}

		void operator *=(float s) {
			r *= s;
			g *= s;
			b *= s;
		}

		void operator +=(const Color& c) {
			r += c.r;
			g += c.g;
			b += c.b;
		}

		Color clamped() const;

	protected:
	};
}
