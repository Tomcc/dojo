cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

project("Dojo")

option(IWYU "IWYU" OFF)

include (AddDojoIncludes.cmake)
include (MSVCSetup.cmake)
include (ClangGCCSetup.cmake)

include_directories("include/dojo") #private include

set(COMPILE_DEFINITIONS_RELEASE "${COMPILE_DEFINITIONS_RELEASE} -DPUBLISH")

if (WIN32)
    include_directories("include/dojo/win32")

    # prevent definition of min and max macros through inclusion of Windows.h
    add_definitions("-DNOMINMAX")
    add_definitions("-DWIN32_LEAN_AND_MEAN")
    add_definitions ("/WX")
else()

    if(APPLE)
        include_directories("include/dojo/apple")
    elseif(UNIX)
        include_directories("include/dojo/linux")
    endif()

    CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)

    if(COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()

    add_definitions ("-Wall")
    add_definitions ("-Wno-unknown-pragmas")
    add_definitions ("-Wno-reorder")
    add_definitions ("-Werror")
endif()

file(GLOB common_src
    "include/dojo/*.h"
    "include/dojo/glad/*.h"
    "src/*.cpp"
    "dependencies/include/glad/*"
)

if (WIN32)
    file(GLOB platform_src
        "include/dojo/win32/*.h"
        "src/win32/*.cpp"
    )
elseif(APPLE)
    file(GLOB platform_src
        "include/dojo/apple/OSX/*.h"
        "include/dojo/apple/*.h"
        "src/apple/*.mm"
        "src/apple/OSX/*.mm"
        )
endif()

set(dojo_SRC ${common_src} ${platform_src})

add_library(Dojo ${dojo_SRC})

if(WIN32)
    set(LIBS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/win32/lib")
    set(RELEASE_LIBS "${LIBS_ROOT}/Release")
    set(DEBUG_LIBS "${LIBS_ROOT}/Debug")

    target_link_libraries(Dojo
        debug "${DEBUG_LIBS}/freetype.lib" optimized "${RELEASE_LIBS}/freetype.lib"
        debug "${DEBUG_LIBS}/FreeImage.lib" optimized "${RELEASE_LIBS}/FreeImage.lib"
        debug "${DEBUG_LIBS}/libogg_static.lib" optimized "${RELEASE_LIBS}/libogg_static.lib"
        debug "${DEBUG_LIBS}/libvorbis_static.lib" optimized "${RELEASE_LIBS}/libvorbis_static.lib"
        debug "${DEBUG_LIBS}/libvorbisfile_static.lib" optimized "${RELEASE_LIBS}/libvorbisfile_static.lib"
        debug "${DEBUG_LIBS}/zziplib.lib" optimized "${RELEASE_LIBS}/zziplib.lib"
        "${LIBS_ROOT}/OpenAL32.lib"
        "XInput.lib"
    )
else()

    find_library(FREEIMAGE freeimage)
    find_library(OGG ogg)
    find_library(VORBIS vorbis)
    find_library(VORBISFILE vorbisfile)
    find_library(ZZIP zzip)
    find_library(OPENAL openal)
    find_library(COREFOUNDATION CoreFoundation)
    find_library(COREGRAPHICS CoreGraphics)

    target_link_libraries(Dojo ${FREETYPE_LIBRARIES} ${FREEIMAGE} ${OGG} ${VORBIS} ${VORBISFILE} ${ZZIP} ${OPENAL_LIBRARY} ${COREFOUNDATION} ${COREGRAPHICS})
endif()

if (IWYU)
    find_program(iwyu_path NAMES include-what-you-use iwyu)
    if(NOT iwyu_path)
      message(FATAL_ERROR "Could not find the program include-what-you-use")
    endif()

    set_property(TARGET Dojo PROPERTY CXX_INCLUDE_WHAT_YOU_USE "${iwyu_path}")
else()
    include(cotire)
    set(ENV{CCACHE_SLOPPINESS} "pch_defines,time_macros")
    set_target_properties(Dojo PROPERTIES
        COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src;${CMAKE_CURRENT_SOURCE_DIR}/include;"
        COTIRE_ADD_UNITY_BUILD FALSE
    )

    cotire(Dojo)
endif()
