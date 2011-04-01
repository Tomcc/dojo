#include "stdafx.h"

#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"

#ifdef PLATFORM_IOS
	#include "IOSPlatform.h"
#else
	#ifdef PLATFORM_WIN32
		#include "Win32Platform.h"
	#else
		#include "LinuxPlatform.h"
	#endif
#endif

using namespace Dojo;

Platform * Platform::singleton = NULL;

Platform* Platform::createNativePlatform()
{
#ifdef PLATFORM_IOS
	singleton = new IOSPlatform();
#endif

#ifdef PLATFORM_WIN32
	singleton = new Win32Platform();
#endif

#ifdef PLATFORM_LINUX
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
