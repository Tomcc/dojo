#pragma once

#include "PixelFormat.h"

namespace Dojo {
	struct TexFormatInfo {
		size_t pixelSizeBytes;
		uint32_t glFormat, elementType;

		static const TexFormatInfo& getFor(PixelFormat format);

		bool isGPUFormat() const {
			return pixelSizeBytes == 2 || pixelSizeBytes == 4;
		}
	};

}

