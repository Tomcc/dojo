#pragma once

#include "dojo_common_header.h"
#include "Table.h"
#include "Log.h"

namespace Dojo 
{
	class SoundManager;
	class Renderer;
	class InputSystem;
	class FontSystem;
	class Game;
	class Email;
	class ApplicationListener;
	class FileStream;
	class BackgroundQueue;
	
	///Platform is the base of the engine; it runs the main loop, creates the windows and updates the Game
	/** the Platform is the first object to be initialized in a Dojo game, using the static method Platform::create() */
	class Platform
	{
	public:

		typedef Unique< FileStream > FilePtr;

		///creates the Platform
		/**
		\param config pass a non-empty Table to override the loading from the Platform User Configuration table, found in APPDATA/GAME_NAME/config.ds
		*/
		static Platform& create( const Table& config = Table::EMPTY );

		///shuts down the platform and the game
		static void shutdownPlatform();

		static Platform& singleton()
		{
			DEBUG_ASSERT( singletonPtr, "The Platform singleton was not created, use Platform::create() to create it" );

			return *singletonPtr;
		}
		
		virtual ~Platform();
		
		Game& getGame()
		{
			return *game;
		}

		///add a format that will be recognized as a zip package by the file loader
		void addZipFormat( const String& ext )
		{
			DEBUG_ASSERT( ext.size(), "addZipFormat: empty format string" );
			mZipExtensions.push_back( ext );
		}

		///returns the SoundManager instance
		SoundManager& getSoundManager()	{	return *sound;	}
		///returns the Render instance
		Renderer& getRenderer()				{	return *render;	}
		///returns the InputSystem instance
		InputSystem& getInput()			{	return *input;	}
		///returns the FontSystem instance
		FontSystem& getFontSystem()		{	return *fonts;	}
		///returns the system Log
		Log& getLog()						{	return *mLog;	}

		///returns the default BackgroundQueue
		BackgroundQueue* getBackgroundQueue()	{	return mBackgroundQueue;	}

		///returns "real frame time" or the time actually consumed by game computations in the last frame
		/**
		useful to evaluate performance when FPS are locked by the fixed run loop.
		*/
		double getRealFrameTime()		{	return realFrameTime;	}

		///gets the ISO locale code, es: en, it, de, se, fr
		const String& getLocale()		{	return locale;	}
		
		///gets the physical screen width
		int getScreenWidth()             {   return screenWidth;     }
		///gets the physical screen height
		int getScreenHeight()			{   return screenHeight;    }

		///gets the physical screen orientation
		Orientation getScreenOrientation()       {   return screenOrientation;   }

		///gets the window width
		int getWindowWidth()             {   return windowWidth;     }
		///gets the window height
		int getWindowHeight()            {   return windowHeight;    }

		bool isPortrait()       {   return screenOrientation == DO_PORTRAIT || screenOrientation == DO_PORTRAIT_REVERSE; }

		///tells if it is running fullscreen
		bool isFullscreen()		{	return mFullscreen; }
		
		bool isRunning()					{	return running; }

		virtual void setMouseLocked(bool locked) {

		}
		
		///initializes the platform and calls Game::onBegin()
		virtual void initialize( Game* game )=0;

		///shuts down the Platform and calls Game::onEnd()
		virtual void shutdown()=0;
		
		///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
		virtual void prepareThreadContext()=0;

		///switches the game to windowed, if supported
		virtual void setFullscreen( bool enabled ) = 0;

		virtual void acquireContext()=0;
		virtual void present()=0;

		virtual void step( float dt )=0;
		virtual void loop()=0;

		///all-in-one method which initializes, loop()s and terminates the Platform with the given game!
		void run( Game* game )
		{
			initialize( game );

			loop( );

			shutdownPlatform();
		}

		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize )=0;
		
		void addApplicationListener( ApplicationListener* f )
		{
			DEBUG_ASSERT( f, "addApplicationListener: null listener passed" );
			DEBUG_ASSERT( !focusListeners.exists( f ), "addApplicationListener: this listener is already registered" );

			focusListeners.add( f );
		}

		void removeApplicationListener( ApplicationListener* f )
		{
			DEBUG_ASSERT( f, "removeApplicationListener: null listener passed" );

			focusListeners.remove( f );
		}

		///returns true if the device is able to manage non-power-of-2 textures
		virtual bool isNPOTEnabled()=0;
		
		///returns TRUE if the screen is physically "small", not dependent on resolution
		/**
			currently it is false on iPhone and iPod
		*/
		virtual bool isSmallScreen()
		{
			return false;
		}
		
		///returns the application data path for this game (eg. to save user files)
		virtual const String& getAppDataPath()=0;
		///returns the read-only root path for this game (eg. working directory)
		virtual const String& getRootPath()=0;
		///returns the read-only resources path, eg working directory on windows or Bundle/Contents/Resources on Mac
		virtual const String& getResourcesPath()=0;
		
		///returns the user configuration table
		const Table& getUserConfiguration()
		{
			return config;
		}

		///creates a new FileStream object for the given path, but does not open it
		FilePtr getFile( const String& path );

		///loads the whole file allocating a new buffer
		int loadFileContent( char*& bufptr, const String& path );
				
		///discovers all the files with an extension in a folder
		/**\param type type extension, es "png"
		\param path path where to look for, non recursive
		\param out vector where the results are appended*/
		void getFilePathsForType( const String& type, const String& path, std::vector<String>& out );

		///loads the table found at absPath
		/**if absPath is empty, the table file is loaded from $(Appdata)/$(GameName)/$(TableName).ds */
		Table load( const String& absPathOrName );
		
		///saves the table found at absPath into dest
		/**if absPath is empty, the table file is saved to $(Appdata)/$(GameName)/$(TableName).ds */
		void save( const Table& table, const String& absPathOrName );
		
		///returns true if an application is blocking the system output for this app, eg. a call or the mp3 player on iOS
		virtual bool isSystemSoundInUse()
		{
			return false;
		}

		///opens a web page in the default browser
		virtual void openWebPage( const String& site )=0;
		
		///send an email object
		virtual void sendEmail( const Email& email )
		{
			DEBUG_TODO;
		}

		///application listening stuff
		void _fireFocusLost();
		void _fireFocusGained();
		void _fireFreeze();
		void _fireDefreeze();
		void _fireTermination();

	protected:

		typedef std::vector< String > ZipExtensionList;
		typedef std::vector< String > PathList;
		typedef std::unordered_map< String, PathList > ZipFoldersMap;
		typedef std::unordered_map< String, ZipFoldersMap > ZipFileMapping;

		static Unique<Platform> singletonPtr;
		
		int screenWidth, screenHeight, windowWidth, windowHeight;
		Orientation screenOrientation;
		
		String locale;

		Table config;

		bool running, mFullscreen, mFrameSteppingEnabled;

		Game* game;

		SoundManager* sound;
		Renderer* render;
		InputSystem* input;
		FontSystem* fonts;
		
		float realFrameTime;

		Log* mLog;
		BackgroundQueue* mBackgroundQueue;

		Array< ApplicationListener* > focusListeners;

		///this "caches" the zip headers for faster access - each zip that has been opened has its paths cached here!
		ZipFileMapping mZipFileMaps;
		ZipExtensionList mZipExtensions;

		String _getTablePath( const String& absPathOrName );

		///for each component in the path, check if a directory.zip file exists
		String _replaceFoldersWithExistingZips( const String& absPath );

		const ZipFoldersMap& _getZipFileMap( const String& path, String& zipPath, String& reminder );

		int _findZipExtension( const String & path );

		///protected singleton constructor
		Platform( const Table& configTable );
	};
}
