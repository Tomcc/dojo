#pragma once

namespace Dojo {
	template <class T, typename BASE>
	class RealNumber {
	public:
		typedef BASE BaseType;

		constexpr explicit RealNumber(BASE v) : mValue(v) {
		}

		T operator+(const T& rhs) const {
			return T(mValue + rhs.mValue);
		}

		T& operator+=(const T& rhs) {
			mValue += rhs.mValue;
			return (T&) * this;
		}

		constexpr operator BASE() const {
			return mValue;
		}

		T operator -() const {
			return T(-mValue);
		}

		T operator*(BASE rhs) const {
			return T(mValue * rhs);
		}

	protected:
		BASE mValue;
	};

	class Degrees;
	class Radians;

	class Radians : public RealNumber<Radians, float> {
	public:
		constexpr explicit Radians(BaseType v) :
			RealNumber(v) {

		}

		Radians(const Degrees& e);
		Radians& operator=(const Degrees& e);
	};

	class Degrees : public RealNumber<Degrees, float> {
	public:
		constexpr explicit Degrees(BaseType v) :
			RealNumber(v) {

		}

		Degrees(const Radians& r);
	};

	constexpr Radians operator "" _rad(long double v) {
		return Radians{ static_cast<Radians::BaseType>(v) };
	}

	constexpr Degrees operator "" _deg(long double v) {
		return Degrees{ static_cast<Degrees::BaseType>(v) };
	}
}
