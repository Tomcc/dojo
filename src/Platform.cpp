#include "Platform.h"
//private:
#include "ZipArchive.h"
//
#include "File.h"
#include "dojomath.h"
#include "ApplicationListener.h"
#include "BackgroundQueue.h"
#include "Log.h"
#include "Path.h"
#include "SoundManager.h"
#include "Renderer.h"
#include "FontSystem.h"
#include "Game.h"

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

Platform& Platform::create(const Table& config /*= Table::EMPTY_TABLE */) {
#if defined (PLATFORM_WIN32)
	singletonPtr = make_unique<Win32Platform>(config);

#elif defined( PLATFORM_OSX )
	singletonPtr = make_unique<OSXPlatform>(config);
    
#elif defined( PLATFORM_IOS )
	singletonPtr = make_unique<IOSPlatform>(config);
    
#elif defined( PLATFORM_LINUX )
	singletonPtr = make_unique<LinuxPlatform>(config);
    
#elif defined( PLATFORM_ANDROID )
    android_main(nullptr); //HACK
	singletonPtr = make_unique<AndroidPlatform>(config);
	
#endif
	return *singletonPtr;
}

void Platform::shutdownPlatform() {
	singleton().shutdown();

	singletonPtr.reset();
}

Platform::Platform(const Table& configTable) :
	config(configTable),
	running(false),
	game(nullptr),
	sound(nullptr),
	render(nullptr),
	input(nullptr),
	realFrameTime(0),
	mFullscreen(0),
	mFrameSteppingEnabled(false),
	mBackgroundQueue(nullptr) {
	addZipFormat(".zip");
	addZipFormat(".dpk");

	mLog = make_unique<Log>();
	gp_log = mLog.get();

	mLogWriter = make_unique<StdoutLog>();
	mLog->addListener(*mLogWriter);
}

Platform::~Platform() {

}

int Platform::_findZipExtension(const std::string& path) {
	for (const std::string& ext : mZipExtensions) {
		int idx = path.find(ext);
		if (idx != std::string::npos)
			return idx + ext.size();
	}
	return std::string::npos;
}

std::string Platform::_replaceFoldersWithExistingZips(const std::string& relPath) {
	//find the root (on windows it is not the first character)
	int next, prev = 0;

	std::string res = relPath.substr(0, prev);

	do {
		next = relPath.find_first_of('/', prev + 1);

		std::string currentFolder = relPath.substr(prev, next - prev);

		//for each possibile zip extension, search a zip named like that
		bool found = false;
		for (const std::string& ext : mZipExtensions) {
			std::string partialFolder = res + currentFolder + ext;

			//check if partialFolder exists as a zip file
			Poco::File zipFile(partialFolder);

			if (zipFile.exists() && zipFile.isFile()) {
				res = partialFolder;
				found = true;
				break;
			}
		}

		if (!found)
			res += currentFolder;

		prev = next;

	}
	while (next != std::string::npos);

	return res;
}

const Platform::ZipFoldersMap& Platform::_getZipFileMap(const std::string& path, std::string& zipPath, std::string& remainder) {
	//find the innermost zip 
	int idx = _findZipExtension(path);

	zipPath = path.substr(0, idx);

	if (idx < path.size())
		remainder = path.substr(idx + 1);
	else
		remainder = String::Empty;

	DEBUG_ASSERT( remainder.find( std::string(".zip") ) == std::string::npos, "Error: nested zips are not supported!" );

	//has this zip been already loaded?
	ZipFileMapping::const_iterator elem = mZipFileMaps.find(zipPath);

	if (elem != mZipFileMaps.end())
		return elem->second;
	else {
		mZipFileMaps[zipPath] = ZipFoldersMap();
		ZipFoldersMap& map = mZipFileMaps.find(zipPath)->second;

		ZipArchive zip(zipPath);
		std::vector<std::string> zip_files;
		zip.getListAllFiles(".", zip_files);

		for (int i = 0; i < zip_files.size(); ++i)
			map[Path::getDirectory(zip_files[i])].push_back(zip_files[i]);


		return map;
	}
}

void Platform::getFilePathsForType(const std::string& type, const std::string& wpath, std::vector<std::string>& out) {
	//check if any part of the path has been replaced by a zip file, so that we're in fact in a zip file
	std::string absPath = getResourcesPath() + "/" + _replaceFoldersWithExistingZips(wpath);

	int idx = _findZipExtension(absPath);
	if (idx != std::string::npos) //there's at least one zip in the path
	{
		//now, get the file/folder mapping in memory for the zip
		//it is cached because parsing the header from disk each time is TOO SLOW
		std::string zipInternalPath, zipPath;
		const ZipFoldersMap& map = _getZipFileMap(absPath, zipPath, zipInternalPath);

		//do we have a folder named "zipInternalPath"?
		auto folderItr = map.find(zipInternalPath);
		if (folderItr != map.end()) {
			//add all the files with the needed extension
			for (std::string filePath : folderItr->second) {
				if (Path::getFileExtension(filePath) == type)
					out.push_back(zipPath + "/" + filePath);
			}
		}
	}
	else {
		try {
			Poco::DirectoryIterator itr(absPath);
			Poco::DirectoryIterator end;

			std::string extension = type;

			while (itr != end) {
				std::string path = itr->path();

				if (Path::getFileExtension(path) == extension) {
					Path::makeCanonical(path);

					out.push_back(path);
				}
				++itr;
			}
		}
		catch (...) {
		}
	}
}

std::unique_ptr<FileStream> Platform::getFile(const std::string& path) {
	using namespace std;

	int internalZipPathIdx = _findZipExtension(path);

	if (internalZipPathIdx == std::string::npos) //normal file
		return make_unique<File>(path);

	else //open a file from a zip
	{
		DEBUG_TODO;
		//TODO use a ZipStream
		return nullptr;
	}
}

void Platform::run(Unique<Game> game) {
	initialize(std::move(game));

	loop();

	shutdownPlatform();
}

std::vector<byte> Platform::loadFileContent(const std::string& path) {
	auto file = getFile(path);
	if (file->open(Stream::Access::Read)) {
		auto size = file->getSize();

		std::vector<byte> buf((size_t)size);

		file->read(buf.data(), size);

		return buf;
	}
	else {
		return{};
	}
}

std::string Platform::_getTablePath(const std::string& absPathOrName) {
	DEBUG_ASSERT(absPathOrName.size() > 0, "Cannot get a path for an unnamed table");

	if (Path::isAbsolute(absPathOrName))
		return absPathOrName;
	else
	//look for this file inside the prefs
		return getAppDataPath() + '/' + absPathOrName + ".ds";
}

Table Platform::load(const std::string& absPathOrName) {
	std::string buf;
	std::string path = _getTablePath(absPathOrName);

	return Table::loadFromFile(path);
}

void Platform::save(const Table& src, const std::string& absPathOrName) {
	std::string buf;

	src.serialize(buf);

	std::string path = _getTablePath(absPathOrName);

	auto f = getFile(path);

	if (f->open(Stream::Access::ReadWrite)) {
		f->write(buf);
	}
	else {
		DEBUG_MESSAGE("WARNING: Table parent directory not found!");
		DEBUG_MESSAGE(path);
		DEBUG_ASSERT(f, "Cannot open a file for saving");
	}
}

void Platform::_fireFocusLost() {
	for (auto&& l : focusListeners)
		l->onApplicationFocusLost();
}

void Platform::_fireFocusGained() {
	for (auto&& l : focusListeners)
		l->onApplicationFocusGained();
}

void Platform::_fireFreeze() {
	for (auto&& l : focusListeners)
		l->onApplicationFreeze();
}

void Platform::_fireDefreeze() {
	for (auto&& l : focusListeners)
		l->onApplicationDefreeze();
}

void Platform::_fireTermination() {
	for (auto&& l : focusListeners)
		l->onApplicationTermination();
}
