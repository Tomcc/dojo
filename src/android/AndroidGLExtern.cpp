#include "AndroidGLExtern.h"

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT=NULL;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT=NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT=NULL;

void ExternInitOpenGL(){
	
	glGenVertexArraysOESEXT=(PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
	glBindVertexArrayOESEXT=(PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
	glDeleteVertexArraysOESEXT=(PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");

}
