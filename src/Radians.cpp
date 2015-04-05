#include "stdafx.h"

#include "Radians.h"

using namespace Dojo;

const float EULER_TO_RADIANS = 0.01745329251994329f;
const float RADIANS_TO_EULER = 57.295779513082325f;

Radians::Radians(const Degrees& e) :
RealNumber((float)e * EULER_TO_RADIANS) {

}

Radians& Radians::operator=(const Degrees& e) {
	mValue = (float)e * EULER_TO_RADIANS;
	return *this;
}

Degrees::Degrees(const Radians& r) :
RealNumber((float)r * RADIANS_TO_EULER) {

}
