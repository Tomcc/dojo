#include "android/AndroidPlatform.h"

#include <jni.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>

#include "Render.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "AndroidGLExtern.h"
#include "FontSystem.h"
#include "SoundManager.h"
#include "InputSystem.h"

#define LODEPNG_COMPILE_DECODER
#include "lodepng.h"
#include <setjmp.h>
#include <jpeg/jpeglib.h>
#include <jpeg/jerror.h>

/* android debug */
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "DOJO", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "DOJO", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "DOJO", __VA_ARGS__))

using namespace Dojo;

/* ANDROID */
//status app
extern "C" int ANDROID_VALID_DEVICE;
//get app
extern  "C" struct android_app* GetAndroidApp();
extern  "C" const char* GetAndroidApk();
//EVENT APP
extern "C"  int32_t android_handle_input(struct android_app* app, AInputEvent* event) {
	//get platform
	AndroidPlatform* self = app->userData ? (AndroidPlatform*)app->userData : NULL;
	DEBUG_MESSAGE("android_handle_input");

	unsigned int flags = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

	if (flags == AMOTION_EVENT_ACTION_DOWN || flags == AMOTION_EVENT_ACTION_POINTER_DOWN) { //down

		int count = AMotionEvent_getPointerCount(event);

		for (int i = 0; i < count ; i++)
			self->input->_fireTouchBeginEvent(Dojo::Vector(AMotionEvent_getX(event, i),
											  AMotionEvent_getY(event, i)));

	}
	else if (flags == AMOTION_EVENT_ACTION_UP || flags == AMOTION_EVENT_ACTION_POINTER_UP) {   //up

		int count = AMotionEvent_getPointerCount(event);

		for (int i = 0; i < count ; i++)
			self->input->_fireTouchEndEvent(Dojo::Vector(AMotionEvent_getX(event, i),
											AMotionEvent_getY(event, i)));

	}
	else if (flags == AMOTION_EVENT_ACTION_MOVE) {                                             //move

		int count = AMotionEvent_getPointerCount(event);
		Dojo::Vector tmp;

		if (count) {
			tmp = Dojo::Vector(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
		}

		if (count == 1) {
			self->input->_fireTouchMoveEvent(tmp, tmp);
		}

		for (int i = 1; i < count ; i++) {
			self->input->_fireTouchMoveEvent(Dojo::Vector(AMotionEvent_getX(event, i),
											 AMotionEvent_getY(event, i)), tmp);
			tmp = Dojo::Vector(AMotionEvent_getX(event, 1), AMotionEvent_getY(event, 1));
		}

	}
	else if (flags == AMOTION_EVENT_ACTION_CANCEL) {                                           //????? DOJO event??
		//save_fingers_input(input,event, IF_CANCEL);
	}
	else {
		return 0;
	}

	return 0;
}
extern "C" void android_handle_cmd(struct android_app* app, int32_t cmd) {
	//get platform
	AndroidPlatform* self = app->userData ? (AndroidPlatform*)app->userData : NULL;

	if ( cmd == APP_CMD_INIT_WINDOW ) {
		ANDROID_VALID_DEVICE = 1; //enable Window_Flip() (and FOR INPUT)
		DEBUG_MESSAGE("APP_INIT_WINDOW");

		if (self) {
			//Reinizialize display
			self->ResetDisplay();
			//is RESUME
			self->isInPause = false;
		}
	}
	else if (cmd == APP_CMD_RESUME) {
		DEBUG_MESSAGE("APP_CMD_RESUME");

		//is RESUME
		if (self) {
			self->isInPause = false;
		}
	}
	else if ( cmd == APP_CMD_TERM_WINDOW ) {
		DEBUG_MESSAGE("APP_CMD_TERM_WINDOW");
		ANDROID_VALID_DEVICE = 0; //disable Window_Flip() (and FOR INPUT)

		//delete
		if (self) {
			//IN PAUSE
			self->isInPause = true;
			//AND IN STOP LOOP
			self->running = false;
		}
	}
	else if (cmd == APP_CMD_PAUSE  && ANDROID_VALID_DEVICE ) {
		DEBUG_MESSAGE("APP_CMD_PAUSE");
		ANDROID_VALID_DEVICE = 0; //disable Window_Flip() (and FOR INPUT)

		if (self) {
			//is IN PAUSE
			self->isInPause = true;
		}

		//waiting
	}
	else if (cmd == APP_CMD_GAINED_FOCUS) {
		// When our app gains focus, we start monitoring the accelerometer.
		if (self != NULL && self->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(self->sensorEventQueue, self->accelerometerSensor);
			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(self->sensorEventQueue, self->accelerometerSensor, (1000L / 60) * 1000);
		}
	}

	if (cmd == APP_CMD_LOST_FOCUS) {
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (self != NULL && self->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(self->sensorEventQueue, self->accelerometerSensor);
		}
	}
}


/* DOJO */
AndroidPlatform::AndroidPlatform(const Table& table) :
	Platform(table) {
	app = NULL;
	sensorManager = NULL;
	accelerometerSensor = NULL;
	sensorEventQueue = NULL;

	addZipFormat( ".apk" );
	this->apkdir = String(GetAndroidApk());
	Utils::makeCanonicalPath(this->apkdir);
	DEBUG_MESSAGE("getPackageCodePath:");
	//TO DO: set languages....
	locale = "en";

}

void AndroidPlatform::ResetDisplay() {
	//initialize OpenGL ES and EGL


#ifdef DEF_SET_OPENGL_ES2
	const EGLint attribs[] = {
		//      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT, //openGL ES 2.0
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_NONE
	};
#else
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
#endif
	//SET ANDROID WINDOW
	EGLint _w, _h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	//get display
	DEBUG_MESSAGE("get display");
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	DEBUG_ASSERT( display );
	eglInitialize(display, 0, 0);
	//set openGL configuration
	DEBUG_MESSAGE("set openGL configuration:eglChooseConfig");
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	DEBUG_MESSAGE("set openGL configuration:eglGetConfigAttrib");
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	DEBUG_MESSAGE("set openGL ANativeWindow_setBuffersGeometry");
	//ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN, 0);<--usare questo todo
	//AWINDOW_FLAG_SCALED
	ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);
	//create a surface, and openGL context
	DEBUG_MESSAGE("create a surface, and openGL context");
	surface = eglCreateWindowSurface(display, config, app->window, NULL);
	DEBUG_ASSERT( surface );
#ifdef DEF_SET_OPENGL_ES2
	const EGLint attrib_list [] = {EGL_CONTEXT_CLIENT_VERSION, 2,  EGL_NONE}; //openGL ES 2.0 //2, EGL_NONE
	context = eglCreateContext(display, config, NULL, attrib_list);
#else
	const EGLint attrib_list [] = {EGL_CONTEXT_CLIENT_VERSION, 1,  EGL_NONE}; //openGL ES 1.0 //1, EGL_NONE
	context = eglCreateContext(display, config, NULL, attrib_list);
#endif
	DEBUG_ASSERT( context );

	//set corrunt openGL context
	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGE("error eglMakeCurrent");
		return;
	}

	//get WIDTH,HEIGHT
	eglQuerySurface(display, surface, EGL_WIDTH, &width);
	eglQuerySurface(display, surface, EGL_HEIGHT, &height);
	DEBUG_MESSAGE("screen : width:" << width << " height:" << height);
	//return size screen
	screenWidth = width;
	screenHeight = height;
	windowWidth = screenWidth;
	windowHeight = screenHeight;
	//init opengl VBA calls
	DEBUG_MESSAGE("!!init opengl VBA calls!");
	ExternInitOpenGL();
	//is not in pause
	isInPause = false;
	//
}
void AndroidPlatform::initialize(Game* g) {
	DEBUG_MESSAGE("AndroidPlatform::initialize()");
	//set game
	game = g;
	DEBUG_ASSERT( game );
	//init app
	DEBUG_ASSERT( GetAndroidApp() );
	GetAndroidApp()->userData = (void*)this;
	app = GetAndroidApp();
	//Set Display
	ResetDisplay();
	//accelerometer
	sensorManager = ASensorManager_getInstance();
	DEBUG_ASSERT( sensorManager );

	accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);

	if (accelerometerSensor == NULL) {
		DEBUG_MESSAGE("Accelerometer feature not supported on this device.");
	}

	//dojo object
	render = new Render( ((int)width), ((int)height), DO_LANDSCAPE_LEFT );
	input = new InputSystem();
	fonts = new FontSystem();
	sound  = new SoundManager();
	input->addDevice(&androidKeyboard);
	//dojo make internal storage:
	std::string filesPath(GetAndroidApp()->activity->internalDataPath);
	/* get exist path "files/" */
	{
		struct stat sb;
		int32_t res = stat(filesPath.c_str(), &sb);

		if (0 == res && sb.st_mode & S_IFDIR) {
			DEBUG_MESSAGE("'files/' dir already in app's internal data storage.");
		}
		else if (ENOENT == errno) {
			res = mkdir(filesPath.c_str(), 0770);

			if (res != 0) {
				DEBUG_MESSAGE("files/ No permission read and write in internal storage");
			}
			else {
				DEBUG_MESSAGE("mkdir(files/) done");
			}
		}
	}
	//set paths
	apkdirResources = apkdir + String("/assets");
	dirAppData = GetAndroidApp()->activity->internalDataPath;
	//enable loop
	running = true;
	//start the game
	game->begin();
}


const String& AndroidPlatform::getAppDataPath() {
	return dirAppData;
}
const String& AndroidPlatform::getResourcesPath() {
	return apkdirResources;
}
const String& AndroidPlatform::getRootPath() {
	return apkdir;
}

void AndroidPlatform::shutdown() {
	DEBUG_MESSAGE("AndroidPlatform::shutdown()");

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

	display = EGL_NO_DISPLAY;
	context = EGL_NO_CONTEXT;
	surface = EGL_NO_SURFACE;
	//enable loop, and disable draw (pause)
	running = false;
	isInPause = true;
	//destroy managers
	delete render;
	render = NULL;
	delete sound;
	sound = NULL;
	delete input;
	input = NULL;
	delete fonts;
	fonts = NULL;
}

void AndroidPlatform::acquireContext() {
	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		DEBUG_MESSAGE("AndroidPlatform::acquireContext() error eglMakeCurrent:");
		DEBUG_MESSAGE("AndroidPlatform::display" << display);
		DEBUG_MESSAGE("AndroidPlatform::surface" << surface);
		DEBUG_MESSAGE("AndroidPlatform::context" << context);
		return;
	}
}



GLenum AndroidPlatform::loadImageFile( void*& bufptr, const String& path, int& width, int& height, int& pixelSize ) {
	//TODO : JPGE & TGA
	char* buf;
	unsigned char* localbufptr;
	int fileSize = loadFileContent( buf, path );
	const Dojo::String& expath = Dojo::Utils::getFileExtension( path ) ;

	if (expath == Dojo::String("png")) {
		//LOAD PNG
		LodePNGState state;
		lodepng_state_init(&state);
		lodepng_decode(&localbufptr, (unsigned int*) & (width), (unsigned int*) & (height), &state, (unsigned const char*)buf, fileSize);
		//SAVE DATA
		pixelSize = lodepng_get_channels(&state.info_png.color);
		bufptr = localbufptr;
		//FREE
		lodepng_state_cleanup(&state);
		free( buf );   //free PNG uncompressed
		//
	}
	else if (expath == Dojo::String("jpg")) {

		struct jpeg_decompress_struct dinfo;
		struct jpeg_error_mgr jerr;

		dinfo.err = jpeg_std_error(&jerr);
		/* Now we can initialize the JPEG decompression object. */
		jpeg_create_decompress(&dinfo);
		/* Step 2: specify data source (eg, a file) */
		jpeg_mem_src(&dinfo, (unsigned char*)buf, fileSize);
		/* Step 3: read file parameters with jpeg_read_header() */
		jpeg_read_header(&dinfo, TRUE);
		/* Step 5: Start decompressor */
		jpeg_start_decompress(&dinfo);
		//alloc image
		width = dinfo.output_width;
		height = dinfo.output_height;
		pixelSize = (int)dinfo.output_components;
		bufptr = (void*)malloc(width * height * pixelSize);
		//calc rowsize
		int row_stride = width * pixelSize;
		void* outbuffer = bufptr;

		//step 6, read the image line by line
		while (dinfo.output_scanline < dinfo.output_height) {
			//IT ALWAYS crash ON THIS JPEG_READ_SCANLINES FUNCTION CALL BELOW
			(void) jpeg_read_scanlines(&dinfo, (JSAMPARRAY)(&outbuffer), 1);
			outbuffer += row_stride;
		}

		/* Step 7: Finish decompression */
		(void) jpeg_finish_decompress(&dinfo);
		/* Step 8: Release JPEG decompression object */
		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_decompress(&dinfo);
		//free JPG uncompressed
		free( buf );
	}
	else {
		return 0;
	}

	static const GLenum formatsForSize[] = { GL_NONE, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA };
	//return
	return formatsForSize[ pixelSize ];
}

void AndroidPlatform::present() {
	realFrameTime = (float)mStepTimer.getElapsedTime();

	// No display or in pause....
	if ( display == EGL_NO_DISPLAY || isInPause ) {
		DEBUG_MESSAGE("no AndroidPlatform::present()");
		return;
	}

	eglSwapBuffers( display, surface);
}

void AndroidPlatform::step( float dt ) {
	DEBUG_ASSERT( running );
	//update accelerometer
	UpdateEvent();
	//update game
	game->loop( dt );
	render->render();
	sound->update( dt );

}

void AndroidPlatform::UpdateEvent() {

	if (accelerometerSensor != NULL) {

		if (sensorEventQueue == NULL) {

			DEBUG_MESSAGE("UpdateEvent::init createEventQueue");

			sensorEventQueue = ASensorManager_createEventQueue(sensorManager,
							   app->looper,
							   LOOPER_ID_USER,
							   NULL, NULL);

			DEBUG_MESSAGE("UpdateEvent::init enableSensor");

			ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
			// We'd like to get 60 events per second (in us).

			DEBUG_MESSAGE("UpdateEvent::init setEventRate");

			ASensorEventQueue_setEventRate(sensorEventQueue,
										   accelerometerSensor,
										   (1000L / 60) * 1000);
		}

		int ident, events;
		struct android_poll_source* source;

		while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
			if (source != NULL) {
				source->process(app, source);
			}

			//GET SENSOR
			if (ident == LOOPER_ID_USER) {
				if (accelerometerSensor != NULL) {
					ASensorEvent event;

					while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {
						input->_fireAccelerationEvent(Dojo::Vector(event.acceleration.x,
													  event.acceleration.y,
													  event.acceleration.z), 0);
					}
				}
			}

			if (app->destroyRequested != 0) {
				return;
			}
		}

	}
}

void AndroidPlatform::loop() {
	DEBUG_ASSERT( game );
	float frameInterval = game->getNativeFrameLength();

	frameTimer.reset();
	float dt;

	DEBUG_MESSAGE("start Platform::loop()");

	while ( running ) {
		dt = frameTimer.getElapsedTime();

		if ( dt > frameInterval ) {
			frameTimer.reset();

			if (!isInPause) {
				step( dt );
			}
		}
	}

	shutdown();
}
