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

		//TODO make variadic?
		static AABB fromPoints(const Vector& A, const Vector& B) {
			return{ Vector::min(A,B), Vector::max(A,B) };
		}

		static AABB fromPoints(vec_view<Vector> points) {
			Vector min = Vector::Max, max = Vector::Min;
			for (auto&& point : points) {
				min = Vector::min(min, point);
				max = Vector::max(max, point);
			}
			return{ min, max };
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
				min.x >= max.x and
				min.y >= max.y and
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

		AABB scale(const Vector& scale) const {
			return{
				{min.x * scale.x, min.y * scale.y, min.z * scale.z},
				{max.x * scale.x, max.y * scale.x, max.z * scale.z}
			};
		}

		Vector getCenter() const {
			return (max + min) * 0.5f;
		}

		Vector getSize() const {
			return max - min;
		}

		float getVolume() const {
			auto sz = getSize();
			return sz.x * sz.y * sz.z;
		}

		bool contains(const Vector& p) const {
			return max.x >= p.x and max.y >= p.y and max.z >= p.z and
							min.x <= p.x and min.y <= p.y and min.z <= p.z;
		}

		bool operator==(const AABB& bb) const {
			return max == bb.max and min == bb.min;
		}

		bool operator!=(const AABB& bb) const {
			return not (self == bb);
		}
	};
}