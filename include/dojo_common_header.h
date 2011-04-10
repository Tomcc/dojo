#ifndef dojo_common_header
#define dojo_common_header

#include "dojo_config.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifndef PLATFORM_WIN32
#include <sys/time.h>
#endif

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <float.h>
#include <fstream>

#ifndef PLATFORM_IOS

	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>

#ifdef PLATFORM_WIN32

	#include <crtdbg.h>

	#include <al.h>
	#include <alc.h>
	#include <AL/alut.h>

	#include <Windows.h>

//this cant be in config.h as it breaks successive system includes
	#ifdef _DEBUG

		#define _CRTDBG_MAP_ALLOC
		#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
		#define new DEBUG_NEW

	#endif

#else //OSX
    #include <AL/al.h>
    #include <AL/alc.h>
    #include <AL/alut.h>
#endif

#else

	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>

	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

#endif

#undef max
#undef min

#endif // dojo_common_headers_h__
