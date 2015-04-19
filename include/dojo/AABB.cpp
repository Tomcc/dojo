#include "AABB.h"

using namespace Dojo;

const AABB
AABB::EMPTY,
AABB::INVALID = { Vector(FLT_MAX), Vector(-FLT_MAX) };