/*
 *  Vector.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 4/25/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "dojo_common_header.h"
#include "Radians.h"

namespace Dojo {
	class Plane;

	typedef glm::mat4x4 Matrix;
	typedef glm::quat Quaternion;

	///a wrapper to glm::tvec3, providing the most common Vector algebra needs
	class Vector : public glm::vec3 {
	public:

		static const Vector Zero;
		static const Vector UnitX;
		static const Vector NegativeUnitX;
		static const Vector UnitY;
		static const Vector NegativeUnitY;
		static const Vector UnitZ;
		static const Vector NegativeUnitZ;
		static const Vector One;
		static const Vector Max, Min;

		///returns a vector which components are the component-wise max of a and b
		static Vector max(const Vector& a, const Vector& b) {
			return{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
		}

		///returns a vector which components are the component-wise min of a and b
		static Vector min(const Vector& a, const Vector& b) {
			return{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
		}

		static Vector clamp(const Vector& val, const Vector& max, const Vector& min) {
			return Vector::max(min, Vector::min(max, val));
		}
		///returns a Vector of abs values
		static Vector abs(const Vector& val) {
			return{ std::abs(val.x), std::abs(val.y), std::abs(val.z) };
		}

		///component-wise multiply two vectors
		static Vector mul(const Vector& A, const Vector& B) {
			return{ A.x * B.x, A.y * B.y, A.z * B.z };
		}

		///component-wise divide two vectors
		static Vector div(const Vector& A, const Vector& B) {
			return{ A.x / B.x, A.y / B.y, A.z / B.z };
		}

		///xyz are set to the same value, or 0
		explicit Vector(float f = 0) : glm::vec3(f) {

		}

		///2D constructor - xy are initialized and z is set to 0
		Vector(float X, float Y) : glm::vec3(X, Y, 0) {

		}

		Vector(float x, float y, float z) : glm::vec3(x, y, z) {

		}

		Vector(const glm::vec3& v) : glm::vec3(v.x, v.y, v.z) {

		}

		Vector(const Vector& v) : glm::vec3(v.x, v.y, v.z) {

		}

		const Vector& operator +=(const Vector& v) {
			x += v.x;
			y += v.y;
			z += v.z;

			return *this;
		}

		const Vector& operator -=(const Vector& v) {
			x -= v.x;
			y -= v.y;
			z -= v.z;

			return *this;
		}

		Vector operator -(const Vector& v) const {
			return Vector(x - v.x, y - v.y, z - v.z);
		}

		Vector operator *(float s) const {
			return Vector(x * s, y * s, z * s);
		}

		Vector operator +(float s) const {
			return Vector(x + s, y + s, z + s);
		}

		Vector operator -(float s) const {
			return Vector(x - s, y - s, z - s);
		}

		float operator *(const Vector& v) const {
			return glm::dot((const glm::vec3&)(*this), (const glm::vec3&)v);
		}

		Vector operator ^(const Vector& v) const {
			return glm::cross(*this, v);
		}

		float lengthSquared() const {
			return *this * *this;
		}

		///returns the length of this Vector
		float length() const {
			return sqrtf(lengthSquared());
		}

		///returns a normalized copy of this Vector
		Vector normalized() const {
			float l = length();
			return Vector(x / l, y / l, z / l);
		}

		///linearly interpolates the two vectors; s = 0 returns this, s = 1 returns v
		Vector lerpTo(float s, const Vector& v) const {
			float invs = 1.f - s;
			return Vector(v.x * s + invs * x, v.y * s + invs * y, v.z * s + invs * z);
		}

		///returns the distance from this to v
		float distance(const Vector& v) const {
			return sqrt(distanceSquared(v));
		}

		///returns the squared (and faster to compute) distance from this to v
		float distanceSquared(const Vector& v) const {
			return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z);
		}

		///returns true if this vector is near to v within the given threshold
		bool isNear(const Vector& v, float threshold = 0.1) {
			return distanceSquared(v) < threshold * threshold;
		}

		///returns true if no element is NaN or inf
		bool isValid() const {
			return !std::isnan(x) && !std::isnan(y) && !std::isnan(z) && !std::isinf(x) && !std::isinf(y) && !std::isinf(z);
		}

		///returns true when all the components of the vector are 0
		bool isZero() const {
			return x == 0.f && y == 0.f && z == 0.f;
		}

		///returns the dot product with this vector with only positive components
		float absDot(const Vector& v) const {
			return std::abs(x * v.x) + std::abs(y * v.y) + std::abs(z * v.z);
		}

		std::string toString() const;

		size_t getHash() const {
			return (((size_t)x) * 73856093) ^ (((size_t)y) * 19349663) ^ (((size_t)z) * 83492791);
		}

		///reflect this vector on the plane with the given normal
		Vector reflect(const Vector& normal) const {
			return 2.f * normal * (normal * *this) - *this;
		}

		float* const data() const {
			return (float * const)this;
		}

		///refracts this vector on the plane with the given normal, where eta is the refraction indices ratio
		Vector refract(const Vector& n, float eta) const {
			const Vector& i = -*this;

			float N_dot_I = n * i;
			float k = 1.f - eta * eta * (1.f - N_dot_I * N_dot_I);

			if (k < 0.f) {
				return Vector::Zero;
			}
			else {
				return eta * i - (eta * N_dot_I + sqrtf(k)) * n;
			}
		}

		///rolls the given vector around the Z axis
		Vector roll(Radians a) const {
			auto c = cosf(a);
			auto s = sinf(a);
			return{x* c - y * s, x* s + y * c, z};
		}

		//swizzles, autogenerated
		Vector xxx() const {
			return{ x, x, x };
		}
		Vector xxy() const {
			return{ x, x, y };
		}
		Vector xxz() const {
			return{ x, x, z };
		}
		Vector xyx() const {
			return{ x, y, x };
		}
		Vector xyy() const {
			return{ x, y, y };
		}
		Vector xzx() const {
			return{ x, z, x };
		}
		Vector xzy() const {
			return{ x, z, y };
		}
		Vector xzz() const {
			return{ x, z, z };
		}
		Vector yxx() const {
			return{ y, x, x };
		}
		Vector yxy() const {
			return{ y, x, y };
		}
		Vector yxz() const {
			return{ y, x, z };
		}
		Vector yyx() const {
			return{ y, y, x };
		}
		Vector yyy() const {
			return{ y, y, y };
		}
		Vector yyz() const {
			return{ y, y, z };
		}
		Vector yzx() const {
			return{ y, z, x };
		}
		Vector yzy() const {
			return{ y, z, y };
		}
		Vector yzz() const {
			return{ y, z, z };
		}
		Vector zxx() const {
			return{ z, x, x };
		}
		Vector zxy() const {
			return{ z, x, y };
		}
		Vector zxz() const {
			return{ z, x, z };
		}
		Vector zyx() const {
			return{ z, y, x };
		}
		Vector zyy() const {
			return{ z, y, y };
		}
		Vector zyz() const {
			return{ z, y, z };
		}
		Vector zzx() const {
			return{ z, z, x };
		}
		Vector zzy() const {
			return{ z, z, y };
		}
		Vector zzz() const {
			return{ z, z, z };
		}

	protected:
	};
}

namespace std {
	///hash specialization for unordered_maps
	template <>
	struct hash<Dojo::Vector> { // hash functor for vector
		size_t operator()(const Dojo::Vector& _Keyval) const {
			return _Keyval.getHash();
		}
	};
}
