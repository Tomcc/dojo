#ifndef Platform_h__
#define Platform_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo 
{
	class SoundManager;
	class Render;
	class TouchSource;
	class Game;

#ifdef PLATFORM_IOS
	class EAGLContext;
#endif

	class Platform : public BaseObject
	{
	public:

		static Platform* createNativePlatform();

		inline static  Platform* getSingleton()
		{
			DEBUG_ASSERT( singleton );

			return singleton;
		}
	
		Platform() :
		game( NULL ),
		render( NULL ),
		sound( NULL ),
		input( NULL )
		{

		}

		inline SoundManager* getSoundManager()	{	return sound;	}
		inline Render* getRender()				{	return render;	}
		inline TouchSource* getInput()			{	return input;	}

		inline void setGame( Game* g )
		{
			DEBUG_ASSERT( g );
			game = g;
		}

		virtual void initialise()=0;
		virtual void shutdown()=0;

		virtual void acquireContext()=0;
		virtual void present()=0;

		virtual void loop()=0;

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )=0;
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )=0;
		virtual uint loadFileContent( char*& bufptr, const std::string& path )=0;
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height )=0;
		

	protected:

		static Platform* singleton;

		Game* game;

		SoundManager* sound;
		Render* render;
		TouchSource* input;
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

