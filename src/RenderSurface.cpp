#include "RenderSurface.h"

#include "dojo_gl_header.h"

namespace Dojo {
	RenderSurface::RenderSurface() :
		internalFormat(PixelFormat::Unknown) {

	}

	RenderSurface::RenderSurface(uint32_t width, uint32_t height, PixelFormat format) :
	internalFormat(format),
	width(width),
	height(height) {
		DEBUG_ASSERT(width > 1, "Invalid width");
		DEBUG_ASSERT(height > 1, "Invalid height");
	}

}