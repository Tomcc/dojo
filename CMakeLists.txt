cmake_minimum_required(VERSION 2.8)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(cotire)
include(CheckCXXCompilerFlag)

include_directories("include/dojo")
include_directories("dependencies/include")

if (WIN32)
    include_directories("include/dojo/win32")
    
    # prevent definition of min and max macros through inclusion of Windows.h
    add_definitions("-DNOMINMAX")
    add_definitions("-DWIN32_LEAN_AND_MEAN")
    add_definitions ("/WX")
else() 
    CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)

    if(COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
    endif()

    add_definitions ("-Wno-reorder")
    add_definitions ("-Werror")
endif()

file(GLOB common_src
	"include/dojo/*.h"
    "src/*.cpp"
)

if (WIN32) 
    file(GLOB common_src
        "include/dojo/win32/*.h"
        "src/win32/*.cpp"
    )
endif()

set(dojo_SRC ${common_src} ${platform_src})

add_library(Dojo ${dojo_SRC})

set_target_properties(Dojo PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_SOURCE_DIR}"
    COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/dependencies"
    COTIRE_ADD_UNITY_BUILD FALSE)

cotire(Dojo)