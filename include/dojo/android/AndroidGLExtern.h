#ifndef ANDROIDGLEXTERN_H
#define ANDROIDGLEXTERN_H

	#include <EGL/egl.h>
	#include <GLES/gl.h>
	#include <GLES/glext.h>
	#include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
	
	
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
	
    	#define glBindVertexArray           glBindVertexArrayOESEXT
    	#define glGenVertexArrays           glGenVertexArraysOESEXT
    	#define glDeleteVertexArrays        glDeleteVertexArraysOESEXT

	//#define GL_FUNC_ADD GL_FUNC_ADD_OES
        extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT;
        extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT;
        extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT;

	extern void ExternInitOpenGL();
	
#endif
