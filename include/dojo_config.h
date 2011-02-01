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

//are we on windows?
#ifdef WIN32
#define PLATFORM_WIN32
#endif

//are we on mac?
//TODO

//set at least one platform; PLATFORM_IOS is default
#ifndef PLATFORM_WIN32
	#ifndef PLATFORM_OSX
		#define PLATFORM_IOS
	#endif
#endif

#ifndef _DEBUG
#define NDEBUG  //to be sure!
#endif

#ifdef _DEBUG

	#define DEBUG_ASSERT( T ) assert(T)
	#define DEBUG_TODO assert( !"METHOD NOT IMPLEMENTED" )
	#define DEBUG_OUT( T ) std::cout << T << ' '
	#define DEBUG_MESSAGE( T ) std::cout << T << std::endl

#else

	#define DEBUG_ASSERT( T )
	#define DEBUG_TODO
	#define DEBUG_OUT( T ) 
	#define DEBUG_MESSAGE( T )

#endif

namespace Dojo
{
	typedef unsigned int uint;
	typedef unsigned char byte;
	typedef unsigned short unichar;
}

#endif
