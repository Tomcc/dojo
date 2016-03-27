#pragma once

#include "dojo_common_header.h"
#include "Table.h"
#include "Log.h"
#include "PixelFormat.h"
#include "FrameSubmitter.h"

namespace Dojo {
	class SoundManager;
	class Renderer;
	class InputSystem;
	class FontSystem;
	class Game;
	class Email;
	class ApplicationListener;
	class FileStream;
	class WorkerPool;

	///Platform is the base of the engine; it runs the main loop, creates the windows and updates the Game
	/** the Platform is the first object to be initialized in a Dojo game, using the static method Platform::create() */
	class Platform : public FrameSubmitter {
	public:

		///creates the Platform
		/**
		\param config pass a non-empty Table to override the loading from the Platform User Configuration table, found in APPDATA/GAME_NAME/config.ds
		*/
		static Platform& create(const Table& config = Table::Empty);

		///shuts down the platform and the game
		static void shutdownPlatform();

		static Platform& singleton() {
			DEBUG_ASSERT( gSingletonPtr, "The Platform singleton was not created, use Platform::create() to create it" );

			return *gSingletonPtr;
		}

		virtual ~Platform();

		Game& getGame() {
			return *game;
		}

		///add a format that will be recognized as a zip package by the file loader
		void addZipFormat(const utf::string& ext) {
			DEBUG_ASSERT( ext.not_empty(), "addZipFormat: empty format string" );
			mZipExtensions.emplace_back(ext);
		}

		///returns the SoundManager instance
		SoundManager& getSoundManager() {
			return *sound;
		}

		///returns the Render instance
		Renderer& getRenderer() {
			return *render;
		}

		///returns the InputSystem instance
		InputSystem& getInput() {
			return *input;
		}

		///returns the FontSystem instance
		FontSystem& getFontSystem() {
			return *fonts;
		}

		///returns the system Log
		Log& getLog() {
			return *mLog;
		}

		///returns the default BackgroundQueue
		WorkerPool& getWorkerPool() {
			return *mWorkerPool;
		}

		///returns "real frame time" or the time actually consumed by game computations in the last frame
		/**
		useful to evaluate performance when FPS are locked by the fixed run loop.
		*/
		double getRealFrameTime() {
			return realFrameTime;
		}

		///gets the ISO locale code, es: en, it, de, se, fr
		const utf::string& getLocale() {
			return locale;
		}

		///gets the physical screen width
		uint32_t getScreenWidth() {
			return screenWidth;
		}

		///gets the physical screen height
		uint32_t getScreenHeight() {
			return screenHeight;
		}

		///gets the physical screen orientation
		Orientation getScreenOrientation() {
			return screenOrientation;
		}

		///gets the window width
		int getWindowWidth() {
			return windowWidth;
		}

		///gets the window height
		int getWindowHeight() {
			return windowHeight;
		}

		bool isPortrait() {
			return screenOrientation == DO_PORTRAIT || screenOrientation == DO_PORTRAIT_REVERSE;
		}

		///tells if it is running fullscreen
		bool isFullscreen() {
			return mFullscreen;
		}

		bool isRunning() {
			return running;
		}

		virtual void setMouseLocked(bool locked) {

		}

		///initializes the platform and calls Game::onBegin()
		virtual void initialize(Unique<Game> g) = 0;

		///shuts down the Platform and calls Game::onEnd()
		virtual void shutdown() = 0;

		///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
		virtual void prepareThreadContext() = 0;

		///switches the game to windowed, if supported
		virtual void setFullscreen(bool enabled) = 0;

		virtual void acquireContext() = 0;

		virtual void step(float dt) = 0;
		virtual void loop() = 0;

		///all-in-one method which initializes, loop()s and terminates the Platform with the given game!
		void run(Unique<Game> game);

		virtual PixelFormat loadImageFile(std::vector<byte>& imageData, const utf::string& path, uint32_t& width, uint32_t& height, int& pixelSize) = 0;

		void addApplicationListener(ApplicationListener& f) {
			focusListeners.emplace(&f);
		}

		void removeApplicationListener(ApplicationListener& f) {
			focusListeners.erase(&f);
		}

		///returns true if the device is able to manage non-power-of-2 textures
		virtual bool isNPOTEnabled() = 0;

		///returns TRUE if the screen is physically "small", not dependent on resolution
		/**
			currently it is false on iPhone and iPod
		*/
		virtual bool isSmallScreen() {
			return false;
		}

		///returns the application data path for this game (eg. to save user files)
		virtual const utf::string& getAppDataPath() = 0;
		///returns the read-only root path for this game (eg. working directory)
		virtual const utf::string& getRootPath() = 0;
		///returns the read-only resources path, eg working directory on windows or Bundle/Contents/Resources on Mac
		virtual const utf::string& getResourcesPath() = 0;

		///returns the user configuration table
		const Table& getUserConfiguration() {
			return config;
		}

		///creates a new FileStream object for the given path, but does not open it
		std::unique_ptr<FileStream> getFile(const utf::string& path);

		///loads the whole file allocating a new buffer
		std::vector<byte> loadFileContent(const utf::string& path);

		///discovers all the files with an extension in a folder
		/**\param type type extension, es "png"
		\param path path where to look for, non recursive
		\param out vector where the results are appended*/
		void getFilePathsForType(const utf::string& type, const utf::string& path, std::vector<utf::string>& out);

		///loads the table found at absPath
		/**if absPath is empty, the table file is loaded from $(Appdata)/$(GameName)/$(TableName).ds */
		Table load(const utf::string& absPathOrName);

		///saves the table found at absPath into dest
		/**if absPath is empty, the table file is saved to $(Appdata)/$(GameName)/$(TableName).ds */
		void save(const Table& table, const utf::string& absPathOrName);

		///returns true if an application is blocking the system output for this app, eg. a call or the mp3 player on iOS
		virtual bool isSystemSoundInUse() {
			return false;
		}

		///opens a web page in the default browser
		virtual void openWebPage(const utf::string& site) = 0;

		///send an email object
		virtual void sendEmail(const Email& email) {
			DEBUG_TODO;
		}

		///application listening stuff
		void _fireFocusLost();
		void _fireFocusGained();
		void _fireFreeze();
		void _fireDefreeze();
		void _fireTermination();

		void _runASyncTasks(float elapsedTime);

	protected:

		typedef std::vector<utf::string> ZipExtensionList;
		typedef std::vector<utf::string> PathList;
		typedef std::unordered_map<utf::string, PathList> ZipFoldersMap;
		typedef std::unordered_map<utf::string, ZipFoldersMap> ZipFileMapping;

		static Unique<Platform> gSingletonPtr;

		int screenWidth, screenHeight, windowWidth, windowHeight;
		Orientation screenOrientation;

		utf::string locale;

		Table config;

		bool running, mFullscreen, mFrameSteppingEnabled;

		Unique<Game> game;

		Unique<SoundManager> sound;
		Unique<Renderer> render;
		Unique<InputSystem> input;
		Unique<FontSystem> fonts;

		float realFrameTime;
		
		std::unique_ptr<Log> mLog;
		std::unique_ptr<LogListener> mLogWriter;

		Unique<WorkerPool> mWorkerPool;

		SmallSet<ApplicationListener*> focusListeners;

		///this "caches" the zip headers for faster access - each zip that has been opened has its paths cached here!
		ZipFileMapping mZipFileMaps;
		ZipExtensionList mZipExtensions;

		utf::string _getTablePath(const utf::string& absPathOrName);

		///for each component in the path, check if a directory.zip file exists
		utf::string _replaceFoldersWithExistingZips(const utf::string& absPath);

		const ZipFoldersMap& _getZipFileMap(const utf::string& path, utf::string& zipPath, utf::string& reminder);

		utf::string::const_iterator _findZipExtension(const utf::string& path);

		///protected singleton constructor
		explicit Platform(const Table& configTable);
	};
}
