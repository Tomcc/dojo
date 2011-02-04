#include "stdafx.h"

#include "Win32Platform.h"

#include <Poco/DirectoryIterator.h>

#include <Freeimage.h>

#include <OIS.h>

using namespace Dojo;

Win32Platform::Win32Platform() :
Platform()
{

}

void Win32Platform::initialise()
{
	DEBUG_TODO;
}

void Win32Platform::shutdown()
{
	DEBUG_TODO;
}

void Win32Platform::acquireContext()
{
	DEBUG_TODO;
}

void Win32Platform::present()
{
	DEBUG_TODO;
}

std::string Win32Platform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	//semplicemente il path relativo all'exe
	return path + "/" + name + "." + type; 
}

bool Win32Platform::_hasExtension( const std::string& ext, const std::string& nameOrPath )
{
	return nameOrPath.size() > ext.size() && ext == nameOrPath.substr( nameOrPath.size() - ext.size() );
}

void Win32Platform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	Poco::DirectoryIterator itr( path );

	std::string extension = "." + type;

	while( itr.name().size() )
	{
		const std::string& name = itr.name();

		if( _hasExtension( extension, name ) )
			out.push_back( itr->path() );

		++itr;
	}
}

uint Win32Platform::loadFileContent( char*& bufptr, const std::string& path )
{
	using namespace std;
	
	fstream file( path.c_str(), ios_base::in | ios_base::ate );

	if( !file.is_open() )
		return 0;

	uint size = file.tellg();

	bufptr = (char*)malloc( size );

	file.read( bufptr, size );

	file.close();

	return size;
}

void Win32Platform::loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )
{
	//puo' caricare tutto ma per coerenza meglio limitarsi alle PNG (TODO: usare freeimage su iPhone?)
	if( !_hasExtension( ".png", path ) )
		return;

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib = NULL;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(path.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(path.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, path.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return;

	//retrieve the image data
	bufptr = (void*)FreeImage_GetBits(dib);
	//get the image width and height, and size per pixel
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
}