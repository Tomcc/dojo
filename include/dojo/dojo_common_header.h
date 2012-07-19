#ifndef dojo_common_header
#define dojo_common_header

#include "dojo_config.h"

#include <cmath>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cfloat>
#include <climits>
#include <stdint.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

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

	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>

	//this cant be in config.h as it breaks successive system includes
	#ifdef _DEBUG

		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>

	#endif
	
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

#elif defined( PLATFORM_OSX )
	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

	#include <OpenGL/gl.h>
	#include <OpenGL/glext.h>

#elif defined( PLATFORM_LINUX )
	#include <AL/al.h>
	#include <AL/alc.h>

	#include <GL/glew.h>
	#include <GL/glext.h>
	#include <GL/gl.h>

#elif defined( PLATFORM_IOS )
	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>

	#include <OpenAL/al.h>
	#include <OpenAL/alc.h>

#else
	#error "No Platform defined!"

#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

//string has to be included here!
#include "dojostring.h"

#endif // dojo_common_headers_h__
