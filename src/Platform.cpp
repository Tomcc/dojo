#include "Platform.h"

#include <tinydir.h>

#include "ZipArchive.h"
#include "File.h"
#include "dojomath.h"
#include "ApplicationListener.h"
#include "WorkerPool.h"
#include "Log.h"
#include "Path.h"
#include "SoundManager.h"
#include "Renderer.h"
#include "FontSystem.h"
#include "Game.h"

#if defined (PLATFORM_WIN32)
	#include "win32/Win32Platform.h"

#elif defined( PLATFORM_OSX )
	//TODO fix this #include "OSXPlatform.h"

#elif defined( PLATFORM_IOS )
	//TODO fix this #include "IOSPlatform.h"

#elif defined( PLATFORM_LINUX )
	//TODO fix this #include "linux/LinuxPlatform.h"

#elif defined( PLATFORM_ANDROID )
	//TODO fix this #include "android/AndroidPlatform.h"

#endif

#include "StringReader.h"
#include "Game.h"

#include "LogListener.h"
#include "TimedEvent.h"
#include "Random.h"
#include "InputSystem.h"

using namespace Dojo;

std::unique_ptr<Platform> Platform::gSingletonPtr;

Platform& Platform::create(const Table& config /*= Table::EMPTY_TABLE */) {
#if defined (PLATFORM_WIN32)
	gSingletonPtr = make_unique<Win32Platform>(config);

#elif defined( PLATFORM_OSX )
    DEBUG_TODO;
    //gSingletonPtr = make_unique<OSXPlatform>(config);

#elif defined( PLATFORM_IOS )
    DEBUG_TODO;
	//gSingletonPtr = make_unique<IOSPlatform>(config);

#elif defined( PLATFORM_LINUX )
	DEBUG_TODO;
	//gSingletonPtr = make_unique<LinuxPlatform>(config);

#elif defined( PLATFORM_ANDROID )
    DEBUG_TODO;
	// android_main(nullptr); //HACK
	// gSingletonPtr = make_unique<AndroidPlatform>(config);

#endif
	return *gSingletonPtr;
}

void Platform::shutdownPlatform() {
	singleton().shutdown();

	gSingletonPtr = {};
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
	mFrameSteppingEnabled(false) {
	addZipFormat(".zip");
	addZipFormat(".dpk");

	mLog = make_unique<Log>();
	gp_log = mLog.get();

	mLogWriter = make_unique<StdoutLog>();
	mLog->addListener(*mLogWriter);

	//create thread pools
	//map the main thread to the thread pool system
	mPools.push_back(make_unique<WorkerPool>(1, false, true)); 

	//allocate cpus-1 threads
	//TODO handle asymmetric processors such as BIG.little that should use half the cores
	mPools.push_back(make_unique<WorkerPool>(std::thread::hardware_concurrency() - 1));

	for(auto&& p : mPools) {
		mAllPools.emplace(p.get());
	}
}

Platform::~Platform() {

}

void Platform::_runASyncTasks(float elapsedTime) {
	auto availableTime = game->getNativeFrameLength();

	Timer timer;

 	TimedEvent::runTimedEvents(std::chrono::high_resolution_clock::now());

//TODO increase frame time if starved
//TODO try to predict if the next task will kill the frame

	bool runAnything = false;
	size_t startFrom = Random::instance.getInt(static_cast<uint32_t>(mAllPools.size()));
	size_t i = 0;
	while(timer.getElapsedTime() < availableTime) {
		runAnything |= mAllPools[(startFrom + i) % mAllPools.size()]->runOneCallback();
		 
		//if we've visited all the pools and nothing was run, return
		if(++i == mAllPools.size()) {
			if (not runAnything) {
				break;
			}
			runAnything = false;
			i = 0;
		}
	}
}

utf::string::const_iterator Platform::_findZipExtension(utf::string_view path) {
	for (auto&& ext : mZipExtensions) {
		auto idx = path.find(ext);

		if (idx != path.end()) {
			return idx + ext.length();
		}
	}

	return path.end();
}

utf::string Platform::_replaceFoldersWithExistingZips(utf::string_view relPath) {
	//find the root (on windows it is not the first character)
	
	auto next = utf::string::const_iterator{};
	auto prev = relPath.begin();

	utf::string res;

	do {
		next = relPath.find("/", prev + 1);

		auto currentFolder = utf::string_view(prev, next);

		//for each possibile zip extension, search a zip named like that
		bool found = false;

		for (auto&& ext : mZipExtensions) {
			utf::string partialFolder = res + currentFolder + ext;

			//check if partialFolder exists as a zip file
			if(Path::isFile(partialFolder)) {
				res = partialFolder;
				found = true;
				break;
			}
		}

		if (not found) {
			res += currentFolder;
		}

		prev = next;

	}
	while (next != relPath.end());

	return res;
}

const Dojo::Platform::ZipFoldersMap& Dojo::Platform::_getZipFileMap(utf::string_view path, utf::string_view& zipPath, utf::string_view& remainder) {
	//find the innermost zip
	auto idx = _findZipExtension(path);

	zipPath = path.substr(path.begin(), idx);

	if (idx != path.end()) {
		remainder = {idx + 1, path.end()};
	}
	else {
		remainder = {};
	}

	DEBUG_ASSERT( remainder.find( ".zip" ) == remainder.end(), "Error: nested zips are not supported!" );

	//has this zip been already loaded?
	ZipFileMapping::const_iterator elem = mZipFileMaps.find(zipPath);

	if (elem != mZipFileMaps.end()) {
		return elem->second;
	}
	else {
		mZipFileMaps.emplace(zipPath.copy(), ZipFoldersMap());
		ZipFoldersMap& map = mZipFileMaps.find(zipPath)->second;

		ZipArchive zip(zipPath);
		std::vector<utf::string> zip_files;
		zip.getListAllFiles(".", zip_files);

		for (auto&& zip_file : zip_files) {
			auto parentPath = Path::getParentDirectory(zip_file);
			auto parent = map.find(parentPath);
			if (parent == map.end()) {
				map.emplace(
					parentPath.copy(),
					PathList{std::move(zip_file)}
				);
			}
			else {
				parent->second.emplace_back(std::move(zip_file));
			}
		}


		return map;
	}
}

void Platform::getFilePathsForType(utf::string_view type, utf::string_view wpath, std::vector<utf::string>& out) {
	//check if any part of the path has been replaced by a zip file, so that we're in fact in a zip file
	utf::string absPath = getResourcesPath() + _replaceFoldersWithExistingZips(wpath) + '/';

	auto idx = _findZipExtension(absPath);

	if (idx != absPath.cend()) { //there's at least one zip in the path
		//now, get the file/folder mapping in memory for the zip
		//it is cached because parsing the header from disk each time is TOO SLOW
		utf::string_view zipInternalPath, zipPath;
		const ZipFoldersMap& map = _getZipFileMap(absPath, zipPath, zipInternalPath);

		//do we have a folder named "zipInternalPath"?
		auto folderItr = map.find(zipInternalPath);

		if (folderItr != map.end()) {
			//add all the files with the needed extension
			for (utf::string filePath : folderItr->second) {
				if (Path::getFileExtension(filePath) == type) {
					utf::string path = zipPath + '/' + filePath;
					out.emplace_back(path);
				}
			}
		}
	}
	else {
		tinydir_dir dir;
		tinydir_open(&dir, absPath.bytes().data());

		while (dir.has_next)
		{
			tinydir_file file;
			tinydir_readfile(&dir, &file);

			auto fullPath = utf::string_view(file.path);

			if (not file.is_dir and Path::getFileExtension(fullPath) == type) {
				out.emplace_back(Path::makeCanonical(fullPath, true));
			}

			tinydir_next(&dir);
		}

		tinydir_close(&dir);
	}
}

std::unique_ptr<FileStream> Platform::getFile(utf::string_view path) {
	using namespace std;

	auto internalZipPathIdx = _findZipExtension(path);

	if (internalZipPathIdx == path.end()) { //normal file
		return make_unique<File>(path);
	}

	else { //open a file from a zip
		DEBUG_TODO;
		//TODO use a ZipStream
		return nullptr;
	}
}

void Platform::run(std::unique_ptr<Game> game) {
	initialize(std::move(game));

	loop();

	shutdownPlatform();
}

std::vector<uint8_t> Platform::loadFileContent(utf::string_view path) {
	auto file = getFile(path);

	if (file->open(Stream::Access::Read)) {
		std::vector<uint8_t> buf((size_t)file->getSize());

		file->read(buf.data(), buf.size());

		return buf;
	}
	return{};
}

utf::string Platform::_getTablePath(utf::string_view absPathOrName) {
	DEBUG_ASSERT(absPathOrName.not_empty(), "Cannot get a path for an unnamed table");

	if (Path::isAbsolute(absPathOrName)) {
		return absPathOrName.copy();
	}
	else { //look for this file inside the prefs

		return getAppDataPath() + '/' + absPathOrName + ".ds";
	}
}

Table Platform::load(utf::string_view absPathOrName) {
	utf::string buf;
	utf::string path = _getTablePath(absPathOrName);

	return Table::loadFromFile(path);
}

void Platform::save(const Table& src, utf::string_view absPathOrName) {
	utf::string buf;

	src.serialize(buf);

	utf::string path = _getTablePath(absPathOrName);

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
	for (auto&& l : focusListeners) {
		l->onApplicationFocusLost();
	}
}

void Platform::_fireFocusGained() {
	for (auto&& l : focusListeners) {
		l->onApplicationFocusGained();
	}
}

void Platform::_fireFreeze() {
	for (auto&& l : focusListeners) {
		l->onApplicationFreeze();
	}
}

void Platform::_fireDefreeze() {
	for (auto&& l : focusListeners) {
		l->onApplicationDefreeze();
	}
}

void Platform::_fireTermination() {
	for (auto&& l : focusListeners) {
		l->onApplicationTermination();
	}
}

void Dojo::Platform::addApplicationListener(ApplicationListener& f) {
	DEBUG_ASSERT(not focusListeners.contains(&f), "Already registered");
	focusListeners.emplace(&f);
}

void Dojo::Platform::removeApplicationListener(ApplicationListener& f) {
	DEBUG_ASSERT(focusListeners.contains(&f), "Already registered");
	focusListeners.erase(&f);
}

void Dojo::Platform::addWorkerPool(WorkerPool& pool) {
	DEBUG_ASSERT(not mAllPools.contains(&pool), "Already registered");
	mAllPools.emplace(&pool);
}

void Dojo::Platform::removeWorkerPool(WorkerPool& pool) {
	DEBUG_ASSERT(mAllPools.contains(&pool), "Already registered");
	mAllPools.erase(&pool);
}
