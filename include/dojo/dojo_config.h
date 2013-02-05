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
#if defined ( WIN32 )
	#define PLATFORM_WIN32
	#define NOMINMAX  //Y U NO LEAVE STL ALONE

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

#ifdef PLATFORM_IOS
	#define BIG_ENDIAN 1
#else
	#define BIG_ENDIAN 0
#endif


//opengl rebindings!
#ifndef PLATFORM_IOS

	#define glOrthof glOrtho
	#define glFrustumf glFrustum

	#ifdef PLATFORM_OSX
		#define GL_FRAMEBUFFER_SRGB GL_FRAMEBUFFER_SRGB_EXT
        #define glDeleteVertexArrays glDeleteVertexArraysAPPLE
        #define glGenVertexArrays glGenVertexArraysAPPLE
        #define glBindVertexArray glBindVertexArrayAPPLE
	#endif
#else
	//iOS platform - OpenglES

	#define glGenRenderbuffers			glGenRenderbuffersOES
	#define glGenFramebuffers			glGenFramebuffersOES
	#define glFramebufferRenderbuffer	glFramebufferRenderbufferOES
	#define glDeleteRenderbuffers		glDeleteRenderbuffersOES
	#define glDeleteFramebuffers		glDeleteFramebuffersOES
	#define glBindRenderbuffer			glBindRenderbufferOES
	#define glBindFramebuffer			glBindFramebufferOES
	#define glGetRenderbufferParameteriv	glGetRenderbufferParameterivOES
	#define glCheckFramebufferStatus	glCheckFramebufferStatusOES
	#define glBlendEquation             glBlendEquationOES

	#define GL_FUNC_ADD GL_FUNC_ADD_OES
	
#endif

//general configurations
#if defined( PLATFORM_IOS ) || defined( PLATFORM_ANDROID )
	#define MESH_32BIT_INDICES 0  //32 bit indices not avaiable on older iOS/Android devices
#else
	//#define DOJO_GAMMA_CORRECTION_ENABLED
	#define DOJO_WIREFRAME_AVAILABLE //WIREFRAME not avaiable on iOS/Android devices
#endif

#ifdef PLATFORM_ANDROID
	#define DOJO_DISABLE_VAOS //a number of Android phones has a faulty VAO implementation
#endif

//do not use the differential state commit //HACK
#define DOJO_FORCE_WHOLE_RENDERSTATE_COMMIT

#define DOJO_MAX_TEXTURE_UNITS 2

//common enums
namespace Dojo
{
	enum Orientation
	{
		DO_PORTRAIT,
		DO_PORTRAIT_REVERSE,
		DO_LANDSCAPE_LEFT,
		DO_LANDSCAPE_RIGHT
	};
}

#endif
