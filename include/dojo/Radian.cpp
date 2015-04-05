#include "stdafx.h"

#include "Radian.h"

using namespace Dojo;

const float EULER_TO_RADIANS = 0.01745329251994329f;
const float RADIANS_TO_EULER = 57.295779513082325f;

Radian::Radian(const EulerAngle& e) :
RealNumber((float)e * EULER_TO_RADIANS) {

}

Radian& Radian::operator=(const EulerAngle& e) {
	mValue = (float)e * EULER_TO_RADIANS;
	return *this;
}

EulerAngle::EulerAngle(const Radian& r) :
RealNumber((float)r * RADIANS_TO_EULER) {

}
