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
#include <unordered_set>
#include <memory>
#include <functional>
#include <queue>
#include <atomic>
#include <thread>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <map>

//TODO move as many libraries as possible as inner dependencies, stop pushing them on users

#ifdef _MSC_VER
	#pragma warning(disable:4201)
#endif
#define GLM_FORCE_RADIANS //for sanity - why would it use degrees in places by default?
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _MSC_VER
	#pragma warning(4:4201)
#endif

#include "SmallSet.h"

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

#include "dojo_gl_header.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include "DebugUtils.h"

//keycode.h is a plain enum
#include "KeyCode.h"

//some STL glue
template< typename T >
using Unique = std::unique_ptr < T >;

template<typename T>
using Shared = std::shared_ptr < T >;

namespace Dojo {
	typedef int64_t RandomSeed;
}

using std::make_unique;
using std::make_shared;

#ifdef _MSC_VER
	//enable additional pragmas on MSVC
	#pragma warning(3:4062) //incomplete switch
	#pragma warning(3:4265) //'class': class has virtual functions, but destructor is not virtual
	#pragma warning(3:4296) //expression is always false
	#pragma warning(3:4701) //use of uninitialized variable
	#pragma warning(3:4702) //unreachable code
	#pragma warning(3:4189) //variable assigned but never used
	#pragma warning(4:4242)
	#pragma warning(default: 4254)

	#pragma warning(disable:4100) //unreferenced formal parameters are ok
	#pragma warning(disable:4458) //there's way too much shadowing here //TODO remove shadowing?
	#pragma warning(disable:4512) //what's this even?
	#pragma warning(disable:4503) //template name is too long and was truncated
#endif

#define UNUSED(X) ((void)(X))

//TODO this stuff could be split off in another library?
#include "UTFString.h"
#include "dojostring.h"

#include "enum_cast.h"

