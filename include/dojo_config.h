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

//general configurations
#define MESH_32BIT_INDICES 0

//are we on windows?
#if defined ( WIN32 )
    #define PLATFORM_WIN32

//are we on mac?
#elif defined( __APPLE__ ) && defined( __MACH__ )
    #define PLATFORM_OSX

//are we on GNU/Linux?
#elif defined( LINUX )
    #define PLATFORM_LINUX

//default is IOS
#else 
    #define PLATFORM_IOS

#endif


#ifndef _DEBUG
#define NDEBUG  //to be sure!
#endif

#ifdef _DEBUG

	#define DEBUG_ASSERT( T ) assert(T)
	#define DEBUG_TODO assert( !"METHOD NOT IMPLEMENTED" ), 0
	#define DEBUG_OUT( T ) std::cout << T << ' '
	#define DEBUG_MESSAGE( T ) std::cout << T << std::endl

#else

	#define DEBUG_ASSERT( T )
	#define DEBUG_TODO 0
	#define DEBUG_OUT( T ) 
	#define DEBUG_MESSAGE( T )

#endif


//opengl rebindings!
#ifndef PLATFORM_IOS

	#define glOrthof glOrtho
	#define glFrustumf glFrustum
#else
	//IOS platform - OpenglES

	#define glGenRenderbuffers			glGenRenderbuffersOES
	#define glGenFramebuffers			glGenFramebuffersOES
	#define glFramebufferRenderbuffer	glFramebufferRenderbufferOES
	#define glDeleteRenderbuffers		glDeleteRenderbuffersOES
	#define glDeleteFramebuffers		glDeleteFramebuffersOES
	#define glBindRenderbuffer			glBindRenderbufferOES
	#define glBindFramebuffer			glBindFramebufferOES
	#define glGetRenderbufferParameteriv	glGetRenderbufferParameterivOES
	#define glCheckFramebufferStatus	glCheckFramebufferStatusOES

	#define GL_RENDERBUFFER			GL_RENDERBUFFER_OES
	#define GL_FRAMEBUFFER			GL_FRAMEBUFFER_OES
	#define GL_COLOR_ATTACHMENT0	GL_COLOR_ATTACHMENT0_OES
	#define GL_RENDERBUFFER_HEIGHT	GL_RENDERBUFFER_HEIGHT_OES
	#define GL_RENDERBUFFER_WIDTH	GL_RENDERBUFFER_WIDTH_OES
	#define GL_FRAMEBUFFER_COMPLETE	GL_FRAMEBUFFER_COMPLETE_OES
#endif

namespace Dojo
{
#ifndef LINUX
    typedef unsigned int uint;
#endif
	typedef unsigned char byte;
	typedef unsigned short unichar;
}

#endif
