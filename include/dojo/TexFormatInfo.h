#pragma once

#include "PixelFormat.h"

namespace Dojo {
	struct TexFormatInfo {
		size_t internalPixelSize;
		uint32_t internalFormat, internalElementType;
		
		size_t sourcePixelSize;
		uint32_t sourceFormat, sourceElementType;

		bool hasAlpha;

		static const TexFormatInfo& getFor(PixelFormat format);

		bool isGPUFormat() const {
			return glm::isPowerOfTwo(internalPixelSize);
		}
	};

}

