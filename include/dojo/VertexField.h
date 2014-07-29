#pragma once

#include "dojo_common_header.h"

namespace Dojo {
	enum class VertexField
	{
		VF_POSITION2D,
		VF_POSITION3D,
		VF_COLOR,
		VF_NORMAL,

		VF_UV_0,
		VF_UV_MAX = VF_UV_0 + DOJO_MAX_TEXTURE_COORDS - 1,

		VF_NONE,
		_VF_COUNT = VF_NONE
	};

}



