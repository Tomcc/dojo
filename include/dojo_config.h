/*
 *  dojo_config.h
 *  Ninja Training
 *
 *  Created by Tommaso Checchi on 7/30/10.
 *  Copyright 2010 none. All rights reserved.
 *
 */

#ifndef DojoConfig_h__
#define DojoConfig_h__

#include "TargetConditionals.h"

#ifndef DEBUG
#define NDEBUG  //to be sure!
#endif

#include <assert.h>

#include <iostream>

#define DOJO_ASSERT( T ) assert( T )

#ifdef DEBUG
#define DEBUG_OUT( T ) std::cout << T << ' '
#define DEBUG_MESSAGE( T ) std::cout << T << std::endl
#else
#define DEBUG_OUT( T )
#define DEBUG_MESSAGE( T )
#endif

namespace Dojo {
	typedef unsigned int uint;
	typedef unsigned char byte;
}

#endif
