#include "stdafx.h"

#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"

#if defined (PLATFORM_WIN32)
	#include "win32/Win32Platform.h"

#elif defined( PLATFORM_OSX )
    #include "OSXPlatform.h"

#elif defined( PLATFORM_IOS )
    #include "IOSPlatform.h"

#elif defined( PLATFORM_LINUX )
    #include "linux/LinuxPlatform.h"

#endif

#include "StringReader.h"
#include "Game.h"

#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>

using namespace Dojo;

Platform * Platform::singleton = NULL;

Platform* Platform::createNativePlatform( const Table & config )
{ 
#if defined (PLATFORM_WIN32)
    singleton = new Win32Platform( config );
    
#elif defined( PLATFORM_OSX )
    singleton = new OSXPlatform( config );
    
#elif defined( PLATFORM_IOS )
    singleton = new IOSPlatform( config );
    
#elif defined( PLATFORM_LINUX )
    singleton = new LinuxPlatform( config );
    
#endif
	return singleton;
}

void Platform::shutdownPlatform()
{
	DEBUG_ASSERT( singleton );

	singleton->shutdown();

	SAFE_DELETE( singleton );
}

void Platform::getFilePathsForType( const String& type, const String& wpath, std::vector<String>& out )
{
	try
	{				
		Poco::DirectoryIterator itr( (getRootPath() + "/" + wpath).ASCII() );
		Poco::DirectoryIterator end;
		
		String extension = type;
		
		while( itr != end )
		{			
			String path = itr->path();
			
			if( Utils::getFileExtension( path ) == extension )
			{
				Utils::makeCanonicalPath( path );
				
				out.push_back( path );
			}
			++itr;
		}
	}
	catch ( ... )
	{
		
	}	
}

uint Platform::loadFileContent( char*& bufptr, const String& path )
{	
	using namespace std;
	
	fstream file( path.ASCII().c_str(), ios_base::in | ios_base::ate | ios_base::binary );
	
	if( !file.is_open() )
		return 0;
	
	int size = (int)file.tellg();
	file.seekg(0);
	
	bufptr = (char*)malloc( size );
	
	file.read( bufptr, size );
	
	file.close();
	
	return size;
}

String Platform::_getTablePath( Table* dest, const String& absPath )
{
	if( absPath.size() == 0 )
	{
		DEBUG_ASSERT( dest->hasName() );
		
		//cerca tra le user prefs un file con lo stesso nome
		return getAppDataPath() + '/' + game->getName() + '/' + dest->getName() + ".ds";
	}
	else
		return absPath;
}

void Platform::load( Table* dest, const String& absPath )
{
	DEBUG_ASSERT( dest );
	
	using namespace std;
	
	String buf;
	String path = _getTablePath( dest, absPath );
	
	FILE* file = fopen( path.ASCII().c_str(), "rb" );
	
	if( !file )
	{
		DEBUG_MESSAGE( "WARNING: Cannot load table " << path.ASCII() );
		return;
	}
	
	fseek( file, 0, SEEK_END);
	uint uchars = ftell (file)/ sizeof( unichar );
	
	fseek( file, 0, SEEK_SET );
	
	buf.resize( uchars, 0 ); //reserve actual bytes
	
	fread( (void*)buf.data(), sizeof( unichar ), uchars, file );
	
	fclose( file );
	
	dest->setName( Utils::getFileName( path ) );
	
	StringReader reader( buf );
	dest->deserialize( reader );
}

void Platform::save( Table* src, const String& absPath )
{
	DEBUG_ASSERT( src );
	
	using namespace std;
	
	String buf;
	
	src->serialize( buf );
	
	std::string path = _getTablePath(src, absPath).ASCII();
	FILE* f = fopen( path.c_str(), "wb" );
	
	if( !f )
	{
		DEBUG_MESSAGE( "WARNING: Table parent directory not found!" );
		DEBUG_MESSAGE( path );
	}
	DEBUG_ASSERT( f );
	
	fwrite( buf.data(), sizeof( unichar ), buf.size(), f );
	
	fclose( f );
}
