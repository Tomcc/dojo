/*
 *  dojo_config.h
 *  Created by Tommaso Checchi on 7/30/10.
 *
 */

#pragma once

//are we on windows?
#if defined ( WIN32 )
	#define PLATFORM_WIN32
	#define NOMINMAX 1  //Y U NO LEAVE STL ALONE
	#define WIN32_LEAN_AND_MEAN 1
	#define _CRT_SECURE_NO_WARNINGS 1

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

#ifdef PLATFORM_ANDROID
	#define DOJO_DISABLE_VAOS //a number of Android phones has a faulty VAO implementation
	#define USING_OPENGLES
#endif

#ifndef USING_OPENGLES
	#define DOJO_32BIT_INDICES_AVAILABLE
	#define DOJO_WIREFRAME_AVAILABLE //WIREFRAME not avaiable on iOS/Android devices
	#define DOJO_SHADERS_AVAILABLE
#endif

#ifndef PLATFORM_ANDROID
	#define DOJO_ANISOTROPIC_FILTERING_AVAILABLE //anisotropic filtering has to be tested on Android //TODO move this to Platform, maybe make a caps class?
#endif

//#define DOJO_GAMMA_CORRECTION_ENABLED

//do not use the differential state commit //HACK
#define DOJO_FORCE_WHOLE_RENDERSTATE_COMMIT

///the cap for the textures bound to a single object
#define DOJO_MAX_TEXTURES 2

///the cap for the texture coords in a single vertex
#define DOJO_MAX_TEXTURE_COORDS 2

//There is VAO experimental support but Valve states that VAOs are slower on all platforms
//source: https://developer.nvidia.com/sites/default/files/akamai/gamedev/docs/Porting%20Source%20to%20Linux.pdf
//needs testing, comment to enable
#define DOJO_DISABLE_VAOS

#define _HAS_AUTO_PTR_ETC 0

//common enums
namespace Dojo {
	enum Orientation {
		DO_PORTRAIT,
		DO_PORTRAIT_REVERSE,
		DO_LANDSCAPE_LEFT,
		DO_LANDSCAPE_RIGHT
	};
}

