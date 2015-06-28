#pragma once

#include "dojo_common_header.h"

#include "SmallSet.h"

namespace Dojo {
	class Renderable;

	class RenderLayer {
	public:
		typedef byte ID;

		static const ID InvalidID = std::numeric_limits<ID>::max();

		bool visible = true,
			depthCheck = false,
			orthographic = true,
			depthClear = true,
			wireframe = false;

		SmallSet<Renderable*> elements;
	};
}
