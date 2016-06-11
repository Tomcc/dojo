cmake_minimum_required(VERSION 2.8)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include(cotire)

include_directories("include/dojo")
include_directories("include/dojo/win32")
include_directories("dependencies/include")

if (WIN32)
    # prevent definition of min and max macros through inclusion of Windows.h
    add_definitions("-DNOMINMAX")
    add_definitions("-DWIN32_LEAN_AND_MEAN")
endif()

file(GLOB dojo_SRC
	"include/dojo/*.h"
	"include/dojo/win32/*.h"
    "src/*.cpp"
    "src/win32/*.cpp"
)

add_library(Dojo ${dojo_SRC})

set_target_properties(Dojo PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_SOURCE_DIR}"
    COTIRE_PREFIX_HEADER_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/dependencies"
    COTIRE_ADD_UNITY_BUILD FALSE)

cotire(Dojo)