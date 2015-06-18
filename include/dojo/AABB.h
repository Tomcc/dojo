#pragma once

#include "dojo_common_header.h"

#include "Vector.h"

namespace Dojo {
	struct AABB {
		static const AABB Empty, Invalid;

		static AABB fromCenterAndSize(const Vector& center, const Vector& size) {
			auto hs = size * 0.5f;
			return{ center - hs, center + hs };
		}

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

		AABB clip(const AABB& bb) const {
			return{
				Vector::max(min, bb.min),
				Vector::min(max, bb.max)
			};
		}

		AABB translate(const Vector& v) const {
			return{
				min + v,
				max + v
			};
		}

		Vector getCenter() const {
			return (max + min) * 0.5f;
		}

		Vector getSize() const {
			return max - min;
		}

		bool contains(const Vector& p) const {
			return max.x >= p.x && max.y >= p.y && max.z >= p.z &&
				min.x <= p.x && min.y <= p.y && min.z <= p.z;
		}
	};
}