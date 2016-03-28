#include "TexFormatInfo.h"

#include "dojo_gl_header.h"

namespace Dojo {
	const Dojo::TexFormatInfo& TexFormatInfo::getFor(PixelFormat format) {
		static const TexFormatInfo GLFormat[] = {
			{ 4, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true },
			{ 3, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE , false },
			{ 2, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, false },
			{ 4, GL_RGBA, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2, true },
			{ 4, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, true },
			{ 3, GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE, false },
			{ 0, 0, 0, 0, 0 },
		};

		return GLFormat[enum_cast(format)];
	}

}

