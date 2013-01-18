#ifndef DebugUtils_h__
#define DebugUtils_h__

#ifndef _DEBUG

	#define DEBUG_ASSERT_MSG( T, MSG ) {if( !(T) ) { Dojo::gp_assert_handler( MSG, #T, __LINE__, __FILE__ ); }}
	#define DEBUG_ASSERT( T ) DEBUG_ASSERT_MSG( T, "No description" )

	#define DEBUG_TODO DEBUG_ASSERT_MSG( false, "METHOD NOT IMPLEMENTED" )
	#define DEBUG_FAIL DEBUG_ASSERT_MSG( false, "This should never happen and is likely due to a bug" )

	#if defined( PLATFORM_ANDROID )	
	
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

#else

	#define DEBUG_ASSERT_MSG( T, MSG )
	#define DEBUG_ASSERT( T )
	#define DEBUG_TODO
	#define DEBUG_FAIL
	#define DEBUG_OUT( T )
	#define DEBUG_MESSAGE( T )

#endif

//a delete to ensure the deleted object was valid
#define SAFE_DELETE( T ) { DEBUG_ASSERT( T ); delete T; T = NULL; }

//synonyms
#define DEBUG_DEPRECATED DEBUG_TODO


namespace Dojo
{
	///the assertion handler type
	typedef void (*AssertHandlerPtr)(const char*, const char*, int, const char*);

	///the globally define assertion handler pointer
	extern AssertHandlerPtr gp_assert_handler;

	///the default assertion handler
	void DEFAULT_ASSERT_HANDLER( const char* desc, const char* arg, int line, const char* file );

}

#endif // DebugUtils_h__
