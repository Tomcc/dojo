#ifndef DebugUtils_h__
#define DebugUtils_h__ 

#ifdef _DEBUG
	
	#define DEBUG_ASSERT_IMPL( T, MSG, INFO )	{if( !(T) )	{ Dojo::gp_assert_handler( MSG, #T, INFO, __LINE__, __FILE__, __FUNCTION__ ); }}
	#define DEBUG_ASSERT_INFO( T, MSG, INFO )	DEBUG_ASSERT_IMPL( T, MSG, (INFO).ASCII().c_str() )
	#define DEBUG_ASSERT( T, MSG )				DEBUG_ASSERT_IMPL( T, MSG, NULL )
	#define DEBUG_ASSERT_N( T )					DEBUG_ASSERT( T, "Internal error" );

	#define DEBUG_TODO DEBUG_ASSERT( false, "METHOD NOT IMPLEMENTED" )
	#define DEBUG_FAIL( MSG ) DEBUG_ASSERT( false, MSG )
	//fix...
	#if defined( __ANDROID__ )	
		#include <sstream>
		#include <string>
		#include <android/log.h>
		#define LOG_TAG "DOJO"
		//buffer output
		extern std::stringstream  debug_stream_android;		
		//
		#define __std_andr_cout debug_stream_android.str(std::string()); debug_stream_android
		#define __std_andr_flush __android_log_print( ANDROID_LOG_ERROR, LOG_TAG, "%s", debug_stream_android.str().c_str() );
		#define DEBUG_OUT( T ) __std_andr_cout<< T << ' '; __std_andr_flush
		#define DEBUG_MESSAGE( T ) __std_andr_cout << T << '\n'; __std_andr_flush
		
		#else
		
		#define DEBUG_OUT( T ) std::cout << T << ' '
		#define DEBUG_MESSAGE( T ) std::cout << T << std::endl

	#endif
	
	#define ___DEFINETOSTRING(X) #X
	#define ___LINETOSTRING ___DEFINETOSTRING(__LINE__)
	#define CHECK_GL_ERROR	Dojo::DEFAULT_CHECK_GL_ERROR_HANDLER(__FILE__,___LINETOSTRING)
	
#else

//TODO turn these into warnings?

	#define DEBUG_ASSERT_IMPL( T, MSG, INFO ) {}
	#define DEBUG_ASSERT_INFO( T, MSG, INFO ) {}
	#define DEBUG_ASSERT( T, MSG ) {}
	#define DEBUG_ASSERT_N( T ) {}
	#define DEBUG_TODO {}
	#define DEBUG_FAIL {}
	#define DEBUG_OUT( T ) {}
	#define DEBUG_MESSAGE( T ) {}
	#define CHECK_GL_ERROR {}
	
#endif

//a delete to ensure the deleted object was valid
#define SAFE_DELETE( T ) { DEBUG_ASSERT( T, "Deleting a NULL pointer" ); delete T; T = NULL; }

//synonyms
#define DEBUG_DEPRECATED DEBUG_TODO


namespace Dojo
{
	///the assertion handler type
	typedef void (*AssertHandlerPtr)(const char*, const char*, const char*, int, const char*, const char* );

	///the globally define assertion handler pointer
	extern AssertHandlerPtr gp_assert_handler;

	///the default assertion handler
	void DEFAULT_ASSERT_HANDLER( const char* desc, const char* arg, const char*, int line, const char* file, const char* function );
	
	///the default openGL assertion handler
	void DEFAULT_CHECK_GL_ERROR_HANDLER(const char *file_source,const char* line_source);
}

#endif // DebugUtils_h__
