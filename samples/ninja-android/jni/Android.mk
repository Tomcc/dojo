
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


LOCAL_MODULE := game
LOCAL_ARM_MODE := arm

DOJO_PATH := ../../dojo2android

############################################################################
############################################################################
#DOJO DIP
#paths
DIP_INCLUDE_PATH_DIP := $(DOJO_PATH)/dependencies-android/include
DIP_INCLUDE_PATH_AND := $(DOJO_PATH)/include/dojo/android
DIP_INCLUDE_PATH_DOJ := $(DOJO_PATH)/include/dojo/
DIP_INCLUDE_PATH := $(DOJO_PATH)/include
DIP_LIB_PATH := $(DOJO_PATH)/dependencies-android/lib
#libs
LOCAL_LDLIBS:=  $(DIP_LIB_PATH)/libdojo-android.a\
		$(DIP_LIB_PATH)/libopenal.a\
		$(DIP_LIB_PATH)/libzzip.a\
                $(DIP_LIB_PATH)/libvorbis.a\
                $(DIP_LIB_PATH)/libfreetype2.a\
                $(DIP_LIB_PATH)/libFreeImage.a\
                $(DIP_LIB_PATH)/libPocoFoundation.a\
		-L$(NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/ \
                -llog -lz -landroid -lGLESv1_CM -lGLESv2 -landroid -lEGL  -s\
	        -lgnustl_static -lsupc++
   
LOCAL_C_INCLUDES:=$(LOCAL_PATH)/$(DIP_INCLUDE_PATH)
LOCAL_C_INCLUDES+=$(LOCAL_PATH)/$(DIP_INCLUDE_PATH_DOJ)
LOCAL_C_INCLUDES+=$(LOCAL_PATH)/$(DIP_INCLUDE_PATH_AND)
LOCAL_C_INCLUDES+=$(LOCAL_PATH)/$(DIP_INCLUDE_PATH_DIP)

#openGL extra propriety
LOCAL_CFLAGS += -D__STDC_LIMIT_MACROS -DDEF_SET_OPENGL_ES2 -DGL_GLEXT_PROTOTYPES -DCP_USE_DOUBLES=0
#LOCAL_CFLAGS += -g -ggdb //debug
#LOCAL_CFLAGS += -ffast-math -O3
LOCAL_CPPFLAGS   += -std=gnu++11 -frtti -fexceptions
############################################################################
############################################################################
		

#GAME MAKE
#c/cpp files
GAME_PATH_ANDROID := game
GAME_PATH_ANDROID_INCLUDE := game
#c/cpp game files
GAME_CPP:=$(wildcard $(LOCAL_PATH)/$(GAME_PATH_ANDROID)/*.cpp)
GAME_C:=$(wildcard $(LOCAL_PATH)/$(GAME_PATH_ANDROID)/*.c)
#h files
GAME_H+=$(wildcard $(LOCAL_PATH)/$(GAME_PATH_ANDROID_INCLUDE)/*.h)

#add cpp file
LOCAL_SRC_FILES+=$(GAME_CPP:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES+=$(GAME_C:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES+=$(GAME_H:$(LOCAL_PATH)/%=%)
LOCAL_C_INCLUDES+=$(GAME_H:$(LOCAL_PATH)/%=%)


include $(BUILD_SHARED_LIBRARY)



