#ifndef ANDROIDPLATFORM_H
#define ANDROIDPLATFORM_H

#include "Platform.h"
#include "InputSystem.h"
#include "SoundManager.h"
#include "Keyboard.h"
#include "Vector.h"

#ifdef PLATFORM_ANDROID

#include <string>
#include "AndroidGLExtern.h"
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include "Timer.h"

extern "C" {
	void android_main(struct android_app* state);
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
		virtual void loop();

		virtual void setFullscreen( bool fullscreen )
		{
			//android can only fullscreen 
		}


		virtual bool isNPOTEnabled()
		{
			DEBUG_MESSAGE("AndroidPlatform::isNPOTEnabled");
			return false; 
		}
                ///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
                virtual void prepareThreadContext(){
			//???
		}
		
		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int & pixelSize );
		virtual const String& getAppDataPath();
		virtual const String& getResourcesPath();
		virtual const String& getRootPath();		
		
		
		
		//TODO
		virtual void openWebPage( const String& site ){}
		

	protected:
		
		void ResetDisplay();
		int32_t width, height;	
		String apkdir;
		String apkdirResources;
		String dirAppData;
		//Keyboard
		Keyboard androidKeyboard;		
		//android	
		//app manager
		struct android_app* app;
		ASensorManager* sensorManager;
		const ASensor* accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;
		//android accelerometer
		void UpdateEvent();
		//timers		
		Timer mStepTimer;
		Timer frameTimer;
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
