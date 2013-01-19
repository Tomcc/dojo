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

#endif

#include "StringReader.h"
#include "Game.h"

#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>
#include <Poco/Zip/ZipStream.h>

using namespace Dojo;

Platform * Platform::singleton = NULL;

Platform* Platform::create( const Table & config )
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
	singleton->shutdown();

	SAFE_DELETE( singleton );
}

bool Platform::_pathContainsZip( const String& path )
{
	return path.find( String(".zip") ) != String::npos;
}

String Platform::_replaceFoldersWithExistingZips( const String& absPath )
{
	//find the root (on windows it is not the first character)
	int next, prev = absPath.find_first_of( '/' );

	String res = absPath.substr( 0, prev );

	do
	{
		next = absPath.find_first_of( '/', prev+1 );

		String currentFolder = absPath.substr( prev, next-prev );
		String partialFolder = res + currentFolder + ".zip";

		//check if partialFolder exists as a zip file
		Poco::File zipFile( partialFolder.ASCII() );

		if( zipFile.exists() && zipFile.isFile() )
			res = partialFolder;
		else
			res += currentFolder;

		prev = next;

	} while( next != String::npos);
	
	return res;
}

Poco::Zip::ZipArchive Platform::_openInnerMostZip( const String& path, String& zipPath, String& reminder )
{
	//find the innermost zip 
	int idx = path.find( String(".zip") ) + 5;

	zipPath = path.substr( 0, idx-1 );

	reminder = (idx < path.size()) ? path.substr( idx ) : String::EMPTY;
	std::ifstream file( zipPath.UTF8(), std::ios_base::in | std::ios_base::binary );

	Poco::Zip::ZipArchive arch( file );

	return arch;

	//TODO open zips inside zips
}

void Platform::getFilePathsForType( const String& type, const String& wpath, std::vector<String>& out )
{
	String cleanAbsPath = getResourcesPath() + "/" + wpath;

	//check if any part of the path has been replaced by a zip file, so that we're in fact in a zip file
	String absPath = _replaceFoldersWithExistingZips( cleanAbsPath );

	if( _pathContainsZip( absPath ) ) //there's at least one zip in the path
	{
		//now, find the innermost zip, that is, the deepest zip which contains this path
		String zipInternalPath, zipPath;
		Poco::Zip::ZipArchive zip = _openInnerMostZip( absPath, zipPath, zipInternalPath );

		auto itr = zip.fileInfoBegin();
		auto end = zip.fileInfoEnd();
		for( ; itr != end; ++itr )
		{
			//now, check if the folder is "in" the required internal folder, zipInternalPath
			String filePath( itr->second.getFileName() );
			String fileDir = Utils::getDirectory( filePath );

			if( fileDir == zipInternalPath && type == Utils::getFileExtension( filePath ) ) //if path and extension match
				out.push_back( zipPath + "/" + itr->second.getFileName() );
		}
	}
	else
	{
		try
		{				
			Poco::DirectoryIterator itr( absPath.ASCII() );
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
		catch ( ... )	{}	
	}
}

uint Platform::loadFileContent( char*& bufptr, const String& path )
{	
	using namespace std;
	
	int size = 0;
	if( !_pathContainsZip( path ) )
	{
		fstream file( path.ASCII().c_str(), ios_base::in | ios_base::ate | ios_base::binary );

		if( !file.is_open() )
			return 0;

		size = (int)file.tellg();
		file.seekg(0);

		bufptr = (char*)malloc( size );

		file.read( bufptr, size );

		file.close();
	}
	else //open a file from a zip
	{
		int idx = path.find( String(".zip") );

		String zipPath = path.substr( 0, idx + 4 );

		String zipInternalPath = path.substr( idx+5 );
		std::ifstream file( zipPath.UTF8(), std::ios_base::in | std::ios_base::binary );

		Poco::Zip::ZipArchive arch( file );

		auto elem = arch.findHeader( zipInternalPath.UTF8() );

		if( elem == arch.headerEnd() )
			return 0;

		Poco::Zip::ZipInputStream zipin (file, elem->second);

		size = elem->second.getUncompressedSize()+1;
		bufptr = (char*)malloc( size );

		zipin.read( bufptr, size );

		//add terminator
		bufptr[ size-1 ] = 0;
	}
	
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
