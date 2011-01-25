#ifndef dojo_common_header
#define dojo_common_header

#include "dojo_config.h"

#include <math.h>
#include <assert.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>

#ifdef PLATFORM_WIN32

	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>

	#include <al.h>
	#include <alc.h>

#else

	#include "TargetConditionals.h"

	#import <Foundation/NSString.h>

	#import <QuartzCore/QuartzCore.h>

	#import <OpenGLES/EAGL.h>
	#import <OpenGLES/EAGLDrawable.h>
	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>

	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

	#ifdef PLATFORM_IOS

		#include <AudioToolbox/AudioToolbox.h>	
		#import <UIKit/UIKit.h>

	#endif

#endif

//redefine opengl extension functions
#ifdef PLATFORM_IOS

	#define glGenRenderbuffers			glGenRenderbuffersOES
	#define glGenFramebuffers			glGenFramebuffersOES
	#define glFramebufferRenderbuffer	glFramebufferRenderbufferOES
	#define glDeleteRenderbuffers		glDeleteRenderbuffersOES
	#define glDeleteFramebuffers		glDeleteFramebuffersOES
	#define glBindRenderbuffer			glBindRenderbufferOES
	#define glBindFramebuffer			glBindFramebufferOES

	#define GL_RENDERBUFFER			GL_RENDERBUFFER_OES
	#define GL_FRAMEBUFFER			GL_FRAMEBUFFER_OES
	#define GL_COLOR_ATTACHMENT0	GL_COLOR_ATTACHMENT0_OES

#else  //mac and win share OGL2.1

	#define glGenRenderbuffers			glGenRenderbuffersEXT
	#define glGenFramebuffers			glGenFramebuffersEXT
	#define glFramebufferRenderbuffer	glFramebufferRenderbufferEXT
	#define glDeleteRenderbuffers		glDeleteRenderbuffersEXT
	#define glDeleteFramebuffers		glDeleteFramebuffersEXT
	#define glBindRenderbuffer			glBindRenderbufferEXT
	#define glBindFramebuffer			glBindFramebufferEXT

	#define GL_RENDERBUFFER			GL_RENDERBUFFER_EXT
	#define GL_FRAMEBUFFER			GL_FRAMEBUFFER_EXT
	#define GL_COLOR_ATTACHMENT0	GL_COLOR_ATTACHMENT0_EXT

#endif

#endif // dojo_common_headers_h__