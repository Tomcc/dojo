#include "stdafx.h"

#include "Platform.h"
//private:
#include "ZipArchive.h"
//
#include "Utils.h"
#include "File.h"
#include "dojomath.h"
#include "ApplicationListener.h"
#include "BackgroundQueue.h"
#include "Log.h"

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
#include "LogListener.h"

using namespace Dojo;

Unique<Platform> Platform::singletonPtr;

Platform& Platform::create( const Table& config /*= Table::EMPTY_TABLE */ )
{ 
#if defined (PLATFORM_WIN32)
	singletonPtr = make_unique<Win32Platform>(config);
    
#elif defined( PLATFORM_OSX )
	singletonPtr = make_unique<OSXPlatform>(config);
    
#elif defined( PLATFORM_IOS )
	singletonPtr = make_unique<IOSPlatform>(config);
    
#elif defined( PLATFORM_LINUX )
	singletonPtr = make_unique<LinuxPlatform>(config);
    
#elif defined( PLATFORM_ANDROID )
    android_main(NULL); //HACK
	singletonPtr = make_unique<AndroidPlatform>(config);
	
#endif
	return *singletonPtr;
}

void Platform::shutdownPlatform()
{
	singleton().shutdown();

	singletonPtr.reset();
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
	mFrameSteppingEnabled( false ),
	mBackgroundQueue( nullptr )
{
	addZipFormat( ".zip" );
	addZipFormat( ".dpk" );

	gp_log = mLog = new Log();
	mLog->addListener( new StdoutLog() );
}	

Platform::~Platform()
{
	SAFE_DELETE( mLog );
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

String Platform::_replaceFoldersWithExistingZips( const String& relPath )
{
	//find the root (on windows it is not the first character)
	int next, prev = 0;

	String res = relPath.substr( 0, prev );
	
	do
	{
		next = relPath.find_first_of('/', prev + 1);

		String currentFolder = relPath.substr(prev, next - prev);

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

	DEBUG_ASSERT( remainder.find( String(".zip") ) == String::npos, "Error: nested zips are not supported!" );

	//has this zip been already loaded?
	ZipFileMapping::const_iterator elem = mZipFileMaps.find( zipPath );

	if( elem != mZipFileMaps.end() )
		return elem->second;
	else
	{
		mZipFileMaps[ zipPath ] = ZipFoldersMap();
		ZipFoldersMap& map = mZipFileMaps.find( zipPath )->second;

		ZipArchive zip( zipPath );
		std::vector<String> zip_files;
		zip.getListAllFiles(".",zip_files);

		for(int i=0;i<zip_files.size();++i)
			map[ Utils::getDirectory( zip_files[i] ) ].push_back( zip_files[i] );
		

		return map;
	}
}

void Platform::getFilePathsForType( const String& type, const String& wpath, std::vector<String>& out )
{
	//check if any part of the path has been replaced by a zip file, so that we're in fact in a zip file
	String absPath = getResourcesPath() + "/" +  _replaceFoldersWithExistingZips( wpath );

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

Platform::FilePtr Platform::getFile( const String& path )
{
	using namespace std;

	int internalZipPathIdx = _findZipExtension( path );
	
	if( internalZipPathIdx == String::npos ) //normal file
		return FilePtr( new File( path ) );

	else //open a file from a zip
	{
		DEBUG_TODO;
		//TODO use a ZipStream
		return nullptr;

		/*
		String zipPath = path.substr( 0, internalZipPathIdx );
		String zipInternalPath = path.substr( internalZipPathIdx+1 );
		//OPEN ZIP
		ZipArchive zip;
		if(zip.open(zipPath)){
			//OPEN FILE IN ZIP
			auto pfile=zip.openFile(  zipInternalPath,"rb");
			if(pfile!=NULL){
				//READ FILE
				size = pfile->size();
				bufptr = (char*)malloc( size+1 );
				pfile->read(bufptr,size,1);
				//CLOSE FILE
				delete pfile;
				//ADD terminator
				bufptr[ size ] = 0;
			}
			else{
				DEBUG_MESSAGE("can't read:"<<path.ASCII());
				return 0;
			}
		}
		else{
			DEBUG_MESSAGE("can't open:"<<zipPath.ASCII());
			return 0;
		}*/
	}
}

int Platform::loadFileContent( char*& bufptr, const String& path )
{
	auto file = Unique<FileStream>( getFile( path ) );
	int size = 0;
	if( file->open() )
	{
		size = file->getSize();
		bufptr = (char*)malloc( size );

		file->read( (byte*)bufptr, size );
	}

	return size;
}

String Platform::_getTablePath( const String& absPathOrName )
{
	DEBUG_ASSERT(absPathOrName.size() > 0, "Cannot get a path for an unnamed table");
	
	if (Utils::isAbsolutePath(absPathOrName))
		return absPathOrName;
	else
		//look for this file inside the prefs
		return getAppDataPath() + '/' + absPathOrName + ".ds";
}

Table Platform::load(const String& absPathOrName)
{		
	String buf;
	String path = _getTablePath(absPathOrName);
	
	return Table::loadFromFile( path );
}

void Platform::save(const Table& src, const String& absPathOrName)
{
	String buf;
	
	src.serialize( buf );
	
	String path = _getTablePath(absPathOrName);

	DEBUG_MESSAGE( path.ASCII() );
	FILE* f = fopen( path.ASCII().c_str(), "w+" );
	
	if( f==NULL )
	{
		DEBUG_MESSAGE( "WARNING: Table parent directory not found!" );
		DEBUG_MESSAGE( path.ASCII() );
	}
	DEBUG_ASSERT( f, "Cannot open a file for saving" );
	
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
