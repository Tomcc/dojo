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

extern "C" {
	int32_t android_handle_input(struct android_app* app, AInputEvent* event);
	void android_handle_cmd(struct android_app* app, int32_t cmd);
}

namespace Dojo
{
	class AndroidPlatform : public Platform
	{
	public:

		AndroidPlatform(const Table& table);

		virtual void initialise(Game *game);
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		/*
                //TODO
		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path);
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out);
		virtual uint loadFileContent( char*& bufptr, const std::string& path);
        	virtual uint loadAudioFileContent(ALuint& i, const std::string& name);
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height);
		*/
		
		virtual void setFullscreen( bool fullscreen )
		{
			//android can only fullscreen 
		}


		virtual bool isNPOTEnabled()
		{
			return false; //it always is on windows
		}
                ///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
                virtual void prepareThreadContext(){
			//???
		}
		//TODO
		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int & pixelSize ){
			return 0;
		}

		virtual void load(  Table* dest ){}
		virtual void save(  Table* table ){}

		virtual void openWebPage( const std::string& site ){}
		
		virtual void loadPNGContent( void*& bufptr, const String& path, int& width, int& height ){}
		
		virtual String getAppDataPath(){ return String(""); }
		virtual String getRootPath(){ return String(""); }
		
		virtual void openWebPage( const String& site ){}
		
		

	protected:
		
		void ResetDisplay();
		int32_t width, height;	
		Timer frameTimer;
		
		//android	
		//app manager
		struct android_app* app;
		ASensorManager* sensorManager;
		const ASensor* accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;
		//android accelerometer
		void UpdateEvent();
		
		//openGL EGL
		int running;
		int isInPause;
		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;

		friend int32_t ::android_handle_input(struct android_app* app, AInputEvent* event) ;
		friend void ::android_handle_cmd(struct android_app* app, int32_t cmd);
		
	private:
	};

}

#endif

#endif
