
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_MODULE := androiddojo
LOCAL_ARM_MODE := arm

#paths
DIP_INCLUDE_PATH := ../../../dependencies/include
DIP_LIB_PATH := $(LOCAL_PATH)/../../../dependencies/lib
DOJO_PATH_INCLUDE := ../../../include
DOJO_PATH_INCLUDE_DOJO := ../../../include/dojo
DOJO_PATH_ANDROID_INCLUDE_DOJO := ../../../include/dojo/android
DOJO_PATH_CPP := ../../../src
DOJO_PATH_ANDROID_CPP := ../../../src/android

#libs
LOCAL_LDLIBS:=  $(DIP_LIB_PATH)/libopenal.a\
		$(DIP_LIB_PATH)/libzzip.a\
                $(DIP_LIB_PATH)/libvorbis.a\
                $(DIP_LIB_PATH)/libfreetype2.a\
                $(DIP_LIB_PATH)/libPocoFoundation.a\
		-L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/ \
                -llog -lz -landroid -lEGL -lGLESv1_CM -lGLESv2 -s\
	        -lgnustl_static -lsupc++
			   
LOCAL_STATIC_LIBRARIES := android_native_app_glue

#include
LOCAL_C_INCLUDES+=$(LOCAL_PATH)/$(DIP_INCLUDE_PATH)\
		  $(LOCAL_PATH)/$(DOJO_PATH_INCLUDE)\
		  $(LOCAL_PATH)/$(DOJO_PATH_INCLUDE_DOJO)\
		  $(LOCAL_PATH)/$(DOJO_PATH_ANDROID_INCLUDE_DOJO)
#c/cpp files
DOJO_CPP:=$(wildcard $(LOCAL_PATH)/$(DOJO_PATH_CPP)/*.cpp)
DOJO_CPP+=$(wildcard $(LOCAL_PATH)/$(DOJO_PATH_ANDROID_CPP)/*.cpp)
DOJO_C:=$(wildcard $(LOCAL_PATH)/$(DOJO_PATH_ANDROID_CPP)/*.c)
#h files
DOJO_H:=$(wildcard $(LOCAL_PATH)/$(DOJO_PATH_INCLUDE_DOJO)/*.h)
DOJO_H+=$(wildcard $(LOCAL_PATH)/$(DOJO_PATH_ANDROID_INCLUDE_DOJO)/*.h)


#add cpp file
LOCAL_SRC_FILES:=$(DOJO_CPP:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES+=$(DOJO_C:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES+=$(DOJO_H:$(LOCAL_PATH)/%=%)
				
#openGL extra propriety
LOCAL_CFLAGS += -D__STDC_LIMIT_MACROS -DGL_GLEXT_PROTOTYPES -DCP_USE_DOUBLES=0
#LOCAL_CFLAGS += -g -ggdb //debug
#LOCAL_CFLAGS += -ffast-math -O3
//-std=c++11
LOCAL_CPPFLAGS   += -std=gnu++11 -frtti -fexceptions

include $(BUILD_SHARED_LIBRARY)

#Build native glue static library as well.
$(call import-module,android/native_app_glue)


