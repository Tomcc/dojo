#pragma once

#include "Vector.h"

namespace Dojo {
	class GlobalUniformData {
	public:
		Matrix view, projection, worldView, worldViewProjection;
		Vector viewDirection, targetDimension;
	};
}

