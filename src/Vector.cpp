#include "Vector.h"

using namespace Dojo;

const Vector Vector::Zero = {0, 0, 0};
const Vector Vector::UnitX = {1, 0, 0};
const Vector Vector::UnitY = {0, 1, 0};
const Vector Vector::UnitZ = {0, 0, 1};
const Vector Vector::NegativeUnitX = { -1, 0, 0};
const Vector Vector::NegativeUnitY = {0, -1, 0};
const Vector Vector::NegativeUnitZ = { 0, 0, -1 };
const Vector Vector::One = { 1, 1, 1 };

const Vector Vector::Max = {FLT_MAX, FLT_MAX, FLT_MAX};
const Vector Vector::Min = { -FLT_MAX, -FLT_MAX, -FLT_MAX};

utf::string Dojo::Vector::toString() const {
	return "(" + utf::to_string(x) + "," + utf::to_string(y) + "," + utf::to_string(z) + ")";
}
