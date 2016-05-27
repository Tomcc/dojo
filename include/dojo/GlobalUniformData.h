#pragma once

#include "Vector.h"

namespace Dojo {
	class GlobalUniformData {
	public:
		Matrix view, projection, world, worldView, worldViewProjection;
		Vector viewDirection, targetDimension;
	};
}

