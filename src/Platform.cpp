
#include "stdafx.h"

#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"
#include "ApplicationListener.h"

#if defined (PLATFORM_WIN32)
	#include "win32/Win32Platform.h"
	
#elif defined( PLATFORM_OSX )
    #include "OSXPlatform.h"

#elif defined( PLATFORM_IOS )
    #include "IOSPlatform.h"

#elif defined( PLATFORM_LINUX )
    #include "linux/LinuxPlatform.h"
	
#elif defined ( PLATFORM_ANDROID )
	#include "android/AndroidPlatform.h"

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
		
		//look for this file inside the prefs
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
	
	Table::loadFromFile( dest, path );
}

void Platform::save( Table* src, const String& absPath )
{
	DEBUG_ASSERT( src );
	
	using namespace std;
	
	String buf;
	
	src->serialize( buf );
	
	String path = _getTablePath(src, absPath);
	FILE* f = fopen( path.ASCII().c_str(), "w" );
	
	if( !f )
	{
		DEBUG_MESSAGE( "WARNING: Table parent directory not found!" );
		DEBUG_MESSAGE( path.c_str() );
	}
	DEBUG_ASSERT( f );
	
	std::string utf8; //TODO a real UTF8 conversion

	for( unichar c : buf )
		utf8 += (char)c;

	fwrite( utf8.c_str(), sizeof( char ), utf8.size(), f );
	
	fclose( f );
}

void Platform::_fireFocusLost()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationFocusLost();	}
void Platform::_fireFocusGained()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationFocusGained();	}
void Platform::_fireFreeze()		{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationFreeze();	}
void Platform::_fireDefreeze()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationDefreeze();	}
void Platform::_fireTermination()	{	for( int i = 0; i < focusListeners.size(); ++i )	focusListeners.at(i)->onApplicationTermination();	}
