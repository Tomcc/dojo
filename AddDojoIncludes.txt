
#win32 libraries are preset, no such a thing as a package manager there
if(WIN32)
	set("OPENAL_INCLUDE_DIR" "${CMAKE_CURRENT_LIST_DIR}/dependencies/win32/include/AL")
	set("FREETYPE_INCLUDE_DIRS" "${CMAKE_CURRENT_LIST_DIR}/dependencies/win32/include/freetype2")

	include_directories(
		"${CMAKE_CURRENT_LIST_DIR}/dependencies/win32/include"
	)
else()
	INCLUDE(FindOpenAL)
	INCLUDE(FindFreetype)

	#add this to add /usr/local/include to XCode, it needs some help as usual
	include_directories(${CMAKE_INSTALL_PREFIX}/include)
endif()

include_directories(
  "${OPENAL_INCLUDE_DIR}"
  "${FREETYPE_INCLUDE_DIRS}"
  "${CMAKE_CURRENT_LIST_DIR}/dependencies/include"
  "${CMAKE_CURRENT_LIST_DIR}/include"
)
