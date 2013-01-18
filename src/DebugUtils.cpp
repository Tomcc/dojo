#include "stdafx.h"

#include "DebugUtils.h"

using namespace Dojo;

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
#else
	#error unsupported platform
#endif
}

Dojo::AssertHandlerPtr Dojo::gp_assert_handler = Dojo::DEFAULT_ASSERT_HANDLER;
