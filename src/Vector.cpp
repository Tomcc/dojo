#include "stdafx.h"

#include "Vector.h"

using namespace Dojo;

const Vector Vector::ZERO = {0,0,0};
const Vector Vector::UNIT_X = {1,0,0};
const Vector Vector::UNIT_Y = {0,1,0};
const Vector Vector::UNIT_Z = {0,0,1};
const Vector Vector::NEGATIVE_UNIT_X = {-1,0,0};
const Vector Vector::NEGATIVE_UNIT_Y = {0,-1,0};
const Vector Vector::NEGATIVE_UNIT_Z = {0,0,-1};
const Vector Vector::ONE = {1,1,1};

const Vector Vector::MAX = { FLT_MAX, FLT_MAX, FLT_MAX };
const Vector Vector::MIN = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
