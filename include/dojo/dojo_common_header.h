#pragma once

#include "dojo_config.h"

#include <cmath>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cfloat>
#include <climits>
#include <cstdint>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <functional>
#include <queue>
#include <atomic>
#include <thread>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <map>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifndef PLATFORM_WIN32
#include <sys/time.h>
#endif

#if defined(PLATFORM_WIN32)
	#include <al.h>
	#include <alc.h>

	//this cant be in config.h as it breaks successive system includes
	#ifdef _DEBUG

		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>

	#endif
	
	#include <windows.h>
	#include <ShellAPI.h>
	#include <ShlObj.h>

	#include <FreeImage.h>

#elif defined( PLATFORM_OSX )
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

#elif defined( PLATFORM_LINUX )
	#include <AL/al.h>
	#include <AL/alc.h>
	#include <signal.h>
	#include <stdint.h>

#elif defined( PLATFORM_IOS )
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

#elif defined( PLATFORM_ANDROID )

	#include <stdint.h>
	#include <AL/al.h>
	#include <AL/alc.h>
	#include "android/AndroidGLExtern.h"
	#include <android/log.h>
	#include <android/native_activity.h>
	#include <android_native_app_glue.h>
	
#else
	#error "No Platform defined!"
	
#endif

#include "dojo_gl_header.h" //TODO remove all the various GLEnum/GLUint from the code so we can remove GL from the interface

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "DebugUtils.h"

//string has to be included here!
#include "dojostring.h"

//keycode.h is a plain enum
#include "KeyCode.h"

//some STL glue
template< typename T >
using Unique = std::unique_ptr < T >;

template<typename T>
using Shared = std::shared_ptr < T >;

//C++14 where not available
#ifndef WIN32
template<typename T, typename ...Args>
Unique<T> make_unique(Args&& ...args) {
	return Unique<T>(new T(std::forward<Args>(args)...));
}
#else
using std::make_unique;
#endif
using std::make_shared;

