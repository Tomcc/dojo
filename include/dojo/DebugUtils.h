#pragma once

#ifndef WIN32
	#define UNREACHABLE_BRANCH __builtin_unreachable()
#else
	#define UNREACHABLE_BRANCH __assume(0)
#endif


#ifndef PUBLISH

	#define DEBUG_ASSERT_IMPL( T, MSG, INFO )	{if( !(T) )	{ Dojo::gp_assert_handler( MSG, #T, INFO, __LINE__, __FILE__, __FUNCTION__ ); }}
	#define DEBUG_ASSERT_INFO( T, MSG, INFO )	DEBUG_ASSERT_IMPL( T, MSG, (INFO).bytes().data() )
	#define DEBUG_ASSERT( T, MSG )				DEBUG_ASSERT_IMPL( T, MSG, nullptr )
	#define DEBUG_ASSERT_N( T )					DEBUG_ASSERT( T, "Internal error" );

	#define DEBUG_TODO {Dojo::gp_assert_handler( "error", "METHOD NOT IMPLEMENTED", nullptr, __LINE__, __FILE__, __FUNCTION__ );};
	#define FAIL( MSG ) {Dojo::gp_assert_handler( "error", MSG, nullptr, __LINE__, __FILE__, __FUNCTION__ ); UNREACHABLE_BRANCH; };
	//fix...7
	#if defined( __ANDROID__ )
		#include <sstream>
		#include <string>
		#include <android/log.h>
		#define LOG_TAG "DOJO"
		//buffer output
		extern std::stringstream  debug_stream_android;
		//
		#define __std_andr_cout debug_stream_android.str(utf::string()); debug_stream_android
		#define __std_andr_flush __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, "%s", debug_stream_android.str().c_str() );
		#define DEBUG_OUT( T ) __std_andr_cout<< T << ' '; __std_andr_flush
		#define DEBUG_MESSAGE( T ) __std_andr_cout << (T) << '\n'; __std_andr_flush

	#else

		#define DEBUG_OUT( T ) std::cout << (T) << ' '
		#define DEBUG_MESSAGE( T ) Dojo::gp_log->append( ((T)) )

	#endif

	#define CHECK_GL_ERROR	Dojo::DEFAULT_CHECK_GL_ERROR_HANDLER(__FILE__,__LINE__, __FUNCTION__ )
	#define CHECK_AL_ERROR	Dojo::DEFAULT_CHECK_AL_ERROR_HANDLER(__FILE__,__LINE__, __FUNCTION__ )

#else

	//TODO turn these into warnings?

	#define DEBUG_ASSERT_IMPL( T, MSG, INFO ) {}
	#define DEBUG_ASSERT_INFO( T, MSG, INFO ) {}
	#define DEBUG_ASSERT( T, MSG ) {}
	#define DEBUG_ASSERT_N( T ) {}
	#define DEBUG_TODO {}
	#define FAIL UNREACHABLE_BRANCH
	#define DEBUG_OUT( T ) {}
	#define DEBUG_MESSAGE( T ) {}
	#define CHECK_GL_ERROR {}
	#define CHECK_AL_ERROR	true

#endif

//synonyms
#define DEBUG_DEPRECATED DEBUG_TODO


namespace Dojo {
	///the assertion handler type
	typedef void (*AssertHandlerPtr)(const char*, const char*, const char*, int, const char*, const char*);

	///the globally define assertion handler pointer
	extern AssertHandlerPtr gp_assert_handler;

	class Log;
	extern Log* gp_log;

	///the default assertion handler
	void DEFAULT_ASSERT_HANDLER(const char* desc, const char* arg, const char*, int line, const char* file, const char* function);

	///the default openGL assertion handler
	void DEFAULT_CHECK_GL_ERROR_HANDLER(const char* file_source, int line, const char* function);

	///the default openAL assertion handler
	bool DEFAULT_CHECK_AL_ERROR_HANDLER(const char* file_source, int line, const char* function);
}
