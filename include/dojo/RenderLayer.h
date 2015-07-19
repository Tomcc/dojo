#pragma once

#include "dojo_common_header.h"

#include "SmallSet.h"

#include "enum_cast.h"

namespace Dojo {
	class Renderable;

	class RenderLayer {
	public:
		struct ID {
			byte value;

			template<typename T>
			ID(T raw) :
			value(static_cast<decltype(value)>(enum_cast(raw))) {

			};

			bool operator != (const ID& rhs) const {
				return value != rhs.value;
			}

			operator byte() {
				return value;
			}
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
