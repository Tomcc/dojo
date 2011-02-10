#ifndef dojo_common_header
#define dojo_common_header

#include "dojo_config.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include <Poco/Mutex.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Semaphore.h>

#ifndef PLATFORM_IOS

	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>

#ifndef LINUX
	#include <al.h>
	#include <alc.h>
	#include <AL/alut.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
    #include <AL/alut.h>
#endif

#else

	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>

	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

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

#undef max
#undef min

#endif // dojo_common_headers_h__
