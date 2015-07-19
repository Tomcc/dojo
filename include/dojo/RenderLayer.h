#pragma once

#include "dojo_common_header.h"

#include "SmallSet.h"

#include "PseudoEnum.h"

namespace Dojo {
	class Renderable;

	class RenderLayer {
	public:
		struct ID : public PseudoEnumClass<byte> {
			template<typename T>
			ID(T raw) : PseudoEnumClass(raw) {}
		};

		static const ID InvalidID;

		bool visible = true,
			depthCheck = false,
			orthographic = true,
			depthClear = true,
			wireframe = false;

		SmallSet<Renderable*> elements;
	};
}
