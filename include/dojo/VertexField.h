#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	enum class VertexField
	{
		Position2D,
		Position3D,
		Color,
		Normal,

		UV0,
		UVMax = UV0 + DOJO_MAX_TEXTURE_COORDS - 1,

		None,
		_Count = None
	};

}



