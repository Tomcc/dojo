#include "AABB.h"

using namespace Dojo;

const AABB
AABB::Empty,
	 AABB::Invalid = { Vector(FLT_MAX), Vector(-FLT_MAX) };