#pragma once

#undef REAL

//the triangle library is included here
//disable "uninitialized variable" warning as it's C
#pragma warning(push)
#pragma warning(disable:4701) //use of uninitialized variable
#pragma warning(disable:4127) //conditional expression is constant

extern "C" {
	//credits and documentation from http://www.cs.cmu.edu/~quake/triangle.html
#include "triangle.c"
};

#pragma warning(pop)
