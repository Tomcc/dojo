#pragma once

#include "dojo_common_header.h"

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

#if defined(PLATFORM_WIN32)
	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>
	#include <gl/glu.h>

#elif defined( PLATFORM_OSX )
	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>

#elif defined( PLATFORM_LINUX )
	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>

#elif defined( PLATFORM_IOS )
	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>

#elif defined( PLATFORM_ANDROID )

#else
	#error "No Platform defined!"

#endif
