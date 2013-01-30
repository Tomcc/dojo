#include "stdafx.h"

#include "dojo_common_header.h"
#include "DebugUtils.h"

#include "dojo_common_header.h"

using namespace Dojo;

#ifdef PLATFORM_ANDROID
//stream string buffer
std::stringstream  debug_stream_android;
#endif

//the default assert fail implementation
void Dojo::DEFAULT_ASSERT_HANDLER( const char* desc, const char* arg, int line, const char* file )
{
	DEBUG_MESSAGE( "Assertion failed: " << desc );
	DEBUG_MESSAGE( "Condition is false: " << arg );
	DEBUG_MESSAGE( "Line: " << file << " @ " << line );

	//either catch this as a breakpoint in the debugger or abort (if not debugged)
#if defined( PLATFORM_IOS ) || defined( PLATFORM_OSX )
	__asm__("int $3");
#elif defined( PLATFORM_WIN32 )
	DebugBreak();
#elif defined( PLATFORM_LINUX )
	raise( SIGTRAP );
#elif defined( PLATFORM_ANDROID )
	//https://groups.google.com/forum/#!msg/android-ndk/jZG9avVjDBY/22WaArngxqYJ
	//__asm__ ("bkpt 0");	
	//TODO implement something
	raise( SIGTRAP ); 
#else
	#error unsupported platform
#endif
}
void Dojo::DEFAULT_CHECK_GL_ERROR_HANDLER(const char *file_source,const char* line_source)
{
	String err;
	bool glerror=false;
    for (GLint g = glGetError(); g != GL_NONE; g = glGetError()) {
        glerror=true;
		switch(g)
		{
			case GL_NO_ERROR:
				return;
			case GL_INVALID_ENUM:           err = "GL_INVALID_ENUM";        break;
			case GL_INVALID_VALUE:          err = "GL_INVALID_VALUE";       break;
			case GL_INVALID_OPERATION:		err = "GL_INVALID_OPERATION";   break;
			case GL_STACK_OVERFLOW:         err = "GL_STACK_OVERFLOW";      break;
			case GL_STACK_UNDERFLOW:		err = "GL_STACK_UNDERFLOW";     break;
			case GL_OUT_OF_MEMORY:          err = "GL_OUT_OF_MEMORY";       break;
		};
		DEBUG_MESSAGE( ( "OpenGL encountered an error: " + err+" line:"+line_source+" file:"+file_source ).ASCII().c_str() );
	}
	if(glerror){ 
		DEBUG_ASSERT_MSG(0,"OpenGL encountered");
	}
}

Dojo::AssertHandlerPtr Dojo::gp_assert_handler = Dojo::DEFAULT_ASSERT_HANDLER;
