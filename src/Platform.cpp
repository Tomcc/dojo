#include "stdafx.h"

#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"

#if defined (PLATFORM_WIN32)
	#include "Win32Platform.h"

#elif defined( PLATFORM_OSX )
    #include "OSXPlatform.h"

#elif defined( PLATFORM_IOS )
    #include "IOSPlatform.h"

#elif defined( PLATFORM_LINUX )
    #include "LinuxPlatform.h"

#endif

using namespace Dojo;

Platform * Platform::singleton = NULL;

Platform* Platform::createNativePlatform()
{
    
#if defined (PLATFORM_WIN32)
    singleton = new Win32Platform();
    
#elif defined( PLATFORM_OSX )
    singleton = new OSXPlatform();
    
#elif defined( PLATFORM_IOS )
    singleton = new IOSPlatform();
    
#elif defined( PLATFORM_LINUX )
    singleton = new LinuxPlatform();
    
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
