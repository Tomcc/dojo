#include "stdafx.h"

#include "Tessellation.h"

//the triangle library is included inline here
extern "C"
{
	//credits and documentation from http://www.cs.cmu.edu/~quake/triangle.html
#include "triangle.c"
};

using namespace Dojo;

void Tessellation::tessellate( bool clearInputs /* = true */ )
{
	


	if( clearInputs )
	{
		positions.clear();
		indices.clear();
	}
}
