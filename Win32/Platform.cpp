#include "stdafx.h"

#include "Platform.h"

using namespace Dojo;

std::string Platform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	DEBUG_TODO;

	return "";
}

void Platform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	DEBUG_TODO;
}

uint Platform::loadFileContent( char*& bufptr, const std::string& path )
{
	DEBUG_TODO;

	return 0;
}

void Platform::loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )
{
	DEBUG_TODO;
}