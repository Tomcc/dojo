#include "TexFormatInfo.h"

#include "glad/glad.h"

namespace Dojo {
	const Dojo::TexFormatInfo& TexFormatInfo::getFor(PixelFormat format) {
		static const TexFormatInfo GLFormat[] = {
			{ 4, GL_RGBA8, GL_UNSIGNED_BYTE,					4, GL_RGBA, GL_UNSIGNED_BYTE, true },
			{ 3, GL_RGB8, GL_UNSIGNED_BYTE,					3, GL_RGB, GL_UNSIGNED_BYTE, false },
			{ 2, GL_RGB8, GL_UNSIGNED_SHORT_5_6_5,		    3, GL_RGB, GL_UNSIGNED_BYTE, false },
			{ 4, GL_RGBA8, GL_UNSIGNED_INT_2_10_10_10_REV,	4, GL_RGBA, GL_UNSIGNED_BYTE,  true },
			{ 4, GL_SRGB8_ALPHA8, GL_UNSIGNED_BYTE,			4, GL_RGBA, GL_UNSIGNED_BYTE, true },
			{ 3, GL_SRGB8, GL_UNSIGNED_BYTE,				3, GL_RGB, GL_UNSIGNED_BYTE, false },
			{ 8, GL_RGBA16F, GL_HALF_FLOAT,					16, GL_RGBA, GL_FLOAT, false },
			{ 1, GL_R8, GL_UNSIGNED_BYTE,					1, GL_RED, GL_UNSIGNED_BYTE, false },
			{ 2, GL_RG8, GL_UNSIGNED_BYTE,					2, GL_RG, GL_UNSIGNED_BYTE, false },
			{ 1, GL_R8, GL_UNSIGNED_BYTE,					1, GL_ALPHA, GL_UNSIGNED_BYTE, true },
			{ 0, 0, 0, 0, 0 },
		};

		return GLFormat[enum_cast(format)];
	}

}

