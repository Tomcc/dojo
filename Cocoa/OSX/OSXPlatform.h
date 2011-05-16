//
//  OSXPlatform.h
//  dojo
//
//  Created by Tommaso Checchi on 5/7/11.
//  Copyright 2011 none. All rights reserved.
//

#ifndef __OSXPlatform_h
#define __OSXPlatform_h

#include "CocoaPlatform.h"

#ifdef __OBJC__
    #import <AppKit/NSOpenGLView.h>
    #import <AppKit/NSWindow.h>

	#import "GenericListener.h"
#endif

namespace Dojo
{
	
#ifndef __OBJC__
	class NSOpenGLView;
	class NSWindow;
	class GenericListener;
#endif

	
    class OSXPlatform : public CocoaPlatform
    {
    public:
     
        OSXPlatform()
        {
            
        }
		
		virtual ~OSXPlatform();
        
        virtual void initialise();
		virtual void shutdown();
        
		virtual void acquireContext();
		virtual void present();
        
		virtual void loop( float frameTime );
        
		///loads the given file in a buffer - WARNING not every format is supported on every platform
		virtual uint loadAudioFileContent( ALuint& buffer, const std::string& path );
	
	protected:
                
		//these always exists because .cpp and .mm compiling this header could get different sizes for the class!!!
        NSOpenGLView* view;
        NSWindow* window;
		NSAutoreleasePool* pool;
		
		GenericListener* callback;
    };
}

#endif