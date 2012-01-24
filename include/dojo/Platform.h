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
		realFrameTime( 0 )
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
		
		inline bool isRunning()					{	return running; }
		
		virtual void initialise()=0;
		virtual void shutdown()=0;
		
		///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
		virtual void prepareThreadContext()=0;

		virtual void acquireContext()=0;
		virtual void present()=0;

		virtual void step( float dt )=0;
		virtual void loop( float frameTime )=0;

		virtual bool loadPNGContent( void*& bufptr, const String& path, int& width, int& height )=0;
		
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

	protected:

		static Platform* singleton;
		
		String locale;

		Table config;

		bool running;

		Game* game;

		SoundManager* sound;
		Render* render;
		InputSystem* input;
		FontSystem* fonts;
		
		float realFrameTime;
		
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

