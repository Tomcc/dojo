/*
 *  dojo_config.h
 *  Created by Tommaso Checchi on 7/30/10.
 *
 */

#pragma once

//are we on windows?
#if defined ( WIN32 )
	#define PLATFORM_WIN32
	//are we on mac?
#elif defined( __APPLE__ )

	#include <TargetConditionals.h>

	#ifdef DOJO_IOS
		#define PLATFORM_IOS
	#else
		#define PLATFORM_OSX

	#endif

#elif defined( __ANDROID__ )
	#define PLATFORM_ANDROID

#elif defined( __linux__ )
	#define PLATFORM_LINUX

#else
	#error "Unsupported Platform"

#endif


#if !defined( _DEBUG ) && !defined( NDEBUG )
	#define NDEBUG  //to be sure!
#endif

///the cap for the textures bound to a single object
#define DOJO_MAX_TEXTURES 4

///the cap for the texture coords in a single vertex
#define DOJO_MAX_TEXTURE_COORDS 2

//common enums
namespace Dojo {
	enum Orientation {
		DO_PORTRAIT,
		DO_PORTRAIT_REVERSE,
		DO_LANDSCAPE_LEFT,
		DO_LANDSCAPE_RIGHT
	};
}

