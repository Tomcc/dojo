#include "stdafx.h"

#include "dojo/Platform.h"

#include "dojo/Utils.h"
#include "dojo/Math.h"

#if defined (PLATFORM_WIN32)
	#include "Win32Platform.h"

#elif defined( PLATFORM_OSX )
    #include "dojo/OSXPlatform.h"

#elif defined( PLATFORM_IOS )
    #include "dojo/IOSPlatform.h"

#elif defined( PLATFORM_LINUX )
    #include "dojo/LinuxPlatform.h"

#endif

using namespace Dojo;

Platform * Platform::singleton = NULL;

Platform* Platform::createNativePlatform( const Table & config )
{ 
#if defined (PLATFORM_WIN32)
    singleton = new Win32Platform( config );
    
#elif defined( PLATFORM_OSX )
    singleton = new OSXPlatform( config );
    
#elif defined( PLATFORM_IOS )
    singleton = new IOSPlatform( config );
    
#elif defined( PLATFORM_LINUX )
    singleton = new LinuxPlatform( config );
    
#endif
	return singleton;
}

void Platform::shutdownPlatform()
{
	DEBUG_ASSERT( singleton );

	singleton->shutdown();

	delete singleton;
	singleton = NULL;
}
