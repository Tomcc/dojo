#include "stdafx.h"

#include "Vector.h"

const Dojo::Vector Dojo::Vector::ZERO = Vector(0,0,0);
const Dojo::Vector Dojo::Vector::UNIT_X = Vector(1,0,0);
const Dojo::Vector Dojo::Vector::UNIT_Y = Vector(0,1,0);
const Dojo::Vector Dojo::Vector::UNIT_Z = Vector(0,0,1);
const Dojo::Vector Dojo::Vector::NEGATIVE_UNIT_X = Vector(-1,0,0);
const Dojo::Vector Dojo::Vector::NEGATIVE_UNIT_Y = Vector(0,-1,0);
const Dojo::Vector Dojo::Vector::NEGATIVE_UNIT_Z = Vector(0,0,-1);
const Dojo::Vector Dojo::Vector::ONE = Vector(1,1,1);

const Dojo::Vector Dojo::Vector::MAX = Vector( FLT_MAX, FLT_MAX, FLT_MAX );
const Dojo::Vector Dojo::Vector::MIN = Vector( -FLT_MAX, -FLT_MAX, -FLT_MAX );