#pragma once

#include "dojo_common_header.h"

#include "SmallSet.h"

#include "PseudoEnum.h"

namespace Dojo {
	class Renderable;

	class RenderLayer {
	public:
		struct ID : public PseudoEnumClass<uint8_t> {
			template<typename T>
			ID(T raw) : PseudoEnumClass(raw) {}
		};

		static const ID InvalidID;

		bool visible = true,
			depthTest = false,
			depthWrite = false,
			orthographic = true;

		void make3D() {
			depthTest = true;
			depthWrite = true;
			orthographic = false;
		}

		float zOffset = 0.f;
		
		SmallSet<Renderable*> elements;

		bool usesDepth() const {
			return depthWrite or depthTest;
		}
	};
}
