#pragma once

namespace Dojo {
	enum class PixelFormat {
		RGBA_8_8_8_8,
		RGB_8_8_8,
		RGB_5_6_5,
		RGBA_10_10_10_2,
		RGBA_8_8_8_8_SRGB,
		RGB_8_8_8_SRGB,
		RGBA_16_16_16_16,
		R_8,
		RG_8,
		A_8, //same as R_8, but counts as transparent
		Unknown
	};
}
