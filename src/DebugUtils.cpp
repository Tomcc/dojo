#include "dojo_common_header.h"
#include "DebugUtils.h"
#include "Log.h"

#include "dojo_common_header.h"
#include "dojo_gl_header.h"
#include "dojo_al_header.h"

#ifdef PLATFORM_WIN32
    #include "dojo_win_header.h"
#endif

//let's assume that the thread who has main() remains the main thread
const std::thread::id gDebugMainThreadID = std::this_thread::get_id();

#ifdef PLATFORM_ANDROID
	//stream string buffer
	std::stringstream  debug_stream_android;
#endif

//the default assert fail implementation
void Dojo::DEFAULT_ASSERT_HANDLER(const char* desc, const char* arg, const char* info, int line, const char* file, const char* function) {
	DEBUG_MESSAGE( "Assertion failed: " + utf::string( desc ) );
	DEBUG_MESSAGE( "Condition is false: " + utf::string( arg ) );

	if (info) {
		DEBUG_MESSAGE( "with " + utf::string( info ) );
	}

	DEBUG_MESSAGE( "Function: " + utf::string(function) + " in " + utf::string(file) + " @ " + utf::to_string(line) );

	//either catch this as a breakpoint in the debugger or abort (if not debugged)
#if defined( PLATFORM_IOS ) || defined( PLATFORM_OSX )

#ifndef __arm__
	__asm__("int $3");
#else
	__asm__("trap");
#endif

#elif defined( PLATFORM_WIN32 )
	DebugBreak();
#elif defined( PLATFORM_LINUX )
	raise( SIGTRAP );
#elif defined( PLATFORM_ANDROID )
	//https://groups.google.com/forum/#!msg/android-ndk/jZG9avVjDBY/22WaArngxqYJ
	//__asm__ ("bkpt 0");
	raise( SIGTRAP );
#else
#error unsupported platform
#endif
}

bool Dojo::DEFAULT_CHECK_AL_ERROR_HANDLER(const char* file_source, int line, const char* function) {
	int error = alGetError();

	const char* err = "";

	switch (error) {
	case AL_INVALID_OPERATION:
		err = "AL_INVALID_OPERATION";
		break;

	case AL_INVALID_NAME:
		err = "AL_INVALID_NAME";
		break;

	case AL_INVALID_ENUM:
		err = "AL_INVALID_ENUM";
		break;

	case AL_INVALID_VALUE:
		err = "AL_INVALID_VALUE";
		break;

	case AL_OUT_OF_MEMORY:
		err = "AL_OUT_OF_MEMORY";
		break;

	default:
		err = "Unknown error";
		break;
	}

	//forward the assertion
	if (error != AL_NO_ERROR)
		Dojo::gp_assert_handler(
			("OpenAL encountered an error: " + utf::string(err)).bytes().data(),
			"error != AL_NO_ERROR",
			nullptr,
			line,
			file_source,
			function);

	return error == AL_NO_ERROR;
}

Dojo::AssertHandlerPtr Dojo::gp_assert_handler = Dojo::DEFAULT_ASSERT_HANDLER;

Dojo::Log* Dojo::gp_log = nullptr;
