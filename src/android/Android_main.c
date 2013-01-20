#include <jni.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>

/* android debug */
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "DOJO", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "DOJO", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "DOJO", __VA_ARGS__))

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
/* ANDROID */
//status app
int ANDROID_VALID_DEVICE;
//app android
struct android_app* app_state=NULL;
//APK
const char* path_apk=NULL;
extern const char* GetAndroidApk(){
	return path_apk;
}
//OPENAL CALL
extern jint JNI_OnLoad(JavaVM* vm, void* reserved);
//EVENT APP
extern int32_t android_handle_input(struct android_app* app, AInputEvent* event);
extern void android_handle_cmd(struct android_app* app, int32_t cmd) ;
extern int main(int argc, char *argv[]);
extern struct android_app* GetAndroidApp(){
	return app_state;
}


/*********************************
Android MAIN
**********************************/
extern void android_main(struct android_app* state) {
    
    //hack
    if(state==NULL) return;
    /////////////////////////////////
    // Make sure glue isn't stripped.
    app_dummy();
    /////////////////////////////////
    //Call main
    LOGE("!android_main!");
    //Init state
    state->onAppCmd = android_handle_cmd;
    state->onInputEvent = android_handle_input;
    ANDROID_VALID_DEVICE=FALSE;
    //save app
    app_state=state;
    //
    int ident;
    int events;
    struct android_poll_source* source;
    //waiting get window
    while (!ANDROID_VALID_DEVICE){
	while ((ident = ALooper_pollAll(0, NULL, &events,(void**)&source)) >= 0){
		if (source != NULL) source->process(state, source);
		if (state->destroyRequested != 0) return;
	}
	usleep( 16 );
    }
    ////////////////////////////
    //GET APK (ZIP FILE)
    ANativeActivity* activity = state->activity;
    JNIEnv* env;
    (*(activity->vm))->AttachCurrentThread(activity->vm, &env, NULL);
    jclass clazz = (*env)->GetObjectClass(env, activity->clazz);
    jmethodID methodID = (*env)->GetMethodID(env, clazz, "getPackageCodePath", "()Ljava/lang/String;");
    jobject result = (*env)->CallObjectMethod(env, activity->clazz, methodID);
    jboolean isCopy;
    path_apk= (const char*) (*env)->GetStringUTFChars(env, (jstring)result, &isCopy);
    ////////////////////////////
    ////////////////////////////
    //INIT openAL/backend
    JNI_OnLoad(activity->vm,0);
    ////////////////////////
    char *argv[2];
    argv[0] = strdup("Dojo");
    argv[1] = NULL;
    int out=main(1, argv);
}

