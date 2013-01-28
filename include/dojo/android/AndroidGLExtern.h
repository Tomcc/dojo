#ifndef ANDROIDGLEXTERN_H
#define ANDROIDGLEXTERN_H

	#ifndef GL_GLEXT_PROTOTYPES
	#define GL_GLEXT_PROTOTYPES 1
	#endif
	
	
	#include <EGL/egl.h>
	#ifdef DEF_SET_OPENGL_ES2
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#endif	
	#include <GLES/gl.h>
	#include <GLES/glext.h>
	
	#define glGenRenderbuffers			    glGenRenderbuffersOES
	#define glGenFramebuffers			    glGenFramebuffersOES
	#define glFramebufferRenderbuffer	    glFramebufferRenderbufferOES
	#define glDeleteRenderbuffers		    glDeleteRenderbuffersOES
	#define glDeleteFramebuffers		    glDeleteFramebuffersOES
	#define glBindRenderbuffer			    glBindRenderbufferOES
	#define glBindFramebuffer			    glBindFramebufferOES
	#define glGetRenderbufferParameteriv	glGetRenderbufferParameterivOES
	#define glCheckFramebufferStatus	    glCheckFramebufferStatusOES
	
	#ifndef DEF_SET_OPENGL_ES2
	#define GL_FUNC_ADD GL_FUNC_ADD_OES
	#define GL_NONE GL_NONE_OES
	//glBlendEquation is unimplemented
	#define glBlendEquation(x) 
	#else
	#define glBlendEquation                 glBlendEquationOES	
	#endif
	
	//#define glClearDepth                glClearDepthf
    #define glBindVertexArray           glBindVertexArrayOESEXT
    #define glGenVertexArrays           glGenVertexArraysOESEXT
    #define glDeleteVertexArrays        glDeleteVertexArraysOESEXT

    extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT;
    extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT;
    extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT;

	extern void ExternInitOpenGL();
	
#endif
