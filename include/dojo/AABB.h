#pragma once

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo {
	struct AABB {
		static const AABB EMPTY, INVALID;

		Vector min, max;

		AABB() {}

		AABB(const Vector& min, const Vector& max) : 
			min(min),
			max(max) {
		}

		AABB grow(float s) const {
			return{
				min - s,
				max + s
			};
		}

		bool isEmpty() const {
			return
				min.x >= max.x &&
				min.y >= max.y &&
				min.z >= max.z;
		}

		AABB expandToFit(const Vector& v) const {
			return{
				Vector::min(min, v),
				Vector::max(max, v)
			};
		}

		AABB expandToFit(const AABB& bb) const {
			return{
				Vector::min(min, bb.min),
				Vector::max(max, bb.max)
			};
		}

		Vector getCenter() const {
			return (max + min) * 0.5f;
		}

		Vector getSize() const {
			return max - min;
		}
	};
}