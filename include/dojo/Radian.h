#pragma once

namespace Dojo {
	template<class T>
	class RealNumber {
	public:
		explicit RealNumber(float v) : mValue(v) {}

		T operator+(const T& rhs) const {
			return T(mValue + rhs.mValue);
		}

		operator float() const {
			return mValue;
		}

	protected:
		float mValue;
	};

	class EulerAngle;
	class Radian;

	class Radian : public RealNumber<Radian> {
	public:
		explicit Radian(float v) :
			RealNumber(v) {

		}

		Radian(const EulerAngle& e);
		Radian& operator=(const EulerAngle& e);
	};

	class EulerAngle : public RealNumber < EulerAngle > {
	public:
		explicit EulerAngle(float v) :
			RealNumber(v) {

		}

		EulerAngle(const Radian& r);
	};
}

