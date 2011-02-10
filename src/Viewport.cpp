#include "stdafx.h"

#include "Viewport.h"

#include "Render.h"

using namespace Dojo;

void Viewport::makeScreenSize( Vector& dest, int w, int h )
{	
	dest.x = (float)w/targetSize.x * size.x * nativeToScreenRatio;
	dest.y = (float)h/targetSize.y * size.y * nativeToScreenRatio;
}

/*
 *  Viewport.mm
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 1/24/11.
 *  Copyright 2011 none. All rights reserved.
 *
 */


