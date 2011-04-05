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


//are we on GNU/Linux?
#ifdef LINUX
#define PLATFORM_LINUX
#endif

//set at least one platform; PLATFORM_IOS is default
#ifndef PLATFORM_WIN32
	#ifndef PLATFORM_OSX
		#ifndef PLATFORM_LINUX
			#define PLATFORM_IOS
		#endif
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


//opengl rebindings!
#ifndef PLATFORM_IOS

	#define glOrthof glOrtho
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
