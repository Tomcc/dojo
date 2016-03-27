#include "TexFormatInfo.h"

#include "dojo_gl_header.h"

namespace Dojo {
	const Dojo::TexFormatInfo& TexFormatInfo::getFor(PixelFormat format) {
		static const TexFormatInfo GLFormat[] = {
			{ 4, GL_RGBA, GL_UNSIGNED_BYTE },
			{ 3, GL_RGB, GL_UNSIGNED_BYTE },
			{ 2, GL_RGB, GL_UNSIGNED_SHORT_5_6_5 },
			{ 0, 0, 0 },
		};

		return GLFormat[enum_cast(format)];
	}

}

