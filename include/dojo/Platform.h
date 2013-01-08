#ifndef Platform_h__
#define Platform_h__

#include "dojo_common_header.h"

#include "Table.h"

namespace Dojo 
{
	class SoundManager;
	class Render;
	class InputSystem;
	class FontSystem;
	class Game;
	class Email;
	class ApplicationListener;
	
	class Platform
	{
	public:

		static Platform* createNativePlatform( const Table& config = Table::EMPTY_TABLE );

		static void shutdownPlatform();

		inline static  Platform* getSingleton()
		{
			DEBUG_ASSERT( singleton );

			return singleton;
		}
	
		Platform( const Table& configTable ) :
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

		}		
		
		virtual ~Platform()
		{
			
		}
		
		inline void setGame( Game* g )
		{
			DEBUG_ASSERT( g );
			game = g;
		}		
		
		inline Game* getGame()
		{
			return game;
		}

		inline SoundManager* getSoundManager()	{	return sound;	}
		inline Render* getRender()				{	return render;	}
		inline InputSystem* getInput()			{	return input;	}
		inline FontSystem* getFontSystem()		{	return fonts;	}
		inline double getRealFrameTime()		{	return realFrameTime;	}
		inline const String& getLocale()		{	return locale;	}
		
		inline int getScreenWidth()             {   return screenWidth;     }
		inline int getScreenHeight()            {   return screenHeight;    }
		inline int getScreenOrientation()       {   return screenOrientation;   }

		inline int getWindowWidth()             {   return windowWidth;     }
		inline int getWindowHeight()            {   return windowHeight;    }
		
		bool isPortrait()       {   return screenOrientation == DO_PORTRAIT || screenOrientation == DO_PORTRAIT_REVERSE; }
		bool isFullscreen()		{	return mFullscreen; }
		
		inline bool isRunning()					{	return running; }
		
		virtual void initialise()=0;
		virtual void shutdown()=0;
		
		///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
		virtual void prepareThreadContext()=0;

		///switches the game to windowed, if supported
		virtual void setFullscreen( bool enabled ) = 0;

		virtual void acquireContext()=0;
		virtual void present()=0;

		virtual void step( float dt )=0;
		virtual void loop( float frameTime )=0;

		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize )=0;
		
		inline void addFocusListener( ApplicationListener* f )
		{
			DEBUG_ASSERT( f );
			DEBUG_ASSERT( !focusListeners.exists( f ) );

			focusListeners.add( f );
		}

		inline void removeFocusListener( ApplicationListener* f )
		{
			DEBUG_ASSERT( f  );

			focusListeners.remove( f );
		}

		virtual bool isNPOTEnabled()=0;
		
		///returns TRUE if the screen is physically "small", not dependent on resolution
		/**
			currently it is false on iPhone and iPod
		*/
		virtual bool isSmallScreen()
		{
			return false;
		}
		
		virtual String getAppDataPath()=0;
		virtual String getRootPath()=0;
		
		const Table& getUserConfiguration()
		{
			return config;
		}

		uint loadFileContent( char*& bufptr, const String& path );
				
		void getFilePathsForType( const String& type, const String& path, std::vector<String>& out );		
		void load( Table* dest, const String& absPath = String::EMPTY );
		void save( Table* table, const String& absPath = String::EMPTY );
		
		virtual bool isSystemSoundInUse()
		{
			return false;
		}

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

		static Platform* singleton;
		
		int screenWidth, screenHeight, windowWidth, windowHeight;
		Orientation screenOrientation;
		
		String locale;

		Table config;

		bool running, mFullscreen, mFrameSteppingEnabled;

		Game* game;

		SoundManager* sound;
		Render* render;
		InputSystem* input;
		FontSystem* fonts;
		
		float realFrameTime;

		Dojo::Array< ApplicationListener* > focusListeners;

		String _getTablePath( Table* dest, const String& absPath );
	};
}

#endif/*
 *  Platform.h
 *  Drafted
 *
 *  Created by Tommaso Checchi on 1/24/11.
 *  Copyright 2011 none. All rights reserved.
 *
 */

