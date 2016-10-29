//
//  OSXPlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#pragma once

#ifdef __OBJC__
    // On OSX Obj-C files self cannot be macro-defined otherwise compliation breaks
    #undef self
    #import <AppKit/NSOpenGLView.h>
    #import <AppKit/NSWindow.h>
    #import "CustomOpenGLView.h"
#endif

#include <dojo_common_header.h>
#include "apple/ApplePlatform.h"
#include "Keyboard.h"
#include "Touch.h"

namespace Dojo {

#ifndef __OBJC__
	class NSOpenGLView;
	class NSWindow;
	class CustomOpenGLView;
#endif

    

	class OSXPlatform : public ApplePlatform {
	public:

		OSXPlatform( const Table& config );
		virtual ~OSXPlatform();

		virtual void initialize( Unique<Game> g ) override;
		virtual void shutdown() override;

        virtual void prepareThreadContext() override;
        
        virtual void setFullscreen( bool f ) override;
        
        virtual bool isNPOTEnabled() override;
		virtual void acquireContext() override;
        void submitFrame() override;
        
        virtual void step(float dt) override;
        virtual void loop() override;
        
        virtual PixelFormat loadImageFile(std::vector<uint8_t>& imageData, utf::string_view path, uint32_t& width, uint32_t& height, int& pixelSize) override;

        virtual utf::string_view getAppDataPath()   override   { return mAppDataPath; }
        virtual utf::string_view getResourcesPath() override   { return mResourcesPath; }
        virtual utf::string_view getRootPath()      override   { return mRootPath; }
        virtual utf::string_view getPicturesPath()  override;
        virtual utf::string_view getShaderCachePath() override;
        
        virtual void openWebPage( const utf::string_view site ) override;
		
        virtual void setMouseLocked(bool locked) override;
        void mouseWheelMoved(int wheelZ);
        void mousePressed(int cursorX, int cursorY, Touch::Type type);
        void mouseMoved(int cursorX, int cursorY);
        void mouseReleased(int cursorX, int cursorY, Touch::Type type);
        
        void keyPressed(int kc);
        void keyReleased(int kc);
        
        float getFrameInterval() { return frameInterval; }
        
        void setVSync(int interval = 1);
        
        virtual void present();

	private:
        Vector
            cursorPos,
            prevCursorPos;
        
        bool
            dragging,
            mMousePressed,
            mouseLocked = false,
            realMouseEvent = true;
        
        
        utf::string_view
            mAppDataPath,
            mPicturesPath,
            mResourcesPath,
            mRootPath,
            mShaderCachePath;
        
        int
            clientAreaYOffset,
            lastPressedText,
            mFramesToAdvance;
        
        float frameInterval;
        
        KeyCode mKeyMap[256];
        Keyboard mKeyboard;
        
        Timer mStepTimer;
        
        
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
		CustomOpenGLView* view;
		NSWindow* window;
		NSAutoreleasePool* pool;
        
        bool _initializeWindow(utf::string_view caption, int w, int h);
        void _initKeyMap();
        void _adjustWindow();
        void _setFullScreen(bool f);
        void _pollDevices(float dt);
	};
}

