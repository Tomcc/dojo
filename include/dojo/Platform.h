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
	class Table;

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
		game( NULL ),
		render( NULL ),
		sound( NULL ),
		input( NULL ),
		config( configTable ),
		running( false ),
		realFrameTime( 0 )
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

		virtual void acquireContext()=0;
		virtual void present()=0;

		virtual void step( float dt )=0;
		virtual void loop( float frameTime )=0;

		virtual String getCompleteFilePath( const String& name, const String& type, const String& path )=0;
		virtual void getFilePathsForType( const String& type, const String& path, std::vector<String>& out )=0;
		virtual uint loadFileContent( char*& bufptr, const String& path )=0;
		virtual void loadPNGContent( void*& bufptr, const String& path, uint& width, uint& height )=0;
		
		///loads the given file in a buffer - WARNING not every format is supported on every platform
		virtual uint loadAudioFileContent( ALuint& buffer, const String& path )=0;

		virtual void load( Table* dest, const String& absPath = String::EMPTY )=0;
		virtual void save( Table* table, const String& absPath = String::EMPTY )=0;
		
		virtual bool isSystemSoundInUse()
		{
			return false;
		}

		virtual void openWebPage( const String& site )=0;

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

