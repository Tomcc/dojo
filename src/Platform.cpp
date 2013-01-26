#include "stdafx.h"

#include "Platform.h"
//private:
#include "ZipArchive.h"
//
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

#elif defined( PLATFORM_ANDROID )
    #include "android/AndroidPlatform.h"

#endif

#include "StringReader.h"
#include "Game.h"

#include <Poco/DirectoryIterator.h>
#include <Poco/Exception.h>
//#include <Poco/Zip/ZipStream.h> not work in android, change in zzip

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
    
#elif defined( PLATFORM_ANDROID )
    android_main(NULL); //HACK
    singleton = new AndroidPlatform( config );
	
#endif
	return singleton;
}

void Platform::shutdownPlatform()
{
	singleton->shutdown();

	SAFE_DELETE( singleton );
}

Platform::Platform( const Table& configTable ) :
	config( configTable ),
	running( false ),
	game( NULL ),
	sound( NULL ),
	render( NULL ),
	input( NULL ),
	realFrameTime( 0 ),
	mFullscreen( 0 ),
	mFrameSteppingEnabled( false )
{
	addZipFormat( ".zip" );
	addZipFormat( ".dpk" );
}	

int Platform::_findZipExtension( const String & path )
{
	for( const String& ext : mZipExtensions )
	{
		int idx = path.find( ext );
		if( idx != String::npos )
			return idx + ext.size();
	}
	return String::npos;
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

		//for each possibile zip extension, search a zip named like that
		bool found = false;
		for( const String& ext : mZipExtensions )
		{
			String partialFolder = res + currentFolder + ext;

			//check if partialFolder exists as a zip file
			Poco::File zipFile( partialFolder.ASCII() );

			if( zipFile.exists() && zipFile.isFile() )
			{
				res = partialFolder;
				found = true;
				break;
			}
		}
		
		if( !found) 
			res += currentFolder;

		prev = next;

	} while( next != String::npos);
	
	return res;
}

const Platform::ZipFoldersMap& Platform::_getZipFileMap( const String& path, String& zipPath, String& remainder )
{
	//find the innermost zip 
	int idx = _findZipExtension( path );

	zipPath = path.substr( 0, idx );

	if( idx < path.size() )
		remainder = path.substr( idx+1 );
	else 
		remainder = String::EMPTY;

	DEBUG_ASSERT_MSG( remainder.find( String(".zip") ) == String::npos, "Error: nested zips are not supported!" );

	//has this zip been already loaded?
	ZipFileMapping::const_iterator elem = mZipFileMaps.find( zipPath );

	if( elem != mZipFileMaps.end() )
		return elem->second;
	else
	{
		mZipFileMaps[ zipPath ] = ZipFoldersMap();
		ZipFoldersMap& map = mZipFileMaps.find( zipPath )->second;

		ZipArchive zip( zipPath.UTF8() );
		std::vector<std::string> zip_files;
		zip.getListAllFiles(".",zip_files);

		for(std::string& itr : zip_files){
			String filePath( itr );
			map[ Utils::getDirectory( filePath ) ].push_back( filePath );
		}

		return map;
	}
}

void Platform::getFilePathsForType( const String& type, const String& wpath, std::vector<String>& out )
{
	String cleanAbsPath = getResourcesPath() + "/" + wpath;

	//check if any part of the path has been replaced by a zip file, so that we're in fact in a zip file
	String absPath = _replaceFoldersWithExistingZips( cleanAbsPath );

	int idx = _findZipExtension( absPath );
	if( idx != String::npos ) //there's at least one zip in the path
	{
		//now, get the file/folder mapping in memory for the zip
		//it is cached because parsing the header from disk each time is TOO SLOW
		String zipInternalPath, zipPath;
		const ZipFoldersMap& map = _getZipFileMap( absPath, zipPath, zipInternalPath );

		//do we have a folder named "zipInternalPath"?
		auto folderItr = map.find( zipInternalPath );
		if( folderItr != map.end() )
		{
			//add all the files with the needed extension
			for( String filePath : folderItr->second )
			{
				if( Utils::getFileExtension( filePath ) == type )
					out.push_back( zipPath + "/" + filePath );
			}
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
	int internalZipPathIdx = _findZipExtension( path );
	if( internalZipPathIdx == String::npos )
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
		String zipPath = path.substr( 0, internalZipPathIdx );
		String zipInternalPath = path.substr( internalZipPathIdx+1 );
		//OPEN ZIP
		ZipArchive zip;
		zip.open(zipPath.UTF8());
		//OPEN FILE IN ZIP
		auto pfile=zip.openFile(  zipInternalPath.UTF8(),"rb");
		//READ FILE
		size = pfile->size();
		bufptr = (char*)malloc( size+1 );
		pfile->read(bufptr,size,1);
		//CLOSE FILE
		delete pfile;
		//ADD terminator
		bufptr[ size ] = 0;
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
