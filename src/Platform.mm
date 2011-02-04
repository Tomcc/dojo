#include "stdafx.h"

#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"

#ifdef PLATFORM_IOS
#include "IOSPlatform.h"
#else
#include "Win32Platform.h"
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
	return singleton;
}