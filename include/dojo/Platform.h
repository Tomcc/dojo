#ifndef Platform_h__
#define Platform_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo 
{
	class SoundManager;
	class Render;
	class InputSystem;
	class Game;
	class Table;

	class Platform : public BaseObject
	{
	public:

		static Platform* createNativePlatform();

		static void shutdownPlatform();

		inline static  Platform* getSingleton()
		{
			DEBUG_ASSERT( singleton );

			return singleton;
		}
	
		Platform() :
		game( NULL ),
		render( NULL ),
		sound( NULL ),
		input( NULL ),
		running( true ),
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
		inline double getRealFrameTime()		{	return realFrameTime;	}
		
		inline bool isRunning()					{	return running; }
		

		virtual void initialise()=0;
		virtual void shutdown()=0;

		virtual void acquireContext()=0;
		virtual void present()=0;

		virtual void step( float dt )=0;
		virtual void loop( float frameTime )=0;

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )=0;
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )=0;
		virtual uint loadFileContent( char*& bufptr, const std::string& path )=0;
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )=0;
		
		///loads the given file in a buffer - WARNING not every format is supported on every platform
		virtual uint loadAudioFileContent( ALuint& buffer, const std::string& path )=0;

		virtual void load( Table* dest, const std::string& absPath = "" )=0;
		virtual void save( Table* table, const std::string& absPath = "" )=0;
		
		virtual bool isSystemSoundInUse()
		{
			return false;
		}

		virtual void openWebPage( const std::string& site )=0;

	protected:

		static Platform* singleton;

		bool running;

		Game* game;

		SoundManager* sound;
		Render* render;
		InputSystem* input;
		
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

