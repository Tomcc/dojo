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
#include "AndroidGLExtern.h"

/* android debug */
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "TH2D ENGINE OUTPUT", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "TH2D ENGINE OUTPUT", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "TH2D ENGINE OUTPUT", __VA_ARGS__))

using namespace Dojo;

/* ANDROID */
//status app
extern "C" int ANDROID_VALID_DEVICE;
//get app
extern  "C" struct android_app* GetAndroidApp();
//EVENT APP
extern "C"  int32_t android_handle_input(struct android_app* app, AInputEvent* event) {
		//get platform
		AndroidPlatform* self = app->userData ? (AndroidPlatform*)app->userData : NULL;

		unsigned int flags = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
		if (flags == AMOTION_EVENT_ACTION_DOWN || flags == AMOTION_EVENT_ACTION_POINTER_DOWN) { //down

			int count = AMotionEvent_getPointerCount(event);
			for(int i = 0;i < count ; i++) 
					input->_fireTouchBeginEvent(Dojo::Vector(AMotionEvent_getX(event, i),
							                         AMotionEvent_getY(event, i)));

		} else if (flags == AMOTION_EVENT_ACTION_UP || flags == AMOTION_EVENT_ACTION_POINTER_UP) { //up

			int count = AMotionEvent_getPointerCount(event);
			for(int i = 0;i < count ; i++) 
					input->_fireTouchEndEvent(Dojo::Vector(AMotionEvent_getX(event, i),
							                       AMotionEvent_getY(event, i)));

		} else if (flags == AMOTION_EVENT_ACTION_MOVE) {                                           //move

			int count = AMotionEvent_getPointerCount(event);
			for(int i = 0;i < count ; i++) 
					input->_fireTouchMoveEvent(Dojo::Vector(AMotionEvent_getX(event, i),
							                        AMotionEvent_getY(event, i)));

		} else if (flags == AMOTION_EVENT_ACTION_CANCEL) {                                         //????? DOJO event??
			//save_fingers_input(input,event, IF_CANCEL);
		} else {
			return 0;
		}

		return 0;
	}
extern "C" void android_handle_cmd(struct android_app* app, int32_t cmd) {
		//get platform
		AndroidPlatform* self = app->userData ? (AndroidPlatform*)app->userData : NULL;

		if( cmd==APP_CMD_INIT_WINDOW ){
			ANDROID_VALID_DEVICE=1; //enable Window_Flip() (and FOR INPUT)
			if(self){
				//Reinizialize display
				self->ResetDisplay();
				//is RESUME
				self->isInPause=false;
			}
		}
		else
		if(cmd==APP_CMD_RESUME){
			//is RESUME
			if(self) self->isInPause=false;
		}
		else
		if( cmd==APP_CMD_TERM_WINDOW ){
			ANDROID_VALID_DEVICE=0; //disable Window_Flip() (and FOR INPUT)
			//delete
			if(self){
				//IN PAUSE
				self->isInPause=true;
				//AND IN STOP LOOP
				self->running=false;
			}
		}
		else
		if(cmd==APP_CMD_PAUSE  && ANDROID_VALID_DEVICE ){
			ANDROID_VALID_DEVICE=0; //disable Window_Flip() (and FOR INPUT)
			if(self){				
				//is IN PAUSE
				self->isInPause=true;
			}
			//waiting
		}else
		if (cmd==APP_CMD_GAINED_FOCUS){
		    // When our app gains focus, we start monitoring the accelerometer.
		   if (self != NULL && self->accelerometerSensor != NULL) {
		        ASensorEventQueue_enableSensor(self->sensorEventQueue,self->accelerometerSensor);
		        // We'd like to get 60 events per second (in us).
		        ASensorEventQueue_setEventRate(self->sensorEventQueue,self->accelerometerSensor,(1000L/60)*1000);
		    }
		}
		if(cmd==APP_CMD_LOST_FOCUS){
		    // When our app loses focus, we stop monitoring the accelerometer.
		    // This is to avoid consuming battery while not being used.
		    if (self != NULL && accelerometerSensor != NULL) {
		        ASensorEventQueue_disableSensor(self->sensorEventQueue,self->accelerometerSensor);
		    }
		}
	}

extern "C" int main(int argc, char *argv[]){
	return 0;
}

/* DOJO */
AndroidPlatform::AndroidPlatform(const Table& table) :
Platform(table){

}

void AndroidPlatform::ResetDisplay(){
 //initialize OpenGL ES and EGL
    const EGLint attribs[] = {
    //      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, //openGL ES 2.0
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
//	    EGL_ALPHA_SIZE, 0,
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
    //init opengl VBA calls
    ExternInitOpenGL();
    //is not in pause
    isInPause=false;
    //
}
void AndroidPlatform::initialise(Game *g)
{
    //set game
    game=g;
    DEBUG_ASSERT( game );
    //Set Display	
    ResetDisplay();
    //init app
    DEBUG_ASSERT( GetAndroidApp() );
    GetAndroidApp()->userData=(void*)this;
    this->app=GetAndroidApp();
    //accelerometer
    this->sensorManager = ASensorManager_getInstance();
    this->accelerometerSensor = ASensorManager_getDefaultSensor(input_android->sensorManager,ASENSOR_TYPE_ACCELEROMETER);
    //dojo object
    render = new Render( ((int)width), ((int)height), DO_LANDSCAPE_LEFT );
    input  = new TouchSource();
    sound  = new SoundManager();
    //start the game
    game->begin();
}   

void AndroidPlatform::shutdown()
{
	// and a cheesy fade exit
	if (display != EGL_NO_DISPLAY) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (context != EGL_NO_CONTEXT) {
		    eglDestroyContext(display, context);
		}
		if (surface != EGL_NO_SURFACE) {
		    eglDestroySurface(display, surface);
		}
		eglTerminate(display);
	}
 	running = 0;
    	display = EGL_NO_DISPLAY;
    	context = EGL_NO_CONTEXT;
    	surface = EGL_NO_SURFACE;
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
   // No display or in pause....
   if ( display == EGL_NO_DISPLAY || !isInPause ){
		return;
   } 
   eglSwapBuffers( display, surface);
}

void AndroidPlatform::step( float dt )
{
	DEBUG_ASSERT( running );
	//update accelerometer	
	UpdateEvent();
	//update game
	game->loop( dt );
	render->render();	
	//sound->update( dt );

}

void AndroidPlatform::UpdateEvent(){
	if(sensorEventQueue==0){
		sensorEventQueue =ASensorManager_createEventQueue(sensorManager,
								  GET_LOOPER,
								  LOOPER_ID_USER,
								  NULL, NULL);
		 ASensorEventQueue_enableSensor(sensorEventQueue,accelerometerSensor);
		// We'd like to get 60 events per second (in us).
		ASensorEventQueue_setEventRate(sensorEventQueue,
		                               accelerometerSensor,
		                               (1000L/60)*1000);
	}

	int ident,events;
	struct android_poll_source* source;
	while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0){
		if (source != NULL) source->process(GET_STATE, source);
		//GET SENSOR
                if (ident == LOOPER_ID_USER) {
                    if (accelerometerSensor != NULL) {
                        ASensorEvent event;
                        while (ASensorEventQueue_getEvents(input_android->sensorEventQueue, &event, 1) > 0) {
			   input->_fireAccelerationEvent(Dojo::Vector(event.acceleration.x,
								      event.acceleration.y,
								      event.acceleration.z),0);
                        }
                    }
                }
		if (app->destroyRequested != 0) {
			return;
		}
	}

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
			if(!isInPause) step( dt );
		}
	}
	shutdown();
}
