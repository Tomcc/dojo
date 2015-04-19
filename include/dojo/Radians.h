#pragma once

namespace Dojo {
	template <class T, typename BASE>
	class RealNumber {
	public:
		typedef BASE BaseType;

		explicit RealNumber(BASE v) : mValue(v) {
		}

		T operator+(const T& rhs) const {
			return T(mValue + rhs.mValue);
		}

		T& operator+=(const T& rhs) {
			mValue += rhs.mValue;
			return (T&)*this;
		}

		operator BASE() const {
			return mValue;
		}

		T operator -() const {
			return T(-mValue);
		}

	protected:
		BASE mValue;
	};

	class Degrees;
	class Radians;

	class Radians : public RealNumber<Radians, float> {
	public:
		explicit Radians(BaseType v) :
			RealNumber(v) {

		}

		Radians(const Degrees& e);
		Radians& operator=(const Degrees& e);
	};

	class Degrees : public RealNumber<Degrees, float> {
	public:
		explicit Degrees(BaseType v) :
			RealNumber(v) {

		}

		Degrees(const Radians& r);
	};
}
