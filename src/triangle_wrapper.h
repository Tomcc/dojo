#pragma once

#undef REAL

//the triangle library is included here
//disable "uninitialized variable" warning as it's C

#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable:4701) //use of uninitialized variable
	#pragma warning(disable:4127) //conditional expression is constant
#else
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wstrict-overflow"
	#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

extern "C" {
	//credits and documentation from http://www.cs.cmu.edu/~quake/triangle.html
#include "triangle.c"
};

#ifdef _MSC_VER
	#pragma warning(pop)
#else
	#pragma GCC diagnostic pop
#endif

