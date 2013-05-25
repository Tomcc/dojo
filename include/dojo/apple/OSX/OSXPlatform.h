//
//  OSXPlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef __OSXPlatform_h
#define __OSXPlatform_h

#include "ApplePlatform.h"

#include "Keyboard.h"

#ifdef __OBJC__
    #import <AppKit/NSOpenGLView.h>
    #import <AppKit/NSWindow.h>

	#import "CustomOpenGLView.h"
#endif

namespace Dojo
{
	
#ifndef __OBJC__
	class NSOpenGLView;
	class NSWindow;
	class CustomOpenGLView;
#endif

	
    class OSXPlatform : public ApplePlatform
    {
    public:
     
        OSXPlatform( const Table& config );
		
		virtual ~OSXPlatform();
        
        virtual void initialize( Game* g );
		virtual void loop();
		virtual void shutdown();
        
		virtual void prepareThreadContext();
		virtual void acquireContext();
		virtual void present();
        
        virtual void setFullscreen( bool f );        
		
		virtual const String& getRootPath()
        {
            return mRootPath;
        }
        
		virtual const String& getAppDataPath()
        {
            return mAppDataPath;
        }
        
        virtual const String& getResourcesPath()
        {
            return mResourcesPath;
        }

		virtual void openWebPage( const String& site );
		
		virtual bool isNPOTEnabled()
		{
			return true;	
		}
	
	protected:
        
        String mRootPath, mResourcesPath, mAppDataPath;
                
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
        CustomOpenGLView* view;
        NSWindow* window;
		NSAutoreleasePool* pool;
    };
}

#endif