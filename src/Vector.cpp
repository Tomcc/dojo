#include "stdafx.h"

#include "Vector.h"

using namespace Dojo;

const Vector Vector::ZERO = Vector(0,0,0);
const Vector Vector::UNIT_X = Vector(1,0,0);
const Vector Vector::UNIT_Y = Vector(0,1,0);
const Vector Vector::UNIT_Z = Vector(0,0,1);
const Vector Vector::ONE = Vector(1,1,1);

const Vector Vector::MAX = Vector( FLT_MAX, FLT_MAX, FLT_MAX );
const Vector Vector::MIN = Vector( -FLT_MAX, -FLT_MAX, -FLT_MAX );