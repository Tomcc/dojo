#ifndef Platform_h__
#define Platform_h__

#include "dojo_common_header.h"

#include "BaseObject.h"

namespace Dojo 
{
	class SoundManager;
	class Render;
	class TouchSource;

#ifdef PLATFORM_IOS
	class EAGLContext;
#endif

	class Platform : public BaseObject
	{
	public:

		static std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );

		static void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );

		static uint loadFileContent( char*& bufptr, const std::string& path );

		static void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height );
		
		Platform() :
		render( NULL ),
		sound( NULL ),
		input( NULL )
		{

		}

		void initialise();

		void shutdown();

		void acquireContext();

		void present();
		
		inline SoundManager* getSoundManager()
		{
			return sound;
		}

		inline Render* getRender()
		{
			return render;
		}

		inline TouchSource* getInput()
		{
			return input;
		}

	protected:

		SoundManager* sound;
		Render* render;
		TouchSource* input;

#ifdef PLATFORM_IOS
		EAGLContext *context;
#else

#endif
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

