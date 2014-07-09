/*
 *  dojo_config.h
 *  Created by Tommaso Checchi on 7/30/10.
 *
 */

#ifndef DojoConfig_h__
#define DojoConfig_h__

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
	#define USING_OPENGLES

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

    #define glGenVertexArrays           glGenVertexArraysOES
    #define glBindVertexArray           glBindVertexArrayOES
    #define glDeleteVertexArrays        glDeleteVertexArraysOES
    #define glFramebufferTexture2D      glFramebufferTexture2DOES

	#define GL_FUNC_ADD                 GL_FUNC_ADD_OES
    #define GL_FRAMEBUFFER              GL_FRAMEBUFFER_OES
    #define GL_COLOR_ATTACHMENT0        GL_COLOR_ATTACHMENT0_OES
    #define GL_DEPTH_COMPONENT16        GL_DEPTH_COMPONENT16_OES
    #define GL_DEPTH_ATTACHMENT         GL_DEPTH_ATTACHMENT_OES
    #define GL_FRAMEBUFFER_COMPLETE     GL_FRAMEBUFFER_COMPLETE_OES

    #define GL_NONE 0

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
#define DOJO_MAX_TEXTURES 8

///the cap for the texture coords in a single vertex
#define DOJO_MAX_TEXTURE_COORDS 2

//There is VAO experimental support but Valve states that VAOs are slower on all platforms
//source: https://developer.nvidia.com/sites/default/files/akamai/gamedev/docs/Porting%20Source%20to%20Linux.pdf
//needs testing, comment to enable
#define DOJO_DISABLE_VAOS

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
