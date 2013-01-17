#include "android/AndroidPlatform.h"

#include <jni.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>

#include "Render.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"

/* android debug */
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "TH2D ENGINE OUTPUT", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "TH2D ENGINE OUTPUT", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "TH2D ENGINE OUTPUT", __VA_ARGS__))

using namespace Dojo;

/* ANDROID */
//status app
extern "C" int ANDROID_VALID_DEVICE;
//EVENT APP
extern "C" int32_t android_handle_input(struct android_app* app, AInputEvent* event) {
		if(app->userData){
			//struct OsWindow* win = (OsWindow*)app->userData;
			//input_android(win->input,event);
		}
		return 0;
	}
extern "C" void android_handle_cmd(struct android_app* app, int32_t cmd) {
		if( cmd==APP_CMD_INIT_WINDOW ){
			ANDROID_VALID_DEVICE=1; //enable Window_Flip()
			//init
		}
		else
		if(cmd==APP_CMD_RESUME){
		}
		else
		if( cmd==APP_CMD_TERM_WINDOW ){
			ANDROID_VALID_DEVICE=0; //disable Window_Flip()
			//delete
		}
		else
		if(cmd==APP_CMD_PAUSE  && ANDROID_VALID_DEVICE ){
			ANDROID_VALID_DEVICE=0; //disable Window_Flip()
			//waiting
		}
		if(app->userData){
				//struct OsWindow* win = (OsWindow*)app->userData;
				//cmd_android(win->input,cmd);
		}
	}
extern "C" int main(int argc, char *argv[]){
	return 0;
}

/* DOJO */
AndroidPlatform::AndroidPlatform(const Table& table) :
Platform(table){

}

void AndroidPlatform::initialise()
{
	DEBUG_ASSERT( game );	
	//initialize OpenGL ES and EGL
    const EGLint attribs[] = {
         // EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, //openGL ES 2.0
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
		//	EGL_ALPHA_SIZE, 0,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };
	//SET ANDROID WINDOW
	EGLint _w, _h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
    //get display
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    //set openGL configuration
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);

    //create a surface, and openGL context	
    surface = eglCreateWindowSurface(display, config,app->window, NULL);
	const EGLint attrib_list [] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE}; //openGL ES 2.0
    context = eglCreateContext(display, config, NULL, attrib_list);
    //set corrunt openGL context
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("error eglMakeCurrent");
        return;
    }
	//get WIDTH,HEIGHT
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

	//is not in pause
	isInPause=false;

	//dojo object
	render = new Render( ((int)width), ((int)height), DO_LANDSCAPE_LEFT );
	//sound  = new SoundManager();
	//input  = new TouchSource();

	//start the game
	game->begin();
}

void AndroidPlatform::shutdown()
{
	// and a cheesy fade exit
	exit(0);
}

void AndroidPlatform::acquireContext()
{
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("error eglMakeCurrent");
        return;
    }
}

void AndroidPlatform::present()
{

	if ( !isInPause ){
		return;
	} // No display or in pause....
    eglSwapBuffers( display, surface);
}

void AndroidPlatform::step( float dt )
{
	DEBUG_ASSERT( running );

	game->loop( dt);
	render->render();	
	//sound->update( dt );

}


void AndroidPlatform::loop( float frameTime )
{
	frameTimer.reset();

	float dt;
	while( running )
	{
		
		dt = frameTimer.getElapsedTime();

		if( dt > frameTime )
		{
			frameTimer.reset();

			step( dt );
		}
	}

	shutdown();
}
