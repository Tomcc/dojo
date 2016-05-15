#pragma once

#include "dojo_common_header.h"

#include "PixelFormat.h"

namespace Dojo {
	class Texture;

	///this class represents just the basic information about something that can be rendered to
	class RenderSurface {
	public:
		RenderSurface();
		RenderSurface(uint32_t width, uint32_t height, PixelFormat format);

		uint32_t getWidth() const {
			return width;
		}

		uint32_t getHeight() const {
			return height;
		}

		PixelFormat getFormat() const {
			return internalFormat;
		}

		bool isPowerOfTwo() const {
			return glm::isPowerOfTwo(width) and glm::isPowerOfTwo(height);
		}

	protected:
		uint32_t width, height;
		PixelFormat internalFormat;
	};
}

