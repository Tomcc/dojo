#pragma once

#include "dojo_common_header.h"

#include "PixelFormat.h"

namespace Dojo {
	struct TexFormatInfo {
		int internalPixelSize;
		uint32_t internalFormat, internalElementType;
		
		int sourcePixelSize;
		uint32_t sourceFormat, sourceElementType;

		bool hasAlpha;

		static const TexFormatInfo& getFor(PixelFormat format);

		bool isGPUFormat() const {
			return glm::isPowerOfTwo(internalPixelSize);
		}
	};

}

