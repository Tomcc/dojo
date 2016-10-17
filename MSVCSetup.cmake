
if (WIN32)
    # prevent definition of min and max macros through inclusion of Windows.h
    add_definitions("-DNOMINMAX=1 -DWIN32_LEAN_AND_MEAN=1")
endif()

if (MSVC)
	string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W3 /MP /arch:SSE2 /fp:fast")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Oi /GL /GF")

	add_definitions("-D_HAS_EXCEPTIONS=0")
endif()