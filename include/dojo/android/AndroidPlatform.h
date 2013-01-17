#ifndef ANDROIDPLATFORM_H
#define ANDROIDPLATFORM_H

#include "Platform.h"
#include "Vector.h"

#ifdef PLATFORM_ANDROID

#include <string>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include "Timer.h"


namespace Dojo
{
	class AndroidPlatform : public Platform
	{
	public:

		AndroidPlatform(const Table& table);

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path);
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out);
		virtual uint loadFileContent( char*& bufptr, const std::string& path);
        virtual uint loadAudioFileContent(ALuint& i, const std::string& name);
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height);
		
		virtual void load(  Table* dest );
		virtual void save(  Table* table );

		virtual void openWebPage( const std::string& site );
		
		virtual void loadPNGContent( void*& bufptr, const String& path, int& width, int& height );
		
		virtual String getAppDataPath();
		virtual String getRootPath();
		
		virtual void openWebPage( const String& site );
		

	protected:
	
		int32_t width, height;	
		Timer frameTimer;		
		//android	
		//app manager
		struct android_app* app;
		ASensorManager* sensorManager;
		const ASensor* accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;
		
		//openGL EGL
		int running;
		int isInPause;
		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;
		
		
	private:
	};
}

#endif

#endif